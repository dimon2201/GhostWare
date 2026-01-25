// context.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "factory.hpp"
#include "types.hpp"

namespace triton
{
	class cMemoryAllocator;

	class cContext
	{
	public:
		explicit cContext() = default;
		~cContext() = default;

		template <typename T, typename... Args>
		T* Create(Args&&... args);

		template <typename T, typename... Args>
		T* Create(types::u8* ptr, types::u32 index, Args&&... args);

		template <typename T>
		void Destroy(T* object);

		void CreateMemoryAllocator();

		template <typename T>
		void RegisterFactory();

		void RegisterSubsystem(iObject* object);

		inline cMemoryAllocator* GetMemoryAllocator() const { return _allocator; }

		template <typename T>
		inline T* GetFactory() const;

		template <typename T>
		inline T* GetSubsystem() const;

	private:
		cMemoryAllocator* _allocator = nullptr;
		std::unordered_map<ClassType, std::shared_ptr<iObject>> _factories;
		std::unordered_map<ClassType, std::shared_ptr<iObject>> _subsystems;
	};

	template <typename T, typename... Args>
	T* cContext::Create(Args&&... args)
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _factories.find(type);
		if (it != _factories.end())
			return ((cFactory<T>*)it->second.get())->Create(std::forward<Args>(args)...);
		else
			return nullptr;
	}

	template <typename T, typename... Args>
	T* cContext::Create(types::u8* ptr, types::u32 index, Args&&... args)
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _factories.find(type);
		if (it != _factories.end())
			return ((cFactory<T>*)it->second.get())->Create(ptr, index, std::forward<Args>(args)...);
		else
			return nullptr;
	}

	template <typename T>
	void cContext::Destroy(T* object)
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _factories.find(type);
		if (it != _factories.end())
			((cFactory<T>*)it->second.get())->Destroy(object);
	}

	template <typename T>
	void cContext::RegisterFactory()
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _factories.find(type);
		if (it == _factories.end())
			_factories.insert({type, std::make_shared<iObject>(this)});
	}

	template <typename T>
	T* cContext::GetFactory() const
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _factories.find(type);
		if (it != _factories.end())
			return (T*)it->second.get();
		else
			return nullptr;
	}

	template <typename T>
	T* cContext::GetSubsystem() const
	{
		const ClassType type = T::GetTypeStatic();
		const auto it = _subsystems.find(type);
		if (it != _subsystems.end())
			return (T*)it->second.get();
		else
			return nullptr;
	}
}