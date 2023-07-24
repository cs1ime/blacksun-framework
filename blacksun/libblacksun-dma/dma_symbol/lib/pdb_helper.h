#ifndef QUERY_PDB_PDB_HELPER_H
#define QUERY_PDB_PDB_HELPER_H

// copied from raw_pdb "Examples/ExampleTypes.cpp"

#include <PDB_RawFile.h>
#include <PDB_DBIStream.h>
#include <PDB_TPIStream.h>

uint8_t GetLeafSize(PDB::CodeView::TPI::TypeRecordKind kind);
const char* GetLeafName(const char* data, PDB::CodeView::TPI::TypeRecordKind kind);
const char* GetTypeName(const PDB::TPIStream& tpiStream, uint32_t typeIndex, uint8_t& pointerLevel, const PDB::CodeView::TPI::Record** referencedType, const PDB::CodeView::TPI::Record** modifierRecord);


#endif //QUERY_PDB_PDB_HELPER_H
