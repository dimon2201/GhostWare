// id_vec.hpp

#pragma once

#include <vector>
#include "object.hpp"
#include "log.hpp"
#include "types.hpp"

namespace harpy
{
	class cContext;

	template <typename T>
	class cIdVector : public iObject
	{
		HARPY_OBJECT(cIdVector)

	public:
		explicit cIdVector(cContext* context, types::usize maxElementCount);
		~cIdVector();

		template<typename... Args>
		T* Add(const std::string& id, Args&&... args);
		T* Find(const std::string& id);
		void Delete(const std::string& id);

		inline T* GetElements() { return _elements; }
		inline types::usize GetElementCount() { return _indexCount; }
		inline types::usize GetMaxElementCount() { return _maxElementCount; }

	private:
		static constexpr types::s32 K_INVALID_INDEX = -1;

	private:
		types::usize _maxElementCount = 0;
		types::usize _indexCount = 0;
		types::s32* _indices = nullptr;
		T* _elements = nullptr;
	};

	template <typename T>
	cIdVector<T>::cIdVector(cContext* context, types::usize maxElementCount) : iObject(context)
	{
		_maxElementCount = maxElementCount;
		_indexCount = 0;
		_indices = (types::s32*)std::malloc(sizeof(types::s32) * _maxElementCount);
		_elements = (T*)std::malloc(sizeof(T) * _maxElementCount);
		
		for (types::usize i = 0; i < _maxElementCount; i++)
			_indices[i] = K_INVALID_INDEX;
	}

	template <typename T>
	cIdVector<T>::~cIdVector()
	{
		if (_indices)
			std::free(_indices);
		for (types::usize i = 0; i < _indexCount; i++)
		{
			const types::s32 index = _indices[i];
			((T*)&_elements[index])->~T();
		}
		if (_elements)
			std::free(_elements);
	}

	template <typename T>
	template <typename... Args>
	T* cIdVector<T>::Add(const std::string& id, Args&&... args)
	{
		if (_indexCount >= _maxElementCount)
			return nullptr;

		const types::s32 index = _indices[_indexCount];
		if (index == K_INVALID_INDEX)
		{
			new (&_elements[_indexCount]) T(std::forward<Args>(args)...);
			((iObject*)&_elements[_indexCount])->_identifier = cIdentifier::GenerateIdentifier(T::GetTypeStatic());

			_indices[_indexCount] = _indexCount;

			return (T*)&_elements[_indexCount++];
		}
		else
		{
			new (&_elements[index]) T(std::forward<Args>(args)...);
			_indexCount++;

			return (T*)&_elements[index];
		}
	}

	template <typename T>
	T* cIdVector<T>::Find(const std::string& id)
	{
		for (types::usize i = 0; i < _indexCount; i++)
		{
			const types::s32 index = _indices[i];
			if (_elements[index].GetIdentifier().GetID() == id)
				return (T*)&_elements[index];
		}

		return nullptr;
	}

	template <typename T>
	void cIdVector<T>::Delete(const std::string& id)
	{
		for (types::usize i = 0; i < _indexCount; i++)
		{
			const types::s32 index = _indices[i];
			if (_elements[index].GetID() == id)
			{
				((T*)&_elements[index])->~T();

				const types::s32 tempIndex = _indices[i];
				_indices[i] = _indices[_indexCount - 1];
				_indices[_indexCount - 1] = tempIndex;
				_indexCount -= 1;

				return;
			}
		}
	}
}