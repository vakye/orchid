
local void KernelEntry(memory_map* MemoryMap)
{
    ArchSetup();

    arch_page_map* PageMap = ArchNewPageMap(MemoryMap);

    for (
        usize Address = 0;
        Address < GB(32);
        Address += ArchGetPageSize()
    )
    {
        usize Physical = Address;
        usize Virtual  = Address;

        ArchMapPage(MemoryMap, PageMap, Physical, Virtual);
    }

    ArchUsePageMap(PageMap);

    for (;;);
}
