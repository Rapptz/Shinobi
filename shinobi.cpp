#include <boost/filesystem.hpp>
#include <util/parser.hpp>
#include <util/string.hpp>
#include <util/ninja.hpp>
#include <algorithm>
#include <iostream>

namespace fs = boost::filesystem;

void make_default_shinobi() {
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

int main() {
    auto dir = fs::current_path();
    util::parser shinobi;

    if(!shinobi.is_open()) {
        make_default_shinobi();
        shinobi.reopen();
    }

    shinobi.parse();

    std::ofstream out("build.ninja");
    util::ninja maker(out);

    maker.variable("ninja_required_version", "1.3");
    maker.variable("builddir", shinobi.get("BUILDDIR", "bin"));
    maker.variable("objdir", shinobi.get("OBJDIR", "obj"));
    maker.variable("cxx", shinobi.get("CXX", "g++"));
    maker.variable("cxxflags", shinobi.get("CXXFLAGS", "-std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2"));
    maker.variable("incflags", shinobi.get("INCLUDE_FLAGS", "-I."));
    maker.variable("ldflags", shinobi.get("LINK_FLAGS", "-static"));
    maker.variable("libpath", shinobi.get("LIB_PATHS", ""));
    maker.variable("libs", shinobi.get("LIBS", ""));
    maker.variable("def", shinobi.get("DEFINES", "-DNDEBUG"));

    maker.newline();

    maker.rule("compile", "deps = gcc", 
                     "depfile = $out.d", 
                     "command = $cxx -MMD -MF $out.d $cxxflags $def -c $in -o $out $incflags",
                     "description = Building $in to $out");
    maker.rule("link", "command = $cxx $in -o $out $ldflags $libpath $libs", "description = Linking $out");

    std::vector<fs::path> input;
    std::vector<std::string> output;

    for(fs::recursive_directory_iterator it(shinobi.get("SRCDIR", ".")), end; it != end; ++it) {
        auto p = it->path();
        if(util::extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            input.push_back(p);
        }
    }

    fs::path bin(shinobi.get("BUILDDIR", "bin"));
    fs::path obj(shinobi.get("OBJDIR", "obj"));

    if(!fs::is_directory(bin)) {
        fs::create_directory(bin);
    }

    if(!fs::is_directory(obj)) {
        fs::create_directory(obj);
    }

    // Generate build sequences

    for(auto&& p : input) {
        auto copy = p;
        auto appended_dir = (dir / obj / p).parent_path();
        if(!fs::is_directory(appended_dir)) {
            fs::create_directories(appended_dir);
        }

        std::string output_file = "$objdir/" + remove_symlink(copy.replace_extension(".o"));
        output.push_back(output_file);
        maker.build(remove_symlink(p), output_file, "compile");
    }
    
    // Generate link sequence

    maker.build(util::stringify_list(output), "$builddir/" + shinobi.get("PROJECT_NAME", "untitled"), "link");
}