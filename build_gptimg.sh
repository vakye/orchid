
#!/bin/bash

Compiler="clang"
Flags="-std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -Wno-unused-function"
Source="../code/gptimg.c"
Target="gptimg"

cd build
$Compiler $Flags $Source -o $Target
cd ..
