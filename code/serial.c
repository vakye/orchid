
local usize SerialPrintfv(string Format, va_list ArgList)
{
    usize BytesWritten = 0;

    persist char Buffer[KB(4)] = {0};

    BytesWritten = SPrintfv(Buffer, sizeof(Buffer), Format, ArgList);
    ArchWriteSerial(Buffer, BytesWritten);

    return (BytesWritten);
}

local usize SerialPrintf(string Format, ...)
{
    va_list ArgList = {0};
    va_start(ArgList, Format);

    usize BytesWritten = SerialPrintfv(Format, ArgList);

    va_end(ArgList);

    return (BytesWritten);
}

local usize SerialDebugf(string Format, ...)
{
    char Prefix[] = "[Debug]: ";
    ArchWriteSerial(Prefix, sizeof(Prefix) - 1);

    va_list ArgList = {0};
    va_start(ArgList, Format);

    usize BytesWritten = SerialPrintfv(Format, ArgList);

    va_end(ArgList);

    return (BytesWritten);
}

local usize SerialInfof(string Format, ...)
{
    char Prefix[] = "[Info ]: ";
    ArchWriteSerial(Prefix, sizeof(Prefix) - 1);

    va_list ArgList = {0};
    va_start(ArgList, Format);

    usize BytesWritten = SerialPrintfv(Format, ArgList);

    va_end(ArgList);

    return (BytesWritten);
}

local usize SerialErrorf(string Format, ...)
{
    char Prefix[] = "[Error]: ";
    ArchWriteSerial(Prefix, sizeof(Prefix) - 1);

    va_list ArgList = {0};
    va_start(ArgList, Format);

    usize BytesWritten = SerialPrintfv(Format, ArgList);

    va_end(ArgList);

    return (BytesWritten);
}
