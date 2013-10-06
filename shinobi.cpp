#include <boost/filesystem.hpp>
#include <util/parser.hpp>
#include <util/string.hpp>
#include <util/ninja.hpp>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <set>

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

void show_help() noexcept {
    std::cout << "usage: shinobi [options]\n\n";
    std::cout << std::left << '\t' << std::setw(25) << "-h, --help" << "show this message and exit" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-d, --debug" << "create debug ninja file" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-r, --release" << "create release ninja file (default)" << '\n';
}

int main(int argc, char* argv[]) {
    auto dir = fs::current_path();
    util::parser shinobi;

    if(!shinobi.is_open()) {
        make_default_shinobi();
        shinobi.reopen();
    }

    std::set<std::string> args(argv, argv + argc);

    if(args.count("-h") || args.count("--help")) {
        show_help();
        return 0;
    }

    if(args.count("-d") || args.count("--debug")) {
        shinobi.debug = true;
    }

    if(args.count("-r") || args.count("--release")) {
        shinobi.debug = false;
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

    auto input_dirs = shinobi.get_list("SRCDIR");

    if(input_dirs.empty()) {
        input_dirs.push_back(".");
    }

    for(auto&& d : input_dirs) {
        for(fs::recursive_directory_iterator it(d), end; it != end; ++it) {
            auto p = it->path();
            if(util::extension_is(p.string(), ".cpp", ".cxx", ".cc", ".c", ".c++")) {
                input.push_back(p);
            }
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