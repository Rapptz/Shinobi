#ifndef UTIL_PARSER_HPP
#define UTIL_PARSER_HPP

#include <fstream>
#include <unordered_map>
#include <vector>
#include "config.hpp"

namespace util {
class parser {
private:
    std::unordered_map<std::string, std::vector<std::string>> file;
    std::ifstream reader;
    std::string platform;
    bool if_block;
    void parse_if_block() noexcept;
    void parse_assignment(const std::string&, const std::string&) noexcept;
    void parse_append(const std::string&, const std::string&) noexcept;
    void parse_subtract(const std::string&, const std::string&) noexcept;
public:
    bool debug;
    parser() noexcept;
    bool is_open() const noexcept;
    void reopen() noexcept;
    void parse() noexcept;
    std::string get(const std::string&, const std::string&) const noexcept;
    std::vector<std::string> get_list(const std::string&) const noexcept;
    std::string get_platform() const noexcept;
    auto begin() const noexcept -> decltype(file.begin());
    auto end() const noexcept -> decltype(file.end());
};
} // util

#endif // UTIL_PARSER_HPP