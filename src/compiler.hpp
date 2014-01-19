#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>
#include <sol/object.hpp>

class compiler {
protected:
    std::string compiler_name;
public:
    compiler() = default;
    compiler(std::string s): compiler_name(std::move(s)) {}

    std::string name() const noexcept {
        return compiler_name;
    }

    virtual std::string initial() const = 0;
    virtual std::string include_path(const sol::object&) const = 0;
    virtual std::string compiler_flags(const sol::object&) const = 0;
    virtual std::string output() const = 0;
    virtual std::string library_path(const sol::object&) const = 0;
    virtual std::string linker_flags(const sol::object&) const = 0;
    virtual std::string libraries(const sol::object&) const = 0;
};

#endif // COMPILER_HPP