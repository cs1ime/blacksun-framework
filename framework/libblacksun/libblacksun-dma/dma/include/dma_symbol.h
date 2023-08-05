#pragma once

#ifndef _DMA_SYMBOL_H_
#define _DMA_SYMBOL_H_

#include <string>
#include <stdint.h>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <vector>
#include <set>

struct dma_field_info
{
    int64_t offset;
    int64_t bitfield_offset;
    int64_t bitfield_length;
};

class dma_symbol_interface
{
public:
    virtual std::map<std::string, int64_t> get_symbols(const std::set<std::string> &names) = 0;
    virtual std::map<std::string, int64_t> get_all_symbols() = 0;
    std::pair<bool, int64_t> get_symbol_single(std::string name)
    {
        auto r = get_symbols(std::set<std::string>{name});
        if (r.find(name) != r.end())
        {
            return std::make_pair<bool, int64_t>(true, std::move(r.at(name)));
        }
        return std::make_pair<bool, int64_t>(false, 0);
    }
    virtual std::map<std::string, std::map<std::string, dma_field_info>>
    get_struct(const std::map<std::string, std::set<std::string>> &names) = 0;
    virtual std::map<std::string, std::map<std::string, dma_field_info>>
    get_all_structures() = 0;
    std::pair<bool, dma_field_info> get_struct_field(std::string name, std::string field)
    {
        auto r = get_struct({{name, {field}}});
        if (r.find(name) != r.end())
        {
            auto f = r.at(name);
            if (!f.empty())
            {
                return std::make_pair<bool, dma_field_info>(true, std::move((*f.begin()).second));
            }
        }
        return std::make_pair<bool, dma_field_info>(false, {});
    }
    virtual std::map<std::string, std::map<std::string, int64_t>>
    get_enum(const std::map<std::string, std::set<std::string>> &names) = 0;
};

class dma_symbol_factory
{
public:
    virtual std::shared_ptr<dma_symbol_interface>
    create_interface(const std::string &name, const std::string &guid, uint32_t &age) = 0;
};

#endif