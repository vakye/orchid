
local void ArchSetup(void)
{
    __asm volatile ("cli");

    x64_gdt_entry Entries[] =
    {
        {0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00}, // NOTE(vak): Null
        {0x0000, 0x0000, 0x00, 0x9A, 0xA0, 0x00}, // NOTE(vak): Kernel code
        {0x0000, 0x0000, 0x00, 0x92, 0xA0, 0x00}, // NOTE(vak): Kernel data
    };

    x64_gdt_register GDTR =
    {
        .Limit   = sizeof(Entries) - 1,
        .Address = (u64)Entries,
    };

    __asm volatile
    (
        "    lgdtq %0\n"

        "    movw $0x10, %%ax\n"
        "    movw %%ax, %%es\n"
        "    movw %%ax, %%ds\n"
        "    movw %%ax, %%fs\n"
        "    movw %%ax, %%gs\n"
        "    movw %%ax, %%ss\n"

        "    lea (DummyLabel), %%rax\n"
        "    pushq $0x08\n"
        "    pushq %%rax\n"
        "    lretq\n"
        "DummyLabel:\n"

        :: "m"(GDTR) : "rax"
    );
}

local usize ArchGetPageSize(void)
{
    return KB(4);
}

local arch_page_map* ArchNewPageMap(memory_map* MemoryMap)
{
    arch_page_map* PageMap = ReservePage(
        MemoryMap,
        MemoryRegionKind_Usable
    );

    ZeroType(PageMap);

    return (PageMap);
}

local arch_page_map* x64LookupPageTable(
    memory_map*    MemoryMap,
    arch_page_map* PageMap,
    usize          Index
)
{
    if (Index >= 512) return (0);

    if ((PageMap->Entries[Index] & x64_PageFlag_Present) == 0)
    {
        arch_page_map* New = ArchNewPageMap(MemoryMap);

        u64 Flags   = x64_PageFlag_Present | x64_PageFlag_ReadWrite;
        u64 Address = (u64)New;

        PageMap->Entries[Index] = Address | Flags;
    }

    u64 Entry = PageMap->Entries[Index];
    arch_page_map* Result = (arch_page_map*)(Entry & x64_PageAddressMask);

    return (Result);
}

local void ArchMapPage(
    memory_map*    MemoryMap,
    arch_page_map* PageMap,
    usize          PhysicalAddress,
    usize          VirtualAddress
)
{
    usize IndexPML5 = (VirtualAddress >> 48) & 0x1FF;
    usize IndexPML4 = (VirtualAddress >> 39) & 0x1FF;
    usize IndexPDPT = (VirtualAddress >> 30) & 0x1FF;
    usize IndexPDT  = (VirtualAddress >> 21) & 0x1FF;
    usize IndexPT   = (VirtualAddress >> 12) & 0x1FF;

    if (IndexPML5 > 0) return;

    arch_page_map* PML4 = PageMap;
    arch_page_map* PDPT = x64LookupPageTable(MemoryMap, PML4, IndexPML4);
    arch_page_map* PDT  = x64LookupPageTable(MemoryMap, PDPT, IndexPDPT);
    arch_page_map* PT   = x64LookupPageTable(MemoryMap, PDT , IndexPDT );

    PT->Entries[IndexPT] = (
        PhysicalAddress        |
        x64_PageFlag_Present   |
        x64_PageFlag_ReadWrite
    );
}

local void ArchUsePageMap(arch_page_map* PageMap)
{
    __asm volatile
    (
        "mov %0, %%cr3\n"
        :: "r"(PageMap)
    );
}
