#include <boost/filesystem.hpp>
#include <utility>
#include <vector>
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

    "linker": {
        "non-msvc": {
            "flags": ["-static"]
        }
    },

    "release": {
        "non-msvc": {
            "flags": ["-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall", "-O2", "-DNDEBUG"]
        },

        "msvc": {
            "flags": ["/O2", "/DNDEBUG", "/EHsa", "/TP", "/W3"]
        }
    },

    "debug": {
        "non-msvc": {
            "flags": ["-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall"]
        },

        "msvc": {
            "flags": ["/EHsa", "/TP", "/W3", "/F5120"]
        },

        "linker": {
            "msvc": {
                "flags": ["/DEBUG"]
            }
        }
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

    // un-escape all escaped /
    replace_all(result, "\\/", "/");

    // Replace all \ with /
    replace_all(result, "\\", "/");

    // annoying quotes
    if(result.front() == '"' && result.back() == '"') {
        return std::string(&result[1], &result[result.size() - 1]);
    }

    return result;
}

writer::writer(std::ofstream& out): file(out), dir(fs::current_path()) {
    if(!parser.is_open()) {
        make_default_shinobi();
        parser.reopen();
    }

    file.comment("This file has been generated by shinobi version " SHINOBI_VERSION);
    file.newline();
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

void writer::general_variables() {
    file.variable("builddir", parser.database("directory.build"));
    file.variable("objdir", parser.database("directory.object"));
    
    file.variable("cxx", parser.compiler_name());
    std::string compile_command("$cxx ");
    std::string linker_command("$cxx ");

    if(!parser.is_msvc()) {
        compile_command += "-MMD -MF $out.d ";
        if(parser.in_database("compiler.flags")) {
            auto cxxflags = parser.database("compiler.flags");

            #if defined(SHINOBI_LINUX)
            if(parser.is_library()) {
                cxxflags += " -fPIC";
            }
            #endif // linux specific

            file.variable("cxxflags", cxxflags);
            compile_command += "$cxxflags ";
        }

        compile_command += "-c $in -o $out";
        if(parser.in_database("include.paths")) {
            file.variable("incflags", parser.database("include.paths"));
            compile_command += " $incflags";
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
        file.rule("compile", compile_command, "deps = gcc", "depfile = $out.d", "description = Compiling $in");
        file.newline();
        file.rule("link", linker_command, "description = Creating $out");
        file.newline();
    }
    else {
        compile_command += "/nologo /showIncludes";

        if(parser.in_database("include.paths")) {
            file.variable("incflags", parser.database("include.paths"));
            compile_command += " $incflags";
        }

        if(parser.in_database("compiler.flags")) {
            auto cxxflags = parser.database("compiler.flags");
            file.variable("cxxflags", cxxflags);
            compile_command += " $cxxflags";
        }

        compile_command += " /c /Fo$out $in";
        linker_command += "/link /nologo /out:$out";

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

        linker_command += " @out.rsp";

        file.newline();
        file.rule("compile", compile_command, "deps = msvc", "description = Compiling $in");
        file.newline();
        file.rule("link", linker_command, "rspfile = $out.rsp", "rspfile_content = $in", "description = Creating $out");
        file.newline();
    }
}

void writer::build_sequence() {
    // Generate build sequence

    std::string output_file;
    for(auto&& p : input) {
        auto directory = dir / object / p;
        if(!fs::exists(directory.parent_path())) {
            fs::create_directories(directory.parent_path());
        }

        if(!parser.is_msvc()) {
            output_file = "$objdir/" + sanitise(fs::path(p).replace_extension(".o"));
        }
        else {
            output_file = "$objdir/" + sanitise(fs::path(p).replace_extension(".obj"));
        }

        output.insert(output_file);
        file.build(output_file, p, "compile");
    }
}

void writer::create_software_file() {
    general_variables();
    build_sequence();
    auto name = parser.database("project.name");

    file.newline();
    // Generate link sequence
    file.build("$builddir/" + name, flatten_list(output), "link");
    file.newline();
    file.build("install", "$builddir/" + name, "phony");
    file.newline();
    file.set_default("install");
}

void writer::fill_input() {
    auto input_dir = parser.database("directory.source");
    auto ignored_dir = parser.database("directory.ignored");
    std::set<std::string> ignored_files;

    // Todo: Improve ignored directory check
    if(!ignored_dir.empty()) {
        for(fs::recursive_directory_iterator it(ignored_dir), end; it != end; ++it) {
            auto p = it->path();
            if(extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
                ignored_files.insert(sanitise(p));
            }
        }
    }

    for(fs::recursive_directory_iterator it(input_dir), end; it != end; ++it) {
        auto p = it->path();
        if(extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            auto sanitised = sanitise(p);
            if(ignored_files.count(sanitised)) {
                continue;
            }
            input.insert(sanitised);
        }
    }

    if(parser.in_database("files.ignored")) {
        std::stringstream ss(parser.database("files.ignored"));
        for(std::string f; ss >> f; ) {
            f = sanitise(fs::path(f));
            input.erase(f);
        }
    }

    if(parser.in_database("files.extra")) {
        std::stringstream ss(parser.database("files.extra"));
        for(std::string f; ss >> f; ) {
            input.insert(sanitise(fs::path(f)));
        }
    }
}

void writer::library_variables() {
    if(!parser.is_msvc()) {
        std::string archive_command(parser.database("archive.name"));
        archive_command.push_back(' ');
        archive_command.append(parser.database("archive.options"));
        archive_command += " $out $in";
        file.newline();
        file.rule("static", archive_command, "description = Creating static library $out");
        file.newline();
        file.rule("shared", "$cxx -shared $extras -o $out $in", "description = Creating shared library $out");
        file.newline();
    }
    else {
        file.newline();
        file.rule("static", "lib /out:$out $in", "description = Creating static library $out");
        file.newline();
    }
}

void writer::create_library_file() {
    general_variables();
    library_variables();
    std::vector<std::string> library_names;

    for(unsigned i = 0; i < parser.library_count(); ++i) {
        parser.parse_library(i);
        input.clear();
        output.clear();
        fill_input();
        create_directories();
        build_sequence();

        // link sequences
        file.newline();
        std::string builddir("$builddir/");

        if(parser.is_static_library(i)) {
            if(!parser.is_msvc()) {
                builddir += "lib" + parser.library_name(i) + ".a";
            }

            file.build(builddir, flatten_list(output), "static");
        }
        else {
            if(!parser.is_msvc()) {
                #if defined(SHINOBI_WINDOWS)
                    builddir += parser.library_name(i) + ".dll";
                    auto libname = "lib" + parser.library_name(i) + ".a";
                    auto extras = "extras = -Wl,--out-implib=$builddir/" + libname + ",--export-all-symbols,--enable-auto-import";
                    file.build(builddir, flatten_list(output), "shared", extras);
                #else
                    auto version = parser.database("project.version");
                    auto pos = version.find('.');
                    if(pos == std::string::npos) {
                        throw shinobi_error("invalid version specified '" + version + '\'');
                    }
                    auto libname = "lib" + parser.library_name(i) + ".so";
                    auto subversion = libname + version.substr(0, pos);
                    builddir += libname + '.' + parser.database("project.version");
                    file.build(builddir, flatten_list(output), "shared", "extras = -Wl,-soname," + subversion);
                #endif
            }
        }

        file.newline();
        file.build(parser.library_name(i), builddir, "phony");
        library_names.push_back(builddir);
        file.newline();
    }

    file.build("all", flatten_list(library_names), "phony");
    file.newline();
    file.set_default("all");
}

void writer::debug(bool b) {
    if(b) {
        parser.enable_debug();
    }
    else {
        parser.disable_debug();
    }
}

void writer::compiler(const std::string& name) {
    parser.compiler_name(name);
}

void writer::create() {
    parser.parse();

    if(parser.is_software()) {
        create_directories();
        fill_input();
        create_software_file();
    }
    else if(parser.is_library()) {
        create_library_file();
    }
}