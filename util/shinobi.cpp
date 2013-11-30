#include <sol.hpp>
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
directory.object = "obj";)delim"
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

void shinobi::parse(const std::string& compiler_name) {
    if(!fs::exists("shinobi.lua")) {
        create_default_file("shinobi.lua");
    }

    lua->open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);
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
}

void shinobi::release(bool b) {
    config.release = b;
    config.debug = !b;
}
} // util