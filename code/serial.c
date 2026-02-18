
local usize SerialPrintf(string Format, ...)
{
    usize BytesWritten = 0;

    persist char Buffer[KB(4)] = {0};

    va_list ArgList = {0};
    va_start(ArgList, Format);

    BytesWritten = SPrintfv(Buffer, sizeof(Buffer), Format, ArgList);
    ArchWriteSerial(Buffer, BytesWritten);

    va_end(ArgList);

    return (BytesWritten);
}
