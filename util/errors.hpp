#ifndef UTIL_ERRORS_HPP
#define UTIL_ERRORS_HPP

#include <stdexcept>
#include <string>

namespace util {
class shinobi_error : public std::logic_error {
public:
    shinobi_error(const std::string& str) noexcept: std::logic_error("shinobi: error: " + str) {}
};

class missing_property : public std::logic_error {
public:
    missing_property(const std::string& str) noexcept: 
    std::logic_error("shinobi: fatal error: missing '" + str + "' property") {}
};

class no_library_name : public std::logic_error {
public:
    no_library_name(unsigned index) noexcept: 
    std::logic_error("shinobi: fatal error: no library name provided at index " + std::to_string(index)) {}
};
} // util

#endif // UTIL_ERRORS_HPP