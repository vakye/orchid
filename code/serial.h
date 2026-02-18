
#pragma once

local usize SerialPrintfv(string Format, va_list ArgList);
local usize SerialPrintf(string Format, ...);

local usize SerialDebugf(string Format, ...);
local usize SerialInfof (string Format, ...);
local usize SerialErrorf(string Format, ...);
