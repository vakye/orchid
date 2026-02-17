
// NOTE(vak): Memory

local void ZeroMemory(void* DestInit, usize Size)
{
    u8* Dest = (u8*)DestInit;
    while (Size--)
        *Dest++ = 0;
}

local void FillMemory(void* DestInit, u8 Byte, usize Size)
{
    u8* Dest = (u8*)DestInit;
    while (Size--)
        *Dest++ = Byte;
}

local void CopyMemory(void* DestInit, void* SourceInit, usize Size)
{
    u8* Dest = (u8*)DestInit;
    u8* Source = (u8*)SourceInit;
    while (Size--)
        *Dest++ = *Source++;
}
