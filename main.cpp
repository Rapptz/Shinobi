#include <util/writer.hpp>
#include <command_line.hpp>
#include <iostream>
#include <iomanip>

namespace cli = gears::command_line;

void show_version() noexcept {
    std::cout << "shinobi version " << SHINOBI_VERSION << '\n';
    std::cout << "Copyright (C) 2013 Rapptz\n"
                 "This is free software; see the source for copying conditions. There is NO\n"
                 "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
}

int main(int argc, char* argv[]) {
    cli::parser po = {
        {"help", "shows this message and exits", 'h'},
        {"version", "shows the version and exits", 'v'},
        {"debug", "creates a debug ninja file", 'd'}
    };

    po.name("shinobi");
    po.usage("[options]");

    try {
        po.parse(argc, argv);

        if(po.is_active("help")) {
            std::cout << po << '\n';
            return 0;
        }
        else if(po.is_active("version")) {
            show_version();
            return 0;
        }

        std::ofstream out("build.ninja");
        util::writer result(out);
        result.debug(po.is_active("debug"));
        result.create();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}