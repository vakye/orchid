
local u8 x64InByte(u16 Port)
{
    u8 Result = 0;

    __asm volatile
    (
        "inb %%dx, %%al\n"
        : "=a"(Result) : "d"(Port)
    );

    return (Result);
}

local void x64OutByte(u16 Port, u8 Byte)
{
    __asm volatile
    (
        "outb %%al, %%dx\n"
        :: "a"(Byte), "d"(Port)
    );
}

local void ArchSetup(void)
{
    // NOTE(vak): Clear interrupts
    {
        __asm volatile ("cli");
    }

    // NOTE(vak): Setup global descriptor table (GDT)
    {
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
            // NOTE(vak): Load global descriptor table register (GDTR)
            "    lgdtq %0\n"

            // NOTE(vak): Load data segment offset
            "    movw $0x10, %%ax\n"
            "    movw %%ax, %%es\n"
            "    movw %%ax, %%ds\n"
            "    movw %%ax, %%fs\n"
            "    movw %%ax, %%gs\n"
            "    movw %%ax, %%ss\n"

            // NOTE(vak): Load code segment offset by performing a long
            // jump, which pops the return address and the code segment
            // offset off of the stack.
            "    lea (DummyLabel), %%rax\n"
            "    pushq $0x08\n" // NOTE(vak): Code segment offset
            "    pushq %%rax\n" // NOTE(vak): Return address
            "    lretq\n"
            "DummyLabel:\n"

            :: "m"(GDTR) : "rax"
        );
    }

    // NOTE(vak): Setup serial port
    {
        x64OutByte(x64_COM1 + 3, 0x00); // NOTE(vak): Set DLAB to 0
        x64OutByte(x64_COM1 + 1, 0x00); // NOTE(vak): Disable interrupts

        x64OutByte(x64_COM1 + 3, 0x80); // NOTE(vak): Set DLAB to 1

        // NOTE(vak): Set divisor to 0x0003, which corresponds
        // to a baud rate of 38400 baud.
        x64OutByte(x64_COM1 + 0, 0x03);
        x64OutByte(x64_COM1 + 1, 0x00);

        // NOTE(vak): Initialize line control register
        //     + Set data length to 8 bits
        //     + Set stop bits to 1
        //     + Set no parity bits
        //     + Disable break
        //     + Set DLAB to 0
        x64OutByte(x64_COM1 + 3, 0x03);

        // NOTE(vak): Initialize FIFO control register
        //     + Enable FIFO
        //     + Enable clearing for receive/transmit buffers,
        //     + Set byte threshold to 14 byte.
        x64OutByte(x64_COM1 + 2, 0xC7);

        // NOTE(vak): Initialize modem control register
        //     + Enable "Data Terminal Ready"
        //     + Enable "Request To Send"
        //     + Enable "Out 1"
        //     + Enable "Out 2"
        //     + Disable "Loop"
        x64OutByte(x64_COM1 + 4, 0x0F);
    }
}

local void ArchWriteSerial(void* Buffer, usize Size)
{
    u8* Bytes = (u8*)Buffer;

    for (usize Index = 0; Index < Size; Index++)
    {
        // NOTE(vak): Wait for transmission buffer to be empty

        for (;;)
        {
            u8 LineStatus = x64InByte(x64_COM1 + 5);
            if (LineStatus & 0x20)
                break;
        }

        // NOTE(vak): Transmit byte

        x64OutByte(x64_COM1 + 0, Bytes[Index]);
    }
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

    // NOTE(vak): Allocate a new page table if it doesn't exist.

    if ((PageMap->Entries[Index] & x64_PageFlag_Present) == 0)
    {
        arch_page_map* New = ArchNewPageMap(MemoryMap);

        u64 Flags   = x64_PageFlag_Present | x64_PageFlag_ReadWrite;
        u64 Address = (u64)New;

        PageMap->Entries[Index] = Address | Flags;
    }

    // NOTE(vak): Retreive the page table address from the entry.

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
    // NOTE(vak): Page table entry indices

    usize IndexPML5 = (VirtualAddress >> 48) & 0x1FF;
    usize IndexPML4 = (VirtualAddress >> 39) & 0x1FF;
    usize IndexPDPT = (VirtualAddress >> 30) & 0x1FF;
    usize IndexPDT  = (VirtualAddress >> 21) & 0x1FF;
    usize IndexPT   = (VirtualAddress >> 12) & 0x1FF;

    if (IndexPML5 > 0) return;

    // NOTE(vak): Lookup page tables

    arch_page_map* PML4 = PageMap;
    arch_page_map* PDPT = x64LookupPageTable(MemoryMap, PML4, IndexPML4);
    arch_page_map* PDT  = x64LookupPageTable(MemoryMap, PDPT, IndexPDPT);
    arch_page_map* PT   = x64LookupPageTable(MemoryMap, PDT , IndexPDT );

    // NOTE(vak): Set corresponding page table entry

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
