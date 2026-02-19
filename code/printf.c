
typedef struct
{
    char* Base;
    usize Size;

    // NOTE(vak): Only for output

    usize Width;
    usize Precision;
    b8 ForceSign;
    b8 LeftPad;
    b8 ZeroPad;
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

local void PrintfPad(printf_stream* Out, usize Padding)
{
    for (usize Index = 0; Index < Padding; Index++)
    {
        PrintfPush(Out, Out->ZeroPad ? '0' : ' ');
    }
}

local void PrintfPushBuffer(printf_stream* Out, void* Buffer, usize Size)
{
    u8* Bytes = (u8*)Buffer;

    usize Count = Minimum(Size, Out->Precision);
    usize Padding = Out->Width - Minimum(Count, Out->Width);

    if (Out->LeftPad)
        PrintfPad(Out, Padding);

    for (usize Index = 0; Index < Count; Index++)
    {
        PrintfPush(Out, Bytes[Index]);
    }

    if (!Out->LeftPad)
        PrintfPad(Out, Padding);
}

local void PrintfPushNumber(
    printf_stream* Out,
    usize Value,
    usize Base,
    b32 Uppercase,
    b32 Negative
)
{
    if (Base < 2)  return;
    if (Base > 16) return;

    persist char Buffer[65] = {0};
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

    if (Negative)
    {
        DigitCount++;
        DigitIndex--;

        Buffer[DigitIndex] = '-';
    }
    else if (Out->ForceSign)
    {
        DigitCount++;
        DigitIndex--;

        Buffer[DigitIndex] = '+';
    }

    PrintfPushBuffer(Out, Buffer + DigitIndex, DigitCount);
}

local usize PrintfParseNumber(printf_stream* In)
{
    usize Result = 0;

    while (In->Size)
    {
        char Character = PrintfPeek(In);

        if ((Character < '0') || (Character > '9'))
            break;

        Result *= 10;
        Result += Character - '0';

        PrintfConsume(In);
    }

    return (Result);
}

local usize SPrintfv(void* Buffer, usize BufferSize, string Format, va_list ArgList)
{
    printf_stream In  = {0};
    printf_stream Out = {0};

    Out.Base      = Buffer;
    Out.Size      = BufferSize;
    Out.ForceSign = false;
    Out.ZeroPad   = false;
    Out.LeftPad   = false;
    Out.Width     = 0;
    Out.Precision = USizeMax;

    In.Base = Format.Data;
    In.Size = Format.Size;

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

        // NOTE(vak): Reset configuration

        Out.ForceSign = false;
        Out.ZeroPad   = false;
        Out.LeftPad   = false;
        Out.Width     = 0;
        Out.Precision = USizeMax;

        // NOTE(vak): Parse flags

        b32 KeepParsingFlags = true;

        while (KeepParsingFlags)
        {
            char Character = PrintfPeek(&In);

            switch (Character)
            {
                default: KeepParsingFlags = false; break;

                case '+': Out.ForceSign = true; break;
                case '0': Out.ZeroPad   = true; break;
                case '-': Out.LeftPad   = true; break;
            }

            if (KeepParsingFlags)
                PrintfConsume(&In);
        }

        // NOTE(vak): Parse width

        if (PrintfPeek(&In) == '*')
        {
            PrintfConsume(&In);
            Out.Width = va_arg(ArgList, usize);
        }
        else
        {
            Out.Width = PrintfParseNumber(&In);
        }

        // NOTE(vak): Parse precision

        if (PrintfPeek(&In) == '.')
        {
            PrintfConsume(&In);

            if (PrintfPeek(&In) == '*')
            {
                PrintfConsume(&In);
                Out.Precision = va_arg(ArgList, usize);
            }
            else
            {
                Out.Precision = PrintfParseNumber(&In);
            }
        }

        // NOTE(vak): Parse specifier

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

        Match("p",     LowerHex,    usize )
        Match("P",     UpperHex,    usize )

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
                b32 Negative = false;

                if (Value.SignedInt < 0)
                {
                    Negative = true;
                    Value.SignedInt = -Value.SignedInt;
                }

                PrintfPushNumber(&Out, Value.SignedInt, 10, false, Negative);
            } break;

            case PrintfFormatType_UnsignedInt:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 10, false, false);
            } break;

            case PrintfFormatType_LowerHex:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 16, false, false);
            } break;

            case PrintfFormatType_UpperHex:
            {
                PrintfPushNumber(&Out, Value.UnsignedInt, 16, true, false);
            } break;

            case PrintfFormatType_Character:
            {
                PrintfPushBuffer(&Out, &Value.Character, 1);
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

local usize SPrintf(void* Buffer, usize BufferSize, string Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);

    usize Result = SPrintfv(Buffer, BufferSize, Format, ArgList);

    va_end(ArgList);

    return (Result);
}
