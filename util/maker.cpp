#include "maker.hpp"

namespace fs = boost::filesystem;

namespace util {
void make_default_shinobi() noexcept {
    std::ofstream out("Shinobi");
    out << "# The default Shinobi file. See reference.md for syntax help.\n\n"
           "PROJECT_NAME := untitled\n"
           "BUILDDIR := bin\n"
           "OBJDIR := obj\n"
           "CXX := g++\n"
           "CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2\n"
           "INCLUDE_FLAGS += -I.\n"
           "LINK_FLAGS += -static\n"
           "LIB_PATHS +=\n"
           "LIBS +=\n"
           "DEFINES += -DNDEBUG";
}

std::string remove_symlink(const fs::path& p) noexcept {
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

maker::maker(std::ofstream& out): dir(fs::current_path()), file(out) {
    if(!shinobi.is_open()) {
        make_default_shinobi();
        shinobi.reopen();
    }
}

void maker::debug(bool b) noexcept {
    shinobi.debug = b;
}

void maker::create_variables() noexcept {
    file.variable("ninja_required_version", "1.3");
    file.variable("builddir", shinobi.get("BUILDDIR", "bin"));
    file.variable("objdir", shinobi.get("OBJDIR", "obj"));
    file.variable("cxx", shinobi.get("CXX", "g++"));
    file.variable("cxxflags", shinobi.get("CXXFLAGS", "-std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2"));
    file.variable("incflags", shinobi.get("INCLUDE_FLAGS", "-I."));
    file.variable("ldflags", shinobi.get("LINK_FLAGS", "-static"));
    file.variable("libpath", shinobi.get("LIB_PATHS", ""));
    file.variable("libs", shinobi.get("LIBS", ""));
    file.variable("def", shinobi.get("DEFINES", "-DNDEBUG"));

    file.newline();

    file.rule("compile", "deps = gcc", 
                     "depfile = $out.d", 
                     "command = $cxx -MMD -MF $out.d $cxxflags $def -c $in -o $out $incflags",
                     "description = Building $in to $out");
    file.rule("link", "command = $cxx $in -o $out $ldflags $libpath $libs", "description = Linking $out");
}

void maker::fill_source_files() noexcept {
    auto input_dirs = shinobi.get_list("SRCDIR");

    if(input_dirs.empty()) {
        input_dirs.push_back(".");
    }

    for(auto&& d : input_dirs) {
        for(fs::recursive_directory_iterator it(d), end; it != end; ++it) {
            auto p = it->path();
            if(extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
                input.emplace(remove_symlink(p));
            }
        }
    }

    auto added = shinobi.get_list("FILES");

    for(auto&& i : added) {
        if(extension_is(i, ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            input.emplace(i);
        }
    }

    auto ignored = shinobi.get_list("IGNORED_FILES");

    for(auto&& i : ignored) {
        auto it = input.find(i);
        if(it != input.end()) {
            input.erase(it);
        }
    }
}

void maker::regular_parse() {
    shinobi.parse();

    create_variables();
    fill_source_files();
    create_directories();

    // Generate build sequences

    for(auto&& p : input) {
        auto appended_dir = (dir / obj / p).parent_path();
        if(!fs::exists(appended_dir)) {
            fs::create_directories(appended_dir);
        }

        std::string output_file = "$objdir/" + remove_symlink(fs::path(p).replace_extension(".o"));
        output.push_back(output_file);
        file.build(p, output_file, "compile");
    }
    
    // Generate link sequence

    file.build(stringify_list(output), "$builddir/" + shinobi.get("PROJECT_NAME", "untitled"), "link");
}

void maker::create_directories() {
    fs::path bin(shinobi.get("BUILDDIR", "bin"));
    fs::path obj(shinobi.get("OBJDIR", "obj"));

    if(!fs::is_directory(bin)) {
        fs::create_directory(bin);
    }

    if(!fs::is_directory(obj)) {
        fs::create_directory(obj);
    }
}
} // util