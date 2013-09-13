#include <boost/filesystem.hpp>
#include <util/parser.hpp>
#include <util/string.hpp>
#include <util/ninja.hpp>
#include <algorithm>
#include <iostream>

namespace fs = boost::filesystem;

void make_default_shinobi() {
    std::ofstream out("Shinobi");
    out << "# Project name is given by the PROJECT_NAME variable.\n"
           "# Compiler name is given by the CXX variable.\n"
           "# Compiler flags are given by the CXXFLAGS variable.\n"
           "# Include flags are given by the INCLUDE_FLAGS variable. These are prefixed by -I\n"
           "# Library paths are given by the LIBRARY_PATHS variable. These are preixed by -L\n"
           "# Library flags are given by the LIBRARY_FLAGS variable. These are prefixed by -l\n"
           "# Preprocessor defines are given by the DEFINES variable. These are prefixed by -D\n"
           "# These are all the variables Shinobi recognises. You can assign with :=\n"
           "# and concatenate or append with +=. Comments are done by # and ; characters\n"
           "# they are only ignored if they appear in the beginning.\n"
           "# Variables containing lists, like CXXFLAGS, require a space to be the delimiter.\n"
           "# Currently only g++ and clang++ compilers are supported.\n"
           "# Along with .cpp, .cc, .cxx and .c++ files (case sensitive).\n\n\n"
           "PROJECT_NAME += untitled\n"
           "CXX += g++\n"
           "CXXFLAGS += -std=c++11 -pedantic -pedantic-errors -Wextra -Wall -O3 -static-libstdc++\n"
           "INCLUDE_FLAGS := -I.\n"
           "LIBRARY_PATHS :=\n"
           "LIBRARY_FLAGS :=\n"
           "DEFINES := -DNDEBUG";
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

    maker.rule("bd", "${cxx} ${cxxflags} ${def} -c ${in} -o ${out} ${incflags}");
    maker.rule("ld", "${cxx} ${in} -o ${out} ${libpath} ${lib}");

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