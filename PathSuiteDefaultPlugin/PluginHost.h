#pragma once

#include "SpotPlugin.h"

namespace PluginHost
{
    extern SpotPluginApi::host_action_func_t ActionFunc;
    extern uintptr_t pluginHandle;

    inline bool DoAction(SpotPluginApi::host_action_t action, uintptr_t info, void *data)
    {
        return ActionFunc(pluginHandle, action, info, data);
    }
};
