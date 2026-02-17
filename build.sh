
#!/bin/bash

Compiler="clang"

Flags="\
    -std=c11 \
    -Wall -Wextra -Wpedantic -Werror \
    -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable \
    --target=x86_64-unknown-windows \
    -ffreestanding -nostdlib -mno-red-zone -mno-stack-arg-probe \
    -nostdlib \
    -fuse-ld=lld \
    -Wl,-subsystem:efi_application,-entry:UEFIBoot"
Source="../code/uefi_boot.c"
Target="BOOTX64.EFI"

if [ ! -d build ]; then
    mkdir -p build
    sh build_gptimg.sh
fi

if [ ! -d build/gptimg ]; then
    sh build_gptimg.sh
fi

cd build
$Compiler $Flags $Source -o $Target
./gptimg
cd ..
