#pragma once
#include <functional>
#include <type_traits>

 
template<typename T> 
struct function_traits;
 
template<typename R, typename ...Args> 
struct function_traits<std::function<R(Args...)>>
{
    static const size_t nargs = sizeof...(Args);
    
    typedef R result_type;
 
    template <size_t i>
    struct arg
    {
        static_assert(i <= nargs, "The function does not have this many arguments");
        typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
    };
};
