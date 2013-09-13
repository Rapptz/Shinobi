#ifndef UTIL_TRIM_HPP
#define UTIL_TRIM_HPP

#include <string>

namespace util {
inline void rtrim(std::string& str) {
    std::string::size_type end = str.find_last_not_of(" \t\n\r\f\v");
    if(end != std::string::npos) {
        str.substr(0, end + 1).swap(str);
    }
}
inline void ltrim(std::string& str) {
    std::string::size_type start = str.find_first_not_of(" \t\n\r\f\v");
    if(start != std::string::npos) {
        str.substr(start).swap(str);
    }
}

inline void trim(std::string& str) {
    rtrim(str);
    ltrim(str);
}
} // util

#endif // UTIL_TRIM_HPP