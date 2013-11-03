#ifndef UTIL_SHINOBI_HPP
#define UTIL_SHINOBI_HPP

#include <sstream>
#include <fstream>
#include <jsonxx/jsonxx.h>
#include <map>
#include "config.hpp"
#include "errors.hpp"

namespace util {
namespace js = jsonxx;

struct shinobi {
private:
    js::Object json;
    std::fstream file;
    std::map<std::string, std::string> data;

    std::string join_json_list(const js::Array& arr, char delim = ' ') {
        std::ostringstream out;
        auto first = arr.values().cbegin();
        auto last = arr.values().cend();

        if(first != last) {
            out << *(*first++);
        }

        while(first != last) {
            out << delim << *(*first++);
        }
        return out.str();
    }
public:
    shinobi(): file("Shinobi2") {}

    void parse() {
        if(!json.has<js::String>("project")) {
            throw missing_property("project");
        }

        // parse type-independent defaults

        if(json.has<js::Object>("compiler")) {
            auto compiler_object = json.get<js::Object>("compiler");

            if(!compiler_object.has<js::String>("name")) {
                throw shinobi_error("missing 'name' sub-property of 'compiler'");
            }

            data["compiler.name"] = compiler_object.get<js::String>("name");

            if(compiler_object.has<js::Array>("flags")) {
                data["compiler.flags"] = join_json_list(compiler_object.get<js::Array>("flags"));
            }
        }

        if(!json.has<js::String>("type")) {
            throw missing_property("type");
        }
    }
};
} // util

#endif // UTIL_SHINOBI_HPP