
@echo off

if not exist build mkdir build

if not exist build/gptimg.exe call build_gptimg.bat

set Compiler=clang

set Flags=
set Flags=%Flags% -std=c11
set Flags=%Flags% -O0
set Flags=%Flags% -Wall -Wextra -Wpedantic -Werror
set Flags=%Flags% -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-varargs
set Flags=%Flags% --target=x86_64-unknown-windows
set Flags=%Flags% -ffreestanding -nostdlib -mno-red-zone -mno-stack-arg-probe
set Flags=%Flags% -nostdlib
set Flags=%Flags% -fuse-ld=lld
set Flags=%Flags% -Wl,-subsystem:efi_application,-entry:"UEFIBoot"

set Source=../code/uefi_boot.c
set Target=BOOTX64.EFI

pushd build
call %Compiler% %Flags% %Source% -o %Target%
call gptimg.exe
popd
