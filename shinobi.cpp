#include <util/maker.hpp>
#include <iostream>
#include <iomanip>

void show_help() noexcept {
    std::cout << "usage: shinobi [options]\n\n";
    std::cout << std::left << '\t' << std::setw(25) << "-h, --help" << "show this message and exit" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-d, --debug" << "create debug ninja file" << '\n';
    std::cout << std::left << '\t' << std::setw(25) << "-r, --release" << "create release ninja file (default)" << '\n';
}

int main(int argc, char* argv[]) {

    std::ofstream out("build.ninja");
    util::maker creator{out};

    std::set<std::string> args{argv, argv + argc};

    if(args.count("-h") || args.count("--help")) {
        show_help();
        return 0;
    }

    if(args.count("-d") || args.count("--debug")) {
        creator.debug(true);
    }

    if(args.count("-r") || args.count("--release")) {
        creator.debug(false);
    }

    creator.regular_parse();
}