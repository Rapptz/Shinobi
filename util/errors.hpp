#ifndef UTIL_ERRORS_HPP
#define UTIL_ERRORS_HPP

#include <stdexcept>
#include <string>

namespace util {
class shinobi_error : public std::logic_error {
public:
    shinobi_error(const std::string& str) noexcept: std::logic_error("shinobi: error: " + str) {}
};

class shinobi_fatal_error : public std::logic_error {
public:
    shinobi_fatal_error(const std::string& str) noexcept: std::logic_error("shinobi: fatal error: " + str) {}
};
} // util

#endif // UTIL_ERRORS_HPP