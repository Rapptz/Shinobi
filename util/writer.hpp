#ifndef UTIL_WRITER_HPP
#define UTIL_WRITER_HPP

#include <boost/filesystem/path.hpp>
#include <set>
#include "ninja.hpp"
#include "shinobi.hpp"

namespace util {
struct writer {
private:
    ninja file;
    shinobi parser;
    std::set<std::string> input;
    std::set<std::string> output;
    boost::filesystem::path source, object, build;
    boost::filesystem::path dir;
    void create_directories();
    void create_software_file();
    void create_library_file();
    void general_variables();
    void fill_input();
    void build_sequence();
public:
    writer(std::ofstream&);
    void create();
    void debug(bool);
};
} // util

#endif // UTIL_WRITER_HPP