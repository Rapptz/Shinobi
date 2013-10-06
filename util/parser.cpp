#include "parser.hpp"
#include "string.hpp"
#include <boost/xpressive/xpressive.hpp>
#include <boost/xpressive/regex_actions.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>

namespace xp = boost::xpressive;

static const xp::sregex var_names = xp::as_xpr("LIBS") | "PROJECT_NAME"  | "BUILDDIR"  | "CXX"        | "CXXFLAGS" |
                                    "DEFINES"          | "INCLUDE_FLAGS" | "LIB_PATHS" | "LINK_FLAGS" | "OBJDIR"   |
                                    "SRCDIR";

static const xp::sregex bools = xp::icase("Windows") | xp::icase("MacOS")   | xp::icase("Linux") | 
                                xp::icase("Debug")   | xp::icase("Release");

// \s*(vars)\s*:=\s*(.+)
static const xp::sregex assign = *xp::_s >> (xp::s1 = var_names) >> *xp::_s >> ":=" >> *xp::_s >> (xp::s2 = +xp::_);

// \s*(vars)\s*\+=\s*(.+)
static const xp::sregex append = *xp::_s >> (xp::s1 = var_names) >> *xp::_s >> "+=" >> *xp::_s >> (xp::s2 = +xp::_);

// \s*(vars)\s*-=\s*(.+)
static const xp::sregex subtract = *xp::_s >> (xp::s1 = var_names) >> *xp::_s >> "-=" >> *xp::_s >> (xp::s2 = +xp::_);

// \s*if\s*(bools)\s*
static const xp::sregex if_stm = *xp::_s >> "if" >> *xp::_s >> xp::as_xpr('(') >> (xp::s1 = bools) >> 
                                                               xp::as_xpr(')') >> *xp::_s;

// \s*(endif)\s*
static const xp::sregex endif = *xp::_s >> (xp::s1 = xp::as_xpr("endif")) >> *xp::_s;

// [;#].+
static const xp::sregex comment = (xp::set = ';', '#') >> +xp::_;

namespace util {
parser::parser() noexcept: reader("Shinobi"), if_block(false), debug(false) {
    #if defined(SHINOBI_WINDOWS)
    platform = "windows";

    #elif defined(SHINOBI_LINUX)
    platform = "linux";

    #elif defined(SHINOBI_MACOS)
    platform = "macos";

    #endif
}

bool parser::is_open() const noexcept {
    return reader.is_open();
}

void parser::reopen() noexcept {
    reader.close();
    reader.open("Shinobi");
}

void parser::parse_assignment(const std::string& key, const std::string& value) noexcept {
    std::istringstream in(value);

    std::string temp;

    file[key].clear();

    while(in >> temp) {
        file[key].emplace_back(temp);
    }
}

void parser::parse_append(const std::string& key, const std::string& value) noexcept {
    std::istringstream in(value);

    std::string temp;

    while(in >> temp) {
        file[key].emplace_back(temp);
    }
}

void parser::parse_subtract(const std::string& key, const std::string& value) noexcept {

    std::vector<std::string> temp;
    // (\S+)
    static const xp::sregex non_space = (xp::s1 = (+(~xp::_s)))[xp::ref(temp)->*xp::push_back(xp::s1)];

    static const xp::sregex action = non_space >> *(+xp::_s >> non_space);

    auto it = file.find(key);

    auto f = [&temp](const std::string& str) -> bool {
        return std::find(std::begin(temp), std::end(temp), str) != std::end(temp);
    };

    if(it != file.end()) {
        xp::regex_match(value, action);
        it->second.erase(std::remove_if(it->second.begin(), it->second.end(), f), it->second.end());
    }
}

void parser::parse_if_block() noexcept {
    std::string lines;
    xp::smatch what;

    while(std::getline(reader, lines)) {
        if(xp::regex_match(lines, what, endif)) {
            if_block = false;
            break;
        }

        if(xp::regex_match(lines, what, assign)) {
            parse_assignment(what[1].str(), what[2].str());
            continue;
        }

        if(xp::regex_match(lines, what, append)) {
            parse_append(what[1].str(), what[2].str());
            continue;
        }

        if(xp::regex_match(lines, what, subtract)) {
            parse_subtract(what[1].str(), what[2].str());
        }
    }
}

void parser::parse() noexcept {
    std::string lines;
    xp::smatch what;

    while(std::getline(reader, lines)) {
        if(xp::regex_match(lines, what, comment))
            continue;

        // If block parsing
        
        if(xp::regex_match(lines, what, if_stm)) {
            if_block = true;
            const std::string& x = what[1].str();
            bool debug_found = (debug && boost::iequals(x, std::string("debug")));
            bool release_found = (!debug && boost::iequals(x, std::string("release")));
            if(boost::iequals(x, platform) || debug_found || release_found) {
                parse_if_block();
            }
            continue;
        }

        if(xp::regex_match(lines, what, endif)) {
            if_block = false;
            continue;
        }

        if(!if_block && xp::regex_match(lines, what, assign)) {
            parse_assignment(what[1].str(), what[2].str());
            continue;
        }

        if(!if_block && xp::regex_match(lines, what, append)) {
            parse_append(what[1].str(), what[2].str());
            continue;
        }

        if(!if_block && xp::regex_match(lines, what, subtract)) {
            parse_subtract(what[1].str(), what[2].str());
        }
    }
}

std::string parser::get(const std::string& key, const std::string& default_value) const noexcept {
    auto it = file.find(key);
    return it != file.end() ? stringify_list(it->second) : default_value;
}

std::vector<std::string> parser::get_list(const std::string& key) const noexcept {
    auto it = file.find(key);
    return it != file.end() ? it->second : std::vector<std::string>{};
}

std::string parser::get_platform() const {
    return platform;
}

auto parser::begin() -> decltype(file.begin()) {
    return file.begin();
}

auto parser::end() -> decltype(file.end()) {
    return file.end();
}
} // util