
// NOTE(vak): This program searches for "BOOTX64.EFI" in the caller's
// directory, then proceeds to create a UEFI GPT image with "BOOTX64.EFI"
// placed in the "/EFI/BOOT" directory. The resulting image is then
// outputted to "orchid.img".

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define CTAssert(Expression) _Static_assert(Expression, "Compile-time assertion failed")

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Minimum(A, B) ((A) < (B) ? (A) : (B))
#define Maximum(A, B) ((A) > (B) ? (A) : (B))

#define packed __attribute__((packed))
#define nonstring __attribute__((nonstring))

#define KB(Amount) ((intptr_t)(Amount) << 10)
#define MB(Amount) ((intptr_t)(Amount) << 20)
#define GB(Amount) ((intptr_t)(Amount) << 30)
#define TB(Amount) ((intptr_t)(Amount) << 40)

#define Align(Value, PowerOf2) (((Value) + (PowerOf2) - 1) & ~((PowerOf2) - 1))

typedef struct packed
{
    uint8_t  BootIndicator;
    uint8_t  StartHead;
    uint8_t  StartSector;
    uint8_t  StartTrack;
    uint8_t  OSIndicator;
    uint8_t  EndHead;
    uint8_t  EndSector;
    uint8_t  EndTrack;
    uint32_t StartingLBA;
    uint32_t SizeInLBA;
} mbr_partition_record;

CTAssert(sizeof(mbr_partition_record) == 16);

typedef struct packed
{
    uint8_t              BootCode[440];
    uint32_t             UniqueMBRSignature;
    uint16_t             Unknown;
    mbr_partition_record Partitions[4];
    uint16_t             Signature;
} master_boot_record;

CTAssert(sizeof(master_boot_record) == 512);

typedef union
{
    uint64_t Parts64[2];
    uint32_t Parts32[4];
    uint16_t Parts16[8];
    uint8_t  Parts8[16];
} guid128;

typedef struct packed
{
    uint64_t Signature;
    uint32_t Revision;
    uint32_t HeaderSize;
    uint32_t HeaderCRC32;
    uint32_t Reserved0;
    uint64_t MyLBA;
    uint64_t AlternateLBA;
    uint64_t FirstUsableLBA;
    uint64_t LastUsableLBA;
    guid128  DiskGUID;
    uint64_t PartitionEntryLBA;
    uint32_t NumberOfPartitionEntries;
    uint32_t SizeOfPartitionEntry;
    uint32_t PartitionArrayCRC32;
    uint8_t  Reserved1[420];
} gpt_header;

CTAssert(sizeof(gpt_header) == 512);

typedef struct packed
{
    guid128  PartitionTypeGUID;
    guid128  UniquePartitionGUID;
    uint64_t StartingLBA;
    uint64_t EndingLBA;
    uint64_t Attributes;
    uint16_t PartitionName[36];
} gpt_entry;

CTAssert(sizeof(gpt_entry) == 128);

typedef struct packed
{
    uint8_t  JumpInstruction[3];
    uint8_t  OEMName[8] nonstring;
    uint16_t BytesPerBlock;
    uint8_t  BlocksPerCluster;
    uint16_t ReservedBlockCount;
    uint8_t  NumberOfFATs;
    uint16_t RootDirectoryEntries;
    uint16_t TotalBlocks16;
    uint8_t  MediaDescriptor;
    uint16_t BlocksPerFAT16;
    uint16_t BlocksPerTrack;
    uint16_t HeadCount;
    uint32_t HiddenBlocks;
    uint32_t TotalBlocks32;
    uint32_t BlocksPerFAT32;
    uint16_t ExtendedFlags;
    uint16_t FilesystemVersion;
    uint32_t RootCluster;
    uint16_t FilesystemInfo;
    uint16_t BackupBootSector;
    uint8_t  Reserved0[12];
    uint8_t  DriveNumber;
    uint8_t  Reserved1;
    uint8_t  BootSignature;
    uint8_t  VolumeID[4];
    uint8_t  VolumeLabel[11] nonstring;
    uint8_t  FilesystemType[8] nonstring;

    uint8_t  BootCode[420];
    uint16_t BootSectorSignature;
} fat32_vbr;

CTAssert(sizeof(fat32_vbr) == 512);

typedef struct packed
{
    uint32_t LeadSignature;
    uint8_t  Reserved0[480];
    uint32_t StructureSignature;
    uint32_t FreeCount;
    uint32_t NextFree;
    uint8_t  Reserved1[12];
    uint32_t TrailingSignature;
} fat32_filesystem_info;

CTAssert(sizeof(fat32_filesystem_info) == 512);

typedef uint8_t fat32_directory_attributes;

#define ATTR_READ_ONLY (0x01)
#define ATTR_HIDDEN    (0x02)
#define ATTR_SYSTEM    (0x04)
#define ATTR_VOLUME_ID (0x08)
#define ATTR_DIRECTORY (0x10)
#define ATTR_ARCHIVE   (0x20)
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

typedef struct packed
{
    uint8_t  Name[11] nonstring;
    uint8_t  Attributes;
    uint8_t  NTRes;
    uint8_t  CreateTimeLength;
    uint16_t CreateTime;
    uint16_t CreateDate;
    uint16_t LastAccessDate;
    uint16_t FirstClusterHigh;
    uint16_t WriteTime;
    uint16_t WriteDate;
    uint16_t FirstClusterLow;
    uint32_t FileSize;
} fat32_directory_entry;

CTAssert(sizeof(fat32_directory_entry) == 32);

typedef struct
{
    size_t BytesPerBlock;       // NOTE(vak): Per UEFI specifications, restricted to be 512/1024/2048/4096
    size_t SystemPartitionSize; // NOTE(vak): Contains boot code and data. Measured in bytes
    size_t Alignment;           // NOTE(vak): Partition alignment. Must be MB(1) per UEFI spec
    char*  ImageName;           // NOTE(vak): Name of the output file.
} settings;

settings Settings =
{
    .BytesPerBlock       = 512,
    .SystemPartitionSize = MB(33), // NOTE(vak): FAT32 so must be larger than 32MB
    .Alignment           = MB(1),
    .ImageName           = "orchid.img",
};

#define EFI_SYSTEM_PARTITION_GUID \
    (guid128){.Parts64 = {0xC12A7328F81F11D2, 0xBA4B00A0C93EC93B}}

static guid128 NewGUID(void)
{
    guid128 Result = {0};

    // TODO(vak): Check the UEFI specification for the GUID
    // generation algorithm

    srand(time(0));

    for (size_t Index = 0; Index < 16; Index++)
    {
        Result.Parts8[Index] = rand() & 0xFF;
    }

    return (Result);
}

static size_t ComputeBlocksSize(size_t BlockCount)
{
    size_t Result = BlockCount * Settings.BytesPerBlock;
    return (Result);
}

static size_t ComputeBlockCount(size_t SizeInBytes)
{
    size_t Aligned = Align(SizeInBytes, Settings.BytesPerBlock);
    size_t Result = Aligned / Settings.BytesPerBlock;

    return (Result);
}

static size_t ComputeAlignedBlockCount(size_t SizeInBytes)
{
    size_t Aligned = Align(SizeInBytes, Settings.Alignment);
    size_t Result = Aligned / Settings.BytesPerBlock;

    return (Result);
}

static void WritePadding(FILE* File, size_t Size)
{
    uint8_t Zero = 0;

    for (size_t Index = 0; Index < Size; Index++)
        fwrite(&Zero, 1, sizeof(Zero), File);
}

static void SeekFile(FILE* File, intptr_t Offset)
{
    fseek(File, Offset, SEEK_SET);
}

static void SeekFileCurrent(FILE* File, intptr_t Offset)
{
    fseek(File, Offset, SEEK_CUR);
}

static size_t GetFileSize(FILE* File)
{
    size_t OldOffset = ftell(File);

    fseek(File, 0, SEEK_END);

    size_t Result = ftell(File);

    fseek(File, OldOffset, SEEK_SET);

    return (Result);
}

static size_t ReadBytes(FILE* File, void* Buffer, size_t Size)
{
    size_t Result = fread(Buffer, 1, Size, File);
    return (Result);
}

static size_t WriteBytes(FILE* File, void* Buffer, size_t Size)
{
    size_t Result = Size;

    fwrite(Buffer, 1, Size, File);

    return (Result);
}

static size_t WriteBlocks(FILE* File, void* Buffer, size_t Size)
{
    size_t Aligned = Align(Size, Settings.BytesPerBlock);
    size_t Padding = Aligned - Size;

    WriteBytes(File, Buffer, Size);
    WritePadding(File, Padding);

    return (Aligned);
}

static uint32_t ComputeCRC32(void* DataInit, size_t Size)
{
    static uint32_t Table[256] =
    {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
        0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
        0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
        0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
        0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
        0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
        0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
        0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
        0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
        0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
        0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
        0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
        0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
        0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
        0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
        0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
        0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
        0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
        0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
        0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
        0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
        0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
        0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
        0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
        0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
        0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
        0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
        0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
        0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
        0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
        0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
        0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
        0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
    };

    uint8_t* Bytes = (uint8_t*)DataInit;
    uint32_t Result = 0xFFFFFFFF;

    for (size_t Index = 0; Index < Size; Index++)
    {
        uint8_t Byte = Bytes[Index];
        uint8_t TableIndex = (Result ^ Byte) & 0xFF;

        Result = Table[TableIndex] ^ (Result >> 8);
    }

    Result ^= 0xFFFFFFFF;

    return (Result);
}

static void SetFAT32Time(fat32_directory_entry* Entry)
{
    time_t CurrentTime = time(0);
    struct tm Time = *localtime(&CurrentTime);

    Time.tm_sec = Minimum(59, Time.tm_sec);

    Entry->CreateDate =
        ((Time.tm_year - 80) << 9) |
        ((Time.tm_mon  +  1) << 5) |
        ((Time.tm_mday));
    ;

    Entry->CreateTime =
        (Time.tm_hour << 11) |
        (Time.tm_min  <<  5) |
        (Time.tm_sec / 2)
    ;

    Entry->WriteDate = Entry->CreateDate;
    Entry->WriteTime = Entry->CreateTime;
}

int main(int ArgCount, char* Args[])
{
    (void) ArgCount;
    (void) Args;

    FILE* BOOTX64 = fopen("BOOTX64.EFI", "rb+");
    if (!BOOTX64)
    {
        printf("error: unable to open 'BOOTX64.EFI'\n");
        return (1);
    }

    FILE* OutFile = fopen(Settings.ImageName, "wb+");
    if (!OutFile)
    {
        printf("error: unable to create/open file '%s'\n", Settings.ImageName);
        return (1);
    }

    if (Settings.SystemPartitionSize <= MB(32))
    {
        printf("error: EFI system partition size is not larger than 32MB (%zu MB)\n", Settings.SystemPartitionSize / MB(1));
    }

    printf("Settings:\n");
    printf("    + Bytes per block:       %zu\n",    Settings.BytesPerBlock);
    printf("    + System partition size: %zu MB\n", Settings.SystemPartitionSize / MB(1));

    // NOTE(vak): Add extra padding for:
    //     2 aligned partitions
    //     2 GPT tables (primary and alternative)
    //     2 GPT headers (primary and alternative)
    //     MBR

    size_t GPTTableSize = KB(16); // NOTE(vak): Per UEFI spec, GPT entry array must be a minimum of KB(16)
    size_t GPTTableBlockCount = ComputeBlockCount(GPTTableSize);

    size_t Padding =
        (Settings.Alignment * 2)     + // NOTE(vak): 2 aligned partitions
        (GPTTableSize * 2)           + // NOTE(vak): 2 GPT tables
        (Settings.BytesPerBlock * 2) + // NOTE(vak): 2 GPT headers
        (Settings.BytesPerBlock)       // NOTE(vak): MBR
    ;

    size_t ImageSize =
        Padding                      +
        Settings.SystemPartitionSize
    ;

    size_t ImageBlockCount = ComputeBlockCount(ImageSize);

    size_t SystemPartitionLBA = ComputeAlignedBlockCount(Settings.Alignment);
    size_t SystemPartitionBlocks = ComputeAlignedBlockCount(Settings.SystemPartitionSize);

    printf("Image name: %s\n", Settings.ImageName);
    printf("Image size: %zu MB (%zu blocks)\n", ImageSize / MB(1), ImageBlockCount);

    // NOTE(vak): Protective MBR
    {
        master_boot_record MBR =
        {
            .BootCode  = {0xEB, 0x00}, // NOTE(vak): JMP $
            .Signature = 0xAA55,

            .Partitions =
            {
                {
                    .StartSector = 0x02,
                    .OSIndicator = 0xEE, // NOTE(vak): Protective GPT
                    .EndHead     = 0xFF,
                    .EndSector   = 0xFF,
                    .EndTrack    = 0xFF,
                    .StartingLBA = 0x00000001,
                    .SizeInLBA   = Minimum(ImageBlockCount - 1, 0xFFFFFFFF),
                },
            },
        };

        WriteBlocks(OutFile, &MBR, sizeof(MBR));
    }

    // NOTE(vak): Partition table
    {
        size_t PrimaryLBA = 0x01;
        size_t AlternativeLBA = ImageBlockCount - 1;

        gpt_header PrimaryHeader =
        {
            .Signature                = 0x5452415020494645ULL,
            .Revision                 = 0x00010000,
            .HeaderSize               = sizeof(gpt_header),
            .HeaderCRC32              = 0, // NOTE(vak): Computed later
            .MyLBA                    = PrimaryLBA,
            .AlternateLBA             = AlternativeLBA,
            .FirstUsableLBA           = 1 + 1 + GPTTableBlockCount,
            .LastUsableLBA            = AlternativeLBA - GPTTableBlockCount - 1,
            .DiskGUID                 = NewGUID(),
            .PartitionEntryLBA        = PrimaryLBA + 1,
            .NumberOfPartitionEntries = 128,
            .SizeOfPartitionEntry     = sizeof(gpt_entry),
            .PartitionArrayCRC32      = 0, // NOTE(vak): Computed later
        };

        gpt_header AlternativeHeader =
        {
            .Signature                = 0x5452415020494645ULL,
            .Revision                 = 0x00010000,
            .HeaderSize               = sizeof(gpt_header),
            .HeaderCRC32              = 0, // NOTE(vak): Computed later
            .MyLBA                    = AlternativeLBA,
            .AlternateLBA             = PrimaryLBA,
            .FirstUsableLBA           = 1 + 1 + GPTTableBlockCount,
            .LastUsableLBA            = AlternativeLBA - GPTTableBlockCount - 1,
            .DiskGUID                 = NewGUID(),
            .PartitionEntryLBA        = AlternativeLBA - GPTTableBlockCount,
            .NumberOfPartitionEntries = 128,
            .SizeOfPartitionEntry     = sizeof(gpt_entry),
            .PartitionArrayCRC32      = 0, // NOTE(vak): Computed later
        };

        gpt_entry Entries[128] =
        {
            // NOTE(vak): EFI system partition
            {
                .PartitionTypeGUID   = EFI_SYSTEM_PARTITION_GUID,
                .UniquePartitionGUID = NewGUID(),
                .StartingLBA         = SystemPartitionLBA,
                .EndingLBA           = SystemPartitionLBA + SystemPartitionBlocks - 1,
                .Attributes          = 0x1,
                .PartitionName       = u"EFI SYSTEM",
            },
        };

        size_t EntriesCRC32 = ComputeCRC32(Entries, sizeof(Entries));

        PrimaryHeader.PartitionArrayCRC32 = EntriesCRC32;
        AlternativeHeader.PartitionArrayCRC32 = EntriesCRC32;

        PrimaryHeader.HeaderCRC32 = ComputeCRC32(&PrimaryHeader, sizeof(PrimaryHeader));
        AlternativeHeader.HeaderCRC32 = ComputeCRC32(&AlternativeHeader, sizeof(AlternativeHeader));

        WriteBlocks(OutFile, &PrimaryHeader, sizeof(PrimaryHeader));
        WriteBlocks(OutFile, Entries, sizeof(Entries));

        SeekFile(OutFile, ComputeBlocksSize(AlternativeHeader.MyLBA - GPTTableBlockCount));

        WriteBlocks(OutFile, Entries, sizeof(Entries));
        WriteBlocks(OutFile, &AlternativeHeader, sizeof(AlternativeHeader));
    }

    // NOTE(vak): EFI System Partition
    {
        fat32_vbr VBR =
        {
            .JumpInstruction     = {0xEB, 0x00, 0x00}, // TODO(vak): JMP $
            .OEMName             = "THISDISK",
            .BytesPerBlock       = Settings.BytesPerBlock,
            .BlocksPerCluster    = 1,
            .ReservedBlockCount  = 32, // NOTE(vak): FAT32
            .NumberOfFATs        = 2,
            .MediaDescriptor     = 0xF8, // NOTE(vak): Fixed non-removable media
            .HiddenBlocks        = SystemPartitionLBA, // NOTE(vak): Blocks before this partition
            .TotalBlocks32       = SystemPartitionBlocks,
            .BlocksPerFAT32      = 0, // NOTE(vak): Filled in later
            .RootCluster         = 2,
            .FilesystemInfo      = 1,
            .BackupBootSector    = 6,
            .DriveNumber         = 0x80, // NOTE(vak): 1st hard drive
            .BootSignature       = 0x29,
            .VolumeLabel         = "NO NAME    ",
            .FilesystemType      = "FAT32   ",
            .BootSectorSignature = 0xAA55,
        };

        uint32_t AvailableBlocks = VBR.TotalBlocks32 - VBR.ReservedBlockCount;
        uint32_t Temp = ((256 * VBR.BlocksPerCluster) + VBR.NumberOfFATs) / 2;

        VBR.BlocksPerFAT32 = (AvailableBlocks + Temp - 1) / Temp;

        fat32_filesystem_info FilesystemInfo =
        {
            .LeadSignature      = 0x41615252,
            .StructureSignature = 0x61417272,
            .FreeCount          = 0xFFFFFFFF,
            .NextFree           = 5,
            .TrailingSignature  = 0xAA550000,
        };

        size_t TableLBA = SystemPartitionLBA + VBR.ReservedBlockCount;
        size_t DataLBA  = TableLBA + (VBR.NumberOfFATs * VBR.BlocksPerFAT32);

        // NOTE(vak): Write VBR and FSInfo

        SeekFile(OutFile, ComputeBlocksSize(SystemPartitionLBA));

        WriteBlocks(OutFile, &VBR, sizeof(VBR));
        WriteBlocks(OutFile, &FilesystemInfo, sizeof(FilesystemInfo));

        // NOTE(vak): Write backup VBR and FSInfo

        SeekFile(OutFile, ComputeBlocksSize(SystemPartitionLBA + VBR.BackupBootSector));

        WriteBlocks(OutFile, &VBR, sizeof(VBR));
        WriteBlocks(OutFile, &FilesystemInfo, sizeof(FilesystemInfo));

        // NOTE(vak): Write FATs

        for (uint32_t TableIndex = 0; TableIndex < VBR.NumberOfFATs; TableIndex++)
        {
            SeekFile(
                OutFile,
                ComputeBlocksSize(TableLBA + (TableIndex * VBR.BlocksPerFAT32))
            );

            uint32_t Cluster = 0;

            // NOTE(vak): Cluster 0: FAT identifier, lower 8 bits are the media type
            Cluster = 0xFFFFFF00 | VBR.MediaDescriptor;
            WriteBytes(OutFile, &Cluster, sizeof(Cluster));

            // NOTE(vak): Cluster 1: End Of Chain (EOC) marker
            Cluster = 0xFFFFFFFF;
            WriteBytes(OutFile, &Cluster, sizeof(Cluster));

            // NOTE(vak): Cluster 2: Cluster for directory '/'
            Cluster = 0xFFFFFFFF;
            WriteBytes(OutFile, &Cluster, sizeof(Cluster));

            // NOTE(vak): Cluster 3: Cluster for directory '/EFI'
            Cluster = 0xFFFFFFFF;
            WriteBytes(OutFile, &Cluster, sizeof(Cluster));

            // NOTE(vak): Cluster 4: Cluster for directory '/EFI/BOOT'
            Cluster = 0xFFFFFFFF;
            WriteBytes(OutFile, &Cluster, sizeof(Cluster));
        }

        // NOTE(vak): Data region
        {
            // NOTE(vak): EFI directory

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + 0));

            fat32_directory_entry DirectoryEntry =
            {
                .Name            = "EFI        ",
                .Attributes      = ATTR_DIRECTORY,
                .FirstClusterLow = 3,
            };

            SetFAT32Time(&DirectoryEntry);

            WriteBlocks(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            // NOTE(vak): EFI directory entries
            // .
            // ..
            // BOOT

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + 1));

            memcpy(DirectoryEntry.Name, ".          ", 11);
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            memcpy(DirectoryEntry.Name, "..         ", 11);
            DirectoryEntry.FirstClusterLow = 0;
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            memcpy(DirectoryEntry.Name, "BOOT       ", 11);
            DirectoryEntry.FirstClusterLow = 4;
            WriteBlocks(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            // NOTE(vak): 'EFI/BOOT' directory entries

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + 2));

            memcpy(DirectoryEntry.Name, ".          ", 11);
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            memcpy(DirectoryEntry.Name, "..         ", 11);
            DirectoryEntry.FirstClusterLow = 3;
            WriteBlocks(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));
        }

        // NOTE(vak): Add BOOTX64.EFI to '/EFI/BOOT/'

        {
            size_t FileSize = GetFileSize(BOOTX64);
            void* FileData = calloc(1, FileSize);

            ReadBytes(BOOTX64, FileData, FileSize);

            size_t FileBlocks = ComputeBlockCount(FileSize);

            uint32_t NextFreeCluster = FilesystemInfo.NextFree;
            uint32_t StartingCluster = NextFreeCluster;

            for (uint8_t TableIndex = 0; TableIndex < VBR.NumberOfFATs; TableIndex++)
            {
                uint32_t Cluster = FilesystemInfo.NextFree;
                NextFreeCluster = Cluster;

                SeekFile(
                    OutFile,
                    ComputeBlocksSize(TableLBA + (TableIndex * VBR.BlocksPerFAT32)) +
                    NextFreeCluster * sizeof(uint32_t)
                );

                // NOTE(vak): Write file clusters

                for (uint64_t Block = 0; Block < FileBlocks; Block++)
                {
                    Cluster++;
                    NextFreeCluster++;

                    WriteBytes(OutFile, &Cluster, sizeof(Cluster));
                }

                // NOTE(vak): End Of Chain (EOC) marker for this file

                Cluster = 0xFFFFFFFF;
                NextFreeCluster++;
                WriteBytes(OutFile, &Cluster, sizeof(Cluster));
            }

            // NOTE(vak): Update FSInfo

            FilesystemInfo.NextFree  = NextFreeCluster;

            SeekFile(OutFile, ComputeBlocksSize(SystemPartitionLBA + 1));
            WriteBlocks(OutFile, &FilesystemInfo, sizeof(FilesystemInfo));

            // NOTE(vak): Add new directory entry in '/EFI/BOOT'

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + 2));

            fat32_directory_entry DirectoryEntry = {0};

            for (;;)
            {
                ReadBytes(OutFile, &DirectoryEntry, sizeof(fat32_directory_entry));

                if (DirectoryEntry.Name[0] == 0)
                    break;
            }

            SeekFileCurrent(OutFile, -sizeof(fat32_directory_entry));

            memcpy(DirectoryEntry.Name, "BOOTX64 EFI", 11);

            SetFAT32Time(&DirectoryEntry);

            DirectoryEntry.FirstClusterHigh = (StartingCluster >> 16) & 0xFFFF;
            DirectoryEntry.FirstClusterLow  = (StartingCluster & 0xFFFF);
            DirectoryEntry.FileSize         = FileSize;

            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            // NOTE(vak): Write file data

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + StartingCluster - 2));
            WriteBlocks(OutFile, FileData, FileSize);
        }
    }

    return (0);
}
