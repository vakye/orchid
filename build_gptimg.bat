
@echo off

set Compiler=clang
set Flags=-std=c11 -O2 -Wall -Wextra -Wpedantic -Werror -Wno-unused-function
set Source=../code/gptimg.c
set Target=gptimg.exe

pushd build
%Compiler% %Flags% %Source% -o %Target%
popd
