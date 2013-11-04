#include "writer.hpp"
#include <boost/filesystem.hpp>

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

writer::writer(std::fstream& out): file(out), dir(fs::current_path()) {
    if(!parser.is_open()) {
        make_default_shinobi();
        parser.reopen();
    }

    file.variable("ninja_required_version", "1.3");
}

void writer::create_directories() {
    build = parser.get("directory.build");
    object = parser.get("directory.object");

    if(!fs::is_directory(build)) {
        fs::create_directories(build);
    }

    if(!fs::is_directory(object)) {
        fs::create_directories(object);
    }
}

void writer::create() {
    create_directories();
}