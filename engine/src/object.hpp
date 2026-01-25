// object.hpp

#pragma once

#include <string>
#include <array>
#include "log.hpp"
#include "event_types.hpp"
#include "memory_pool.hpp"
#include "tag.hpp"
#include "types.hpp"

namespace triton
{
	class cContext;
	class cDataBuffer;
	class cGameObject;

	using ClassType = std::string;

	#define TRITON_OBJECT(typeName) \
		public: \
			static ClassType GetTypeStatic() { return #typeName; } \
			virtual ClassType GetType() const override { return GetTypeStatic(); } \

	class cIdentifier
	{
	public:
		static cTag Generate(const std::string& seed);
	};

	class iObject;
	class cObjectPtr
	{
	public:
		iObject* object = nullptr;
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

		void Subscribe(eEventType type, EventFunction&& function);
		void Unsubscribe(eEventType type);
		void Send(eEventType type);
		void Send(eEventType type, cDataBuffer* data);

		inline cContext* GetContext() const { return _context; }
		inline const cTag& GetID() const { return _id; }

	protected:
		cContext* _context = nullptr;
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		cTag _id;
	};
}