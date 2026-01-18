// memory_pool.cpp

#include <iostream>
#include <cstdlib>
#include "memory_pool.hpp"
#include "application.hpp"
#include "render_context.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
{
	cMemoryAllocator::~cMemoryAllocator()
	{
		if (_bins)
		{
			for (usize i = 0; i < MAX_BIN_COUNT; i++)
				std::free(_bins[i]._blocks);
		}

		if (_memSizeToBin)
			std::free(_memSizeToBin);
	}

	void* cMemoryAllocator::Allocate(types::usize byteSize, types::usize alignment)
	{
		if (byteSize < MAX_ALLOCATION_BYTE_SIZE)
		{
			sAllocatorBin* bin = _memSizeToBin[byteSize];

			for (usize i = 0; i < bin->_maxBlockCount; i++)
			{
				cObject* obj = (cObject*)((u8*)bin->_blocks + bin->_blockSize * i);

				if (obj->_occupied == K_FALSE)
				{
					obj->_occupied = K_TRUE;

					return (void*)obj;
				}
			}

			return std::malloc(byteSize);
		}
		else
		{
			return std::malloc(byteSize);
		}
	}

	void cMemoryAllocator::Deallocate(void* ptr)
	{
		if (ptr == nullptr)
			return;

		cObject* obj = (cObject*)ptr;
		obj->_occupied = K_FALSE;
	}

	void cMemoryAllocator::SetBins(usize maxBinByteSize)
	{
		if (_bins || _memSizeToBin)
			return;

		_bins = (sAllocatorBin*)std::malloc(MAX_BIN_COUNT * sizeof(sAllocatorBin));
		_memSizeToBin = (sAllocatorBin**)std::malloc(MAX_ALLOCATION_BYTE_SIZE * sizeof(sAllocatorBin*));

		static const usize blockSizes[MAX_BIN_COUNT] =
		{
			0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096, 4608, 5120,
			5632, 6144, 6656, 7168, 7680, 8192, 8704, 9216, 9728, 10240,
			10752, 11264, 11776, 12288, 12800, 13312, 13824, 14336, 14848,
			15360, 15872, 16384, 16896, 17408, 17920, 18432, 18944, 19456,
			19968, 20480, 20992, 21504, 22016, 22528, 23040, 23552, 24064,
			24576, 25088, 25600, 26112, 26624, 27136, 27648, 28160, 28672,
			29184, 29696, 30208, 30720, 31232, 31744, 32256, 32768
		};

		for (usize i = 0; i < MAX_BIN_COUNT; i++)
		{
			_bins[i]._blockSize = blockSizes[i];
			_bins[i]._maxBlockCount = maxBinByteSize / _bins[i]._blockSize;
			_bins[i]._blocks = std::malloc(maxBinByteSize);
		}

		for (usize i = 0; i < MAX_ALLOCATION_BYTE_SIZE; i++)
		{
			usize index = 0;
			while (_bins[index]._blockSize < i)
				++index;

			_memSizeToBin[i] = &_bins[index];
		}
	}
}