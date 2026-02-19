
#pragma once

typedef s8           INT8;
typedef s16          INT16;
typedef s32          INT32;
typedef s64          INT64;

typedef u8           UINT8;
typedef u16          UINT16;
typedef u32          UINT32;
typedef u64          UINT64;

typedef ssize        INTN;
typedef usize        UINTN;

typedef b8           BOOLEAN;
typedef UINT16       CHAR16;
typedef void         VOID;

typedef UINTN        EFI_STATUS;
typedef VOID*        EFI_HANDLE;
typedef VOID*        EFI_EVENT;

typedef struct
{
    u32 TimeLow;
    u16 TimeMid;
    u16 TimeHighAndVersion;
    u8  Nodes[8];
} EFI_GUID;

// NOTE(vak): For ACPI 1.0
#define ACPI_TABLE_GUID (EFI_GUID) \
    {0xeb9d2d30,0x2d88,0x11d3,\
    {0x9a,0x16,0x00,0x90,0x27,0x3f,0xc1,0x4d}}

// NOTE(vak): For ACPI 2.0 and above
#define EFI_ACPI_TABLE_GUID (EFI_GUID) \
    {0x8868e871,0xe4f1,0x11d3,\
    {0xbc,0x22,0x00,0x80,0xc7,0x3c,0x88,0x81}}

#define IN
#define OUT
#define OPTIONAL

#define EFIAPI __attribute__((ms_abi))

// NOTE(vak): Status codes

#define EFIStatusHighBitSet(Value) ((EFI_STATUS)(Value) | ((EFI_STATUS)1 << (sizeof(EFI_STATUS)*8 - 1)))

#define EFI_SUCCESS              (0)
#define EFI_LOAD_ERROR           EFIStatusHighBitSet(1)
#define EFI_INVALID_PARAMETER    EFIStatusHighBitSet(2)
#define EFI_UNSUPPORTED          EFIStatusHighBitSet(3)
#define EFI_BAD_BUFFER_SIZE      EFIStatusHighBitSet(4)
#define EFI_BUFFER_TOO_SMALL     EFIStatusHighBitSet(5)
#define EFI_NOT_READY            EFIStatusHighBitSet(6)
#define EFI_DEVICE_ERROR         EFIStatusHighBitSet(7)
#define EFI_WRITE_PROTECTED      EFIStatusHighBitSet(8)
#define EFI_OUT_OF_RESOURCES     EFIStatusHighBitSet(9)
#define EFI_VOLUME_CORRUPTED     EFIStatusHighBitSet(10)
#define EFI_VOLUME_FULL          EFIStatusHighBitSet(11)
#define EFI_NO_MEDIA             EFIStatusHighBitSet(12)
#define EFI_MEDIA_CHANGED        EFIStatusHighBitSet(13)
#define EFI_NOT_FOUND            EFIStatusHighBitSet(14)
#define EFI_ACCESS_DENIED        EFIStatusHighBitSet(15)
#define EFI_NO_RESPONSE          EFIStatusHighBitSet(16)
#define EFI_NO_MAPPING           EFIStatusHighBitSet(17)
#define EFI_TIMEOUT              EFIStatusHighBitSet(18)
#define EFI_NOT_STARTED          EFIStatusHighBitSet(19)
#define EFI_ALREADY_STARTED      EFIStatusHighBitSet(20)
#define EFI_ABORTED              EFIStatusHighBitSet(21)
#define EFI_ICMP_ERROR           EFIStatusHighBitSet(22)
#define EFI_TFTP_ERROR           EFIStatusHighBitSet(23)
#define EFI_PROTOCOL_ERROR       EFIStatusHighBitSet(24)
#define EFI_INCOMPATIBLE_VERSION EFIStatusHighBitSet(25)
#define EFI_SECURITY_VIOLATION   EFIStatusHighBitSet(26)
#define EFI_CRC_ERROR            EFIStatusHighBitSet(27)
#define EFI_END_OF_MEDIA         EFIStatusHighBitSet(28)
#define EFI_END_OF_FILE          EFIStatusHighBitSet(31)
#define EFI_INVALID_LANGUAGE     EFIStatusHighBitSet(32)
#define EFI_COMPROMISED_DATA     EFIStatusHighBitSet(33)
#define EFI_IP_ADDRESS_CONFLICT  EFIStatusHighBitSet(34)
#define EFI_HTTP_ERROR           EFIStatusHighBitSet(35)

// NOTE(vak): Simple text input protocol

typedef struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef struct
{
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)
(
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL*  This,
    OUT EFI_INPUT_KEY*                  Key
);

struct _EFI_SIMPLE_TEXT_INPUT_PROTOCOL
{
    VOID*               Reset;
    EFI_INPUT_READ_KEY  ReadKeyStroke;
    EFI_EVENT           WaitForKey;
};

// NOTE(vak): Simple text output protocol

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)
(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,
    IN CHAR16*                          String
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)
(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This
);

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
{
    VOID*                  Reset;
    EFI_TEXT_STRING        OutputString;
    VOID*                  TestString;
    VOID*                  QueryMode;
    VOID*                  SetMode;
    VOID*                  SetAttribute;
    EFI_TEXT_CLEAR_SCREEN  ClearScreen;
    VOID*                  SetCursorPosition;
    VOID*                  EnableCursor;
    VOID*                  Mode;
};

// NOTE(vak): Table header

typedef struct
{
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// NOTE(vak): Runtime services

typedef enum
{
    EfiResetCold,
    EfiResetHot,
    EfiResetShutdown,
} EFI_RESET_TYPE;

typedef VOID (EFIAPI *EFI_RESET_SYSTEM)
(
    IN EFI_RESET_TYPE   ResetType,
    IN EFI_STATUS       ResetStatus,
    IN UINTN            DataSize,
    IN VOID*            ResetData OPTIONAL
);

typedef struct
{
    EFI_TABLE_HEADER    Header;

    VOID*               GetTime;
    VOID*               SetTime;
    VOID*               GetWakeupTime;
    VOID*               SetWakeupTime;

    VOID*               SetVirtualAddressMap;
    VOID*               ConvertPointer;

    VOID*               GetVariable;
    VOID*               GetNextVariableName;
    VOID*               SetVariable;

    VOID*               GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM    ResetSystem;

    VOID*               UpdateCapsule;
    VOID*               QueryCapsuleCapabilities;

    VOID*               QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

// NOTE(vak): Boot services

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiUnacceptedMemoryType,
    EfiMaxMemoryType,
} EFI_MEMORY_TYPE;

typedef UINTN EFI_PHYSICAL_ADDRESS;
typedef UINTN EFI_VIRTUAL_ADDRESS;

#define EFI_MEMORY_DESCRIPTOR_VERSION 1

#define EFI_MEMORY_UC              0x0000000000000001
#define EFI_MEMORY_WC              0x0000000000000002
#define EFI_MEMORY_WT              0x0000000000000004
#define EFI_MEMORY_WB              0x0000000000000008
#define EFI_MEMORY_UCE             0x0000000000000010
#define EFI_MEMORY_WP              0x0000000000001000
#define EFI_MEMORY_RP              0x0000000000002000
#define EFI_MEMORY_XP              0x0000000000004000
#define EFI_MEMORY_NV              0x0000000000008000
#define EFI_MEMORY_MORE_RELIABLE   0x0000000000010000
#define EFI_MEMORY_RO              0x0000000000020000
#define EFI_MEMORY_SP              0x0000000000040000
#define EFI_MEMORY_CPU_CRYPTO      0x0000000000080000
#define EFI_MEMORY_RUNTIME         0x8000000000000000
#define EFI_MEMORY_ISA_VALID       0x4000000000000000
#define EFI_MEMORY_ISA_MASK        0x0FFFF00000000000

typedef struct
{
    UINT32                  Type;
    EFI_PHYSICAL_ADDRESS    PhysicalStart;
    EFI_VIRTUAL_ADDRESS     VirtualStart;
    UINT64                  NumberOfPages;
    UINT64                  Attribute;
} EFI_MEMORY_DESCRIPTOR;

typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP)
(
    IN OUT UINTN*               MemoryMapSize,
    OUT EFI_MEMORY_DESCRIPTOR*  MemoryMap,
    OUT UINTN*                  MemoryMapKey,
    OUT UINTN*                  DescriptorSize,
    OUT UINT32*                 DescriptorVersion
);

typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL)
(
    IN EFI_MEMORY_TYPE          PoolType,
    IN UINTN                    Size,
    IN VOID**                   Buffer
);

typedef EFI_STATUS (EFIAPI* EFI_FREE_POOL)
(
    IN VOID*                    Buffer
);

typedef EFI_STATUS (EFIAPI* EFI_EXIT_BOOT_SERVICES)
(
    IN EFI_HANDLE               ImageHandle,
    IN UINTN                    MemoryMapKey
);

typedef struct
{
    EFI_TABLE_HEADER        Header;

    VOID*                   RaiseTPL;
    VOID*                   RestoreTPL;

    VOID*                   AllocatePages;
    VOID*                   FreePages;
    EFI_GET_MEMORY_MAP      GetMemoryMap;
    EFI_ALLOCATE_POOL       AllocatePool;
    EFI_FREE_POOL           FreePool;

    VOID*                   CreateEvent;
    VOID*                   SetTimer;
    VOID*                   WaitForEvent;
    VOID*                   SignalEvent;
    VOID*                   CloseEvent;
    VOID*                   CheckEvent;

    VOID*                   InstallProtocolInterface;
    VOID*                   ReinstallProtocolInterface;
    VOID*                   UninstallProtocolInterface;
    VOID*                   HandleProtocol;
    VOID*                   Reserved;
    VOID*                   RegisterProtocolNotify;
    VOID*                   LocateHandle;
    VOID*                   LocateDevicePath;
    VOID*                   InstallConfigurationTable;

    VOID*                   LoadImage;
    VOID*                   StartImage;
    VOID*                   Exit;
    VOID*                   UnloadImage;
    EFI_EXIT_BOOT_SERVICES  ExitBootServices;

    VOID*                   GetNextMonotonicCount;
    VOID*                   Stall;
    VOID*                   SetWatchdogTimer;

    VOID*                   ConnectController;
    VOID*                   DisconnectController;

    VOID*                   OpenProtocol;
    VOID*                   CloseProtocol;
    VOID*                   OpenProtocolInformation;

    VOID*                   ProtocolsPerHandle;
    VOID*                   LocateHandleBuffer;
    VOID*                   LocateProtocol;

    VOID*                   InstallMultipleProtocolInterfaces;
    VOID*                   UninstallMultipleProtocolInterfaces;

    VOID*                   CalculateCRC32;

    VOID*                   CopyMem;
    VOID*                   SetMem;
    VOID*                   CreateEventEx;
} EFI_BOOT_SERVICES;

// NOTE(vak): System table

typedef struct
{
    EFI_GUID    VendorGuid;
    VOID*       VendorTable;
} EFI_CONFIGURATION_TABLE;

typedef struct
{
    EFI_TABLE_HEADER                    Header;
    CHAR16*                             FirmwareVendor;
    UINT32                              FirmwareRevision;
    EFI_HANDLE                          ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL*     ConIn;
    EFI_HANDLE                          ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*    ConOut;
    EFI_HANDLE                          StandardErrorHandle;
    VOID*                               StdErr;
    VOID*                               RuntimeServices;
    VOID*                               BootServices;
    UINTN                               NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE*            ConfigurationTable;
} EFI_SYSTEM_TABLE;
