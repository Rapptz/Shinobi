#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>
#include <sstream>
#include <sol/object.hpp>
#include <sol/table.hpp>

class compiler {
protected:
    std::string compiler_name;
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
    compiler() = default;
    compiler(std::string s): compiler_name(std::move(s)) {}

    std::string name() const noexcept {
        return compiler_name;
    }

    virtual std::string initial() = 0;
    virtual std::string include_path(const sol::object&) = 0;
    virtual std::string output() = 0;
    virtual std::string library_path(const sol::object&) = 0;

    virtual std::string compiler_flags(const sol::object& obj) {
        if(obj.is<sol::table>()) {
            const auto& t = obj.as<sol::table>();
            to_string(t);
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

#endif // COMPILER_HPP