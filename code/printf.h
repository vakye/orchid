
#pragma once

local usize SPrintfv(void* Buffer, usize BufferSize, string Format, va_list ArgList);

// NOTE(vak): Formatting guide
//
// All format specifiers start with a '%', which is then followed by "Width" (optional),
// "Precision" (optional), and "Specifier" (required). If the "Specifier" doesn't match
// any of the specifiers defined below, then the character after '%' will be printed.
//
//     ('%') [Flags] [Width] ['.' Precision] (Specifier)
//
// (...) means required
// [...] means optional
//
// Width: is a 'usize' specifying the minimum characters to be printed.
//        This can be a number or a '*' character, which means to take
//        a 'usize' from variadic arguments.
//
// Precision: is a 'usize' specifying the maximum characters to be printed
//        This can be a number or a '*' character, which means to take
//        a 'usize' from variadic arguments.
//
// Specifiers:
//     s8:    8-bit signed integer
//     s16:   16-bit signed integer
//     s32:   32-bit signed integer
//     s64:   64-bit signed integer
//     ssize: largest signed integer
//
//     u8:    8-bit unsigned integer
//     u16:   16-bit unsigned integer
//     u32:   32-bit unsigned integer
//     u64:   64-bit unsigned integer
//     usize: largest signed integer
//
//     x8:    8-bit unsigned integer (lowercase hex)
//     x16:   16-bit unsigned integer (lowercase hex)
//     x32:   32-bit unsigned integer (lowercase hex)
//     x64:   64-bit unsigned integer (lowercase hex)
//     xsize: largest signed integer (lowercase hex)
//
//     X8:    8-bit unsigned integer (uppercase hex)
//     X16:   16-bit unsigned integer (uppercase hex)
//     X32:   32-bit unsigned integer (uppercase hex)
//     X64:   64-bit unsigned integer (uppercase hex)
//     Xsize: largest signed integer (uppercase hex)
//
//     char:  an ASCII character
//     str:   string represented as a `string` struct
//     cstr:  pointer to a null-terminated string

// NOTE(vak): Remarks:
//

// NOTE(vak): Example usage:
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("%u32"), 100);
//         => StrData(Buffer, BytesWritten) = "100"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("My favorite string: %str"), Str("Hello, world!"));
//         => StrData(Buffer, BytesWritten) = "My favorite string: Hello, world!"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("My number: %ssize"), -100ll);
//         => StrData(Buffer, BytesWritten) = "My number: -100"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("Lowercase hex: %xsize"), 0xDEADBEEF);
//         => StrData(Buffer, BytesWritten) = "Lowercase hex: deadbeef"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("Uppercase hexadecimal: %X32"), 0xdeadcafe);
//         => StrData(Buffer, BytesWritten) = "Uppercase hexadecimal: DEADCAFE"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("Another string: %cstr"), "This is a null-terminated string!");
//         => StrData(Buffer, BytesWritten) = "Another string: This is a null-terminated string!"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("%%"));
//         => StrData(Buffer, BytesWritten) = "%"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("%haha"));
//         => StrData(Buffer, BytesWritten) = "haha"
//
//     usize BytesWritten = SPrintfv(Buffer, BufferSize, Str("%10u32"), 100);
//         => StrData(Buffer, BytesWritten) = "100       "
