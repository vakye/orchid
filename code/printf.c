
typedef struct
{
    char* Base;
    usize Size;
} printf_stream;

typedef enum
{
    PrintfFormatType_Unknown = 0,

    PrintfFormatType_SignedInt,
    PrintfFormatType_UnsignedInt,
    PrintfFormatType_LowerHex,
    PrintfFormatType_UpperHex,

    PrintfFormatType_Character,
    PrintfFormatType_String,
    PrintfFormatType_CString,
} printf_format_type;

typedef union
{
    ssize  SignedInt;
    usize  UnsignedInt;
    usize  LowerHex;
    usize  UpperHex;

    char   Character;
    string String;
    char*  CString;
} printf_format_value;

local char PrintfPeek(printf_stream* Stream)
{
    char Character = 0;

    if (Stream->Size)
        Character = Stream->Base[0];

    return (Character);
}

local char PrintfConsume(printf_stream* Stream)
{
    char Character = 0;

    if (Stream->Size)
    {
        Character = Stream->Base[0];

        Stream->Base++;
        Stream->Size--;
    }

    return (Character);
}

local void PrintfPush(printf_stream* Stream, char Character)
{
    if (Stream->Size)
    {
        Stream->Base[0] = Character;

        Stream->Base++;
        Stream->Size--;
    }
}

local b32 PrintfMatchAndConsume(printf_stream* Stream, string Compare)
{
    b32 Result = (Stream->Size >= Compare.Size);

    if (Result)
    {
        for (usize Index = 0; Index < Compare.Size; Index++)
        {
            if (Stream->Base[Index] != Compare.Data[Index])
            {
                Result = false;
                break;
            }
        }
    }

    if (Result)
    {
        Stream->Base += Compare.Size;
        Stream->Size -= Compare.Size;
    }

    return (Result);
}

local void PrintfPushBuffer(printf_stream* Out, void* Buffer, usize Size)
{
    u8* Bytes = (u8*)Buffer;
    usize Count = Minimum(Out->Size, Size);

    for (usize Index = 0; Index < Count; Index++)
    {
        Out->Base[Index] = Bytes[Index];
    }

    Out->Base += Count;
    Out->Size -= Count;
}

local void PrintfPushNumber(printf_stream* Out, usize Value, usize Base, b32 Uppercase)
{
    persist char Buffer[64] = {0};
    persist char DigitMapLower[] = "0123456789abcdef";
    persist char DigitMapUpper[] = "0123456789ABCDEF";

    char* DigitMap = (Uppercase) ? (DigitMapUpper) : (DigitMapLower);

    usize DigitCount = 0;
    usize DigitIndex = ArrayCount(Buffer);

    do
    {
        u32 Digit = (Value % Base);
        Value /= Base;

        DigitCount++;
        DigitIndex--;

        Buffer[DigitIndex] = DigitMap[Digit];
    } while (Value);

    PrintfPushBuffer(Out, Buffer + DigitIndex, DigitCount);
}

local usize SPrintfv(void* Buffer, usize BufferSize, string Format, va_list ArgList)
{
    printf_stream In  = {Format.Data, Format.Size};
    printf_stream Out = {Buffer, BufferSize};

    while (In.Size && Out.Size)
    {
        // NOTE(vak): Consume and output characters until a
        // '%' character is encountered.

        while (In.Size && Out.Size)
        {
            char Character = PrintfPeek(&In);

            if (Character == '%')
                break;

            PrintfPush(&Out, Character);
            PrintfConsume(&In);
        }

        // NOTE(vak): Skip '%'

        PrintfConsume(&In);

        if ((In.Size && Out.Size) == 0)
            break;

        // NOTE(vak): Parse format

        printf_format_type  Type  = PrintfFormatType_Unknown;
        printf_format_value Value = {0};

        #define Match(MatchString, ValueType, ArgType) \
            else if (PrintfMatchAndConsume(&In, Str(MatchString))) \
            { \
                Type = PrintfFormatType_##ValueType; \
                Value.ValueType = va_arg(ArgList, ArgType); \
            }

        if (0) {}

        Match("s8",    SignedInt,   s8    )
        Match("s16",   SignedInt,   s16   )
        Match("s32",   SignedInt,   s32   )
        Match("s64",   SignedInt,   s64   )
        Match("ssize", SignedInt,   ssize )

        Match("u8",    UnsignedInt, u8    )
        Match("u16",   UnsignedInt, u16   )
        Match("u32",   UnsignedInt, u32   )
        Match("u64",   UnsignedInt, u64   )
        Match("usize", UnsignedInt, usize )

        Match("x8",    LowerHex,    u8    )
        Match("x16",   LowerHex,    u16   )
        Match("x32",   LowerHex,    u32   )
        Match("x64",   LowerHex,    u64   )
        Match("xsize", LowerHex,    usize )

        Match("X8",    UpperHex,    u8    )
        Match("X16",   UpperHex,    u16   )
        Match("X32",   UpperHex,    u32   )
        Match("X64",   UpperHex,    u64   )
        Match("Xsize", UpperHex,    usize )

        Match("cstr",  CString,     char* )
        Match("str",   String,      string)
        Match("char",  Character,   char  )

        #undef Match

        // NOTE(vak): Format the output

        switch (Type)
        {
            default:
            {
                if (In.Size && Out.Size)
                {
                    char Character = PrintfConsume(&In);
                    PrintfPush(&Out, Character);
                }
            } break;

            case PrintfFormatType_SignedInt:
            {
                if (Value.SignedInt < 0)
                {
                    PrintfPush(&Out, '-');
                    Value.SignedInt = -Value.SignedInt;
                }

                PrintfPushNumber(&Out, Value.SignedInt, 10, false);
            } break;

            case PrintfFormatType_UnsignedInt:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 10, false);
            } break;

            case PrintfFormatType_LowerHex:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 16, false);
            } break;

            case PrintfFormatType_UpperHex:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 16, true);
            } break;

            case PrintfFormatType_Character:
            {
                PrintfPush(&Out, Value.Character);
            } break;

            case PrintfFormatType_String:
            {
                PrintfPushBuffer(&Out, Value.String.Data, Value.String.Size);
            } break;

            case PrintfFormatType_CString:
            {
                if (Value.CString)
                {
                    usize Size = 0;
                    while (Value.CString[Size])
                        Size++;

                    PrintfPushBuffer(&Out, Value.CString, Size);
                }
            } break;
        }
    }

    usize BytesWritten = (BufferSize - Out.Size);

    return (BytesWritten);
}
