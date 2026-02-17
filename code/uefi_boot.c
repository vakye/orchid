
typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;
typedef signed long long INT64;

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

typedef INT64 INTN;
typedef UINT64 UINTN;

typedef void VOID;

typedef UINT16 CHAR16;

typedef unsigned int EFI_STATUS;
typedef VOID* EFI_HANDLE;

#define EFIAPI __attribute__((ms_abi))
#define IN
#define OUT

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

EFI_STATUS EFIAPI UEFIBoot(void* ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
    (void) ImageHandle;

    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut = SystemTable->ConOut;

    ConOut->ClearScreen(ConOut);
    ConOut->OutputString(ConOut, L"Hello, world!\r\n");

    for (;;);
}
