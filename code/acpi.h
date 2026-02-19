
#pragma once

packed(typedef struct
{
    u64 Signature;
    u8  Checksum;
    u8  OEMID[6];
    u8  Revision;
    u32 AddressOfRSDT;

    // NOTE(vak): ACPI 2.0 and above (Revision = 2)

    u32 Length;
    u64 AddressOfXSDT;
    u8  ExtendedChecksum;
    u8  Reserved[3];
} acpi_rsdp)

CTAssert(sizeof(acpi_rsdp) == 36);

packed(typedef struct
{
    u32 Signature;
    u32 Length;
    u8  Revision;
    u8  Checksum;
    u8  OEMID[6];
    u8  OEMTableID[8];
    u32 OEMRevision;
    u32 CreatorID;
    u32 CreatorRevision;
} acpi_description_header)

CTAssert(sizeof(acpi_description_header) == 36);

local void ACPIValidateRSDP(acpi_rsdp* RSDP);

local usize ACPIGetTableCount(acpi_rsdp* RSDP);
local acpi_description_header* ACPIGetTableAddress(acpi_rsdp* RSDP, usize Index);
local acpi_description_header* ACPIFindTableAddress(acpi_rsdp* RSDP, u32 Signature);
