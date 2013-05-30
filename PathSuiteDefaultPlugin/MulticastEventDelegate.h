#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include "EventDelegate.h"

template<typename ArgType>
class MulticastEventDelegate : public EventDelegate<ArgType>
{
private:
    typedef std::vector<std::shared_ptr<EventDelegate<ArgType>>> delegate_container_t;
    delegate_container_t delegates;

public:
    MulticastEventDelegate() : EventDelegate()
    {
    }

    virtual ~MulticastEventDelegate()
    {
    }

    MulticastEventDelegate(MulticastEventDelegate && rhs)
    {
        delegates = std::move(rhs.delegates);
    }

    MulticastEventDelegate& operator=(MulticastEventDelegate && rhs)
    {
        if (this != &rhs)
        {
            delegates.clear();
            delegates = std::move(rhs.delegates);
        }
        return *this;
    }

    void AddDelegate(std::shared_ptr<EventDelegate<ArgType>> d)
    {
        delegates.push_back(d);
    }

    void RemoveDelegate(std::shared_ptr<EventDelegate<ArgType>> d)
    {
        delegates.erase(std::remove(delegates.begin(), delegates.end(), d), delegates.end());
    }

    void RemoveDelegate(const EventDelegate<ArgType>* d)
    {
        auto exitingVal = std::find_if(delegates.begin(), delegates.end(), [=] (delegate_container_t::const_reference item)
        {
            return item.get() == d;
        });
        delegates.erase(exitingVal);
    }

    void RemoveAllDelegates()
    {
        delegates.clear();
    }

    virtual void operator()(ArgType& args)
    {
        for (auto func : delegates)
            (*func)(args);
    }
};