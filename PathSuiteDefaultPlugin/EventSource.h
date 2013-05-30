#pragma once
#include "SpotPlugin.h"
#include "PluginHost.h"
#include "MulticastEventDelegate.h"
#include <functional>


/// Summary:
///   Helper class to create an custom event source
template<typename ArgTransformFunc, typename EventArgType = typename ArgTransformFunc::result_type>
class EventSource
{
public:
    typedef typename EventArgType arg_type;
    typedef typename ArgTransformFunc unary_function;

private:
    MulticastEventDelegate<arg_type> eventDelegate;
    SpotPluginApi::host_event_t targetEvent;
    bool isEnabled;
    ArgTransformFunc argTransformFunc;


    static void SPOTPLUGINAPI dispatch_to_owner(SpotPluginApi::host_event_t hostEvent, uintptr_t args, uintptr_t source)
    {
        (reinterpret_cast<EventSource*>(source))->HandleEvent(args);
    }

    // no copies allowed. This must be a singleton since only one function can be assigned to the host callback
    EventSource(const EventSource&); 
    EventSource& operator = (const EventSource& );

    void HandleEvent(uintptr_t rawArgs)
    {
        auto realArg = argTransformFunc(rawArgs);
        eventDelegate(realArg);
    }


public:

    EventSource(SpotPluginApi::host_event_t hostEvent, ArgTransformFunc argTransform = ArgTransformFunc()) :
        targetEvent(hostEvent), argTransformFunc(argTransform), isEnabled(false)
    {
        Enable();
    }
   
    // Move constructor
    EventSource (EventSource && rhs)
    {
        eventDelegate = std::move(rhs.eventDelegate);
        targetEvent = std::move(rhs.targetEvent);
        argTransformFunc = std::move(rhs.argTransformFunc);
        isEnabled = rhs.isEnabled;
        // Setting this will disable ownership of the event
        if (rhs.isEnabled)
        {
            Enable(); // Tell the host that the callback information has changed
            rhs.isEnabled = false;
        }
    }

    EventSource& operator = (EventSource && rhs)
    {
        if (this != &other)
        {
            eventDelegate = std::move(rhs.eventDelegate);
            targetEvent = std::move(rhs.targetEvent);
            argTransformFunc = std::move(rhs.argTransformFunc);
            isEnabled = rhs.isEnabled;
            // Setting this will disable ownership of the event
            if (rhs.isEnabled)
            {
                Enable(); // Tell the host that the callback information has changed
                rhs.isEnabled = false;
            }
        }
        return *this;
    }

    ~EventSource()
    {
        Disable();
    }

    void AddDelegate(std::shared_ptr<EventDelegate<EventArgType>> d)
    {
        eventDelegate.AddDelegate(d);
    }

    void RemoveDelegate(std::shared_ptr<EventDelegate<EventArgType>> d)
    {
        eventDelegate.RemoveDelegate(d);
    }

    void RemoveDelegate(const EventDelegate<EventArgType>* d)
    {
        eventDelegate.RemoveDelegate(d);
    }
    
    void Enable()
    {
        SpotPluginApi::msg_event_handler_binding_t eventBinding;
        eventBinding.EventHandler = dispatch_to_owner;
        eventBinding.EventSourceListLength = 1;
        eventBinding.UserData = reinterpret_cast<uintptr_t>(this);
        eventBinding.HostEventSourceList = &targetEvent;
        isEnabled = PluginHost::DoAction( SpotPluginApi::HostActionRequest::BindEventHandler, 0, &eventBinding);
    }

    void Disable()
    {
        if (!isEnabled)
            return;
        SpotPluginApi::msg_event_handler_binding_t eventBinding;
        eventBinding.EventSourceListLength = 1;
        eventBinding.HostEventSourceList = &targetEvent;
        eventBinding.UserData = reinterpret_cast<uintptr_t>(this);
        PluginHost::DoAction( SpotPluginApi::HostActionRequest::UnbindEventHandler, 0, &eventBinding);
        isEnabled = false;
    }

    bool Listening() const { return isEnabled; }
};
