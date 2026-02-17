
typedef struct packed
{
    u16 Limit0;
    u16 Base0;
    u8  Base1;
    u8  AccessFlags;
    u8  Limit1AndFlags;
    u8  Base;
} x64_gdt_entry;

CTAssert(sizeof(x64_gdt_entry) == 8);

typedef struct packed
{
    u16 Limit;
    u64 Address;
} x64_gdt_register;
