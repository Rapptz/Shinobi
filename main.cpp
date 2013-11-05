#include <util/writer.hpp>
#include <iostream>

int main() {
    std::ofstream out("build.ninja");
    out.unsetf(std::ios::hex);
    util::writer result(out);

    try {
        result.create();
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}