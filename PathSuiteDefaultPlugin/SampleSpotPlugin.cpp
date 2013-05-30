// SampleSpotPlugin.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SampleSpotPlugin.h"

using namespace SpotPluginApi;
using namespace HostInterop;
using namespace std;

void DoActionCode(uintptr_t code)
{
    msg_get_set_variable_t setVariableMessage;
    switch(code)
    {
    case 1:
        setVariableMessage.DataType = msg_get_set_variable_t::Text;
        setVariableMessage.VariableName = "TextVar1";
        setVariableMessage.TextValue = make_text_variable("Hello world. from SPOT sample plug-in");
        PluginHost::DoAction(HostActionRequest::SetVariable, 0, &setVariableMessage);
        break;
    default:
        OutputDebugString(_T("Unknown action code sent to plug-in\n"));
        break;
    }
}

void SPOTPLUGINAPI OnHostIdle(host_event_t hostEvent, uintptr_t args, uintptr_t userData)
{
    static int callCount = 0;
    if (++callCount >= 10)
    {
        // Unbind the event handler after a few notifications
        msg_event_handler_binding_t eventBinding;
        eventBinding.EventSourceListLength = 1;
        eventBinding.HostEventSourceList = &hostEvent;
        PluginHost::DoAction(HostActionRequest::UnbindEventHandler, 0, &eventBinding);
    }
    OutputDebugString(_T("OnHostIdle\n"));
}

void SPOTPLUGINAPI OnApplicationClosing(host_event_t hostEvent, uintptr_t args, uintptr_t userData)
{
    OutputDebugString(_T("OnApplicationClosing\n"));
}

void SPOTPLUGINAPI DispatchEvent(host_event_t hostEvent, uintptr_t args, uintptr_t userData)
{
    ostringstream logMessage;
    logMessage << "Dispatching event: " << hostEvent << " with argument: " << args << endl;
    auto str = logMessage.str();
    OutputDebugStringA(str.c_str());
    switch(hostEvent)
    {
    case HostEvent::ApplicationClosing:
        OnApplicationClosing(hostEvent, args, userData);
        break;
    case HostEvent::Idle:
        OnHostIdle(hostEvent, args, userData);
        break;
    default:
        break;
    }
}

CallbackDispatcher dispatcher;

void OnUnloadingPlugin()
{
    OutputDebugString(_T("Plug-in is unloading\n"));
}

/// Summary:
///     This is the function that is registered with the host application to receive callbacks
void SPOTPLUGINAPI DispatchCallback(callback_reason_t reason, uintptr_t info, uintptr_t userData)
{
    switch (reason)
    {
    case CallbackReason::UnloadingPlugin:
        OnUnloadingPlugin();
        break;
    case CallbackReason::ActionCode:
        DoActionCode(info);
        break;
    default:
        break;
    }
}

struct DummyFunc : public EventDelegate<read_string_event_t::arg_type>
{
    void operator() (arg_type& val)
    {
        auto text = HostInterop::GetTextVariable("TextVar1");
        HostInterop::SetTextVariable("TextVar2", text+text);
        HostInterop::SetNumericVariable("NumVar1", 5867316000);
    }
};

void SetStandardEventHandlers()
{
    add_logger_to_event( HostInterop::HostEvents::ApplicationClosing(), "Application closing");
    add_logger_to_event( HostInterop::HostEvents::CameraInit(), "Camera initialized");
    add_logger_to_event( HostInterop::HostEvents::ImageDocChanged(), "Image document changed");
}

/// Summary:
///   Main export function that must be implemented by a plug-in.
///   This method will be called by the host application upon loading the library.
///   If the plug-in is allowed to load it will return a value of true.
///   Returning a value of false will cause the plug-in to unload
/// Parameters:
///   hostActionFunction (IN) -
///      A pointer to the host function to issue commands back to. 
///      Note that this function has usage limitations during the call to this function. See docs for more details.
///   handle (IN) - 
///      The handle for this plug-in. This value must be passed back to the host application.
///   info (IN) - 
///      Reserved for future use.
///   pluginCallbackFunc (OUT) -
///      A pointer to a value that will hold the callback function pointer. This value must be set be the function.
///      If a callback function is not needed then the pointer should be set to a value of NULL.
///   userData (OUT) -
///      A pointer to value that can be set by the function and the resulting value will be sent as an argument to the callback function. 
/// Returns:
///   true to continue loading the plug-in library, otherwise false.
bool SPOTPLUGINAPI SPOTPLUGIN_INIT_FUNC(host_action_func_t hostActionFunc, uintptr_t handle, uintptr_t info, callback_func_t *pluginCallbackFunc, uintptr_t *userData)
{
    // This following items must be initialized before anything else can be done. They are required for all plug-ins
    PluginHost::ActionFunc = hostActionFunc;
    PluginHost::pluginHandle = handle;

#ifdef USE_SIMPLE_FUNCTION_BASED_EXAMPLE
    // Set the callback function to handle requests from the host.
    *pluginCallbackFunc = DispatchCallback;
    *userData = 0;
    // --------------------------------------
    // Set up optional event bindings needed
    //---------------------------------------
    // Multiple events can be handled by a single function by passing an array of events needed
    host_event_t eventsToMonitor[] = {HostEvent::Idle, HostEvent::ApplicationClosing};
    msg_event_handler_binding_t multiEventBinding;
    multiEventBinding.EventHandler = DispatchEvent;
    // Set the length of the array
    multiEventBinding.EventSourceListLength = std::distance(std::begin(eventsToMonitor), std::end(eventsToMonitor)); 
    // set the pointer to the first element in the array
    multiEventBinding.HostEventSourceList = eventsToMonitor;
    PluginHost::DoAction(HostActionRequest::BindEventHandler, 0, &multiEventBinding);

    // Obviously binding one event to a function is also supported by setting it individually
    msg_event_handler_binding_t singleEventBinding;
    host_event_t eventToMonitor = HostEvent::Idle;
    singleEventBinding.EventHandler = OnHostIdle;
    singleEventBinding.EventSourceListLength = 1;
    singleEventBinding.HostEventSourceList = &eventToMonitor;
    PluginHost::DoAction(HostActionRequest::BindEventHandler, 0, &singleEventBinding);
#else 
    // -- Object Model Example--

    //===============================
    // Setup callback handler
    //
    // This callback handles all callback requests
    *pluginCallbackFunc = CallbackDispatcher::master_callback_func;
    *userData = reinterpret_cast<uintptr_t>(&dispatcher);
    
    // assign actions to the associated action id.
    dispatcher.SetAction(1, []()
    {
        if(!GetBoolVariable("LiveImgRunning"))
            PluginHost::DoAction(HostActionRequest::StartLive, 0, nullptr);
    });

    dispatcher.SetAction(10, []()
    {
        auto stdVars = VariableManager::StandardVars();
        auto argT1 = stdVars.GetByName<TextVariable>("_argT1");
        auto argT2 = stdVars.GetByName<TextVariable>("_argT2");
        auto argN1 = stdVars.GetByName<IntegerVariable>("LiveImgCount");
        stdVars.SetValue("_argT3", argT1.Value() + argT2.Value() + argN1.ToString());
    });

    //===============================
    // Setup optional event bindings
    //
    // Construct an EventSource<T> object by supplying an argument conversion function object
    // which is derived from std::unary_function and the host event to listen to.
    // It will automatically bind a listener to the event and allow an unlimited number of delegates to be called on the event.
    // To assign more than one listener to an event source use a 
    // The event arguments are type safe and allow for inheritance and validation.

    //auto cameraEventSource = new read_string_event_t(HostEvent::CameraInitialized);
    //add_logger_to_event(*cameraEventSource, "camera initialized event");
    //// lambda functions can be added if captured into a std::function object.
    // std::function<void(const char*)> testcode = [](const char* val)
    // {
    //     HostInterop::SetTextVariable("TextVar1", string("The last camera that was initialized was ").append(val)); 
    // };
    // cameraEventSource->AddDelegate(make_event_delegate(testcode));
    // cameraEventSource->AddDelegate(make_shared<DummyFunc>());

    SetStandardEventHandlers();

    std::function<void(HostEvents::application_closing_t::arg_type)> backupOnExit = [] (HostEvents::application_closing_t::arg_type)
    {
        string path = VariableManager::StandardVars().GetByName<TextVariable>("PrefsFilePath").Value();
        VariableManager::StandardVars().SaveAll(path + "\\BackupVars");
    };
    HostEvents::ApplicationClosing().AddDelegate(make_event_delegate(backupOnExit));

#endif // USE_SIMPLE_FUNCTION_BASED_EVENTS

    return true; // Tell the host that we want to load
}