#include <boost/filesystem.hpp>
#include <utility>
#include "writer.hpp"

namespace fs = boost::filesystem;
using namespace util;

void make_default_shinobi() {
    std::ofstream out("Shinobi2");
    out << 
R"shin({
    "project": {
        "name": "untitled",
        "type": "software"
    },

    "compiler": {
        "name": "g++",
        "flags": ["-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall", "-O2"]
    },

    "linker": {
        "flags": ["-static"]
    },

    "include_paths": ["."],

    "directory": {
        "source": ".",
        "build": "bin",
        "object": "obj"
    }
})shin";
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

template<typename Cont>
std::string flatten_list(const Cont& c) {
    std::ostringstream ss;
    auto first = c.cbegin();
    auto last = c.cend();

    if(first != last) {
        ss << *first++;
    }

    while(first != last) {
        ss << ' ' << *first++;
    }

    return ss.str();
}

std::string sanitise(const fs::path& p) noexcept {
    // Remove ./
    auto result = p.string();
    auto pos = result.find("./");

    if(pos == std::string::npos) {
        pos = result.find(".\\");
        if(pos == std::string::npos) {
            return result;
        }
    }

    result.replace(pos, 2, "");
    return result;
}

writer::writer(std::ofstream& out): file(out), dir(fs::current_path()) {
    if(!parser.is_open()) {
        make_default_shinobi();
        parser.reopen();
    }

    file.variable("ninja_required_version", "1.3");
}

void writer::create_directories() {
    build = parser.database("directory.build");
    object = parser.database("directory.object");

    if(!fs::is_directory(build)) {
        fs::create_directories(build);
    }

    if(!fs::is_directory(object)) {
        fs::create_directories(object);
    }
}

void writer::software_variables() {
    file.variable("builddir", parser.database("directory.build"));
    file.variable("objdir", parser.database("directory.object"));
    
    auto compiler = parser.database("compiler.name");
    file.variable("cxx", compiler);
    std::string compile_command("$cxx ");
    std::string linker_command("$cxx ");

    if(compiler != "cl") {
        compile_command += "-MMD -MF $out.d ";
        if(parser.in_database("compiler.flags")) {
            file.variable("cxxflags", parser.database("compiler.flags"));
            compile_command += "$cxxflags ";
        }

        compile_command += "-c $in -o $out";
        if(parser.in_database("include.flags")) {
            file.variable("incflag", parser.database("include.flags"));
            compile_command += " $incflag";
        }

        linker_command += "$in -o $out";

        if(parser.in_database("linker.flags")) {
            file.variable("linkflags", parser.database("linker.flags"));
            linker_command += " $linkflags";
        }

        if(parser.in_database("linker.library_paths")) {
            file.variable("libpaths", parser.database("linker.library_paths"));
            linker_command += " $libpaths";
        }

        if(parser.in_database("linker.libraries")) {
            file.variable("libs", parser.database("linker.libraries"));
            linker_command += " $libs";
        }

        file.newline();
        file.rule("compile", compile_command, "deps = gcc", "depfile = $out.d", "description = Building $in to $out");
        file.newline();
        file.rule("link", linker_command, "description = Creating $out");
        file.newline();
    }
}

void writer::create_software_file() {
    software_variables();

    auto compiler = parser.database("compiler.name");

    // Generate build sequence
    for(auto&& p : input) {
        auto directory = dir / object / p;
        if(fs::exists(directory.parent_path())) {
            fs::create_directories(directory.parent_path());
        }

        if(compiler != "cl") {
            auto output_file = "$objdir/" + sanitise(fs::path(p).replace_extension(".o"));
            output.insert(output_file);
            file.build(output_file, p, "compile");   
        }
    }

    auto name = parser.database("project.name");

    // Generate link sequence
    file.build("$builddir/" + name, flatten_list(output), "link");
}

void writer::fill_input() {
    auto input_dir = parser.database("directory.source");

    for(fs::recursive_directory_iterator it(input_dir), end; it != end; ++it) {
        auto p = it->path();
        if(extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            input.insert(sanitise(p));
        }
    }
}

void writer::create() {
    parser.parse();
    create_directories();
    fill_input();

    if(parser.is_software()) {
        create_software_file();
    }
}