
local b32 ACPIIsChecksumValid(void* Buffer, usize Size)
{
    u8* Bytes = (u8*)Buffer;
    usize Sum = 0;

    for (usize Index = 0; Index < Size; Index++)
    {
        Sum += Bytes[Index];
    }

    b32 Result = ((Sum & 0xFF) == 0);
    return (Result);
}

local void ACPIValidateRSDP(acpi_rsdp* RSDP)
{
    u32 Revision = RSDP->Revision;

    if (Revision == 0)
    {
        // NOTE(vak): ACPI 1.0

        SerialInfof(Str("ACPI Version: 1.0"));

        if (!ACPIIsChecksumValid(RSDP, 20))
        {
            SerialErrorf(Str("Checksum for ACPI RSDP failed."));
        }

        acpi_description_header* RSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfRSDT);

        if (!ACPIIsChecksumValid(RSDT, RSDT->Length))
        {
            SerialErrorf(Str("Checksum for ACPI RSDT failed."));
        }
    }
    else if (Revision == 2)
    {
        // NOTE(vak): ACPI 2.0 and above

        SerialInfof(Str("ACPI Version: 2.0"));

        if (!ACPIIsChecksumValid(RSDP, 36))
        {
            SerialErrorf(Str("Checksum for ACPI RSDP failed."));
        }

        acpi_description_header* XSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfXSDT);

        if (!ACPIIsChecksumValid(XSDT, XSDT->Length))
        {
            SerialErrorf(Str("Checksum for ACPI XSDT failed."));
        }
    }
    else
    {
        SerialErrorf(Str("Invalid revision (%u8) in ACPI RSDP"), RSDP->Revision);
    }
}

local usize ACPIGetTableCount(acpi_rsdp* RSDP)
{
    usize Result = 0;

    u32 Revision = RSDP->Revision;
    if (Revision == 0)
    {
        // NOTE(vak): ACPI 1.0

        acpi_description_header* RSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfRSDT);

        Result = (RSDT->Length - sizeof(acpi_description_header)) / sizeof(u32);
    }
    else if (Revision == 2)
    {
        // NOTE(vak): ACPI 2.0 and above

        acpi_description_header* XSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfXSDT);

        Result = (XSDT->Length - sizeof(acpi_description_header)) / sizeof(u64);
    }

    return (Result);
}

local acpi_description_header* ACPIGetTableAddress(acpi_rsdp* RSDP, usize Index)
{
    usize Result = 0;

    u32 Revision = RSDP->Revision;
    if (Revision == 0)
    {
        // NOTE(vak): ACPI 1.0

        acpi_description_header* RSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfRSDT);

        usize EntryCount = (RSDT->Length - sizeof(acpi_description_header)) / sizeof(u32);
        u32* Entries = (u32*)(RSDT + 1);

        if (Index < EntryCount)
        {
            Result = Entries[Index];
        }
    }
    else if (Revision == 2)
    {
        // NOTE(vak): ACPI 2.0 and above

        acpi_description_header* XSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfXSDT);

        usize EntryCount = (XSDT->Length - sizeof(acpi_description_header)) / sizeof(u64);
        u64* Entries = (u64*)(XSDT + 1);

        if (Index < EntryCount)
        {
            Result = Entries[Index];
        }
    }

    return (acpi_description_header*)(Result);
}

local acpi_description_header* ACPIFindTableAddress(acpi_rsdp* RSDP, u32 Signature)
{
    usize Result = 0;

    u32 Revision = RSDP->Revision;
    if (Revision == 0)
    {
        // NOTE(vak): ACPI 1.0

        acpi_description_header* RSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfRSDT);

        usize EntryCount = (RSDT->Length - sizeof(acpi_description_header)) / sizeof(u32);
        u32* Entries = (u32*)(RSDT + 1);

        for (usize Index = 0; Index < EntryCount; Index++)
        {
            acpi_description_header* Table = (acpi_description_header*)
                (usize)(Entries[Index]);

            if (Table->Signature == Signature)
            {
                Result = Entries[Index];
                break;
            }
        }
    }
    else if (Revision == 2)
    {
        // NOTE(vak): ACPI 2.0 and above

        acpi_description_header* XSDT = (acpi_description_header*)
            (usize)(RSDP->AddressOfXSDT);

        usize EntryCount = (XSDT->Length - sizeof(acpi_description_header)) / sizeof(u64);
        u64* Entries = (u64*)(XSDT + 1);

        for (usize Index = 0; Index < EntryCount; Index++)
        {
            acpi_description_header* Table = (acpi_description_header*)
                (usize)(Entries[Index]);

            if (Table->Signature == Signature)
            {
                Result = Entries[Index];
                break;
            }
        }
    }

    return (acpi_description_header*)(Result);
}
