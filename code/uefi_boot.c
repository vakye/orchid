
#include "shared.h"
#include "printf.h"
#include "serial.h"
#include "memory.h"
#include "arch.h"
#include "kernel.h"

#include "shared.c"
#include "printf.c"
#include "serial.c"
#include "memory.c"
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

local memory_map UEFIObtainMemoryMap(
    EFI_SYSTEM_TABLE*       SystemTable,
    UINTN*                  MemoryMapKey
)
{
    EFI_BOOT_SERVICES* BootServices = SystemTable->BootServices;

    memory_map Result = {0};

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
            usize NewSize  = MemoryMapSize + 4*DescriptorSize;
            usize NewCount = NewSize / DescriptorSize;

            if (MemoryDescriptors)
                UEFIFree(SystemTable, MemoryDescriptors);

            if (Result.Regions)
                UEFIFree(SystemTable, Result.Regions);

            MemoryDescriptors = UEFIAllocate(SystemTable, NewSize);
            Result.Regions    = UEFIAllocate(SystemTable, NewCount * sizeof(memory_region));
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

    Result.RegionCount = MemoryMapSize / DescriptorSize;
    ZeroArray(Result.Regions, Result.RegionCount);

    u8* Base = (u8*)MemoryDescriptors;

    for (usize Index = 0; Index < Result.RegionCount; Index++)
    {
        EFI_MEMORY_DESCRIPTOR* Descriptor = (EFI_MEMORY_DESCRIPTOR*)Base;
        memory_region* Region = Result.Regions + Index;

        Region->Kind        = MemoryRegionKind_Unknown;
        Region->PageCount   = Descriptor->NumberOfPages;
        Region->BaseAddress = Descriptor->PhysicalStart;

        switch (Descriptor->Type)
        {
            default: {} break;

            case EfiLoaderCode:
            case EfiBootServicesCode:
            {
                Region->Kind = MemoryRegionKind_BootCode;
            } break;

            case EfiLoaderData:
            case EfiBootServicesData:
            {
                Region->Kind = MemoryRegionKind_BootData;
            } break;

            case EfiRuntimeServicesCode:
            case EfiRuntimeServicesData:
            case EfiConventionalMemory:
            {
                Region->Kind = MemoryRegionKind_Usable;
            } break;
        }

        Base += DescriptorSize;
    }

    return (Result);
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
    UEFISetupConsole(SystemTable);

    UINTN MemoryMapKey = 0;

    memory_map MemoryMap = UEFIObtainMemoryMap(
        SystemTable,
        &MemoryMapKey
    );

    UEFIExitBootServices(
        SystemTable,
        ImageHandle,
        MemoryMapKey
    );

    KernelEntry(&MemoryMap);

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
