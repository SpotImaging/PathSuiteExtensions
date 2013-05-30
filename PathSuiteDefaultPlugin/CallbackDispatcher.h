#pragma once

#include <string>
#include <unordered_map>
#include <algorithm>
#include "SpotPlugin.h"

typedef void (*action_func_t)(void); 

class CallbackDispatcher
{
private:
    std::unordered_map<uintptr_t, action_func_t> actionFunctions;
    
public:

    void SetAction(uintptr_t actionId, action_func_t func)
    {
        actionFunctions[actionId] = func;
    }

    void RemoveAction(uintptr_t actionId)
    {
        actionFunctions.erase(actionId);
    }

    static void SPOTPLUGINAPI master_callback_func(SpotPluginApi::callback_reason_t reason, uintptr_t info, uintptr_t userData)
    {
        CallbackDispatcher *obj = reinterpret_cast<CallbackDispatcher*>(userData);
        switch (reason)
        {
        case SpotPluginApi::CallbackReason::UnloadingPlugin:
            obj->actionFunctions.clear();
            break;
        case SpotPluginApi::CallbackReason::ActionCode:
            if (obj->actionFunctions[info] != nullptr)
                obj->actionFunctions[info]();
            break;
        default:
            break;
        }
    }
};