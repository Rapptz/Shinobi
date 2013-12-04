#include <sol/sol.hpp>
#include <boost/filesystem.hpp>
#include "shinobi.hpp"
#include "errors.hpp"

namespace fs = boost::filesystem;

namespace util {
void create_default_file(const std::string& filename) {
    std::ofstream out(filename);
    out << R"delim(-- the default shinobi.lua file

-- the project settings
project.name = "untitled";

-- compiler and linker settings
if compiler.name == "g++" or compiler.name == "clang++" then
    compiler.flags = { "-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall", "-O2" };
    linker.flags = { "-static" };
end

-- release and debug configuration
if config.release then
    table.insert(compiler.flags, "-DNDEBUG");
else
    table.insert(compiler.flags, "-g");
end

-- include paths
compiler.paths = { ".", "libs" };

-- directory information
directory.source = ".";
directory.build = "bin";
directory.object = "obj";)delim";
}

bool ends_with(const std::string& in, const std::string& other) {
    if(in.length() >= other.length())
        return in.compare(in.length() - other.length(), other.length(), other) == 0;
    else
        return false;
}

bool extension_is(const std::string&) {
    return false;
}

template<typename T, typename... Args>
bool extension_is(const std::string& str, T&& t, Args&&... args) {
    return ends_with(str, std::forward<T>(t)) || extension_is(str, std::forward<Args>(args)...);
}

void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string sanitise(const fs::path& p) noexcept {
    // Remove ./
    auto result = p.string();
    auto pos = result.find("./");

    if(pos == std::string::npos) {
        pos = result.find(".\\");
    }

    if(pos != std::string::npos) {
        result.replace(pos, 2, "");
    }

    // Replace all \ with /
    replace_all(result, "\\", "/");

    return result;
}

shinobi::shinobi(std::ostream& out): lua(new sol::state), file(out) {}
shinobi::~shinobi() = default;

void shinobi::fill_config_table() {
    std::string table("config = constant {\n    release = ");
    table += (config.release ? "true" : "false");
    table += ",\n    ";
    table += (config.debug ? "true" : "false");
    table += "\n}\n";
    lua->script(table);
}

void shinobi::parse() {}

void shinobi::initialise_lua(const std::string& compiler_name) {
    if(!fs::exists("shinobi.lua")) {
        create_default_file("shinobi.lua");
    }

    lua->open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::os, sol::lib::io);
    // inject defaults.
    lua->script(R"delim(project = {}
compiler = {}
linker = {}
directory = {}

function constant(table)
    return setmetatable({}, {
        __index = table,
        __newindex = function(table, key, value)
                        error("Attempt to modify constant table")
                     end,
        __metatable = false
    });
end
)delim");
    lua->get<sol::table>("compiler").set("name", compiler_name);
    fill_config_table();
    register_lua_functions();
    lua->open_file("shinobi.lua");
}

void shinobi::register_lua_functions() {
    // extended os functions
    auto os = lua->get<sol::table>("os");
    os.set_function("mkdir", [](std::string dir) {
        try {
            if(!fs::exists(dir)) {
                fs::create_directory(dir);
            }
        }
        catch(const std::exception&) {
            throw shinobi_error("failed to create directory " + dir);
        }
    });
    os.set_function("mkdirs", [](std::string dir) {
        try {
            if(!fs::exists(dir)) {
                fs::create_directories(dir);
            }
        }
        catch(const std::exception&) {
            throw shinobi_error("failed to create directory " + dir);
        }
    });
    os.set_function("rmdir", [](std::string dir) {
        try {
            fs::remove_all(dir);
        }
        catch(const std::exception&) {
            throw shinobi_error("failed to remove directory " + dir);
        }
    });

    #if defined(SHINOBI_WINDOWS)
    os.set("name", "windows");
    #elif defined(SHINOBI_LINUX)
    os.set("name", "linux");
    #elif defined(SHINOBI_MACOS)
    os.set("name", "osx");
    #else
    os.set("name", "unknown");
    #endif
}

void shinobi::release(bool b) {
    config.release = b;
    config.debug = !b;
}

void shinobi::fill_input(const std::string& directory) {
    for(fs::recursive_directory_iterator it(directory), end; it != end; ++it) {
        auto p = it->path();
        if(extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            input.insert(sanitise(p));
        }
    }
}

std::string shinobi::directory() {
    auto dir = lua->get<sol::table>("directory");
    auto source = dir.get<sol::object>("source");
    if(!source.is<std::string>()) {
        throw shinobi_error("directory.source has no value or is of invalid type (must be string)");
    }

    fill_input(source.as<std::string>());
    auto build = dir.get<sol::object>("build");
    auto object = dir.get<sol::object>("object");
    fs::path bin = build.is<std::string>() ? build.as<std::string>() : "bin";
    std::string obj = object.is<std::string>() ? object.as<std::string>() : "obj";
    file.variable("builddir", bin.string());
    file.variable("objdir", obj);

    if(!fs::exists(bin)) {
        fs::create_directories(bin);
    }

    return obj;
}

void shinobi::create() {
    file.comment("This file has been generated by shinobi version " SHINOBI_VERSION);
    file.newline();
    file.variable("ninja_required_version", "1.3");
    auto obj = directory();
}
} // util