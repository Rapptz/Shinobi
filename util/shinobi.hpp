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
    js::Array libraries;
    std::fstream file;
    std::map<std::string, std::string> data;
    std::string platform;
    std::string compiler;
    bool debug;

    void erase_all(std::string& str, const std::string& erase) {
        size_t start_pos = 0;
        while((start_pos = str.find(erase, start_pos)) != std::string::npos) {
            str.replace(start_pos, erase.length(), "");
        }
    }

    std::string prefix_list(const js::Array& arr, bool sanitise = false, const std::string& prefix = std::string()) {
        std::ostringstream out;
        auto first = arr.values().cbegin();
        auto last = arr.values().cend();
        char delim[2] = {'\0', '\0'};

        while(first != last) {
            out << delim << prefix << *(*first++);
            delim[0] = ' ';
        }

        std::string result = out.str();

        // un-escape \/ to /
        size_t pos = 0;
        while((pos = result.find("\\/", pos)) != std::string::npos) {
            result.replace(pos, 2, "/");
        }

        if(sanitise) {
            erase_all(result, "\"");
            return result;
        }

        return result;
    }

    void parse_files(const js::Object& o) {
        if(o.has<js::Object>("files")) {
            auto files = o.get<js::Object>("files");

            if(files.has<js::Array>("extra")) {
                data["files.extra"] = prefix_list(files.get<js::Array>("extra"));
            }

            if(files.has<js::Array>("ignored")) {
                data["files.ignored"] = prefix_list(files.get<js::Array>("ignored"));
            }
        }
    }

    void parse_compiler(const js::Object& o) {
        if(o.has<js::Object>(compiler)) {
            auto comp = o.get<js::Object>(compiler);

            if(comp.has<js::Array>("flags")) {
                data["compiler.flags"] = prefix_list(comp.get<js::Array>("flags"), true);
            }
        }
        else if(compiler == "g++" || compiler == "clang++" || compiler == "clang" || compiler == "gcc") {
            if(o.has<js::Object>("non-msvc")) {
                auto comp = o.get<js::Object>("non-msvc");

                if(comp.has<js::Array>("flags")) {
                    data["compiler.flags"] = prefix_list(comp.get<js::Array>("flags"), true);
                }
            }
        }
    }

    void parse_linker(const js::Object& o) {
        if(o.has<js::Object>("linker")) {
            auto linker = o.get<js::Object>("linker");

            if(linker.has<js::Array>("flags")) {
                data["linker.flags"] = prefix_list(linker.get<js::Array>("flags"), true);
            }

            if(linker.has<js::Array>("libraries")) {
                data["linker.libraries"] = prefix_list(linker.get<js::Array>("libraries"), true);
            }

            if(linker.has<js::Array>("library_paths")) {
                if(!compiler.empty() && compiler != "cl") {
                    data["linker.library_paths"] = prefix_list(linker.get<js::Array>("library_paths"), false, "-L");
                }
                else {
                    data["linker.library_paths"] = prefix_list(linker.get<js::Array>("library_paths"));
                }
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
            if(!compiler.empty() && compiler != "cl") {
                data["include.paths"] = prefix_list(o.get<js::Array>("include_paths"), false, "-I");
            }
            else {
                data["include.paths"] = prefix_list(o.get<js::Array>("include_paths"), false, "/I");
            }
        }
    }

    void parse_archive(const js::Object& o) {
        if(o.has<js::Object>("archive")) {
            auto ar = o.get<js::Object>("archive");
            data["archive.name"] = ar.get<js::String>("name", "ar");
            data["archive.options"] = ar.get<js::String>("options", "rcs");
        }
    }

    void parse_helper(const js::Object& o) {
        parse_compiler(o);
        parse_linker(o);
        parse_include(o);
        parse_directory(o);
        parse_files(o);

        if(is_library()) {
            parse_archive(o);
        }

        if(debug) {
            parse_subtree(o, "debug");
        }
        else {
            parse_subtree(o, "release");
        }

        parse_subtree(o, platform);
    }

    void parse_subtree(const js::Object& o, const std::string& str) {
        if(o.has<js::Object>(str)) {
            auto sub = o.get<js::Object>(str);
            parse_helper(sub);
        }
    }
public:
    shinobi(): file("Shinobi2"), platform("other"), debug(false) {
        #if defined(SHINOBI_WINDOWS)
        platform = "windows";
        #elif defined(SHINOBI_LINUX)
        platform = "linux";
        #elif defined(SHINOBI_MACOS)
        platform = "osx";
        #endif
    }

    void parse() {
        if(!json.parse(file)) {
            throw shinobi_error("unable to parse file");
        }

        file.unsetf(std::ios::hex);

        // required settings
        if(!json.has<js::Object>("project")) {
            throw missing_property("project");
        }

        auto project = json.get<js::Object>("project");
        if(!project.has<js::String>("type")) {
            throw missing_property("type");
        }

        // required project info
        data["project.name"]    = project.get<js::String>("name", "untitled");
        data["project.type"]    = project.get<js::String>("type");
        data["project.version"] = project.get<js::String>("version", "1.0.0");

        // default directory info
        data["directory.source"] = ".";
        data["directory.build"]  = "bin";
        data["directory.object"] = "obj";

        // default file info
        data["files.extra"]   = "";
        data["files.ignored"] = "";

        // default archive info
        data["archive.name"]    = "ar";
        data["archive.options"] = "rcs";

        parse_helper(json);

        if(is_library()) {
            if(!json.has<js::Array>("libraries")) {
                throw missing_property("libraries");
            }
            libraries = json.get<js::Array>("libraries");
        }

        if(compiler.empty()) {
            throw shinobi_error("no compiler name provided");
        }
    }

    const std::string& database(const std::string& key) const {
        return data.find(key)->second;
    }

    bool in_database(const std::string& key) const noexcept {
        auto it = data.find(key);
        return it != data.end() && !it->second.empty();
    }

    const std::string& compiler_name() const noexcept {
        return compiler;
    }

    shinobi& compiler_name(std::string str) noexcept {
        compiler = std::move(str);
        return *this;
    }

    const std::string& platform_name() const noexcept {
        return platform;
    }

    bool is_msvc() const {
        return compiler == "cl";
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
        return data.find("project.type")->second == "library";
    }

    bool is_software() const {
        return data.find("project.type")->second == "software";
    }

    auto library_count() const -> decltype(libraries.size()) {
        return libraries.size();
    }

    void parse_library(unsigned index) {
        // parses library on the fly
        if(!libraries.has<js::Object>(index)) {
            throw shinobi_error("unable to retrieve library at index " + std::to_string(index));
        }

        auto lib = libraries.get<js::Object>(index);

        if(!lib.has<js::String>("name")) {
            throw no_library_name(index);
        }

        parse_helper(lib);
    }

    std::string library_name(unsigned index) {
        return libraries.get<js::Object>(index).get<js::String>("name");
    }

    bool is_static_library(unsigned index) {
        return libraries.get<js::Object>(index).get<bool>("static", true);
    }

    void enable_debug() {
        debug = true;
    }

    void disable_debug() {
        debug = false;
    }

    void reopen() noexcept {
        file.close();
        file.open("Shinobi2");
    }
};
} // util

#endif // UTIL_SHINOBI_HPP