// event_manager.cpp

#pragma once

#include "application.hpp"
#include "context.hpp"
#include "gameobject_manager.hpp"
#include "engine.hpp"
#include "event_manager.hpp"
#include "buffer.hpp"

using namespace types;

namespace triton
{
    cEventHandler::cEventHandler(cContext* context, iObject* receiver, eEventType type, EventFunction&& function) : iObject(context), _receiver(receiver), _type(type), _function(std::make_shared<EventFunction>(std::move(function))) {}

    void cEventHandler::Invoke(cDataBuffer* data)
    {
        _function->operator()(data);
    }

    cEventDispatcher::cEventDispatcher(cContext* context) : iObject(context) {}

    void cEventDispatcher::Subscribe(iObject* receiver, eEventType type, EventFunction&& function)
    {
        const auto listener = _listeners.find(type);
        if (listener == _listeners.end())
        {
            const sCapabilities* caps = _context->GetSubsystem<cEngine>()->GetApplication()->GetCapabilities();
            _listeners.insert({ type, std::make_shared<cCache<cEventHandler>>(_context, caps->maxEventPerTypeCount) });
        }

        _listeners[type]->Add(receiver->GetID(), _context, receiver, type, std::move(function));
    }

    void cEventDispatcher::Unsubscribe(iObject* receiver, eEventType type)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        const cEventHandler* elements = events->GetElements();
        for (usize i = 0; i < events->GetElementCount(); i++)
        {
            const iObject* listenerReceiver = elements[i].GetReceiver();
            if (listenerReceiver == receiver)
            {
                events->Delete(listenerReceiver->GetID());

                return;
            }
        }
    }

    void cEventDispatcher::Send(eEventType type)
    {
        cDataBuffer data(_context);

        Send(type, &data);
    }

    void cEventDispatcher::Send(eEventType type, cDataBuffer* data)
    {
        if (_listeners.find(type) == _listeners.end())
            return;

        auto& events = _listeners[type];
        cEventHandler* elements = events->GetElements();
        for (usize i = 0; i < events->GetElementCount(); i++)
            elements[i].Invoke(data);
    }
}