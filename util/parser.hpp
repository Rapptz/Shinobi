#ifndef UTIL_PARSER_HPP
#define UTIL_PARSER_HPP

#include <fstream>
#include <unordered_map>
#include "trim.hpp"

namespace util {
class parser {
private:
    std::ifstream reader;
    std::unordered_map<std::string, std::string> file;
public:
    parser() noexcept: reader("Shinobi") {}

    bool is_open() const noexcept {
        return reader.is_open();
    }

    void reopen() noexcept {
        reader.close();
        reader.open("Shinobi");
    }

    void parse() noexcept {
        std::string lines;
        while(std::getline(reader, lines)) {
            auto begin = lines.find_first_not_of(" \f\t\v");
            if(begin == std::string::npos)
                continue;
            if(std::string("#").find(lines[begin]) != std::string::npos)
                continue;
            if(std::string(";").find(lines[begin]) != std::string::npos)
                continue;


            auto equals = lines.find(":=");
            auto concat = lines.find("+=");

            if(concat != std::string::npos && (concat + 2) < lines.size()) {
                std::string key(lines.data(), lines.data() + concat);
                std::string value(lines.data() + concat + 2, lines.data() + lines.size());
                trim(key);
                trim(value);

                if(file.count(key)) {
                    file[key].push_back(' ');
                    file[key] += value;
                    continue;
                }

                file.emplace(key, value);
                continue;
            }

            if(equals != std::string::npos && (equals + 2) < lines.size()) {
                std::string key(lines.data(), lines.data() + equals);
                std::string value(lines.data() + equals + 2, lines.data() + lines.size());
                trim(key);
                trim(value);

                file.emplace(key, value);
            }
        }
    }

    std::string get(const std::string& key, const std::string& default_value) const noexcept {
        auto it = file.find(key);
        return it != file.end() ? it->second : default_value;
    }
};
} // util

#endif // UTIL_PARSER_HPP