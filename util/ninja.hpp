#ifndef UTIL_NINJA_HPP
#define UTIL_NINJA_HPP

#include <ostream>

namespace util {
template<class CharT>
struct basic_ninja {
    std::basic_ostream<CharT>& out;
    basic_ninja(std::basic_ostream<CharT>& o): out(o) {};
    basic_ninja& variable(const std::string& name, const std::string& content) {
        out << name << " = " << content << "\n\n";
        return *this;
    }

    basic_ninja& rule(const std::string& name, const std::string& content) {
        out << "rule " << name << "\n    command = " << content << "\n\n";
        return *this;
    }

    basic_ninja& build(const std::string& input, const std::string& output, const std::string& rule) {
        out << "build " << output << ": " << rule << ' ' << input << "\n\n";
        return *this;
    }
};

using ninja = basic_ninja<char>;
} // util

#endif // UTIL_NINJA_HPP