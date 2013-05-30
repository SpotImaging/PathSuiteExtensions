#pragma once 


#if !defined(MACRO_VALUE_TO_STRING) && !defined(MACRO_NAME_TO_STRING)
#  define MACRO_VALUE_TO_STRING(s) MACRO_NAME_TO_STRING(s)
#  define MACRO_NAME_TO_STRING(s) #s
#endif

#define AT_FILE_LOCATION  __FILE__ " @" MACRO_VALUE_TO_STRING(__LINE__)