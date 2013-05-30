#pragma once
#include "stdafx.h"
#include <memory>
#include <string>
#include <sstream>
#include "EventDelegate.h"


template<typename ArgType>
class EventLogger : public EventDelegate<ArgType>
{
private:
    std::string name;
public:
    EventLogger(const std::string name) : name(name) {}

    virtual void operator()(ArgType &args)
    {
        std::ostringstream message;
        message << "Event: {"<< name << "} Args: {" << args << "}" << std::endl;
        auto msg = message.str();
        OutputDebugStringA(msg.c_str());
    }
};


template <typename EvSource>
void add_logger_to_event(EvSource &eventSource, const std::string eventName)
{
    typedef typename EvSource::arg_type arg_type;
    return eventSource.AddDelegate(std::make_shared<EventLogger<arg_type>>(eventName));
}
