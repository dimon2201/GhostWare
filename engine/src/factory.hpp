// factory.hpp

#pragma once

#include <string>
#include "log.hpp"
#include "context.hpp"
#include "memory_pool.hpp"
#include "engine.hpp"
#include "object.hpp"
#include "types.hpp"

namespace harpy
{
	class cContext;

	template <typename T>
	class cFactory : public iObject
	{
		HARPY_OBJECT(cFactory)

	public:
		explicit cFactory(cContext* context) : iObject(context) {}
		virtual ~cFactory() override final = default;

		template <typename... Args>
		T* Create(Args&&... args);
		void Destroy(T* object);

	private:
		types::usize _counter = 0;
	};

	template <typename T>
	template <typename... Args>
	T* cFactory<T>::Create(Args&&... args)
	{
		if (_counter >= types::K_USIZE_MAX)
		{
			Print("Error: can't create object of type '" + T::GetTypeStatic() + "'!");

			return nullptr;
		}
		
		const sEngineCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		const types::usize objectByteSize = sizeof(T);
		T* object = (T*)memoryAllocator->Allocate(objectByteSize, caps->memoryAlignment);
		new (object) T(std::forward<Args>(args)...);
		
		object->_identifier = cIdentifier::GenerateIdentifier(T::GetTypeStatic());

		return object;
	}

	template <typename T>
	void cFactory<T>::Destroy(T* object)
	{
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		object->~T();
		memoryAllocator->Deallocate(object);
	}
}