/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nathan Osman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <qmdnsengine/abstractserver.h>
#include <qmdnsengine/browser.h>
#include <qmdnsengine/cache.h>
#include <qmdnsengine/dns.h>
#include <qmdnsengine/mdns.h>
#include <qmdnsengine/message.h>
#include <qmdnsengine/query.h>
#include <qmdnsengine/record.h>

#include "browser_p.h"

using namespace QMdnsEngine;

BrowserPrivate::BrowserPrivate(Browser *browser, AbstractServer *server, const QByteArray &type, Cache *existingCache)
    : QObject(browser),
      q(browser),
      server(server),
      type(type),
      cache(existingCache ? existingCache : new Cache(this))
{
    connect(server, &AbstractServer::messageReceived, this, &BrowserPrivate::onMessageReceived);
    connect(cache, &Cache::shouldQuery, this, &BrowserPrivate::onShouldQuery);
    connect(cache, &Cache::recordExpired, this, &BrowserPrivate::onRecordExpired);
    connect(&queryTimer, &QTimer::timeout, this, &BrowserPrivate::onQueryTimeout);
    connect(&serviceTimer, &QTimer::timeout, this, &BrowserPrivate::onServiceTimeout);

    queryTimer.setInterval(60 * 1000);
    queryTimer.setSingleShot(true);

    serviceTimer.setInterval(100);
    serviceTimer.setSingleShot(true);

    // Immediately begin browsing for services
    onQueryTimeout();
}

// TODO: multiple SRV records not supported

bool BrowserPrivate::updateService(const QByteArray &fqName)
{
    // Split the FQDN into service name and type
    int index = fqName.indexOf('_') - 1;
    QByteArray serviceName = fqName.left(index);
    QByteArray serviceType = fqName.mid(index + 1);

    // Immediately return if a PTR record does not exist
    Record ptrRecord;
    if (!cache->lookupRecord(serviceType, PTR, ptrRecord)) {
        return false;
    }

    // If a SRV record is missing, query for it (by returning true)
    Record srvRecord;
    if (!cache->lookupRecord(fqName, SRV, srvRecord)) {
        return true;
    }

    Service service;
    service.setName(serviceName);
    service.setType(serviceType);
    service.setHostname(srvRecord.target());
    service.setPort(srvRecord.port());

    // If TXT records are available for the service, add their values
    QList<Record> txtRecords;
    if (cache->lookupRecords(fqName, TXT, txtRecords)) {
        QMap<QByteArray, QByteArray> attributes;
        foreach (Record record, txtRecords) {
            for (auto i = record.attributes().constBegin();
                    i != record.attributes().constEnd(); ++i) {
                attributes.insert(i.key(), i.value());
            }
        }
        service.setAttributes(attributes);
    }

    // If the service existed, this is an update; otherwise it is a new
    // addition; emit the appropriate signal
    if (!services.contains(fqName)) {
        emit q->serviceAdded(service);
    } else if(services.value(fqName) != service) {
        emit q->serviceUpdated(service);
    }

    services.insert(fqName, service);

    return false;
}

void BrowserPrivate::onMessageReceived(const Message &message)
{
    if (!message.isResponse()) {
        return;
    }

    // Invalidate each record in the cache first. This ensures
    // that we properly handle the case where we have multiple
    // records of the same type with 'flush cache' set.
    foreach (Record record, message.records()) {
        cache->invalidateRecord(record);
    }

    // Use a set to track all services that are updated in the message to
    // prevent unnecessary queries for SRV and TXT records
    QSet<QByteArray> updateNames;
    foreach (Record record, message.records()) {
        cache->addRecord(record);
        bool any = type == MdnsBrowseType;
        switch (record.type()) {
        case PTR:
            if (any && record.name() == MdnsBrowseType) {
                ptrTargets.insert(record.target());
                serviceTimer.start();
            } else if (any || record.name() == type) {
                updateNames.insert(record.target());
            }
            break;
        case SRV:
        case TXT:
            if (any || record.name().endsWith("." + type)) {
                updateNames.insert(record.name());
            }
            break;
        }
    }

    // For each of the services marked to be updated, perform the update and
    // make a list of all missing SRV records
    QSet<QByteArray> queryNames;
    foreach (QByteArray name, updateNames) {
        if (updateService(name)) {
            queryNames.insert(name);
        }
    }

    // Build and send a query for all of the SRV and TXT records
    if (queryNames.count()) {
        Message queryMessage;
        foreach (QByteArray name, queryNames) {
            Query query;
            query.setName(name);
            query.setType(SRV);
            queryMessage.addQuery(query);
            query.setType(TXT);
            queryMessage.addQuery(query);
        }
        server->sendMessageToAll(queryMessage);
    }
}

void BrowserPrivate::onShouldQuery(const Record &record)
{
#if 1
    // Moonlight: Never query for expiring DNS records. This action effectively
    // results in amplification of mDNS traffic by retransmitting queries
    // sent by all devices on the network, because we snoop on all other mDNS traffic
    // and populate our cache with it. This breaks accurately reporting serviceRemoved()
    // events since we don't keep those records up to date, however Moonlight doesn't
    // use that signal.
    Q_UNUSED(record);
#else
    // Assume that all messages in the cache are still in use (by the browser)
    // and attempt to renew them immediately

    Query query;
    query.setName(record.name());
    query.setType(record.type());
    Message message;
    message.addQuery(query);
    server->sendMessageToAll(message);
#endif
}

void BrowserPrivate::onRecordExpired(const Record &record)
{
    // If the PTR or SRV record has expired for a service, then it must be
    // removed - TXT records on the other hand, cause an update

    QByteArray serviceName;
    switch (record.type()) {
    case PTR:
        serviceName = record.target();
        break;
    case SRV:
        serviceName = record.name();
        break;
    case TXT:
        updateService(record.name());
        return;
    default:
        return;
    }
    Service service = services.value(serviceName);
    if (!service.name().isNull()) {
        emit q->serviceRemoved(service);
        services.remove(serviceName);
    }
}

void BrowserPrivate::onQueryTimeout()
{
    Query query;
    query.setName(type);
    query.setType(PTR);
    Message message;
    message.addQuery(query);

    // TODO: including too many records could cause problems

    // Include PTR records for the target that are already known
    QList<Record> records;
    if (cache->lookupRecords(query.name(), PTR, records)) {
        foreach (Record record, records) {
            message.addRecord(record);
        }
    }

    server->sendMessageToAll(message);
    queryTimer.start();
}

void BrowserPrivate::onServiceTimeout()
{
    if (ptrTargets.count()) {
        Message message;
        foreach (QByteArray target, ptrTargets) {

            // Add a query for PTR records
            Query query;
            query.setName(target);
            query.setType(PTR);
            message.addQuery(query);

            // Include PTR records for the target that are already known
            QList<Record> records;
            if (cache->lookupRecords(target, PTR, records)) {
                foreach (Record record, records) {
                    message.addRecord(record);
                }
            }
        }

        server->sendMessageToAll(message);
        ptrTargets.clear();
    }
}

Browser::Browser(AbstractServer *server, const QByteArray &type, Cache *cache, QObject *parent)
    : QObject(parent),
      d(new BrowserPrivate(this, server, type, cache))
{
}
