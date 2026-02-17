
#include "shared.h"
#include "uefi_boot.h"

EFI_STATUS EFIAPI UEFIBoot(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    (void) ImageHandle;

    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut = SystemTable->ConOut;

    ConOut->ClearScreen(ConOut);
    ConOut->OutputString(ConOut, L"Hello, world!\r\n");

    for (;;);
}
