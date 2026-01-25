// hash_table.hpp

#pragma once

#include "object.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "types.hpp"

namespace triton
{
	struct sChunkElement
	{
		types::u32 chunk = 0;
		types::u32 position = 0;
	};

	struct sChunkAllocatorDescriptor
	{
		types::usize chunkByteSize = 16 * 1024;
		types::usize maxChunkCount = 256;
		types::usize hashTableByteSize = 4096;
	};

	template <typename T>
	class cHashTable : public iObject
	{
	public:
		explicit cHashTable(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc);
		virtual ~cHashTable() override final;

		template<typename... Args>
		T* Insert(Args&&... args);
		T* Find(const cTag& key);
		void Erase(const cTag& key);

		inline const T* GetElement(types::u32 index) const;
		inline types::usize GetElementCount() const { return _elementCount; }

	private:
		types::u32 AllocateChunk();
		void DeallocateChunk(types::u32 chunkIndex);
		types::u32 GetChunkIndex(types::u32 globalPosition);
		types::u32 GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition);
		types::cpuword MakeHashMask(types::usize size);

	private:
		sChunkAllocatorDescriptor _allocatorDesc = {};
		types::usize _chunkCount = 0;
		types::usize _objectByteSize = 0;
		types::usize _objectCountPerChunk = 0;
		types::usize _elementCount = 0;
		T** _chunks = nullptr;
		types::usize _hashTableSize = 0;
		types::cpuword _hashMask = 0;
		sChunkElement* _hashTable = nullptr;
	};

	template <typename T>
	cHashTable<T>::cHashTable(cContext* context, const sChunkAllocatorDescriptor& allocatorDesc) : iObject(context)
	{
		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		_allocatorDesc = allocatorDesc;
		_objectByteSize = sizeof(T);
		_objectCountPerChunk = chunkByteSize / _objectByteSize;
		_chunks = (T**)memoryAllocator->Allocate(_maxChunkCount * sizeof(T*), caps->memoryAlignment);
		_hashTableSize = hashTableSize;
		_hashMask = MakeHashMask(hashTableSize);
		_hashTable = (sChunkElement*)memoryAllocator->Allocate(_hashTableSize * sizeof(sChunkElement), caps->memoryAlignment);

		AllocateNewChunk();
	}

	template <typename T>
	cHashTable<T>::~cHashTable()
	{
		for (types::usize i = 0; i < _chunkCount; i++)
			DeallocateChunk(i);

		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		memoryAllocator->Deallocate(_hashTable);
		memoryAllocator->Deallocate(_chunks);
	}

	template <typename T>
	template <typename... Args>
	T* cHashTable<T>::Insert(Args&&... args)
	{
		types::u32 elementChunkIndex = GetChunkIndex(_elementCount);
		types::u32 elementLocalPosition = GetChunkLocalPosition(elementChunkIndex, _elementCount);

		const types::usize chunkObjectCount = GetChunkLocalPosition(_chunkCount - 1, _elementCount);
		if (chunkObjectCount >= _objectCountPerChunk)
		{
			elementChunkIndex = AllocateChunk();
			elementLocalPosition = 0;
		}

		_elementCount += 1;

		cHandle::index idx = (cHandle::index)elementLocalPosition;
		T* object = _context->Create<T>(_chunks[elementChunkIndex], idx, std::forward<Args>(args)...);

		sChunkElement ce;
		ce.chunk = elementChunkIndex;
		ce.position = elementLocalPosition;

		const uint32_t hash = Hash(object->GetID(), _hashMask);
		_hashTable[hash] = ce;

		return object;
	}

	template <typename T>
	T* cHashTable<T>::Find(const cTag& key)
	{
		const types::cpuword hash = Hash(key.GetData(), key.GetByteSize());
		const sChunkElement& ce = _hashTable[hash];
		const types::usize chunkObjectCount = GetChunkLocalPosition(_chunkCount - 1, _elementCount);
		if (ce.chunk < _chunkCount && ce.position < chunkObjectCount)
		{
			const T* object = &_chunks[ce.chunk][ce.position];
			if (key.Compare(object->GetID()))
				return (const T*)object;
		}

		for (types::usize i = 0; i < _chunkCount; i++)
		{
			for (types::usize j = 0; j < _objectCountPerChunk; j++)
			{
				const T* object = &_chunks[i][j];
				if (key.Compare(object->GetID()))
					return (const T*)object;
			}
		}

		return nullptr;
	}

	template <typename T>
	void cHashTable<T>::Erase(const cTag& key)
	{
		for (types::usize i = 0; i < _chunkCount; i++)
		{
			for (types::usize j = 0; j < _objectCountPerChunk; j++)
			{
				if (key.Compare(_chunks[i][j]->GetID()))
				{
					const types::u32 lastChunkIndex = GetChunkIndex(_elementCount - 1);
					const types::usize lastChunkObjectCount = GetChunkLocalPosition(lastChunkIndex, _elementCount);

					const types::u32 lastIndex = lastChunkObjectCount - 1;
					_chunks[i][j] = _chunks[lastChunkIndex][lastIndex];
					_elementCount -= 1;

					if (lastChunkObjectCount == 1)
						DeallocateChunk(lastChunkIndex);

					return;
				}
			}
		}
	}

	template <typename T>
	const T* cHashTable<T>::GetElement(types::u32 index) const
	{
		const types::u32 chunkIndex = GetChunkIndex(index);

		if (chunkIndex >= _chunkCount)
			return nullptr;

		const types::usize chunkObjectCount = GetChunkLocalPosition(info.chunk, _elementCount);
		const types::u32 localPosition = GetChunkLocalPosition(chunkIndex, index);
		if (localPosition >= chunkObjectCount)
			return nullptr;

		return _chunks[chunkIndex][localPosition];
	}

	template <typename T>
	types::u32 cHashTable<T>::AllocateChunk()
	{
		if (_chunkCount >= _maxChunkCount)
			return 0;

		const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		_chunks[_chunkCount] = (T*)memoryAllocator->Allocate(_chunkByteSize, caps->memoryAlignment);

		return _chunkCount++;
	}

	template <typename T>
	void cHashTable<T>::DeallocateChunk(types::u32 chunkIndex)
	{
		if (chunkIndex >= _chunkCount)
			return;

		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
		memoryAllocator->Deallocate(_chunks[chunkIndex]);
		_chunkCount -= 1;

		return _chunkCount++;
	}

	template <typename T>
	types::u32 cHashTable<T>::GetChunkIndex(types::u32 globalPosition)
	{
		return globalPosition / _objectCountPerChunk;
	}

	template <typename T>
	types::u32 cHashTable<T>::GetChunkLocalPosition(types::u32 chunkIndex, types::u32 globalPosition)
	{
		const types::u32 chunkIndexBoundary = chunkIndex * _objectCountPerChunk;
		const types::u32 localPosition = globalPosition - chunkIndexBoundary;

		return localPosition;
	}

	template <typename T>
	types::cpuword cHashTable<T>::MakeHashMask(types::usize size)
	{
		unsigned int count = __lzcnt((unsigned int)size);
		types::cpuword mask = (types::cpuword)((1 << (31 - count)) - 1);

		return mask;
	}
}