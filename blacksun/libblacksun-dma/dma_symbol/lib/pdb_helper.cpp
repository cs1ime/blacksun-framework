#include "pdb_helper.h"

uint8_t GetLeafSize(PDB::CodeView::TPI::TypeRecordKind kind) {
  if (kind < PDB::CodeView::TPI::TypeRecordKind::LF_NUMERIC) {
    // No leaf can have an index less than LF_NUMERIC (0x8000)
    // so word is the value...
    return sizeof(PDB::CodeView::TPI::TypeRecordKind);
  }

  switch (kind) {
  case PDB::CodeView::TPI::TypeRecordKind::LF_CHAR:
    return sizeof(PDB::CodeView::TPI::TypeRecordKind) + sizeof(uint8_t);

  case PDB::CodeView::TPI::TypeRecordKind::LF_USHORT:
  case PDB::CodeView::TPI::TypeRecordKind::LF_SHORT:
    return sizeof(PDB::CodeView::TPI::TypeRecordKind) + sizeof(uint16_t);

  case PDB::CodeView::TPI::TypeRecordKind::LF_LONG:
  case PDB::CodeView::TPI::TypeRecordKind::LF_ULONG:
    return sizeof(PDB::CodeView::TPI::TypeRecordKind) + sizeof(uint32_t);

  case PDB::CodeView::TPI::TypeRecordKind::LF_QUADWORD:
  case PDB::CodeView::TPI::TypeRecordKind::LF_UQUADWORD:
    return sizeof(PDB::CodeView::TPI::TypeRecordKind) + sizeof(uint64_t);

  default:
    printf("Error! 0x%04x bogus type encountered, aborting...\n",
           PDB_AS_UNDERLYING(kind));
  }
  return 0;
}

const char *GetLeafName(const char *data,
                        PDB::CodeView::TPI::TypeRecordKind kind) {
  return &data[GetLeafSize(kind)];
}

const char *GetTypeName(const PDB::TPIStream &tpiStream, uint32_t typeIndex,
                        uint8_t &pointerLevel,
                        const PDB::CodeView::TPI::Record **referencedType,
                        const PDB::CodeView::TPI::Record **modifierRecord) {
  const char *typeName = nullptr;
  const PDB::CodeView::TPI::Record *underlyingType = nullptr;

  if (referencedType)
    *referencedType = nullptr;

  if (modifierRecord)
    *modifierRecord = nullptr;

  auto typeIndexBegin = tpiStream.GetFirstTypeIndex();
  if (typeIndex < typeIndexBegin) {
    auto type = static_cast<PDB::CodeView::TPI::TypeIndexKind>(typeIndex);
    switch (type) {
    case PDB::CodeView::TPI::TypeIndexKind::T_HRESULT:
      return "HRESULT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PHRESULT:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PHRESULT:
      return "PHRESULT";
    case PDB::CodeView::TPI::TypeIndexKind::T_VOID:
      return "void";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PVOID:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PVOID:
    case PDB::CodeView::TPI::TypeIndexKind::T_PVOID:
      return "PVOID";

    case PDB::CodeView::TPI::TypeIndexKind::T_BOOL08:
    case PDB::CodeView::TPI::TypeIndexKind::T_BOOL16:
    case PDB::CodeView::TPI::TypeIndexKind::T_BOOL32:
      return "BOOL";
    case PDB::CodeView::TPI::TypeIndexKind::T_RCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_CHAR:
      return "CHAR";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PRCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_32PCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_PRCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_PCHAR:
      return "PCHAR";

    case PDB::CodeView::TPI::TypeIndexKind::T_UCHAR:
      return "UCHAR";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PUCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PUCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_PUCHAR:
      return "PUCHAR";
    case PDB::CodeView::TPI::TypeIndexKind::T_WCHAR:
      return "WCHAR";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PWCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PWCHAR:
    case PDB::CodeView::TPI::TypeIndexKind::T_PWCHAR:
      return "PWCHAR";
    case PDB::CodeView::TPI::TypeIndexKind::T_SHORT:
      return "SHORT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PSHORT:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PSHORT:
    case PDB::CodeView::TPI::TypeIndexKind::T_PSHORT:
      return "PSHORT";
    case PDB::CodeView::TPI::TypeIndexKind::T_USHORT:
      return "USHORT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PUSHORT:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PUSHORT:
    case PDB::CodeView::TPI::TypeIndexKind::T_PUSHORT:
      return "PUSHORT";
    case PDB::CodeView::TPI::TypeIndexKind::T_LONG:
      return "LONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PLONG:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PLONG:
    case PDB::CodeView::TPI::TypeIndexKind::T_PLONG:
      return "PLONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_ULONG:
      return "ULONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PULONG:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PULONG:
    case PDB::CodeView::TPI::TypeIndexKind::T_PULONG:
      return "PULONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_REAL32:
      return "FLOAT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PREAL32:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PREAL32:
    case PDB::CodeView::TPI::TypeIndexKind::T_PREAL32:
      return "PFLOAT";
    case PDB::CodeView::TPI::TypeIndexKind::T_REAL64:
      return "DOUBLE";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PREAL64:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PREAL64:
    case PDB::CodeView::TPI::TypeIndexKind::T_PREAL64:
      return "PDOUBLE";
    case PDB::CodeView::TPI::TypeIndexKind::T_QUAD:
      return "LONGLONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PQUAD:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PQUAD:
    case PDB::CodeView::TPI::TypeIndexKind::T_PQUAD:
      return "PLONGLONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_UQUAD:
      return "ULONGLONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PUQUAD:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PUQUAD:
    case PDB::CodeView::TPI::TypeIndexKind::T_PUQUAD:
      return "PULONGLONG";
    case PDB::CodeView::TPI::TypeIndexKind::T_INT4:
      return "INT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PINT4:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PINT4:
    case PDB::CodeView::TPI::TypeIndexKind::T_PINT4:
      return "PINT";
    case PDB::CodeView::TPI::TypeIndexKind::T_UINT4:
      return "UINT";
    case PDB::CodeView::TPI::TypeIndexKind::T_32PUINT4:
    case PDB::CodeView::TPI::TypeIndexKind::T_64PUINT4:
    case PDB::CodeView::TPI::TypeIndexKind::T_PUINT4:
      return "PUINT";
    default:
      break;
    }
  } else {
    auto typeRecord = tpiStream.GetTypeRecord(typeIndex);
    if (!typeRecord)
      return nullptr;

    switch (typeRecord->header.kind) {
    case PDB::CodeView::TPI::TypeRecordKind::LF_MODIFIER:
      if (modifierRecord)
        *modifierRecord = typeRecord;
      return GetTypeName(tpiStream, typeRecord->data.LF_MODIFIER.type,
                         pointerLevel, nullptr, nullptr);
    case PDB::CodeView::TPI::TypeRecordKind::LF_POINTER:
      ++pointerLevel;
      if (referencedType)
        *referencedType = typeRecord;
      if (typeRecord->data.LF_POINTER.utype >= typeIndexBegin) {
        underlyingType =
            tpiStream.GetTypeRecord(typeRecord->data.LF_POINTER.utype);
        if (!underlyingType)
          return nullptr;

        if (underlyingType->header.kind ==
            PDB::CodeView::TPI::TypeRecordKind::LF_POINTER)
          return GetTypeName(tpiStream, typeRecord->data.LF_POINTER.utype,
                             pointerLevel, referencedType, modifierRecord);
      }

      return GetTypeName(tpiStream, typeRecord->data.LF_POINTER.utype,
                         pointerLevel, &typeRecord, modifierRecord);
    case PDB::CodeView::TPI::TypeRecordKind::LF_PROCEDURE:
      *referencedType = typeRecord;

      return nullptr;
    case PDB::CodeView::TPI::TypeRecordKind::LF_BITFIELD:
      if (typeRecord->data.LF_BITFIELD.type < typeIndexBegin) {
        typeName = GetTypeName(tpiStream, typeRecord->data.LF_BITFIELD.type,
                               pointerLevel, nullptr, modifierRecord);
        *referencedType = typeRecord;
        return typeName;
      } else {
        *referencedType = typeRecord;
        return nullptr;
      }
    case PDB::CodeView::TPI::TypeRecordKind::LF_ARRAY:
      *referencedType = typeRecord;
      return GetTypeName(tpiStream, typeRecord->data.LF_ARRAY.elemtype,
                         pointerLevel, &typeRecord, modifierRecord);
    case PDB::CodeView::TPI::TypeRecordKind::LF_CLASS:
    case PDB::CodeView::TPI::TypeRecordKind::LF_STRUCTURE:
      return GetLeafName(typeRecord->data.LF_CLASS.data,
                         typeRecord->header.kind);
    case PDB::CodeView::TPI::TypeRecordKind::LF_UNION:
      return GetLeafName(typeRecord->data.LF_UNION.data,
                         typeRecord->header.kind);
    case PDB::CodeView::TPI::TypeRecordKind::LF_ENUM:
      return &typeRecord->data.LF_ENUM.name[0];
    default:
      break;
    }
  }

  return "unknown_type";
}
