
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
