#ifndef UTIL_NINJA_HPP
#define UTIL_NINJA_HPP

#include <string>
#include <utility>
#include <ostream>

namespace util {
struct ninja {
private:
    std::ostream& out;
    unsigned width;

    int dollar_count(const std::string& str, int index) {
        int total = 0;
        for(int i = index - 1; i > 0 && str[i] == '$'; --i) {
            ++total;
        }

        return total;
    }

    void insert_line(std::string str, unsigned indent = 0) {
        std::string leading_spaces(' ', indent * 2);
        auto space = std::string::npos;

        while(leading_spaces.size() + str.size() > width) {
            auto available_spaces = width - leading_spaces.size() - 2;
            space = available_spaces;

            while(1) {
                space = str.rfind(' ', space);
                if(space == std::string::npos || dollar_count(str, space) % 2 == 0) {
                    break;
                }
            }

            if(space == std::string::npos) {
                space = available_spaces - 1;
                while(1) {
                    space = str.find(' ', space + 1);
                    if(space == std::string::npos || dollar_count(str, space) % 2 == 0) {
                        break;
                    }
                }
            }

            if(space == std::string::npos) {
                break;
            }

            out << leading_spaces << str.substr(0, space) << " $\n";
            str = str.substr(space + 1);

            leading_spaces.insert(leading_spaces.size(), ' ', 4);
        }

        out << leading_spaces << str << '\n';
    }
public:
    ninja(std::ostream& out, unsigned width = 78): out(out), width(width) {}

    ninja& newline() {
        out << '\n';
        return *this;
    }

    ninja& variable(const std::string& name, const std::string& value, unsigned indent = 0) {
        if(value.empty())
            return *this;
        insert_line(name + " = " + value, indent);
        return *this;
    }

    template<typename... Args>
    ninja& rule(const std::string& name, const std::string& command, Args&&... args) {
        std::string commands[sizeof...(args)] = { std::forward<Args>(args)... };
        insert_line("rule " + name);
        variable("command", command, 1);

        for(auto&& arg : commands) {
            insert_line(arg, 1);
        }

        return *this;
    }

    ninja& build(const std::string& output, const std::string& input, const std::string& rule) {
        insert_line("build " + output + ": " + rule + ' ' + input);
        return *this;
    }
};

} // util

#endif // UTIL_NINJA_HPP