#pragma once

#ifndef _DMA_SYMBOL_IMPL_H_
#define _DMA_SYMBOL_IMPL_H_

#include <dma_symbol.h>
#include <downloader.h>
#include <map>
#include <memory>

class dma_symbol_factory_remote_pdb : public dma_symbol_factory
{
private:
    std::unique_ptr<downloader> m_pdb_downloader;

public:
    dma_symbol_factory_remote_pdb(std::unique_ptr<downloader> pdb_downloader)
        : m_pdb_downloader(std::move(pdb_downloader))
    {
    }
    std::shared_ptr<dma_symbol_interface>
    create_interface(const std::string &name, const std::string &guid, uint32_t &age) override;
};

#endif
