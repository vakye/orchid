
@echo off

set Emulator=qemu-system-x86_64
set Flags=-serial stdio -machine q35 -m 256 -drive format=raw,index=0,file=orchid.img -bios ../bios/OVMF.fd

pushd build
%Emulator% %Flags%
popd
