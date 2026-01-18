// object.cpp

#include "object.hpp"
#include "event_manager.hpp"
#include "context.hpp"

namespace arachne
{
	cIdentifier::cIdentifier(const std::string& id) : _id(id) {}

    cObject::~cObject()
    {
        delete _identifier;
    }

    void cObject::Unsubscribe(eEventType type, cGameObject* receiver)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Unsubscribe(type, receiver);
    }

    void cObject::Send(eEventType type)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type);
    }

    void cObject::Send(eEventType type, cDataBuffer* data)
    {
        cEventDispatcher* dispatcher = _context->GetSubsystem<cEventDispatcher>();
        dispatcher->Send(type, data);
    }
}