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

    std::string prefix_list(const js::Array& arr, const std::string& prefix = std::string()) {
        std::ostringstream out;
        auto first = arr.values().cbegin();
        auto last = arr.values().cend();

        if(first != last) {
            out << prefix << *(*first++);
        }

        while(first != last) {
            out << ' ' << prefix << *(*first++);
        }
        return out.str();
    }
public:
    shinobi(): file("Shinobi2") {}

    void parse() {
        json.parse(file);
        if(!json.has<js::String>("project")) {
            throw missing_property("project");
        }

        // parse type-independent defaults

        if(json.has<js::Object>("compiler")) {
            auto compiler = json.get<js::Object>("compiler");

            if(!compiler.has<js::String>("name")) {
                throw shinobi_error("missing 'name' sub-property of 'compiler'");
            }

            data["compiler.name"] = compiler.get<js::String>("name");

            if(compiler.has<js::Array>("flags")) {
                data["compiler.flags"] = prefix_list(compiler.get<js::Array>("flags"));
            }
        }

        if(json.has<js::Object>("linker")) {
            auto linker = json.get<js::Object>("linker");

            if(linker.has<js::Array>("flags")) {
                data["linker.flags"] = prefix_list(linker.get<js::Array>("flags"));
            }

            if(linker.has<js::Array>("libraries")) {
                data["linker.libraries"] = prefix_list(linker.get<js::Array>("libraries"));
            }

            if(linker.has<js::Array>("library_paths")) {
                data["linker.library_paths"] = prefix_list(linker.get<js::Array>("library_paths"));
            }
        }

        if(json.has<js::Array>("include_paths")) {
            data["include.paths"] = prefix_list(json.get<js::Array>("include_paths"));
        }

        if(json.has<js::Object>("directory")) {
            auto directory = json.get<js::Object>("directory");
            data["directory.source"] = directory.get<js::String>("source", ".");
            data["directory.build"] = directory.get<js::String>("build", "bin");
            data["directory.object"] = directory.get<js::String>("object", "obj");
        }
        else {
            // default values
            data["directory.source"] = ".";
            data["directory.build"] = "bin";
            data["directory.object"] = "obj";
        }

        if(!json.has<js::String>("type")) {
            throw missing_property("type");
        }
    }

    const std::string& database(const std::string& key) const {
        return data.find(key)->second;
    }

    std::string& database(const std::string& key) {
        return data.find(key)->second;
    }

    bool in_database(const std::string& key) const noexcept {
        return data.find(key) != data.end();
    }

    template<typename T>
    const T& get(const std::string& key) const {
        return json.get<T>(key);
    }

    template<typename T>
    T& get(const std::string& key) {
        return json.get<T>(key);
    }

    bool is_open() const noexcept {
        return file.is_open();
    }

    bool is_library() const {
        return file.find("type")->second == "library";
    }

    bool is_software() const {
        return file.find("type")->second == "software";
    }

    void reopen() noexcept {
        file.close();
        file.open("Shinobi2");
    }
};
} // util

#endif // UTIL_SHINOBI_HPP