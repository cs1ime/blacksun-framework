#pragma once

#ifndef _WINDOWS_DEFS_H_
#define _WINDOWS_DEFS_H_

#include "../include/dma_type.h"

// typedef unsigned short              wint_t;

typedef u64 ULONG64;
typedef u32 DWORD, DWORD32;
typedef u64 DWORD64;
typedef unsigned long long ULONGLONG;
typedef long long LONGLONG;
typedef int LONG;
typedef u32 ULONG;
typedef short SHORT;

typedef ULONG *PULONG;

typedef u16 WORD;
typedef u16 USHORT;
typedef wchar_t WCHAR;

typedef u8 BYTE;
typedef unsigned char UCHAR, BOOLEAN;
typedef UCHAR *PUCHAR;
typedef u64 ULONG_PTR;

typedef char CHAR;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef const char *PCSTR;
typedef WCHAR *LPWSTR;
typedef WCHAR *PWSTR;
typedef const WCHAR *LPCWSTR;
typedef void *PVOID;
typedef char *PCHAR;

typedef void VOID;

typedef union _LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    };
    LONGLONG QuadPart;
} LARGE_INTEGER;

typedef struct _LIST_ENTRY
{
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, *PRLIST_ENTRY;

typedef struct _IMAGE_DOS_HEADER
{                    // DOS .EXE header
    WORD e_magic;    // Magic number
    WORD e_cblp;     // Bytes on last page of file
    WORD e_cp;       // Pages in file
    WORD e_crlc;     // Relocations
    WORD e_cparhdr;  // Size of header in paragraphs
    WORD e_minalloc; // Minimum extra paragraphs needed
    WORD e_maxalloc; // Maximum extra paragraphs needed
    WORD e_ss;       // Initial (relative) SS value
    WORD e_sp;       // Initial SP value
    WORD e_csum;     // Checksum
    WORD e_ip;       // Initial IP value
    WORD e_cs;       // Initial (relative) CS value
    WORD e_lfarlc;   // File address of relocation table
    WORD e_ovno;     // Overlay number
    WORD e_res[4];   // Reserved words
    WORD e_oemid;    // OEM identifier (for e_oeminfo)
    WORD e_oeminfo;  // OEM information; e_oemid specific
    WORD e_res2[10]; // Reserved words
    LONG e_lfanew;   // File address of new exe header
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER
{
    WORD Machine;
    WORD NumberOfSections;
    DWORD TimeDateStamp;
    DWORD PointerToSymbolTable;
    DWORD NumberOfSymbols;
    WORD SizeOfOptionalHeader;
    WORD Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY
{
    DWORD VirtualAddress;
    DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_DEBUG_DIRECTORY
{
    DWORD Characteristics;
    DWORD TimeDateStamp;
    WORD MajorVersion;
    WORD MinorVersion;
    DWORD Type;
    DWORD SizeOfData;
    DWORD AddressOfRawData;
    DWORD PointerToRawData;
} IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

#define IMAGE_DEBUG_TYPE_UNKNOWN 0
#define IMAGE_DEBUG_TYPE_COFF 1
#define IMAGE_DEBUG_TYPE_CODEVIEW 2
#define IMAGE_DEBUG_TYPE_FPO 3
#define IMAGE_DEBUG_TYPE_MISC 4
#define IMAGE_DEBUG_TYPE_EXCEPTION 5
#define IMAGE_DEBUG_TYPE_FIXUP 6
#define IMAGE_DEBUG_TYPE_OMAP_TO_SRC 7
#define IMAGE_DEBUG_TYPE_OMAP_FROM_SRC 8
#define IMAGE_DEBUG_TYPE_BORLAND 9
#define IMAGE_DEBUG_TYPE_RESERVED10 10
#define IMAGE_DEBUG_TYPE_CLSID 11
#define IMAGE_DEBUG_TYPE_VC_FEATURE 12
#define IMAGE_DEBUG_TYPE_POGO 13
#define IMAGE_DEBUG_TYPE_ILTCG 14
#define IMAGE_DEBUG_TYPE_MPX 15
#define IMAGE_DEBUG_TYPE_REPRO 16
#define IMAGE_DEBUG_TYPE_EX_DLLCHARACTERISTICS 20

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _IMAGE_OPTIONAL_HEADER64
{
    WORD Magic;
    BYTE MajorLinkerVersion;
    BYTE MinorLinkerVersion;
    DWORD SizeOfCode;
    DWORD SizeOfInitializedData;
    DWORD SizeOfUninitializedData;
    DWORD AddressOfEntryPoint;
    DWORD BaseOfCode;
    ULONGLONG ImageBase;
    DWORD SectionAlignment;
    DWORD FileAlignment;
    WORD MajorOperatingSystemVersion;
    WORD MinorOperatingSystemVersion;
    WORD MajorImageVersion;
    WORD MinorImageVersion;
    WORD MajorSubsystemVersion;
    WORD MinorSubsystemVersion;
    DWORD Win32VersionValue;
    DWORD SizeOfImage;
    DWORD SizeOfHeaders;
    DWORD CheckSum;
    WORD Subsystem;
    WORD DllCharacteristics;
    ULONGLONG SizeOfStackReserve;
    ULONGLONG SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve;
    ULONGLONG SizeOfHeapCommit;
    DWORD LoaderFlags;
    DWORD NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef IMAGE_OPTIONAL_HEADER64 IMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_NT_HEADERS64
{
    DWORD Signature;
    IMAGE_FILE_HEADER FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef IMAGE_NT_HEADERS64 IMAGE_NT_HEADERS;

typedef struct _IMAGE_EXPORT_DIRECTORY
{
    DWORD Characteristics;
    DWORD TimeDateStamp;
    WORD MajorVersion;
    WORD MinorVersion;
    DWORD Name;
    DWORD Base;
    DWORD NumberOfFunctions;
    DWORD NumberOfNames;
    DWORD AddressOfFunctions;    // RVA from base of image
    DWORD AddressOfNames;        // RVA from base of image
    DWORD AddressOfNameOrdinals; // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER
{
    BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        DWORD PhysicalAddress;
        DWORD VirtualSize;
    } Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD NumberOfRelocations;
    WORD NumberOfLinenumbers;
    DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

typedef struct _IMAGE_RUNTIME_FUNCTION_ENTRY
{
    DWORD BeginAddress;
    DWORD EndAddress;
    union
    {
        DWORD UnwindInfoAddress;
        DWORD UnwindData;
    } DUMMYUNIONNAME;
} _IMAGE_RUNTIME_FUNCTION_ENTRY, *_PIMAGE_RUNTIME_FUNCTION_ENTRY;
typedef _IMAGE_RUNTIME_FUNCTION_ENTRY IMAGE_RUNTIME_FUNCTION_ENTRY;

// IMAGE_FIRST_SECTION doesn't need 32/64 versions since the file header is the same either way.

#define IMAGE_FIRST_SECTION(ntheader) ((PIMAGE_SECTION_HEADER)((ULONG_PTR)(ntheader) +                          \
                                                               FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + \
                                                               ((ntheader))->FileHeader.SizeOfOptionalHeader))

// Subsystem Values

#define IMAGE_SUBSYSTEM_UNKNOWN 0                  // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE 1                   // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI 2              // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI 3              // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI 5                  // image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI 7                // image runs in the Posix character subsystem.
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS 8           // image is a native Win9x driver.
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI 9           // Image runs in the Windows CE subsystem.
#define IMAGE_SUBSYSTEM_EFI_APPLICATION 10         //
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11 //
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER 12      //
#define IMAGE_SUBSYSTEM_EFI_ROM 13
#define IMAGE_SUBSYSTEM_XBOX 14
#define IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16
#define IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG 17

// DllCharacteristics Entries

//      IMAGE_LIBRARY_PROCESS_INIT            0x0001     // Reserved.
//      IMAGE_LIBRARY_PROCESS_TERM            0x0002     // Reserved.
//      IMAGE_LIBRARY_THREAD_INIT             0x0004     // Reserved.
//      IMAGE_LIBRARY_THREAD_TERM             0x0008     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA 0x0020 // Image can handle a high entropy 64-bit virtual address space.
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE 0x0040    // DLL can move.
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY 0x0080 // Code Integrity Image
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT 0x0100       // Image is NX compatible
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION 0x0200    // Image understands isolation and doesn't want it
#define IMAGE_DLLCHARACTERISTICS_NO_SEH 0x0400          // Image does not use SEH.  No SE handler may reside in this image
#define IMAGE_DLLCHARACTERISTICS_NO_BIND 0x0800         // Do not bind this image.
#define IMAGE_DLLCHARACTERISTICS_APPCONTAINER 0x1000    // Image should execute in an AppContainer
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER 0x2000      // Driver uses WDM model
#define IMAGE_DLLCHARACTERISTICS_GUARD_CF 0x4000        // Image supports Control Flow Guard.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE 0x8000

// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0    // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1    // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE 2  // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3 // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY 4  // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5 // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6     // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE 7    // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR 8       // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS 9             // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10    // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT 11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT 12            // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT 13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14 // COM Runtime descriptor

#define MM_ZERO_ACCESS 0 // this value is not used.
#define MM_READONLY 1
#define MM_EXECUTE 2
#define MM_EXECUTE_READ 3
#define MM_READWRITE 4 // bit 2 is set if this is writable.
#define MM_WRITECOPY 5
#define MM_EXECUTE_READWRITE 6
#define MM_EXECUTE_WRITECOPY 7

#define PAGE_NOACCESS 0x01
#define PAGE_READONLY 0x02
#define PAGE_READWRITE 0x04
#define PAGE_WRITECOPY 0x08
#define PAGE_EXECUTE 0x10
#define PAGE_EXECUTE_READ 0x20
#define PAGE_EXECUTE_READWRITE 0x40
#define PAGE_EXECUTE_WRITECOPY 0x80
#define PAGE_GUARD 0x100
#define PAGE_NOCACHE 0x200
#define PAGE_WRITECOMBINE 0x400

#define MEM_PRIVATE 0x00020000
#define MEM_MAPPED 0x00040000
#define MEM_IMAGE 0x01000000

#define MEM_COMMIT 0x00001000
#define MEM_RESERVE 0x00002000
#define MEM_REPLACE_PLACEHOLDER 0x00004000
#define MEM_RESERVE_PLACEHOLDER 0x00040000
#define MEM_RESET 0x00080000
#define MEM_TOP_DOWN 0x00100000
#define MEM_WRITE_WATCH 0x00200000
#define MEM_PHYSICAL 0x00400000
#define MEM_ROTATE 0x00800000
#define MEM_DIFFERENT_IMAGE_BASE_OK 0x00800000
#define MEM_RESET_UNDO 0x01000000
#define MEM_LARGE_PAGES 0x20000000
#define MEM_4MB_PAGES 0x80000000
#define MEM_64K_PAGES (MEM_LARGE_PAGES | MEM_PHYSICAL)
#define MEM_UNMAP_WITH_TRANSIENT_BOOST 0x00000001
#define MEM_COALESCE_PLACEHOLDERS 0x00000001
#define MEM_PRESERVE_PLACEHOLDER 0x00000002
#define MEM_DECOMMIT 0x00004000
#define MEM_RELEASE 0x00008000
#define MEM_FREE 0x00010000

typedef enum _MI_VAD_TYPE
{
    VadNone = 0,
    VadDevicePhysicalMemory = 1,
    VadImageMap = 2,
    VadAwe = 3,
    VadWriteWatch = 4,
    VadLargePages = 5,
    VadRotatePhysical = 6,
    VadLargePageSection = 7
} MI_VAD_TYPE;

#define FALSE (0)
#define TRUE (1)

typedef struct LIST_ENTRY32
{
    DWORD Flink;
    DWORD Blink;
} LIST_ENTRY32;
typedef LIST_ENTRY32 *PLIST_ENTRY32;

#define RtlZeroMemory(Destination, Length) memset((Destination), 0, (Length))

#define InterlockedCompareExchange16(a1, a2, a3) (__sync_val_compare_and_swap(a1, a3, a2))
ULONG64 GetRealTime();
ULONG64 GetTickCount();

#endif
