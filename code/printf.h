
#pragma once

// NOTE(vak): Supported formats:
//     %s8:    8-bit signed integer
//     %s16:   16-bit signed integer
//     %s32:   32-bit signed integer
//     %s64:   64-bit signed integer
//     %ssize: largest signed integer
//
//     %u8:    8-bit unsigned integer
//     %u16:   16-bit unsigned integer
//     %u32:   32-bit unsigned integer
//     %u64:   64-bit unsigned integer
//     %usize: largest signed integer
//
//     %x8:    8-bit unsigned integer (lowercase hex)
//     %x16:   16-bit unsigned integer (lowercase hex)
//     %x32:   32-bit unsigned integer (lowercase hex)
//     %x64:   64-bit unsigned integer (lowercase hex)
//     %xsize: largest signed integer (lowercase hex)
//
//     %X8:    8-bit unsigned integer (uppercase hex)
//     %X16:   16-bit unsigned integer (uppercase hex)
//     %X32:   32-bit unsigned integer (uppercase hex)
//     %X64:   64-bit unsigned integer (uppercase hex)
//     %Xsize: largest signed integer (uppercase hex)
//
//     %c:     an ASCII character
//     %str:   string represented as a `string` struct
//     %cstr:  pointer to a null-terminated string

local usize SPrintfv(void* Buffer, usize BufferSize, string Format, va_list ArgList);
