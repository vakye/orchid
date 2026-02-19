
local void* ReservePage(memory_map* MemoryMap, memory_region_kind Kind)
{
    void* Result = 0;
    b32 Found = false;

    for (usize Index = 0; Index < MemoryMap->RegionCount; Index++)
    {
        memory_region* Region = MemoryMap->Regions + Index;

        if (Region->Kind != Kind)
            continue;

        if (Region->PageCount)
        {
            Result = (void*)Region->BaseAddress;

            Region->BaseAddress += ArchGetPageSize();
            Region->PageCount--;

            Found = true;
            break;
        }
    }

    if (!Found)
    {
        SerialErrorf(Str("Unable to reserve a page."));
    }

    return (Result);
}
