#ifndef UTIL_NINJA_HPP
#define UTIL_NINJA_HPP

#include <ostream>
#include <type_traits>
#include <string>

namespace util {
inline std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

template<class CharT>
struct basic_ninja {
    std::basic_ostream<CharT>& out;
    basic_ninja(std::basic_ostream<CharT>& o): out(o) {};
    basic_ninja& variable(const std::string& name, const std::string& content) {
        out << name << " = " << content << '\n';
        return *this;
    }

    basic_ninja& newline() {
        out << '\n';
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
        auto out_str = replace_all(output, "\\", "/");
        auto in_str = replace_all(input, "\\", "/");
        out << "build " << out_str << ": " << rule << ' ' << in_str << "\n\n";
        return *this;
    }
};

using ninja = basic_ninja<char>;
} // util

#endif // UTIL_NINJA_HPP