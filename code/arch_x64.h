
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

#define x64_PageFlag_Present        ((u64)(1) << 0)
#define x64_PageFlag_ReadWrite      ((u64)(1) << 1)
#define x64_PageFlag_User           ((u64)(1) << 2)
#define x64_PageFlag_WriteThrough   ((u64)(1) << 3)
#define x64_PageFlag_CacheDisable   ((u64)(1) << 4)
#define x64_PageFlag_Accessed       ((u64)(1) << 5)
#define x64_PageFlag_ExecuteDisable ((u64)(1) << 63)

#define x64_PageFlagsMask   ((u64)(0xFE00000000000FFF))
#define x64_PageAddressMask ((u64)(0x01FFFFFFFFFFF000))

struct packed arch_page_map
{
    u64 Entries[512];
};

CTAssert(sizeof(arch_page_map) == KB(4));
