
// NOTE(vak): This program searches for "BOOTX64.EFI" in the caller's
// directory, then proceeds to create a UEFI GPT image with "BOOTX64.EFI"
// placed in the "/EFI/BOOT" directory. The resulting image is then
// outputted to "orchid.img".

#include "shared.h"
#include "shared.c"

#include "gptimg.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

settings Settings =
{
    .BytesPerBlock       = 512,    // NOTE(vak): Not sure if values other than 512 even works :)
    .SystemPartitionSize = MB(33), // NOTE(vak): FAT32 so must be larger than 32MB
    .Alignment           = MB(1),  // NOTE(vak): UEFI GPT partitions are aligned to a 2048-block boundary.
    .ImageName           = "orchid.img",
};

#define EFI_SYSTEM_PARTITION_GUID \
    (guid128){.Parts64 = {0xC12A7328F81F11D2, 0xBA4B00A0C93EC93B}}

local guid128 NewGUID(void)
{
    guid128 Result = {0};

    // TODO(vak): Check the UEFI specification for the GUID
    // generation algorithm

    srand(time(0));

    for (usize Index = 0; Index < 16; Index++)
    {
        Result.Parts8[Index] = rand() & 0xFF;
    }

    return (Result);
}

local usize ComputeBlocksSize(usize BlockCount)
{
    usize Result = BlockCount * Settings.BytesPerBlock;
    return (Result);
}

local usize ComputeBlockCount(usize SizeInBytes)
{
    usize Aligned = Align(SizeInBytes, Settings.BytesPerBlock);
    usize Result = Aligned / Settings.BytesPerBlock;

    return (Result);
}

local usize ComputeAlignedBlockCount(usize SizeInBytes)
{
    usize Aligned = Align(SizeInBytes, Settings.Alignment);
    usize Result = Aligned / Settings.BytesPerBlock;

    return (Result);
}

local void WritePadding(FILE* File, usize Size)
{
    u8 Zero = 0;

    for (usize Index = 0; Index < Size; Index++)
        fwrite(&Zero, 1, sizeof(Zero), File);
}

local void SeekFile(FILE* File, ssize Offset)
{
    fseek(File, Offset, SEEK_SET);
}

local void SeekFileCurrent(FILE* File, ssize Offset)
{
    fseek(File, Offset, SEEK_CUR);
}

local usize GetFileSize(FILE* File)
{
    usize OldOffset = ftell(File);

    fseek(File, 0, SEEK_END);

    usize Result = ftell(File);

    fseek(File, OldOffset, SEEK_SET);

    return (Result);
}

local usize ReadBytes(FILE* File, void* Buffer, usize Size)
{
    usize Result = fread(Buffer, 1, Size, File);
    return (Result);
}

local usize WriteBytes(FILE* File, void* Buffer, usize Size)
{
    usize Result = Size;

    fwrite(Buffer, 1, Size, File);

    return (Result);
}

local usize WriteBlocks(FILE* File, void* Buffer, usize Size)
{
    usize Aligned = Align(Size, Settings.BytesPerBlock);
    usize Padding = Aligned - Size;

    WriteBytes(File, Buffer, Size);
    WritePadding(File, Padding);

    return (Aligned);
}

local u32 ComputeCRC32(void* DataInit, usize Size)
{
    persist u32 Table[256] =
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

    u8* Bytes = (u8*)DataInit;
    u32 Result = 0xFFFFFFFF;

    for (usize Index = 0; Index < Size; Index++)
    {
        u8 Byte = Bytes[Index];
        u8 TableIndex = (Result ^ Byte) & 0xFF;

        Result = Table[TableIndex] ^ (Result >> 8);
    }

    Result ^= 0xFFFFFFFF;

    return (Result);
}

local void SetFAT32Time(fat32_directory_entry* Entry)
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

s32 main(s32 ArgCount, char* Args[])
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
        printf("error: EFI system partition size is not larger than 32MB (%llu MB)\n", Settings.SystemPartitionSize / MB(1));
    }

    printf("Settings:\n");
    printf("    + Bytes per block:       %llu\n",    Settings.BytesPerBlock);
    printf("    + System partition size: %llu MB\n", Settings.SystemPartitionSize / MB(1));

    // NOTE(vak): Add extra padding for:
    //     2 aligned partitions
    //     2 GPT tables (primary and alternative)
    //     2 GPT headers (primary and alternative)
    //     MBR

    usize GPTTableSize = KB(16); // NOTE(vak): Per UEFI spec, GPT entry array must be a minimum of KB(16)
    usize GPTTableBlockCount = ComputeBlockCount(GPTTableSize);

    usize Padding =
        (Settings.Alignment * 2)     + // NOTE(vak): 2 aligned partitions
        (GPTTableSize * 2)           + // NOTE(vak): 2 GPT tables
        (Settings.BytesPerBlock * 2) + // NOTE(vak): 2 GPT headers
        (Settings.BytesPerBlock)       // NOTE(vak): MBR
    ;

    usize ImageSize =
        Padding                      +
        Settings.SystemPartitionSize
    ;

    usize ImageBlockCount = ComputeBlockCount(ImageSize);

    usize SystemPartitionLBA = ComputeAlignedBlockCount(Settings.Alignment);
    usize SystemPartitionBlocks = ComputeAlignedBlockCount(Settings.SystemPartitionSize);

    printf("Image name: %s\n", Settings.ImageName);
    printf("Image size: %llu MB (%llu blocks)\n", ImageSize / MB(1), ImageBlockCount);

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
        usize PrimaryLBA = 0x01;
        usize AlternativeLBA = ImageBlockCount - 1;

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

        u32 AvailableBlocks = VBR.TotalBlocks32 - VBR.ReservedBlockCount;
        u32 Temp = ((256 * VBR.BlocksPerCluster) + VBR.NumberOfFATs) / 2;

        VBR.BlocksPerFAT32 = (AvailableBlocks + Temp - 1) / Temp;

        fat32_filesystem_info FilesystemInfo =
        {
            .LeadSignature      = 0x41615252,
            .StructureSignature = 0x61417272,
            .FreeCount          = 0xFFFFFFFF,
            .NextFree           = 5,
            .TrailingSignature  = 0xAA550000,
        };

        usize TableLBA = SystemPartitionLBA + VBR.ReservedBlockCount;
        usize DataLBA  = TableLBA + (VBR.NumberOfFATs * VBR.BlocksPerFAT32);

        // NOTE(vak): Write VBR and FSInfo

        SeekFile(OutFile, ComputeBlocksSize(SystemPartitionLBA));

        WriteBlocks(OutFile, &VBR, sizeof(VBR));
        WriteBlocks(OutFile, &FilesystemInfo, sizeof(FilesystemInfo));

        // NOTE(vak): Write backup VBR and FSInfo

        SeekFile(OutFile, ComputeBlocksSize(SystemPartitionLBA + VBR.BackupBootSector));

        WriteBlocks(OutFile, &VBR, sizeof(VBR));
        WriteBlocks(OutFile, &FilesystemInfo, sizeof(FilesystemInfo));

        // NOTE(vak): Write FATs

        for (u32 TableIndex = 0; TableIndex < VBR.NumberOfFATs; TableIndex++)
        {
            SeekFile(
                OutFile,
                ComputeBlocksSize(TableLBA + (TableIndex * VBR.BlocksPerFAT32))
            );

            u32 Cluster = 0;

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

            CopyMemory(DirectoryEntry.Name, ".          ", 11);
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            CopyMemory(DirectoryEntry.Name, "..         ", 11);
            DirectoryEntry.FirstClusterLow = 0;
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            CopyMemory(DirectoryEntry.Name, "BOOT       ", 11);
            DirectoryEntry.FirstClusterLow = 4;
            WriteBlocks(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            // NOTE(vak): 'EFI/BOOT' directory entries

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + 2));

            CopyMemory(DirectoryEntry.Name, ".          ", 11);
            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            CopyMemory(DirectoryEntry.Name, "..         ", 11);
            DirectoryEntry.FirstClusterLow = 3;
            WriteBlocks(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));
        }

        // NOTE(vak): Add BOOTX64.EFI to '/EFI/BOOT/'

        {
            usize FileSize = GetFileSize(BOOTX64);
            void* FileData = calloc(1, FileSize);

            ReadBytes(BOOTX64, FileData, FileSize);

            usize FileBlocks = ComputeBlockCount(FileSize);

            u32 NextFreeCluster = FilesystemInfo.NextFree;
            u32 StartingCluster = NextFreeCluster;

            for (u8 TableIndex = 0; TableIndex < VBR.NumberOfFATs; TableIndex++)
            {
                u32 Cluster = FilesystemInfo.NextFree;
                NextFreeCluster = Cluster;

                SeekFile(
                    OutFile,
                    ComputeBlocksSize(TableLBA + (TableIndex * VBR.BlocksPerFAT32)) +
                    NextFreeCluster * sizeof(u32)
                );

                // NOTE(vak): Write file clusters

                for (u64 Block = 0; Block < FileBlocks; Block++)
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

            CopyMemory(DirectoryEntry.Name, "BOOTX64 EFI", 11);

            SetFAT32Time(&DirectoryEntry);

            DirectoryEntry.FirstClusterHigh = (StartingCluster >> 16) & 0xFFFF;
            DirectoryEntry.FirstClusterLow  = (StartingCluster & 0xFFFF);
            DirectoryEntry.FileSize         = FileSize;

            WriteBytes(OutFile, &DirectoryEntry, sizeof(DirectoryEntry));

            // NOTE(vak): Write file data

            SeekFile(OutFile, ComputeBlocksSize(DataLBA + StartingCluster - 2));
            WriteBlocks(OutFile, FileData, FileSize);

            printf("Added 'BOOTX64.EFI' to '/EFI/BOOT' (%llu bytes)\n", FileSize);
        }
    }

    return (0);
}
