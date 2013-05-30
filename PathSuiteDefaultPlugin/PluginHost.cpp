#include "stdafx.h"
#include "PluginHost.h"

SpotPluginApi::host_action_func_t PluginHost::ActionFunc = NULL;
uintptr_t PluginHost::pluginHandle = 0;