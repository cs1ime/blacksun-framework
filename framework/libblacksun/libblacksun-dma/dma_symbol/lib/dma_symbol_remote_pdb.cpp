#include "pdb_parser.h"
#include <string>
#include <stdint.h>
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <vector>
#include "downloader.h"
#include "../include/dma_symbol_remote_pdb.h"

class dma_symbol_interface_impl : public dma_symbol_interface
{
private:
    std::unique_ptr<pdb_parser> m_pdb_parser = nullptr;

public:
    dma_symbol_interface_impl(std::unique_ptr<pdb_parser> pdbdata)
        : m_pdb_parser(std::move(pdbdata))
    {
        return;
    }
    std::map<std::string, int64_t> get_symbols(const std::set<std::string> &names) override;
    std::map<std::string, int64_t> get_all_symbols() override;
    std::map<std::string, std::map<std::string, dma_field_info>>
    get_struct(const std::map<std::string, std::set<std::string>> &names) override;
    std::map<std::string, std::map<std::string, dma_field_info>>
    get_all_structures() override;
    std::map<std::string, std::map<std::string, int64_t>>
    get_enum(const std::map<std::string, std::set<std::string>> &names) override;
};

std::map<std::string, int64_t>
dma_symbol_interface_impl::get_symbols(const std::set<std::string> &names)
{
    return std::move(m_pdb_parser->get_symbols(names));
}
std::map<std::string, int64_t>
dma_symbol_interface_impl::get_all_symbols()
{
    return std::move(m_pdb_parser->get_all_symbols());
}
std::map<std::string, std::map<std::string, dma_field_info>>
dma_symbol_interface_impl::get_struct(const std::map<std::string, std::set<std::string>> &names)
{
    return std::move(m_pdb_parser->get_struct(names));
}
std::map<std::string, std::map<std::string, dma_field_info>>
dma_symbol_interface_impl::get_all_structures()
{
    return std::move(m_pdb_parser->get_all_structures());
}
std::map<std::string, std::map<std::string, int64_t>>
dma_symbol_interface_impl::get_enum(const std::map<std::string, std::set<std::string>> &names)
{
    return std::move(m_pdb_parser->get_enum(names));
}

std::shared_ptr<dma_symbol_interface>
dma_symbol_factory_remote_pdb::create_interface(const std::string &name, const std::string &guid, uint32_t &age)
{
    if (!m_pdb_downloader->download(name, guid, age))
    {
        return nullptr;
    }
    auto parser = std::make_unique<pdb_parser>(m_pdb_downloader->get_path(name, guid, age));
    return std::make_shared<dma_symbol_interface_impl>(std::move(parser));
}
