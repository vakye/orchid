
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

    SerialPrintf(Str("\n\n"));
    SerialPrintf(Str("Hello, world!\n"));
    SerialPrintf(Str("\n"));

    SerialPrintf(Str("+---------------------+\n"));
    SerialPrintf(Str("|  SerialPrintf Test  |\n"));
    SerialPrintf(Str("+---------------------+\n"));

    SerialPrintf(Str("\n"));

    SerialPrintf(Str("s8:    %s8\n"),    -34);
    SerialPrintf(Str("s16:   %s16\n"),   -3582);
    SerialPrintf(Str("s32:   %s32\n"),   +25781);
    SerialPrintf(Str("s64:   %s64\n"),   +23857239857);
    SerialPrintf(Str("ssize: %ssize\n"), -25871ll);

    SerialPrintf(Str("\n"));

    SerialPrintf(Str("u8:    %u8\n"),    255);
    SerialPrintf(Str("u16:   %u16\n"),   65535);
    SerialPrintf(Str("u32:   %u32\n"),   23587239);
    SerialPrintf(Str("u64:   %u64\n"),   23398572938572124);
    SerialPrintf(Str("usize: %usize\n"), 1ull);

    SerialPrintf(Str("\n"));

    SerialPrintf(Str("x8:    %x8\n"),    0xff);
    SerialPrintf(Str("x16:   %x16\n"),   0xdead);
    SerialPrintf(Str("x32:   %x32\n"),   0xdeadbeef);
    SerialPrintf(Str("x64:   %x64\n"),   0xcafedeadbeefcafe);
    SerialPrintf(Str("xsize: %xsize\n"), 0xaabbcafedeadaabb);

    SerialPrintf(Str("\n"));

    SerialPrintf(Str("X8:    %X8\n"),    0xBA);
    SerialPrintf(Str("X16:   %X16\n"),   0xBEEF);
    SerialPrintf(Str("X32:   %X32\n"),   0xDEADCAFE);
    SerialPrintf(Str("X64:   %X64\n"),   0xCAFEDEADBEEFCAFE);
    SerialPrintf(Str("Xsize: %Xsize\n"), 0xCAFEDEADBEEFCAFE);

    SerialPrintf(Str("\n"));

    SerialPrintf(Str("c:    %c\n"),    'A');
    SerialPrintf(Str("str:  %str\n"),  Str("Hello, world!"));
    SerialPrintf(Str("cstr: %cstr\n"), "Hello, world!");

    for (;;);
}
