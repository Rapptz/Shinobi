#ifndef UTIL_PARSER_HPP
#define UTIL_PARSER_HPP

#include <fstream>
#include <unordered_map>
#include "trim.hpp"
#include "config.hpp"

namespace util {
class parser {
private:
    std::ifstream reader;
    bool if_block;
    std::string platform;
    std::unordered_map<std::string, std::string> file;

    bool parse_equal(size_t equals, const std::string& lines) noexcept {
        if(equals != std::string::npos && (equals + 2) < lines.size()) {
            std::string key(lines.data(), lines.data() + equals);
            std::string value(lines.data() + equals + 2, lines.data() + lines.size());
            trim(key);
            trim(value);

            file.emplace(key, value);
            return true;
        }
        return false;
    }

    bool parse_concat(size_t concat, const std::string& lines) noexcept {
        if(concat != std::string::npos && (concat + 2) < lines.size()) {
            std::string key(lines.data(), lines.data() + concat);
            std::string value(lines.data() + concat + 2, lines.data() + lines.size());
            trim(key);
            trim(value);

            if(file.count(key)) {
                file[key].push_back(' ');
                file[key] += value;
                return true;
            }

            file.emplace(key, value);
            return true;
        }

        return false;
    }

    void parse_if_block() noexcept {
        std::string line;
        while(std::getline(reader, line)) {
            auto begin = line.find_first_not_of(" \f\t\v");
            if(begin == std::string::npos)
                continue;
            if(std::string("#").find(line[begin]) != std::string::npos)
                continue;
            if(std::string(";").find(line[begin]) != std::string::npos)
                continue;
            if(std::string("endif").find(line[begin]) != std::string::npos) {
                if_block = false;
                break;
            }

            auto equals = line.find(":=");
            auto concat = line.find("+=");

            if(parse_concat(concat, line)) {
                continue;
            }

            parse_equal(equals, line);
        }
    }
public:
    parser() noexcept: reader("Shinobi"), if_block(false), platform("Other") {}

    bool is_open() const noexcept {
        return reader.is_open();
    }

    void reopen() noexcept {
        reader.close();
        reader.open("Shinobi");
    }

    void parse() noexcept {
        std::string lines;

        #if defined(SHINOBI_WINDOWS)
        platform = "Windows";
        #elif defined(SHINOBI_LINUX)
        platform = "Linux";
        #elif defined(SHINOBI_MACOS)
        platform = "MacOS";
        #endif

        while(std::getline(reader, lines)) {
            auto begin = lines.find_first_not_of(" \f\t\v");
            if(begin == std::string::npos)
                continue;
            if(std::string("#").find(lines[begin]) != std::string::npos)
                continue;
            if(std::string(";").find(lines[begin]) != std::string::npos)
                continue;

            if(lines.find("if") != std::string::npos) {
                if_block = true;
                if(lines.find("if " + platform) != std::string::npos) {
                    parse_if_block();
                }
                continue;
            }

            if(lines.find("endif") != std::string::npos) {
                if_block = false;
                continue;
            }

            auto equals = lines.find(":=");
            auto concat = lines.find("+=");

            if(!if_block && parse_concat(concat, lines)) {
                continue;
            }

            if(!if_block && parse_equal(equals, lines)) {
                continue;
            }
        }
    }

    std::string get(const std::string& key, const std::string& default_value) const noexcept {
        auto it = file.find(key);
        return it != file.end() ? it->second : default_value;
    }

    std::string get_platform() const {
        return platform;
    }

    auto begin() -> decltype(file.begin()) {
        return file.begin();
    }

    auto end() -> decltype(file.end()) {
        return file.end();
    }
};
} // util

#endif // UTIL_PARSER_HPP