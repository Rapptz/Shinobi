#include <util/shinobi.hpp>
#include <cli.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

void show_version() noexcept {
    std::cout << "shinobi version " << SHINOBI_VERSION << '\n';
    std::cout << "Copyright (C) 2013 Rapptz\n"
                 "This is free software; see the source for copying conditions. There is NO\n"
                 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
}

int main(int argc, char* argv[]) {
    cli::parser po;
    po.add(cli::flag("help", "shows this message and exits", 'h'));
    po.add(cli::flag("version", "output version information and exits"));
    po.add(cli::option<std::string>("compiler", "specify compiler to use (default: g++)", 'c', false, "g++"));
    po.add(cli::option<std::string>("output", "specify where to place the output (default: build.ninja)", 
                                    'o', false, "build.ninja"));
    po.add(cli::option<std::string>("file", "specify input file (default: shinobi.lua)", 'f', false, "shinobi.lua"));
    po.add(cli::flag("debug", "create debug configuration ninja file"));
    po.add(cli::flag("release", "create release configuration ninja file (default)"));
    po.program_name("shinobi");
    po.usage("[options] ...");

    try {
        po.parse(argc, argv);
        std::ostringstream ss;

        if(po.is_active("help")) {
            std::cout << po;
            return 0;
        }

        if(po.is_active("version")) {
            show_version();
            return 0;
        }

        bool release = po.is_active("release") || !po.is_active("debug");
        util::shinobi result(ss, po.get<std::string>("compiler"), release);
        result.open_file(po.get<std::string>("file"));
        result.create();

        std::ofstream out(po.get<std::string>("output"));
        out << ss.str();
        return 0;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}