
#include "shared.h"
#include "arch.h"
#include "kernel.h"

#include "shared.c"
#include "arch.c"
#include "kernel.c"

#include "uefi_boot.h"

local void UEFISetupConsole(EFI_SYSTEM_TABLE* SystemTable)
{
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut = SystemTable->ConOut;

    ConOut->ClearScreen(ConOut);
}

local void UEFIError(EFI_SYSTEM_TABLE* SystemTable, CHAR16* Message)
{
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL*  ConIn           = SystemTable->ConIn;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut          = SystemTable->ConOut;
    EFI_RUNTIME_SERVICES*            RuntimeServices = SystemTable->RuntimeServices;

    ConOut->OutputString(ConOut, L"ERROR: ");
    ConOut->OutputString(ConOut, Message);
    ConOut->OutputString(ConOut, L"\r\nPress any key to shutdown...\r\n");

    EFI_INPUT_KEY Key = {0};
    while (ConIn->ReadKeyStroke(ConIn, &Key) != EFI_SUCCESS)
    {
    }

    RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);
}

local void* UEFIAllocate(EFI_SYSTEM_TABLE* SystemTable, usize Size)
{
    EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

    void* Result = 0;

    EFI_STATUS Status = BootServices->AllocatePool(
        EfiLoaderData,
        Size,
        &Result
    );

    if (Status == EFI_OUT_OF_RESOURCES)
    {
        UEFIError(SystemTable, L"Ran out of 'EfiLoaderData' pool memory.");
    }
    else if (Status == EFI_INVALID_PARAMETER)
    {
        UEFIError(SystemTable, L"Invalid parameter passed to BootServices->AllocatePool().");
    }
    else if (Status != EFI_SUCCESS)
    {
        UEFIError(SystemTable, L"Unknown error returned by BootServices->AllocatePool().");
    }

    return (Result);
}

local void UEFIFree(EFI_SYSTEM_TABLE* SystemTable, void* Buffer)
{
    if (Buffer)
    {
        EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

        EFI_STATUS Status = BootServices->FreePool(Buffer);

        if (Status == EFI_INVALID_PARAMETER)
        {
            UEFIError(SystemTable, L"Invalid parameter passed to BootServices->FreePool().");
        }
        else if (Status != EFI_SUCCESS)
        {
            UEFIError(SystemTable, L"Unknown error returned by BootServices->FreePool().");
        }
    }
}

local void UEFIObtainMemoryMap(
    EFI_SYSTEM_TABLE*       SystemTable,
    UINTN*                  DescriptorCount,
    VOID**                  Descriptors,
    UINTN*                  MemoryMapKey
)
{
    EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

    usize MemoryMapSize     = 0;
    void* MemoryDescriptors = 0;
    usize DescriptorSize    = 0;
    u32   DescriptorVersion = 0;

    for (;;)
    {
        EFI_STATUS Status = BootServices->GetMemoryMap(
            &MemoryMapSize,
            MemoryDescriptors,
            MemoryMapKey,
            &DescriptorSize,
            &DescriptorVersion
        );

        if (Status == EFI_SUCCESS)
        {
            break;
        }
        else if (Status == EFI_BUFFER_TOO_SMALL)
        {
            usize NewSize = MemoryMapSize + 4*DescriptorSize;

            if (MemoryDescriptors)
                UEFIFree(SystemTable, MemoryDescriptors);

            MemoryDescriptors = UEFIAllocate(SystemTable, NewSize);
        }
        else if (Status == EFI_INVALID_PARAMETER)
        {
            UEFIError(SystemTable, L"Invalid parameter passed to BootServices->GetMemoryMap().");
        }
        else
        {
            UEFIError(SystemTable, L"Unknown error returned by BootServices->GetMemoryMap().");
        }
    }

    *DescriptorCount = MemoryMapSize / DescriptorSize;
    *Descriptors     = MemoryDescriptors;
}

local void UEFIExitBootServices(
    EFI_SYSTEM_TABLE*   SystemTable,
    EFI_HANDLE          ImageHandle,
    UINTN               MemoryMapKey
)
{
    EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

    EFI_STATUS Status = BootServices->ExitBootServices(
        ImageHandle,
        MemoryMapKey
    );

    if (Status == EFI_INVALID_PARAMETER)
    {
        UEFIError(SystemTable, L"Invalid parameter passed to BootServices->ExitBootServices().");
    }
    else if (Status != EFI_SUCCESS)
    {
        UEFIError(SystemTable, L"Unknown error returned by BootServices->ExitBootServices().");
    }
}

EFI_STATUS EFIAPI UEFIBoot(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
)
{
    (void) ImageHandle;

    UEFISetupConsole(SystemTable);

    UINTN DescriptorCount = 0;
    VOID* Descriptors     = 0;
    UINTN MemoryMapKey    = 0;

    UEFIObtainMemoryMap(
        SystemTable,
        &DescriptorCount, &Descriptors,
        &MemoryMapKey
    );

    UEFIExitBootServices(
        SystemTable,
        ImageHandle,
        MemoryMapKey
    );

    KernelEntry();

    return (EFI_SUCCESS);
}

// NOTE(vak): CRT stuff

void* memset(void* DestInit, s32 Byte, usize Size)
{
    u8* Dest = (u8*)DestInit;
    while (Size--)
        *Dest++ = Byte;

    return (DestInit);
}

void* memcpy(void* DestInit, void* SourceInit, usize Size)
{
    u8* Dest = (u8*)DestInit;
    u8* Source = (u8*)SourceInit;
    while (Size--)
        *Dest++ = *Source++;

    return (DestInit);
}
