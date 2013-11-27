// The MIT License (MIT)

// Copyright (c) 2013 Danny Y., Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_STACK_HPP
#define SOL_STACK_HPP

#include "reference.hpp"
#include <utility>
#include <type_traits>

namespace sol {
namespace stack {
namespace detail {
template<typename T>
inline T pop_unsigned(lua_State* L, std::true_type) {
    return lua_tounsigned(L, -1);
}

template<typename T>
inline T pop_unsigned(lua_State* L, std::false_type) {
    return lua_tointeger(L, -1);
}

template<typename T>
inline T pop_arithmetic(lua_State* L, std::false_type) {
    // T is a floating point
    return lua_tonumber(L, -1);
}

template<typename T>
inline T pop_arithmetic(lua_State* L, std::true_type) {
    // T is an integral
    return pop_unsigned<T>(L, std::is_unsigned<T>{});
}

template<typename T>
inline T pop_helper(lua_State* L, std::true_type) {
    // T is a class type
    return T(L, -1);
}

template<typename T>
inline T pop_helper(lua_State* L, std::false_type) {
    // T is a fundamental type
    return pop_arithmetic<T>(L, std::is_integral<T>{});
}

template<typename T>
inline void push_unsigned(lua_State* L, T x, std::true_type) {
    lua_pushunsigned(L, x);
}

template<typename T>
inline void push_unsigned(lua_State* L, T x, std::false_type) {
    lua_pushinteger(L, x);
}

template<typename T>
inline void push_arithmetic(lua_State* L, T x, std::true_type) {
    // T is an integral type
    push_unsigned(L, x, std::is_unsigned<T>{});
}

template<typename T>
inline void push_arithmetic(lua_State* L, T x, std::false_type) {
    // T is an floating point type
    lua_pushnumber(L, x);
}
} // detail

template<typename T>
inline T pop(lua_State* L) {
    auto result = detail::pop_helper<T>(L, std::is_class<T>{});
    lua_pop(L, 1);
    return result;
}

template<>
inline bool pop<bool>(lua_State* L) {
    bool result = lua_toboolean(L, -1) != 0;
    lua_pop(L, 1);
    return result;
}

template<>
inline std::string pop<std::string>(lua_State* L) {
    std::string::size_type len;
    auto str = lua_tolstring(L, -1, &len);
    lua_pop(L, 1);
    return { str, len };
}

template<>
inline const char* pop<const char*>(lua_State* L) {
    auto result = lua_tostring(L, -1);
    lua_pop(L, 1);
    return result;
}

template<typename T>
inline typename std::enable_if<std::is_arithmetic<T>::value>::type push(lua_State* L, T arithmetic) {
    detail::push_arithmetic(L, arithmetic, std::is_integral<T>{});
}

inline void push(lua_State*, reference& ref) {
    ref.push();
}

inline void push(lua_State* L, bool boolean) {
    lua_pushboolean(L, boolean);
}

inline void push(lua_State* L, const nil_t&) {
    lua_pushnil(L);
}

inline void push(lua_State* L, lua_CFunction func) {
    lua_pushcfunction(L, func);
}

template<size_t N>
inline void push(lua_State* L, const char (&str)[N]) {
    lua_pushlstring(L, str, N - 1);
}

inline void push(lua_State* L, const std::string& str) {
    lua_pushlstring(L, str.c_str(), str.size());
}
} // stack
} // sol

#endif // SOL_STACK_HPP