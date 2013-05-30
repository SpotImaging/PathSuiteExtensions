#pragma once

#include <stdint.h>
#include <string>
#include <functional>

struct EventArgNoOp : public std::unary_function<uintptr_t, uintptr_t>
{
    uintptr_t operator() (uintptr_t val) { return val;}
};


template <typename T>
struct EventArgCastTo : public std::unary_function<uintptr_t, T>
{
    T operator() (uintptr_t val) { return (T)val;}
};


struct EventArgToString : public std::unary_function<uintptr_t, std::string>
{
    std::string operator() (uintptr_t val) { return std::string(reinterpret_cast<const char*>(val));}
};
