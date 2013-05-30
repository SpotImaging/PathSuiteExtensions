#ifndef _SPOTPLUGIN_H_INCLUDED_
#define _SPOTPLUGIN_H_INCLUDED_
//===========================================================
// Summary:
//    All primitive types in this file must be C99 compliant.
//    (e.g. standard types or types defined in the file stdint.h such as int<x>_t, uint<x>_t, ...) 
//    This file does not need C complier compatibility.
///   The API just needs to be ABI safe for future builds. Following C like semantics will aid with this.
//======

#include <stdint.h>
#include <stddef.h>
#include <string>

namespace SpotPluginApi
{

#pragma pack(push, 8) // using 8 byte alignment for all structures

#if defined(_WINDOWS)|| defined(_Windows) || defined(_WIN32) || defined(__WIN32__)
#  define SPOTPLUGINAPI __stdcall
#else
#  define SPOTPLUGINAPI
#endif // defined(_WINDOWS)|| defined(_Windows) || defined(_WIN32) || defined(__WIN32__)

typedef uint32_t host_action_t;
namespace HostActionRequest
{
   const host_action_t   Unknown                  = 0;
   const host_action_t   BindEventHandler         = 1;   // Use msg_event_handler_binding_t
   const host_action_t   UnbindEventHandler       = 2;   // Use msg_event_handler_binding_t
   const host_action_t   GetVariable              = 10;  // Use msg_get_set_variable_t
   const host_action_t   SetVariable              = 20;  // Use msg_get_set_variable_t
   const host_action_t   SaveVariable             = 24;  // Use msg_save_recall_variable_t
   const host_action_t   RecallVariable           = 25;  // Use msg_save_recall_variable_t
   const host_action_t   AcqSingleImage           = 30;
   const host_action_t   StartLive                = 40;
   const host_action_t   PauseLive                = 41;
   const host_action_t   EndLive                  = 42;
}

typedef uint32_t host_event_t;
namespace HostEvent
{
   const host_event_t  Idle                       = 0;  // The host application has finished updating the UI from a previous action.
   const host_event_t  ApplicationClosing         = 1;  // The application is about to close 
   const host_event_t  ImageDocChanged            = 10; // A new image document has focus
   const host_event_t  CameraInitialized          = 11; 
}


typedef uint32_t callback_reason_t;
namespace CallbackReason
{
   const callback_reason_t   Unknown              = 0;  // The action is undefined
   const callback_reason_t   UnloadingPlugin      = 1;  // The application is about to unload the library. The info parameter value is undefined.
   const callback_reason_t   ActionCode           = 2;  // Execute an action that returns nothing. The info parameter contains a numeric value of the action code.

#ifdef RESTFUL_PLUGIN_SUPPORT_ENABLED
   // Representational state transfer verbs (based on HTTP)
   const callback_reason_t   Get                  = 10;
   const callback_reason_t   Put                  = 11; 
   const callback_reason_t   Post                 = 12; 
   const callback_reason_t   Delete               = 13; 
#endif // RESTFUL_PLUGIN_SUPPORT_ENABLED
}

// type signature for an event handler function
typedef void (SPOTPLUGINAPI *event_handler_t)(host_event_t hostEvent, uintptr_t args, uintptr_t userData);

// type signature for the host application request function
typedef bool (SPOTPLUGINAPI *host_action_func_t)(uintptr_t pluginHandle, host_action_t action, uintptr_t info, void *data);

// type signature for the main plugin callback function
typedef void (SPOTPLUGINAPI *callback_func_t)(callback_reason_t reason, uintptr_t info, uintptr_t userData);

// type signature for the library export function 
typedef bool (SPOTPLUGINAPI *init_func_t)(host_action_func_t hostActionFunc, uintptr_t pluginHandle, uintptr_t info, callback_func_t *ppCallback, uintptr_t *userData);


#if !defined(MACRO_VALUE_TO_STRING) && !defined(MACRO_NAME_TO_STRING)
#  define MACRO_VALUE_TO_STRING(s) MACRO_NAME_TO_STRING(s)
#  define MACRO_NAME_TO_STRING(s) #s
#endif

#define SPOTPLUGIN_INIT_FUNC   PluginInitialize
#define SPOTPLUGIN_INIT_FUNC_NAME   MACRO_VALUE_TO_STRING(SPOTPLUGIN_INIT_FUNC)

extern "C"
{
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
   ///
   /// Note: This function declaration is placed here for your convenience only. Use it as you see fit.
   bool SPOTPLUGINAPI SPOTPLUGIN_INIT_FUNC(host_action_func_t hostActionFunc, uintptr_t pluginHandle, uintptr_t info, callback_func_t *pluginCallbackFunc, uintptr_t *userData);
}

struct text_variable_t
{
   size_t Length; // A count of writable characters in the Text array or the current string length.
   char * Text;   // A pointer to a char array that has a size of at least (Length + 1).

   // Returns a pointer to an array that contains a null-terminated sequence
   // of characters (i.e., a C-string) representing the current value of the object.
   const char* c_str()
   {
      if (Text[Length] != 0)
         Text[Length] = 0; // Make sure that the string is null-terminated
      return Text;
   }

   // Updates the Length field to reflect the new string length after the text buffer contents have been modified.
   // Note: The string length can only be made smaller than it currently is.
   // Returns a count that the Length has decreased by.
   size_t UpdateLength()
   {
      Text[Length] = 0; 
      size_t originalLength = Length;
      Length = strlen(Text);
      return originalLength - Length;
   }
};

// Helper functions to construct text_variable_t from standard types
///////////////////////////////////////////////////////////////////////////
inline text_variable_t make_text_variable(const char *str)
{
   text_variable_t textVar = {strlen(str), const_cast<char*>(str)};
   return textVar;
}

inline text_variable_t make_text_variable (const std::string &str)
{
   text_variable_t textVar = {str.size(), const_cast<char*>(str.c_str())};
   return textVar;
}

inline text_variable_t make_text_variable(char *str, size_t maxWrite)
{
   text_variable_t textVar = {maxWrite, str};
   return textVar;
}
/////////////////////////////////////////////////////////////////////

struct msg_event_handler_binding_t
{
   msg_event_handler_binding_t() :
      Version(0),
      EventHandler(NULL),
      UserData(0),
      EventSourceListLength(0),
      HostEventSourceList(NULL)
   {  }

   int32_t         Version;                // Read only
   event_handler_t EventHandler;           // Address of the event handler function (ignored if unbinding to an event)
   uintptr_t       UserData;               // Data to be sent back to the event handler function
   size_t          EventSourceListLength;  // The number of elements in the following array
   host_event_t    *HostEventSourceList;   // An array of host events to bind this event handler to
};


struct msg_get_set_variable_t
{
   enum VariableType { Unknown = 0, Text =  1, Numeric = 2, Bool = 4 };

   msg_get_set_variable_t() :
      Version(0),
      Reserved(0),
      VariableName(NULL),
      DialogName(NULL),
      _ignore(Unknown)
   {  }

   int32_t Version;             // Read only
   uint32_t Reserved;
   const char *VariableName;    // Name of variable to get/set value
   const char *DialogName;      // Name of the dialog which owns the variable (for embedded variables)
   union
   {
      intmax_t _ignore; // insures that the enum has room to grow for different compliers
      VariableType DataType;
   };
   union
   {
      double  NumericValue;
      uint8_t BoolValue; // 0 equals false anything else true
      text_variable_t TextValue;
   };
};


struct msg_save_recall_variable_t
{
   msg_save_recall_variable_t() :
      Version(0),
      Reserved(0),
      VariableName(NULL),
      DialogName(NULL),
      FilePath(NULL)
   { }

   int32_t  Version;            // Read only
   uint32_t Reserved;
   const char *VariableName;
   const char *DialogName;      // Name of the dialog which owns the variable (for embedded variables)
   const char *FilePath;
};

#pragma pack(pop) // restore original packing

} // end namespace SpotPluginApi

#endif //_SPOTPLUGIN_H_INCLUDED_
