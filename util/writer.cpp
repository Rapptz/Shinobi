#include <boost/filesystem.hpp>
#include <utility>
#include "writer.hpp"

namespace fs = boost::filesystem;
using namespace util;

void make_default_shinobi() {
    std::ofstream out("Shinobi2");
    out << 
R"shin({
    "project": "untitled",
    "type": "software",

    "compiler": {
        "name": "g++",
        "flags": ["-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall", "-O2"]
    },

    "linker": {
        "flags": ["-static"]
    },

    "include_paths": ["."],

    "directory": {
        "source": "src",
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

writer::writer(std::fstream& out): file(out), dir(fs::current_path()) {
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
    create_directories();
    fill_input();
}