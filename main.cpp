#include <util/writer.hpp>
#include <iostream>
#include <iomanip>

#define cmd_line(name, desc) std::cout << std::left << '\t' << std::setw(25) << name << desc << '\n'

void show_help() noexcept {
    std::cout << "usage: shinobi [options]\n\n";
    cmd_line("-h, --help", "shows this message and exits");
    cmd_line("-d, --debug", "creates a debug ninja file");
}

void show_version() noexcept {
    std::cout << "shinobi version " << SHINOBI_VERSION << '\n';
    std::cout << "Copyright (C) 2013 Rapptz\n"
                 "This is free software; see the source for copying conditions. There is NO\n"
                 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
}

int main(int argc, char* argv[]) {
    std::set<std::string> args(argv, argv + argc);

    try {
        if(args.count("-h") || args.count("--help")) {
            show_help();
            return 0;
        }
        else if(args.count("-v") || args.count("--version")) {
            show_version();
            return 0;
        }
        else if(args.size() > 1) {
            throw util::shinobi_error("unrecognised command line option '" + std::string(argv[1]) + '\'');
        }

        std::ofstream out("build.ninja");
        util::writer result(out);
        result.create();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}