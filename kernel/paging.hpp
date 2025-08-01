#pragma once

#include <cstddef>

const size_t kPageDirectoryCount = 64;

void SetupIdentityPageTable();

void InitializePaging();

union LinearAddress4Level
{
	uint64_t value;

	struct
	{
		uint64_t offset : 12;
		uint64_t page : 9;
		uint64_t dir : 9;
		uint64_t pdp : 9;
		uint64_t pml4 : 9;
		uint64_t : 16;
	} __attribute__((packed)) parts;

	int Part(int page_map_level) const
	{
		switch (page_map_level)
		{
		case 0:
			return parts.offset;
		case 1:
			return parts.page;
		case 2:
			return parts.dir;
		case 3:
			return parts.pdp;
		case 4:
			return parts.pml4;
		default:
			return 0;
		}
	}

	void SetPart(int page_map_level, int value)
	{
		switch (page_map_level)
		{
		case 0:
			parts.offset = value;
			break;
		case 1:
			parts.page = value;
			break;
		case 2:
			parts.dir = value;
			break;
		case 3:
			parts.pdp = value;
			break;
		case 4:
			parts.pml4 = value;
			break;
		}
	}
};

union PageMapEntry
{
	uint64_t data;

	struct
	{
		uint64_t present : 1;
		uint64_t writable : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t chache_disable : 1;
		uint64_t accessed : 1;
		uint64_t dirty : 1;
		uint64_t huge_page : 1;
		uint64_t global : 1;
		uint64_t : 3;

		uint64_t addr : 40;
		uint64_t : 12;
	} __attribute__((packed)) bits;

	PageMapEntry *Pointer() const
	{
		return reinterpret_cast<PageMapEntry *>(bits.addr << 12);
	}

	void SetPointer(PageMapEntry *p)
	{
		bits.addr = reinterpret_cast<uint64_t>(p) >> 12;
	}
};
