// object.hpp

#pragma once

#include <string>
#include "log.hpp"
#include "context.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "types.hpp"

namespace arachne
{
	class cContext;
	class cDataBuffer;
	class cGameObject;

	using ClassType = std::string;

	#define REALWARE_OBJECT(typeName) \
		public: static ClassType GetType() { return #typeName; }

	class cIdentifier
	{
	public:
		using ID = std::string;

	public:
		cIdentifier(const std::string& id);
		~cIdentifier() = default;

		inline const ID& GetID() const { return _id; }

	private:
		ID _id = "";
	};

	class cObject
	{
		friend class cMemoryAllocator;

	public:
		explicit cObject(cContext* context) : _context(context) {}
		virtual ~cObject() = default;

		cObject(const cObject& rhs) = delete;
		cObject& operator=(const cObject& rhs) = delete;

		template <typename... Args>
		void Subscribe(const std::string& id, eEventType type, Args&&... args);
		void Unsubscribe(eEventType type, cGameObject* receiver);
		void Send(eEventType type);
		void Send(eEventType type, cDataBuffer* data);

		inline cContext* GetContext() const { return _context; }
		inline cIdentifier* GetIdentifier() const { return _identifier; }

	protected:
		cContext* _context = nullptr;
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		cIdentifier* _identifier = nullptr;
	};

	template <typename... Args>
	void cObject::Subscribe(const std::string& id, eEventType type, Args&&... args)
	{
		const cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
		dispatcher->Subscribe<Args>(id, type, std::forward<Args>(args)...);
	}
}