// map.hpp

#pragma once

#include "types.hpp"

namespace triton
{
	struct sChunkElement
	{
		types::u32 chunk = 0;
		types::u32 position = 0;
	};

	template <typename T>
	class cMap
	{
	public:
		explicit cMap(types::usize chunkByteSize, types::usize maxChunkCount, types::usize hashTableByteSize);
		~cMap();

		template<typename... Args>
		T* Add(Args&&... args);
		T* Find(const std::string& id);
		void Delete(const std::string& id);

	private:
		int AllocateChunk();
		void DeallocateChunk(types::u32 chunkIndex);
		int GetChunkIndex(types::u32 globalPosition);
		int GetLocalPosition(types::u32 chunkIndex, types::u32 globalPosition);
		types::cpuword MakeHashMask(types::usize size);

	private:
		types::usize _chunkByteSize = 0;
		types::usize _maxChunkCount = 0;
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
	cMap<T>::cMap(types::usize chunkByteSize, types::usize maxChunkCount, types::usize hashTableSize)
	{
		_chunkByteSize = chunkByteSize;
		_maxChunkCount = maxChunkCount;
		_objectByteSize = sizeof(T);
		_objectCountPerChunk = chunkByteSize / _objectByteSize;
		_chunks = (T**)std::malloc(_maxChunkCount * sizeof(T*));
		_hashTableSize = hashTableSize;
		_hashMask = MakeHashMask(hashTableSize);
		_hashTable = (sChunkElement*)std::malloc(_hashTableSize * sizeof(sChunkElement));

		AllocateNewChunk();
	}

	template <typename T>
	cMap<T>::~cMap()
	{
		for (types::usize i = 0; i < _chunkCount; i++)
			DeallocateChunk(i);
		std::free(_hashTable);
		std::free(_chunks);
	}

	template <typename T>
	template <typename... Args>
	T* cMap<T>::Add(Args&&... args)
	{
		types::u32 elementChunkIndex = GetChunkIndex(_elementCount);
		types::u32 elementLocalPosition = GetLocalPosition(elementChunkIndex, _elementCount);

		const types::usize chunkObjectCount = GetLocalPos(_chunkCount - 1, _elementCount);
		if (chunkObjectCount >= _objectCountPerChunk)
		{
			elementChunkIndex = AllocateNewChunk();
			elementLocalPosition = 0;
		}

		_elementCount += 1;

		new (&_chunks[elementChunkIndex][elementLocalPosition]) T(std::forward<Args>(args)...);
		T* object = &_chunks[elementChunkIndex][elementLocalPosition];

		sChunkElement ce;
		ce.chunk = elementChunkIndex;
		ce.position = elementLocalPosition;

		const uint32_t hash = Hash(object->GetIdentifier()->GetID(), _hashMask);
		_hashTable[hash] = ce;

		return object;
	}

	template <typename T>
	T* cMap<T>::Find(const std::string& id)
	{
		const types::cpuword hash = Hash(id, _hashMask);
		const sChunkElement& ce = _hashTable[hash];
		const types::usize chunkObjectCount = GetLocalPosition(_chunkCount - 1, _elementCount);
		if (ce.chunk < _chunkCount && ce.position < chunkObjectCount)
		{
			const T* object = &_chunks[ce.chunk][ce.position];
			if (object->GetIdentifier()->GetID() == id)
				return (const T*)object;
		}

		for (types::usize i = 0; i < _chunkCount; i++)
		{
			for (types::usize j = 0; j < _objectCountPerChunk; j++)
			{
				const T* object = &_chunks[i][j];
				if (object->GetIdentifier()->GetID() == id)
					return (const T*)object;
			}
		}

		return nullptr;
	}

	template <typename T>
	void cMap<T>::Delete(const std::string& id)
	{
		for (types::usize i = 0; i < _chunkCount; i++)
		{
			for (types::usize j = 0; j < _objectCountPerChunk; j++)
			{
				if (_chunks[i][j].id == id)
				{
					const types::u32 lastChunkIndex = GetChunkIndex(_elementCount - 1);
					const types::usize lastChunkObjectCount = GetLocalPosition(lastChunkIndex, _elementCount);

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
	types::u32 cMap<T>::AllocateChunk()
	{
		if (_chunkCount >= _maxChunkCount)
			return 0;

		_chunks[_chunkCount] = (T*)std::malloc(_chunkByteSize);

		return _chunkCount++;
	}

	template <typename T>
	void cMap<T>::DeallocateChunk(types::u32 chunkIndex)
	{
		if (chunkIndex >= _chunkCount)
			return;

		std::free(_chunks[chunkIndex]);
		_chunkCount -= 1;

		return _chunkCount++;
	}

	template <typename T>
	types::u32 cMap<T>::GetChunkIndex(types::u32 globalPosition)
	{
		return globalPosition / _objectCountPerChunk;
	}

	template <typename T>
	types::u32 cMap<T>::GetLocalPosition(types::u32 chunkIndex, types::u32 globalPosition)
	{
		const types::u32 chunkIndexBoundary = chunkIndex * _objectCountPerChunk;
		const types::u32 localPosition = globalPosition - chunkIndexBoundary;

		return localPosition;
	}

	template <typename T>
	types::cpuword cMap<T>::MakeHashMask(types::usize size)
	{
		unsigned int count = __lzcnt((unsigned int)size);
		types::cpuword mask = (types::cpuword)((1 << (31 - count)) - 1);

		return mask;
	}
}