
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

#define IN
#define OUT

#define EFIAPI __attribute__((ms_abi))

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

typedef struct
{
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct
{
    EFI_TABLE_HEADER    Header;
    CHAR16*             FirmwareVendor;
    UINT32              FirmwareRevision;
    EFI_HANDLE          ConsoleInHandle;
    VOID*               ConIn;
    EFI_HANDLE          ConsoleOutHandle;
    VOID*               ConOut;
    EFI_HANDLE          StandardErrorHandle;
    VOID*               StdErr;
    VOID*               RuntimeServices;
    VOID*               BootServices;
    UINTN               NumberOfTableEntries;
    VOID*               ConfigurationTable;
} EFI_SYSTEM_TABLE;
