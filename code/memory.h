
typedef usize memory_region_kind;
enum
{
    MemoryRegionKind_Unknown = 0,

    MemoryRegionKind_BootCode,
    MemoryRegionKind_BootData,
    MemoryRegionKind_Usable,

    MemoryRegionKind_COUNT,
};

typedef struct
{
    memory_region_kind Kind;
    usize              BaseAddress;
    usize              PageCount;
} memory_region;

typedef struct
{
    usize          RegionCount;
    memory_region* Regions;
} memory_map;

local void* ReservePage(memory_map* MemoryMap, memory_region_kind Kind);
