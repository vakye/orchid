
#pragma once

typedef struct arch_page_map arch_page_map;

local void ArchSetup(void);

local void ArchWriteSerial(void* Buffer, usize Size);

local usize ArchGetPageSize(void);

local arch_page_map* ArchNewPageMap(memory_map* MemoryMap);

local void ArchMapPage(
    memory_map*    MemoryMap,
    arch_page_map* PageMap,
    usize          PhysicalAddress,
    usize          VirtualAddress
);

local void ArchUsePageMap(arch_page_map* PageMap);
