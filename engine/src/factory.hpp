// factory.hpp

#pragma once

#include <string>
#include "capabilities.hpp"
#include "memory_pool.hpp"
#include "log.hpp"
#include "object.hpp"
#include "hash_table.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;

	template <typename T>
	class cFactoryObject : public cObjectPtr
	{
		friend class cFactory<T>;

		types::boolean allocated = types::K_FALSE;
	};

	template <typename T>
	class cFactory : public iObject
	{
		TRITON_OBJECT(cFactory)

	public:
		explicit cFactory(cContext* context);
		virtual ~cFactory() override final = default;

		template <typename... Args>
		cFactoryObject<T> Create(Args&&... args);
		template <typename... Args>
		cFactoryObject<T> Create(types::u8* data, types::u32 index, Args&&... args);
		void Destroy(cFactoryObject<T>& object);

	private:
		types::boolean AssertCounter();
		template <typename... Args>
		cFactoryObject<T> New(types::u8* data, types::u32 index, Args&&... args);

	private:
		types::usize _counter = 0;
	};

	template <typename T>
	cFactory<T>::cFactory(cContext* context) : iObject(context) {}

	template <typename T>
	template <typename... Args>
	cFactoryObject<T> cFactory<T>::Create(Args&&... args)
	{
		AssertCounter();
		return New(nullptr, 0, std::forward<Args>(args)...);
	}

	template <typename T>
	template <typename... Args>
	cFactoryObject<T> cFactory<T>::Create(types::u8* ptr, types::u32 index, Args&&... args)
	{
		AssertCounter();
		return New(ptr, index, std::forward<Args>(args)...);
	}

	template <typename T>
	void cFactory<T>::Destroy(cFactoryObject<T>& object)
	{
		T* objectPtr = object.object;

		if (objectPtr == nullptr)
			return;

		objectPtr->~T();

		if (objectPtr->allocated == types::K_TRUE)
		{
			cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
			memoryAllocator->Deallocate(objectPtr);
		}
	}

	template <typename T>
	types::boolean cFactory<T>::AssertCounter()
	{
		if (_counter >= types::K_USIZE_MAX)
		{
			Print("Error: can't create object of type '" + T::GetTypeStatic() + "'!");

			return types::K_TRUE;
		}

		return types::K_FALSE;
	}

	template <typename T>
	template <typename... Args>
	cFactoryObject<T> cFactory<T>::New(types::u8* data, types::u32 index, Args&&... args)
	{
		cFactoryObject<T> fo = {};

		if (data == nullptr)
		{
			const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
			cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();
			fo.object = (T*)memoryAllocator->Allocate(sizeof(T), caps->memoryAlignment);
			fo.allocated = types::K_TRUE;
		}
		else
		{
			fo.object = &(((T*)data)[index]);
			fo.allocated = types::K_FALSE;
		}

		new (fo.object) T(std::forward<Args>(args)...);

		fo.object->_identifier = cIdentifier::Generate(T::GetTypeStatic());

		return fo;
	}
}