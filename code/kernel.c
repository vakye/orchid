
local void KernelEntry(memory_map* MemoryMap)
{
    ArchSetup();

    SerialPrintf(Str("\n\n"));

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

    SerialInfof (Str("Hello, world!\n"));
    SerialDebugf(Str("Mapped first 4GB of memory.\n"));

    for (;;);
}
