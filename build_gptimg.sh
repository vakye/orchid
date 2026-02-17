
#!/bin/bash

Compiler="clang"
Flags="-std=c11 -Wall -Wextra -Wpedantic -Werror"
Source="../code/gptimg.c"
Target="gptimg"

cd build
$Compiler $Flags $Source -o $Target
cd ..
