#ifndef GCC_HPP
#define GCC_HPP

#include <sstream>
#include "compiler.hpp"

class gcc : public compiler {
private:
    std::ostringstream ss;

    void to_string(const sol::table& t, std::string prefix = "", std::string postfix = "") {
        ss.str("");
        const auto size = t.size();
        unsigned index = 1;

        if(size != 1) {
            ss << prefix << t.get<std::string>(index++) << postfix;
        }

        for(; index <= size; ++index) {
            ss << ' ' << prefix << t.get<std::string>(index) << postfix;
        }
    }
public:
    gcc(std::string x): compiler(std::move(x)) {}

    virtual std::string initial() const {
        return "-MMD -MF $out.d";
    }

    virtual std::string include_path(const sol::object& obj) const {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "-I\"", "\"");
            return ss.str();
        }

        return "";
    }

    virtual std::string compiler_flags(const sol::object& obj) const {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }

    virtual std::string output() const {
        return "-c $in -o $out";
    }

    virtual std::string library_path(const sol::object& obj) const {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "-L\"", "\"");
            return ss.str();
        }

        return "";
    }

    virtual std::string linker_flags(const sol::object& obj) const {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }

    virtual std::string libraries(const sol::object& obj) const {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }
};

#endif // GCC_HPP