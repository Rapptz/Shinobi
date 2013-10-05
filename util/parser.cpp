#include "parser.hpp"
#include <boost/xpressive/xpressive.hpp>
#include <boost/algorithm/string/predicate.hpp>

namespace xp = boost::xpressive;

static const xp::sregex var_names = xp::as_xpr("LIBS") | "PROJECT_NAME"  | "BUILDDIR"  | "CXX"        | "CXXFLAGS" |
                                    "DEFINES"          | "INCLUDE_FLAGS" | "LIB_PATHS" | "LINK_FLAGS" | "OBJDIR"   |
                                    "SRCDIR";

static const xp::sregex bools = xp::icase("Windows") | xp::icase("MacOS") | xp::icase("Linux") | xp::icase("Debug");

// \s*(vars)\s*:=\s*(.+)
static const xp::sregex assign = *xp::_s >> (xp::s1 = var_names) >> *xp::_s >> ":=" >> *xp::_s >> (xp::s2 = +xp::_);

// \s*(vars)\s*\+=\s*(.+)
static const xp::sregex append = *xp::_s >> (xp::s1 = var_names) >> *xp::_s >> "+=" >> *xp::_s >> (xp::s2 = +xp::_);

// \s*if\s*(bools)\s*
static const xp::sregex if_stm = *xp::_s >> "if" >> *xp::_s >> xp::as_xpr('(') >> (xp::s1 = bools) >> 
                                                               xp::as_xpr(')') >> *xp::_s;

// \s*(endif)\s*
static const xp::sregex endif = *xp::_s >> (xp::s1 = xp::as_xpr("endif")) >> *xp::_s;

// [;#].+
static const xp::sregex comment = (xp::set = ';', '#') >> +xp::_;

namespace util {
parser::parser() noexcept: reader("Shinobi"), debug(false) {
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

void parser::parse_if_block() noexcept {
    std::string lines;
    xp::smatch what;

    while(std::getline(reader, lines)) {
        if(xp::regex_match(lines, what, assign)) {
            file.emplace(what[1], what[2]);
            continue;
        }

        if(xp::regex_match(lines, what, append)) {
            if(file.count(what[1])) {
                file[what[1]] += ' ' + what[2];
                continue;
            }
            file.emplace(what[1], what[2]);
        }

        if(xp::regex_match(lines, what, endif)) {
            break;
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
            const std::string& x = what[1].str();
            if(boost::iequals(x, platform) || (debug && boost::iequals(x, std::string("debug")))) {
                parse_if_block();
            }
        }

        if(xp::regex_match(lines, what, assign)) {
            file.emplace(what[1].str(), what[2].str());
            continue;
        }

        if(xp::regex_match(lines, what, append)) {
            if(file.count(what[1].str())) {
                file[what[1].str()] += ' ' + what[2].str();
                continue;
            }
            file.emplace(what[1].str(), what[2].str());
        }
    }
}

std::string parser::get(const std::string& key, const std::string& default_value) const noexcept {
    auto it = file.find(key);
    return it != file.end() ? it->second : default_value;
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