
#!/bin/bash

Emulator="qemu-system-x86_64"
Flags="-s -S -machine q35 -m 256 -drive format=raw,index=0,file=orchid.img -bios ../bios/OVMF.fd"

cd build
$Emulator $Flags
cd ..
