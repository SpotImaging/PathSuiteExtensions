#pragma once

#include "PluginHost.h"
#include "EventSource.h"
#include "MulticastEventDelegate.h"
#include "HostVariables.h"
#include "EventLogger.h"
#include "EventArgConverters.h"
#include "EventSourceTypes.h"
#include "HostEvents.h"
#include "CallbackDispatcher.h"

void DoActionCode(int code);
void OnIdleEvent();
void OnShutdown();
