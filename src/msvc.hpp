#ifndef MSVC_HPP
#define MSVC_HPP

#include "compiler.hpp"

class msvc : public compiler {
public:
    msvc(): compiler("cl") {}
    msvc(std::string): compiler("cl") {}

    virtual std::string initial() {
        return "/nologo /showIncludes";
    }

    virtual std::string include_path(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "/I\"", "\"");
            return ss.str();
        }

        return "";
    }

    virtual std::string output() {
        return "/c /Fo$out $in";
    }

    virtual std::string library_path(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t, "/libpath:\"", "\"");
            return ss.str();
        }

        return "";
    }
};

#endif // MSVC_HPP