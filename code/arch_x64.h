
packed(typedef struct
{
    u16 Limit0;
    u16 Base0;
    u8  Base1;
    u8  AccessFlags;
    u8  Limit1AndFlags;
    u8  Base;
} x64_gdt_entry)

CTAssert(sizeof(x64_gdt_entry) == 8);

packed(typedef struct
{
    u16 Limit;
    u64 Address;
} x64_gdt_register)

CTAssert(sizeof(x64_gdt_register) == 10);

#define x64_IDT_InterruptGate (0b1110)
#define x64_IDT_TrapGate      (0b1111)

packed(typedef struct
{
    u16 Address0;
    u16 CS;
    u8  IST;
    u8  Flags;
    u16 Address1;
    u32 Address2;
    u32 Reserved;
} x64_idt_entry)

CTAssert(sizeof(x64_idt_entry) == 16);

#define x64_GateType_Interrupt (0xE)
#define x64_GateType_Trap      (0xF)

#define x64_GatePresent        (1 << 7)

packed(typedef struct
{
    x64_idt_entry Entries[256];
} x64_idt)

CTAssert(sizeof(x64_idt) == 4096);

packed(typedef struct
{
    u16 Limit;
    u64 Address;
} x64_idt_register)

CTAssert(sizeof(x64_idt_register) == 10);

packed(typedef struct
{
    u64 R15;
    u64 R14;
    u64 R13;
    u64 R12;
    u64 R11;
    u64 R10;
    u64 R9;
    u64 R8;
    u64 RDI;
    u64 RSI;
    u64 RDX;
    u64 RCX;
    u64 RBX;
    u64 RAX;
    u64 RBP;

    u64 Vector;
    u64 ErrorCode;
} x64_interrupt_frame)

#define x64_PageFlag_Present        ((u64)(1) << 0)
#define x64_PageFlag_ReadWrite      ((u64)(1) << 1)
#define x64_PageFlag_User           ((u64)(1) << 2)
#define x64_PageFlag_WriteThrough   ((u64)(1) << 3)
#define x64_PageFlag_CacheDisable   ((u64)(1) << 4)
#define x64_PageFlag_Accessed       ((u64)(1) << 5)
#define x64_PageFlag_ExecuteDisable ((u64)(1) << 63)

#define x64_PageFlagsMask   ((u64)(0xFE00000000000FFF))
#define x64_PageAddressMask ((u64)(0x01FFFFFFFFFFF000))

packed(struct arch_page_map
{
    u64 Entries[512];
})

CTAssert(sizeof(arch_page_map) == KB(4));

#define x64_COM1 (0x03F8) // NOTE(vak): Serial port

// NOTE(vak): Interrupts

local naked void x64Interrupt0 (void);
local naked void x64Interrupt1 (void);
local naked void x64Interrupt2 (void);
local naked void x64Interrupt3 (void);
local naked void x64Interrupt4 (void);
local naked void x64Interrupt5 (void);
local naked void x64Interrupt6 (void);
local naked void x64Interrupt7 (void);
local naked void x64Interrupt8 (void);
local naked void x64Interrupt9 (void);
local naked void x64Interrupt10(void);
local naked void x64Interrupt11(void);
local naked void x64Interrupt12(void);
local naked void x64Interrupt13(void);
local naked void x64Interrupt14(void);
local naked void x64Interrupt15(void);
local naked void x64Interrupt16(void);
local naked void x64Interrupt17(void);
local naked void x64Interrupt18(void);
local naked void x64Interrupt19(void);
local naked void x64Interrupt20(void);
local naked void x64Interrupt21(void);
local naked void x64Interrupt22(void);
local naked void x64Interrupt23(void);
local naked void x64Interrupt24(void);
local naked void x64Interrupt25(void);
local naked void x64Interrupt26(void);
local naked void x64Interrupt27(void);
local naked void x64Interrupt28(void);
local naked void x64Interrupt29(void);
local naked void x64Interrupt30(void);
local naked void x64Interrupt31(void);
