#ifndef UTIL_SHINOBI_HPP
#define UTIL_SHINOBI_HPP

#include <memory>
#include <set>
#include <string>
#include "ninja.hpp"
#include "config.hpp"

namespace sol {
class state;
class table;
} // sol

namespace util {
struct shinobi {
private:
    std::set<std::string> input;
    std::set<std::string> output;
    std::unique_ptr<sol::state> lua;
    ninja file;
    void fill_config_table(bool release);
    void register_functions();
    void fill_input(const sol::table& t);
    void build_sequence(const std::string& dir, const bool is_gcc_like);
    void create_executable();
    bool compiler_linker_tree();
    std::string directory();
public:
    shinobi(std::ostream& out, const std::string& compiler_name, bool release);
    ~shinobi();
    void open_file(const std::string& filename);
    void create();
};
} // util

#endif // UTIL_SHINOBI_HPP
