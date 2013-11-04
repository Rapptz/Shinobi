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
    boost::filesystem::path source, object, build;
    boost::filesystem::path dir;
    void create_directories();
    void create_software_file();
    void software_variables();
    void fill_input();
public:
    writer(std::ofstream&);
    void create();
};
} // util

#endif // UTIL_WRITER_HPP