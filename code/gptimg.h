
#pragma once

typedef struct packed
{
    u8  BootIndicator;
    u8  StartHead;
    u8  StartSector;
    u8  StartTrack;
    u8  OSIndicator;
    u8  EndHead;
    u8  EndSector;
    u8  EndTrack;
    u32 StartingLBA;
    u32 SizeInLBA;
} mbr_partition_record;

CTAssert(sizeof(mbr_partition_record) == 16);

typedef struct packed
{
    u8                   BootCode[440];
    u32                  UniqueMBRSignature;
    u16                  Unknown;
    mbr_partition_record Partitions[4];
    u16                  Signature;
} master_boot_record;

CTAssert(sizeof(master_boot_record) == 512);

typedef union
{
    u64 Parts64[2];
    u32 Parts32[4];
    u16 Parts16[8];
    u8  Parts8[16];
} guid128;

typedef struct packed
{
    u64     Signature;
    u32     Revision;
    u32     HeaderSize;
    u32     HeaderCRC32;
    u32     Reserved0;
    u64     MyLBA;
    u64     AlternateLBA;
    u64     FirstUsableLBA;
    u64     LastUsableLBA;
    guid128 DiskGUID;
    u64     PartitionEntryLBA;
    u32     NumberOfPartitionEntries;
    u32     SizeOfPartitionEntry;
    u32     PartitionArrayCRC32;
    u8      Reserved1[420];
} gpt_header;

CTAssert(sizeof(gpt_header) == 512);

typedef struct packed
{
    guid128 PartitionTypeGUID;
    guid128 UniquePartitionGUID;
    u64     StartingLBA;
    u64     EndingLBA;
    u64     Attributes;
    u16     PartitionName[36];
} gpt_entry;

CTAssert(sizeof(gpt_entry) == 128);

typedef struct packed
{
    u8  JumpInstruction[3];
    u8  OEMName[8] nonstring;
    u16 BytesPerBlock;
    u8  BlocksPerCluster;
    u16 ReservedBlockCount;
    u8  NumberOfFATs;
    u16 RootDirectoryEntries;
    u16 TotalBlocks16;
    u8  MediaDescriptor;
    u16 BlocksPerFAT16;
    u16 BlocksPerTrack;
    u16 HeadCount;
    u32 HiddenBlocks;
    u32 TotalBlocks32;
    u32 BlocksPerFAT32;
    u16 ExtendedFlags;
    u16 FilesystemVersion;
    u32 RootCluster;
    u16 FilesystemInfo;
    u16 BackupBootSector;
    u8  Reserved0[12];
    u8  DriveNumber;
    u8  Reserved1;
    u8  BootSignature;
    u8  VolumeID[4];
    u8  VolumeLabel[11] nonstring;
    u8  FilesystemType[8] nonstring;

    u8  BootCode[420];
    u16 BootSectorSignature;
} fat32_vbr;

CTAssert(sizeof(fat32_vbr) == 512);

typedef struct packed
{
    u32 LeadSignature;
    u8  Reserved0[480];
    u32 StructureSignature;
    u32 FreeCount;
    u32 NextFree;
    u8  Reserved1[12];
    u32 TrailingSignature;
} fat32_filesystem_info;

CTAssert(sizeof(fat32_filesystem_info) == 512);

#define ATTR_READ_ONLY (0x01)
#define ATTR_HIDDEN    (0x02)
#define ATTR_SYSTEM    (0x04)
#define ATTR_VOLUME_ID (0x08)
#define ATTR_DIRECTORY (0x10)
#define ATTR_ARCHIVE   (0x20)
#define ATTR_LONG_NAME (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

typedef struct packed
{
    u8  Name[11] nonstring;
    u8  Attributes;
    u8  NTRes;
    u8  CreateTimeLength;
    u16 CreateTime;
    u16 CreateDate;
    u16 LastAccessDate;
    u16 FirstClusterHigh;
    u16 WriteTime;
    u16 WriteDate;
    u16 FirstClusterLow;
    u32 FileSize;
} fat32_directory_entry;

CTAssert(sizeof(fat32_directory_entry) == 32);

typedef struct
{
    usize BytesPerBlock;       // NOTE(vak): Per UEFI specifications, restricted to be 512/1024/2048/4096
    usize SystemPartitionSize; // NOTE(vak): Contains boot code and data. Measured in bytes
    usize Alignment;           // NOTE(vak): Partition alignment. Must be MB(1) per UEFI spec
    char* ImageName;           // NOTE(vak): Name of the output file.
} settings;
