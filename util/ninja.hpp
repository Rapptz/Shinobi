#ifndef UTIL_NINJA_HPP
#define UTIL_NINJA_HPP

#include <ostream>
#include <type_traits>

namespace util {
template<class CharT>
struct basic_ninja {
    std::basic_ostream<CharT>& out;
    basic_ninja(std::basic_ostream<CharT>& o): out(o) {};
    basic_ninja& variable(const std::string& name, const std::string& content) {
        out << name << " = " << content << "\n\n";
        return *this;
    }

    template<typename... Args>
    basic_ninja& rule(const std::string& name, Args&&... args) {
        typename std::common_type<Args...>::type command[] = { std::forward<Args>(args)... };
        out << "rule " << name;

        for(auto&& i : command)
            out << "\n    " << i;

        out << "\n\n";
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