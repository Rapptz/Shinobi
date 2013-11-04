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
    std::string platform;

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

    void parse_software() {
        if(json.has<js::Object>("files")) {
            auto files = json.get<js::Object>("files");

            if(files.has<js::Array>("extra")) {
                data["files.extra"] = prefix_list(files.get<js::Array>("extra"));
            }
            else {
                data["files.extra"] = "";
            }

            if(files.has<js::Array>("ignored")) {
                data["files.ignored"] = prefix_list(files.get<js::Array>("ignored"));
            }

            else {
                data["files.ignored"] = "";
            }

        }
        else {
            data["files.extra"] = "";
            data["files.ignored"] = "";
        }
    }

    void parse_compiler(const js::Object& o) {
        if(o.has<js::Object>("compiler")) {
            auto compiler = o.get<js::Object>("compiler");

            if(!compiler.has<js::String>("name")) {
                throw shinobi_error("missing 'name' sub-property of 'compiler'");
            }

            data["compiler.name"] = compiler.get<js::String>("name");

            if(compiler.has<js::Array>("flags")) {
                data["compiler.flags"] = prefix_list(compiler.get<js::Array>("flags"));
            }
        }
    }

    void parse_linker(const js::Object& o) {
        if(o.has<js::Object>("linker")) {
            auto linker = o.get<js::Object>("linker");

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
    }

    void parse_directory(const js::Object& o) {
        if(o.has<js::Object>("directory")) {
            auto directory = o.get<js::Object>("directory");

            if(directory.has<js::String>("source")) {
                data["directory.source"] = directory.get<js::String>("source");
            }

            if(directory.has<js::String>("build")) {
                data["directory.build"] = directory.get<js::String>("build");
            }

            if(directory.has<js::String>("object")) {
                data["directory.object"] = directory.get<js::String>("object");
            }
        }
    }

    void parse_include(const js::Object& o) {
        if(o.has<js::Array>("include_paths")) {
            data["include.paths"] = prefix_list(o.get<js::Array>("include_paths"));
        }
    }
public:
    shinobi(): file("Shinobi2"), platform("other") {
        #if defined(SHINOBI_WINDOWS)
        platform = "windows";
        #elif defined(SHINOBI_LINUX)
        platform = "linux";
        #elif defined(SHINOBI_MACOS)
        platform = "mac";
        #endif
    }

    void parse() {
        json.parse(file);
        if(!json.has<js::String>("project")) {
            throw missing_property("project");
        }

        // default directory info
        data["directory.source"] = ".";
        data["directory.build"] = "bin";
        data["directory.object"] = "obj";

        // parse type-independent defaults
        parse_compiler(json);
        parse_linker(json);
        parse_directory(json);
        parse_include(json);

        if(!json.has<js::String>("type")) {
            throw missing_property("type");
        }

        if(json.get<js::String>("type") == "software") {
            parse_software();
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