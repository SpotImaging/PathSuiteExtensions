#pragma once

#include "function_traits.h"

template<typename ArgType>
class EventDelegate
{
protected: 
    EventDelegate() {}
public:
    typedef typename ArgType arg_type;

    virtual ~EventDelegate() {}
    virtual void operator()(ArgType &args) = 0;
};


template <typename Func, typename Arg>
class EventDelegateFunctionWrapper : public EventDelegate<Arg>
{
    Func func;
public:
    EventDelegateFunctionWrapper(Func f) : func(f)
    {
    }

    virtual void operator() (arg_type & args)
    {
        func(args);
    }
};


/// Constructs a new EventDelegate on the heap (captured in a shared_ptr) 
/// with a copy of a std::function object as the delegate handler.
template<typename Arg>
std::shared_ptr<EventDelegate<Arg>> make_event_delegate(std::function<void(Arg)> &theFunction)
{
    typedef std::function<void(Arg)> func_t;
    return std::make_shared<EventDelegateFunctionWrapper<func_t, Arg>>(theFunction);
}
