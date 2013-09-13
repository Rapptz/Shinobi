#ifndef UTIL_STRING_HPP
#define UTIL_STRING_HPP

#include <string>
#include <sstream>

namespace util {
bool ends_with(const std::string& in, const std::string& other) {
    if(in.length() >= other.length())
        return in.compare(in.length() - other.length(), other.length(), other) == 0;
    else
        return false;
}

template<typename Cont>
std::string stringify_list(const Cont& list) {
    std::ostringstream out;
    auto first = list.cbegin();
    auto last = list.cend();
    
    if(first != last)
        out << *first++;
    while(first != last)
        out << ' ' << *first++;

    return out.str();
}

bool extension_is(const std::string& str) {
    return false;
}

template<typename T, typename... Args>
bool extension_is(const std::string& str, T&& t, Args&&... args) {
    return ends_with(str, std::forward<T>(t)) || extension_is(str, std::forward<Args>(args)...);
}
} // util

#endif // UTIL_STRING_HPP