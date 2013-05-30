#pragma once
#include "EventSource.h"
#include "EventArgConverters.h"

typedef EventSource<EventArgNoOp>                raw_event_t;           // the original value passed from the host 
typedef raw_event_t                              null_event_t;          // the event has no usable argument
typedef EventSource<EventArgCastTo<int>>         integer_event_t;       // the event is an integer value
typedef EventSource<EventArgCastTo<char*>>       write_string_event_t;  // the event is a pointer to a writable char buffer
typedef EventSource<EventArgCastTo<const char*>> read_string_event_t;   // the event is a pointer to a read only C-style string
typedef EventSource<EventArgToString>            string_event_t;        // the event is a std::string object
