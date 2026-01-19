// event_manager.hpp

#pragma once

#include <memory>
#include <queue>
#include <unordered_map>
#include <functional>
#include <vector>
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace harpy
{
    class cDataBuffer;
    class cContext;
    
    class cEventHandler : public iObject
    {
        HARPY_OBJECT(cEventHandler)

        friend class mEvent;

    public:
        cEventHandler(iObject* receiver, eEventType type, EventFunction&& function);
        ~cEventHandler() = default;

        void Invoke(cDataBuffer* data);
        inline iObject* GetReceiver() const { return _receiver; }
        inline eEventType GetEventType() const { return _type; }
        inline std::shared_ptr<EventFunction> GetFunction() const { return _function; }

    private:
        eEventType _type = eEventType::NONE;
        iObject* _receiver = nullptr;
        mutable std::shared_ptr<EventFunction> _function;
    };

    class cEventDispatcher : public iObject
    {
        HARPY_OBJECT(cEventDispatcher)

    public:
        explicit cEventDispatcher(cContext* context);
        virtual ~cEventDispatcher() override final = default;

        void Subscribe(iObject* receiver, eEventType type, EventFunction&& function);
        void Unsubscribe(iObject* receiver, eEventType type);
        void Send(eEventType type);
        void Send(eEventType type, cDataBuffer* data);

    private:
        std::unordered_map<eEventType, std::shared_ptr<cIdVector<cEventHandler>>> _listeners;
    };
}