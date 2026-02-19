
local void KernelEntry(memory_map* MemoryMap, acpi_rsdp* RSDP)
{
    ArchSetup();

    SerialDebugf(Str("ACPI RSDP Address: 0x%p"), RSDP);

    ACPIValidateRSDP(RSDP);

    acpi_description_header* MCFG = ACPIFindTableAddress(RSDP, FourCC('M', 'C', 'F', 'G'));
    if (!MCFG)
    {
        SerialErrorf(Str("Cannot find ACPI MCFG table."));
    }

    arch_page_map* PageMap = ArchNewPageMap(MemoryMap);

    for (
        usize Address = 0;
        Address < GB(4);
        Address += ArchGetPageSize()
    )
    {
        usize Physical = Address;
        usize Virtual  = Address;

        ArchMapPage(MemoryMap, PageMap, Physical, Virtual);
    }

    ArchUsePageMap(PageMap);

    SerialInfof(Str("Mapped first 4GB of memory."));

    for (;;);
}
