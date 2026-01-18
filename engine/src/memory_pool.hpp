// memory_pool.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
	class cContext;
	
    struct sAllocatorBin
    {
        types::usize _blockSize = 0;
        types::usize _maxBlockCount = 0;
        void* _blocks = nullptr;
    };

    class cMemoryAllocator
    {
    public:
        static constexpr types::usize MAX_BIN_COUNT = 64 + 1;
        static constexpr types::usize MAX_ALLOCATION_BYTE_SIZE = 32 * 1024;

    public:
        explicit cMemoryAllocator() = default;
        virtual ~cMemoryAllocator();

        void* Allocate(types::usize byteSize, types::usize alignment);
        void Deallocate(void* ptr);

        void SetBins(types::usize maxBinByteSize);

    private:
        sAllocatorBin* _bins = nullptr;
        sAllocatorBin** _memSizeToBin = nullptr;
    };
}