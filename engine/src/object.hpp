// object.hpp

#pragma once

#include <string>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "types.hpp"

namespace harpy
{
	class cContext;
	class cDataBuffer;
	class cGameObject;

	using ClassType = std::string;

	#define HARPY_OBJECT(typeName) \
		public: \
			static ClassType GetTypeStatic() { return #typeName; } \
			virtual ClassType GetType() const override { return GetTypeStatic(); } \

	class cIdentifier
	{
	public:
		using ID = std::string;

	public:
		cIdentifier(const std::string& id);
		~cIdentifier() = default;

		static cIdentifier* GenerateIdentifier(const std::string& type);

		inline const ID& GetID() const { return _id; }

	private:
		ID _id = "";
	};

	class iObject
	{
		template <typename T>
		friend class cIdVector;
		template <typename T>
		friend class cFactory;
		friend class cMemoryAllocator;

	public:
		explicit iObject(cContext* context) : _context(context) {}
		virtual ~iObject() = default;

		iObject(const iObject& rhs) = delete;
		iObject& operator=(const iObject& rhs) = delete;

		virtual ClassType GetType() const = 0;

		void Subscribe(const std::string& id, eEventType type);
		void Unsubscribe(eEventType type, cGameObject* receiver);
		void Send(eEventType type);
		void Send(eEventType type, cDataBuffer* data);

		inline cContext* GetContext() const { return _context; }
		inline const cIdentifier::ID& GetID() const { return _identifier->GetID(); }
		inline cIdentifier* GetIdentifier() const { return _identifier; }

	protected:
		cContext* _context = nullptr;
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		cIdentifier* _identifier = nullptr;
	};
}