#include <util/maker.hpp>
#include <iostream>
#include <iomanip>
#include <map>

void show_help() noexcept {
    std::cout << "usage: shinobi [options] [--] [filename]\n\n";
    std::cout << std::left << '\t' << std::setw(25) << "-h, --help" << "show this message and exit" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-d, --debug" << "create debug ninja file" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-r, --release" << "create release ninja file (default)" << '\n';
}

void show_version() noexcept {
    std::cout << "shinobi version " << SHINOBI_VERSION << '\n';
    std::cout << "Copyright (C) 2013 Rapptz\n"
                 "This is free software; see the source for copying conditions. There is NO\n"
                 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
}

int main(int argc, char* argv[]) {
    std::string filename("build.ninja");
    std::map<std::string, int> args;
    for(int i = 0; i < argc; ++i) {
        args.emplace(argv[i], i);
    }

    if(args.count("-h") || args.count("--help")) {
        show_help();
        return 0;
    }

    if(args.count("-v") || args.count("--version")) {
        show_version();
        return 0;
    }

    auto it = args.find("--");

    if(it != args.end()) {
        if(argv[it->second + 1] != nullptr) {
            filename = std::string(argv[it->second + 1]) + ".ninja";
        }
    }

    std::ofstream out(filename);
    util::maker creator{out};

    if(args.count("-d") || args.count("--debug")) {
        creator.debug(true);
    }

    if(args.count("-r") || args.count("--release")) {
        creator.debug(false);
    }

    creator.regular_parse();
}