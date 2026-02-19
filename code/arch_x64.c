
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

local void x64InterruptDispatch(x64_interrupt_frame* Frame)
{
    // NOTE(vak): Retrieve interrupt number and error code

    persist string Names[32] =
    {
        [ 0] = Str("Division By 0"),
        [ 1] = Str("Debug Exception"),
        [ 2] = Str("NMI Interrupt"),
        [ 3] = Str("Breakpoint"),
        [ 4] = Str("Overflow"),
        [ 5] = Str("BOUND Range Exceeded"),
        [ 6] = Str("Invalid Opcode (Undefined Opcode)"),
        [ 7] = Str("Device Not Available (No Math Coprocessor)"),
        [ 8] = Str("Double Fault"),
        [ 9] = Str("Coprocessor Segment Overrun (reserved)"),
        [10] = Str("Invalid TSS"),
        [11] = Str("Segment Not Present"),
        [12] = Str("Stack-Segment Fault"),
        [13] = Str("General Protection"),
        [14] = Str("Page Fault"),

        [16] = Str("x87 FPU Floating-Point Error (Math Fault)"),
        [17] = Str("Alignment Check"),
        [18] = Str("Machine Check"),
        [19] = Str("SIMD Floating-Point Execption"),
        [20] = Str("Virtualization Exception"),
        [21] = Str("Control Protection Exception"),
    };

    if (Frame->Vector < 32)
    {
        string Name = Names[Frame->Vector];
        if (Name.Size)
        {
            if (Frame->ErrorCode)
            {
                SerialErrorf(Str("INT #%u64: %str"), Frame->Vector, Name);
            }
            else
            {
                SerialDebugf(Str("INT #%u64: %str"), Frame->Vector, Name);
            }
        }
    }
}

local naked void x64InterruptStub(void)
{
    __asm volatile
    (
        // NOTE(vak): Push registers

        "pushq %%rbp\n"
        "pushq %%rax\n"
        "pushq %%rbx\n"
        "pushq %%rcx\n"
        "pushq %%rdx\n"
        "pushq %%rsi\n"
        "pushq %%rdi\n"
        "pushq %%r8\n"
        "pushq %%r9\n"
        "pushq %%r10\n"
        "pushq %%r11\n"
        "pushq %%r12\n"
        "pushq %%r13\n"
        "pushq %%r14\n"
        "pushq %%r15\n"

        "movq %%rsp, %%rcx\n"
        "call %P0\n"

        // NOTE(vak): Pop registers

        "popq %%r15\n"
        "popq %%r14\n"
        "popq %%r13\n"
        "popq %%r12\n"
        "popq %%r11\n"
        "popq %%r10\n"
        "popq %%r9\n"
        "popq %%r8\n"
        "popq %%rdi\n"
        "popq %%rsi\n"
        "popq %%rdx\n"
        "popq %%rcx\n"
        "popq %%rbx\n"
        "popq %%rax\n"
        "popq %%rbp\n"

        // NOTE(vak): Pop interrupt number + error code

        "add $16, %%rsp\n"

        // NOTE(vak): Return from interrupt

        "iretq\n"

        :: "i"(x64InterruptDispatch)
    );
}

local void x64SetIDTEntry(
    x64_idt* IDT,
    u8       Index,
    void*    Handler,
    u8       GateType
)
{
    u64 Address = (u64)Handler;

    x64_idt_entry* Entry = IDT->Entries + Index;

    Entry->Address0 = (Address & 0xFFFF);
    Entry->CS       = 0x08;
    Entry->IST      = 0;
    Entry->Flags    = x64_GatePresent | GateType;
    Entry->Address1 = (Address >> 16) & 0xFFFF;
    Entry->Address2 = (Address >> 32) & 0xFFFFFFFF;
    Entry->Reserved = 0;
}

local void ArchSetup(void)
{
    // NOTE(vak): Clear interrupts
    {
        __asm volatile ("cli");
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

        // NOTE(vak): UEFI can output garbage to the serial port,
        // so print a couple of newlines to seperate from that.

        SerialPrintf(Str("\n\n"));
        SerialInfof (Str("Initialized serial port COM1"));
    }

    // NOTE(vak): Setup global descriptor table (GDT)
    {
        persist x64_gdt_entry Entries[] =
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

        SerialInfof(Str("Loaded GDT"));
    }

    // NOTE(vak): Setup interrupt descriptor table (IDT)
    {
        persist x64_idt IDT = {0};

        // NOTE(vak): Fill in the first 32 interrupts, which are used
        // by the processor to report faults, debug breaks, ...

        x64SetIDTEntry(&IDT,  0, (void*)x64Interrupt0 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  1, (void*)x64Interrupt1 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  2, (void*)x64Interrupt2 , x64_GateType_Interrupt);
        x64SetIDTEntry(&IDT,  3, (void*)x64Interrupt3 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  4, (void*)x64Interrupt4 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  5, (void*)x64Interrupt5 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  6, (void*)x64Interrupt6 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  7, (void*)x64Interrupt7 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  8, (void*)x64Interrupt8 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT,  9, (void*)x64Interrupt9 , x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 10, (void*)x64Interrupt10, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 11, (void*)x64Interrupt11, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 12, (void*)x64Interrupt12, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 13, (void*)x64Interrupt13, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 14, (void*)x64Interrupt14, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 15, (void*)x64Interrupt15, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 16, (void*)x64Interrupt16, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 17, (void*)x64Interrupt17, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 18, (void*)x64Interrupt18, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 19, (void*)x64Interrupt19, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 20, (void*)x64Interrupt20, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 21, (void*)x64Interrupt21, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 22, (void*)x64Interrupt22, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 23, (void*)x64Interrupt23, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 24, (void*)x64Interrupt24, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 25, (void*)x64Interrupt25, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 26, (void*)x64Interrupt26, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 27, (void*)x64Interrupt27, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 28, (void*)x64Interrupt28, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 29, (void*)x64Interrupt29, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 30, (void*)x64Interrupt30, x64_GateType_Trap);
        x64SetIDTEntry(&IDT, 31, (void*)x64Interrupt31, x64_GateType_Trap);

        // NOTE(vak): Load IDT

        x64_idt_register IDTR =
        {
            .Limit   = sizeof(IDT) - 1,
            .Address = (usize)&IDT,
        };

        __asm volatile
        (
            "lidt %0\n"
            "sti\n"
            :: "m"(IDTR)
        );

        SerialInfof(Str("Loaded IDT"));
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

// NOTE(vak): Defines an interrupt that doesn't push an error code

#define DefineInterrupt(Vector) \
    local naked void x64Interrupt##Vector(void) \
    { \
        __asm volatile \
        ( \
            "pushq $0\n" \
            "pushq %0\n" \
            "jmp %P1\n" \
            :: "i"(Vector), "i"(x64InterruptStub) \
        ); \
    }

// NOTE(vak): Defines an interrupt that pushes an error code

#define DefineInterruptE(Vector) \
    local naked void x64Interrupt##Vector(void) \
    { \
        __asm volatile \
        ( \
            "pushq %0\n" \
            "jmp %P1\n" \
            :: "i"(Vector), "i"(x64InterruptStub) \
        ); \
    }

DefineInterrupt ( 0)
DefineInterrupt ( 1)
DefineInterrupt ( 2)
DefineInterrupt ( 3)
DefineInterrupt ( 4)
DefineInterrupt ( 5)
DefineInterrupt ( 6)
DefineInterrupt ( 7)
DefineInterruptE( 8)
DefineInterrupt ( 9)
DefineInterruptE(10)
DefineInterruptE(11)
DefineInterruptE(12)
DefineInterruptE(13)
DefineInterruptE(14)
DefineInterrupt (15)
DefineInterrupt (16)
DefineInterruptE(17)
DefineInterrupt (18)
DefineInterrupt (19)
DefineInterrupt (20)
DefineInterruptE(21)
DefineInterrupt (22)
DefineInterrupt (23)
DefineInterrupt (24)
DefineInterrupt (25)
DefineInterrupt (26)
DefineInterrupt (27)
DefineInterrupt (28)
DefineInterrupt (29)
DefineInterrupt (30)
DefineInterrupt (31)
