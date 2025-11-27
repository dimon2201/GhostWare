// object.hpp

#pragma once

#include <string>
#include <typeinfo>
#include "log.hpp"
#include "types.hpp"

namespace realware
{
	class cContext;

	using ClassType = std::string;

	#define REALWARE_CLASS(typeName) \
		public: static ClassType GetType() { return #typeName; }

	class cIdentifier
	{
		REALWARE_CLASS(cIdentifier)

	public:
		using ID = std::string;

	public:
		cIdentifier(const std::string& id);
		~cIdentifier() = default;

		inline const ID& GetID() const { return _id; }

	private:
		ID _id = "";
	};

	class iObject
	{
		REALWARE_CLASS(iObject)

	public:
		explicit iObject(cContext* context) : _context(context) {}
		virtual ~iObject() = default;

		iObject(const iObject& rhs) = delete;
		iObject& operator=(const iObject& rhs) = delete;

		inline cContext* GetContext() const { return _context; }

	protected:
		cContext* _context = nullptr;
	};

	class cFactoryObject : public iObject
	{
		REALWARE_CLASS(cFactoryObject)

		friend class iFactory;

	public:
		explicit cFactoryObject(cContext* context) : iObject(context) {}
		virtual ~cFactoryObject() = default;

		inline cIdentifier* GetIdentifier() const { return _identifier; }

	protected:
		types::boolean _occupied = types::K_FALSE;
		types::s64 _allocatorIndex = 0;
		cIdentifier* _identifier = nullptr;
	};

	class iFactory : public iObject
	{
		REALWARE_CLASS(iFactory)

	public:
		explicit iFactory(cContext* context) : iObject(context) {}
		virtual ~iFactory() = default;

		virtual iObject* Create() = 0;

	protected:
		types::usize _counter = 0;
	};

	template <typename T>
	class ÒFactory : public iFactory
	{
		REALWARE_CLASS(ÒFactory)

	public:
		explicit ÒFactory(cContext* context) : iFactory(context) {}
		virtual ~ÒFactory() = default;

		virtual cFactoryObject* Create() override final;
	};

	template <typename T>
	cFactoryObject* ÒFactory<T>::Create()
	{
		if (_counter >= types::K_USIZE_MAX)
		{
			Print("Error: can't create object of type!");

			return nullptr;
		}

		T* object = new T(_context);
		const std::string id = object->GetType() + std::to_string(_counter++);
		object._identifier = new cIdentifier(id);

		return (cFactoryObject*)object;
	}
}