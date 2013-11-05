#include <util/shinobi.hpp>
#include <iostream>

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
        "source": "src",
        "build": "bin",
        "object": "obj"
    }
})shin";
}

int main() {
    util::shinobi file;

    if(!file.is_open()) {
        make_default_shinobi();
        file.reopen();
    }
}