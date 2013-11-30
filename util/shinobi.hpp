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
    } config;
public:
    shinobi(std::ostream& out);
    ~shinobi();
    void parse();
};
} // util

#endif // UTIL_SHINOBI_HPP