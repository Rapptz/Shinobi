#ifndef GCC_HPP
#define GCC_HPP

#include "compiler.hpp"

class gcc : public compiler {
public:
    gcc(std::string x): compiler(std::move(x)) {}

    virtual std::string initial() {
        return "-MMD -MF $out.d";
    }

    virtual std::string include_path(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "-I\"", "\"");
            return ss.str();
        }

        return "";
    }

    virtual std::string compiler_flags(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }

    virtual std::string output() {
        return "-c $in -o $out";
    }

    virtual std::string library_path(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "-L\"", "\"");
            return ss.str();
        }

        return "";
    }

    virtual std::string linker_flags(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }

    virtual std::string libraries(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
            return ss.str();
        }

        return "";
    }
};

#endif // GCC_HPP