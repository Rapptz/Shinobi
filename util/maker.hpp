#ifndef SHINOBI_UTIL_MAKER_HPP
#define SHINOBI_UTIL_MAKER_HPP

#include "ninja.hpp"
#include "parser.hpp"
#include "string.hpp"
#include <boost/filesystem.hpp>
#include <fstream>
#include <set>
#include <vector>

namespace util {
struct maker {
private:
    std::set<std::string> input;
    std::vector<std::string> output;
    parser shinobi;
    boost::filesystem::path dir;
    boost::filesystem::path bin;
    boost::filesystem::path obj;
    ninja file;
public:
    maker() = default;
    maker(std::ofstream&);
    void debug(bool) noexcept;
    void create_variables() noexcept;
    void fill_source_files() noexcept;
    void regular_parse();
    void create_directories();
};
} // util

#endif // SHINOBI_UTIL_MAKER_HPP