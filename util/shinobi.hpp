#ifndef UTIL_SHINOBI_HPP
#define UTIL_SHINOBI_HPP

#include <memory>
#include <set>
#include <string>
#include "ninja.hpp"
#include "config.hpp"

namespace sol {
class state;
} // sol

namespace util {
struct shinobi {
private:
    std::set<std::string> input;
    std::set<std::string> output;
    std::unique_ptr<sol::state> lua;
    ninja file;
    struct {
        bool release;
        bool debug;
    } config;
    void initialise_lua(const std::string& compiler_name);
    void fill_config_table();
    void register_lua_functions();
    void fill_input(const std::string& directory);
    std::string directory();
public:
    shinobi(std::ostream& out);
    ~shinobi();
    void open_file(const std::string& filename);
    void release(bool b);
    void create();
};
} // util

#endif // UTIL_SHINOBI_HPP
