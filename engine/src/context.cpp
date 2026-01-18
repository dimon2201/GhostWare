// context.cpp

#include "context.hpp"
#include "memory_pool.hpp"
#include "render_context.hpp"

namespace arachne
{
	void cContext::CreateMemoryAllocator()
	{
		_allocator = new cMemoryAllocator();
		_allocator->SetBins(65536);
	}

	void cContext::RegisterSubsystem(cObject* object)
	{
		ClassType type = object->GetType();
		const auto it = _subsystems.find(type);
		if (it == _subsystems.end())
			_subsystems.insert({ type, std::make_shared<cObject>(this) });
	}
}