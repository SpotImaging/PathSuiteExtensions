// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <tchar.h>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <memory>
#include <sstream>
#include <filesystem>
#include <regex>
#include <vector>
#include "Utilities.h"
#include "CppMacroTools.h"
#include "SpotPlugin.h"
#include "PluginHost.h"
#include "EventSource.h"
#include "MulticastEventDelegate.h"
#include "HostVariables.h"
#include "EventLogger.h"
#include "EventArgConverters.h"
#include "EventSourceTypes.h"
#include "HostEvents.h"
#include "CallbackDispatcher.h"


// TODO: reference additional headers your program requires here
