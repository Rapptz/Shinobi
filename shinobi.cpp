#include <boost/filesystem.hpp>
#include <util/parser.hpp>
#include <util/string.hpp>
#include <util/ninja.hpp>
#include <algorithm>
#include <iostream>

namespace fs = boost::filesystem;

void make_default_shinobi() {
    std::ofstream out("Shinobi");
    out << "# The default Shinobi file. See README.md for syntax help.\n\n"
           "PROJECT_NAME := untitled\n"
           "CXX := g++\n"
           "CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O2\n"
           "INCLUDE_FLAGS += -I.\n"
           "LIBRARY_PATHS +=\n"
           "LIBRARY_FLAGS +=\n"
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

    maker.variable("cxx", shinobi.get("CXX", "g++"));
    maker.variable("cxxflags", shinobi.get("CXXFLAGS", "-std=c++11 -pedantic -Wall -O3 -static-libstdc++"));
    maker.variable("incflags", shinobi.get("INCLUDE_FLAGS", "-I."));
    maker.variable("libpath", shinobi.get("LIBRARY_PATHS", ""));
    maker.variable("lib", shinobi.get("LIBRARY_FLAGS", ""));
    maker.variable("def", shinobi.get("DEFINES", "-DNDEBUG"));

    maker.rule("bd", "deps = gcc", "depfile = $out.d", "command = $cxx -MMD -MF $out.d $cxxflags $def -c $in -o $out $incflags");
    maker.rule("ld", "command = $cxx $in -o $out $libpath $lib");

    std::vector<fs::path> input;
    std::vector<std::string> output;

    for(fs::recursive_directory_iterator it("."), end; it != end; ++it) {
        auto p = it->path();
        if(util::extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
            input.push_back(p);
        }
    }

    fs::path bin("bin");

    if(!fs::is_directory(bin)) {
        fs::create_directory(bin);
    }

    // Generate build sequences

    for(auto&& p : input) {
        auto file_dir = fs::path("obj") / p;
        auto appended_dir = (dir / file_dir).parent_path();
        if(!fs::is_directory(appended_dir)) {
            fs::create_directories(appended_dir);
        }

        std::string output_file = remove_symlink(file_dir.replace_extension(".o"));
        output.push_back(output_file);
        maker.build(remove_symlink(p), output_file, "bd");
    }
    
    // Generate link sequence

    maker.build(util::stringify_list(output), (bin / shinobi.get("PROJECT_NAME", "untitled")).string(), "ld");
}