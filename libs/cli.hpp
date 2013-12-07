// The MIT License (MIT)

// Copyright (c) 2013 Danny Y., Rapptz

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef COMMAND_LINE_INTERFACE_PARSER_HPP
#define COMMAND_LINE_INTERFACE_PARSER_HPP

#include <stdexcept>
#include <string>
#include <boost/lexical_cast.hpp>
#include <memory>
#include <unordered_map>

namespace cli {
class cli_error : public std::runtime_error {
public:
    cli_error(const std::string& name, const std::string& str) noexcept: std::runtime_error(name + ": error: " + str) {}
};

class unrecognised_option : public std::runtime_error {
public:
    unrecognised_option(const std::string& name, const std::string& op) noexcept: 
    std::runtime_error(name + ": error: unrecognised command line option '" + op + '\'') {}
};

class value_needed : public std::runtime_error {
public:
    value_needed(const std::string& name, const std::string& op) noexcept: 
    std::runtime_error(name + ": error: command line option needs a value '" + op + '\'') {}
};

class missing_required_option : public std::runtime_error {
public:
    missing_required_option(const std::string& name, const std::string& op) noexcept: 
    std::runtime_error(name + ": error: missing required command line option '" + op + '\'') {}
};

template<typename T>
struct option_parser {
    std::unique_ptr<T> operator()(const std::string& str) {
        return std::unique_ptr<T>(new T(boost::lexical_cast<T>(str)));
    }
};

struct option_base {
protected:
    std::string longer_name;
    std::string desc;
    char shorter_name;
    bool is_active;
public:
    option_base() = default;
    option_base(std::string name, std::string desc, char shorter = '\0'): 
        longer_name(std::move(name)), 
        desc(std::move(desc)), 
        shorter_name(shorter),
        is_active(false) {}

    virtual ~option_base() = default;
    virtual bool has_value() const = 0;
    virtual bool set() = 0;
    virtual bool set(const std::string&) = 0;
    virtual bool required() const = 0;
    virtual std::unique_ptr<option_base> clone() const = 0;

    virtual bool active() const {
        return is_active;
    }

    virtual const std::string& long_name() const noexcept {
        return longer_name;
    }
 
    virtual const std::string& description() const noexcept {
        return desc;
    }
 
    virtual char short_name() const noexcept {
        return shorter_name;
    }
};

struct flag : public option_base {
    using option_base::option_base;

    virtual bool has_value() const {
        return false;
    }

    virtual bool set() {
        is_active = true;
        return true;
    }

    virtual bool set(const std::string&) {
        return false;
    }

    virtual bool required() const {
        return false;
    }

    virtual std::unique_ptr<option_base> clone() const {
        return std::unique_ptr<option_base>(new flag(longer_name, desc, shorter_name));
    }
};

template<typename T, typename Parser = option_parser<T>>
struct option : public option_base {
private:
    std::unique_ptr<T> actual_value = nullptr;
    bool is_required = false;
public:
    option() = default;
    option(const std::string& name, const std::string& desc, char shorter = '\0', bool is_required = false, T&& def = T()):
        option_base(name, desc, shorter),
        actual_value(new T(std::move(def))),
        is_required(is_required) {}

    virtual bool has_value() const {
        return true;
    }

    virtual bool set() {
        return false;
    }

    virtual bool set(const std::string& value) {
        try {
            actual_value = Parser{}(value);
            is_active = true;
        }
        catch(...) {
            return false;
        }

        return actual_value != nullptr;
    }

    virtual bool required() const {
        return is_required;
    }

    virtual std::unique_ptr<option_base> clone() const {
        return std::unique_ptr<option_base>(new option<T, Parser>(longer_name, desc, shorter_name, is_required, get()));
    }

    T get() const {
        if(actual_value != nullptr)
            return *actual_value;
        throw std::logic_error("dereferencing of null pointer (cli::parser is default constructed)");
    }
};

struct parser {
private:
    std::unordered_map<std::string, std::unique_ptr<option_base>> options;
    std::string name;
    std::string program_usage;

    void parse_option(const std::string& key, const std::string& value) {
        auto it = options.find(key);
        if(it == options.end()) {
            throw unrecognised_option(name, "--" + key);
        }

        if(!it->second->set(value)) {
            throw cli_error(name, "invalid command line option value: '" + value + "'");
        }
    }

    void parse_flag(const std::string& key) {
        auto it = options.find(key);
        if(it == options.end()) {
            throw unrecognised_option(name, "--" + key);
        }

        if(!it->second->set()) {
            throw value_needed(name, "--" + key);
        }
    }
public:
    parser() = default;

    parser& add(const option_base& t) {
        options[t.long_name()] = t.clone();
        return *this;
    }

    bool is_active(const std::string& name) {
        auto it = options.find(name);
        return it != options.end() && it->second->active();
    }

    template<typename T>
    T get(const std::string& name) const {
        auto it = options.find(name);
        if(it != options.end()) {
            if(it->second->has_value()) {
                auto ptr = dynamic_cast<const option<T>*>(it->second.get());
                if(ptr == nullptr) {
                    throw cli_error(name, "unable to retrieve value of '--" + name + "'due to type mismatch.");
                }

                return ptr->get();
            }

            throw cli_error(name, "unable to retrieve value. '--" + name + "' is a flag, not an option.");
        }

        throw cli_error(name, "flag not found: '--" + name + '\'');
    }

    void parse(int argc, char** argv) {
        if(argc < 1 || argv == nullptr) {
            return; // nothing there.
        }

        // program name is required to be argv[0]
        // per the C and C++ standards.
        if(name.empty()) {
            name = argv[0];
        }

        // cache lookup for short characters
        std::unordered_map<char, std::string> cache;
        for(auto&& opt : options) {
            if(opt.first.length() == 0) {
                continue;
            }
            char c = opt.second->short_name();
            if(c) {
                auto it = cache.find(c);
                if(it != cache.end()) {
                    throw cli_error(name, std::string("ambiguous short option given: \"") + c + '"');
                }
                else {
                    cache[c] = opt.first;
                }
            }
        }

        std::string current;

        // actual parsing of command line arguments
        for(int i = 1; i < argc; ++i) {
            current = argv[i];

            if(current == "--")
                break; // no more processing

            // long option (--hello)
            if(current.find("--") == 0) {
                // has value (--hello=test)
                auto pos = current.find('=', 2);
                if(pos != std::string::npos) {
                    std::string key = current.substr(2, pos - 2);
                    std::string value = current.substr(pos + 1);
                    parse_option(key, value);
                    continue;
                }

                // no value (--hello) or (--hello test)
                std::string name(argv[i] + 2);
                auto it = options.find(name);
                if(it != options.end()) {
                    // (--hello test)
                    if(it->second->has_value()) {
                        if(i + 1 >= argc) {
                            throw value_needed(name, "--" + name);
                        }
                        parse_option(name, argv[++i]);
                        continue;
                    }
                    // (--hello)
                    parse_flag(name);
                    continue;
                }

                // long option not found.
                throw unrecognised_option(name, "--" + name);
            }
            // short option (-h)
            else if(current.front() == '-') {
                // loop through
                for(unsigned j = 1; j < current.size(); ++j) {
                    auto it = cache.find(current[j]);
                    if(it != cache.end()) {
                        auto op = options.find(it->second);
                        if(op != options.end()) {
                            if(op->second->has_value()) {
                                // (-ohi)
                                if(j + 1 < current.size()) {
                                    std::string value = current.substr(j + 1);
                                    parse_option(op->first, value);
                                    break;
                                }
                                // (-o hi)
                                if(i + 1 >= argc) {
                                    throw value_needed(name, current);
                                }
                                parse_option(op->first, argv[++i]);
                                continue;
                            }
                            // regular flag (-h)
                            else {
                                parse_flag(op->first);
                            }
                        }
                    }
                    else {
                        throw unrecognised_option(name, std::string("-") + current[j]);
                    }
                }
            }
            else {
                throw unrecognised_option(name, current);
            }
        }
    }


    void check_required_arguments() const {
        for(auto&& arg : options) {
            if(arg.second->required() && !arg.second->active()) {
                throw missing_required_option(name, "--" + arg.first);
            }
        }
    }

    void program_name(std::string prog_name) {
        name = std::move(prog_name);
    }

    std::string program_name() const {
        return name;
    }

    void usage(std::string prog_usage) {
        program_usage = std::move(prog_usage);
    }

    std::string usage() const {
        return program_usage;
    }

    template<typename CharT, typename Elem>
    friend auto operator<<(std::basic_ostream<CharT, Elem>& out, const parser& p) -> decltype(out) {
        out << "usage: " << p.program_name() << ' ' << p.usage() << '\n';
        int spaces;
        for(auto&& arg : p.options) {
            spaces = 30;

            out << "    ";
            auto&& arg_name = arg.second->long_name();
            if(!arg_name.empty()) {
                auto short_name = arg.second->short_name();

                if(short_name != '\0') {
                    out << '-' << short_name << ", ";
                    spaces -= 4;
                }

                out << "--" << arg_name;
                spaces -= 2 + arg_name.size();

                for(int i = 0; i < spaces; ++i) {
                    out << ' ';
                }

                out << arg.second->description() << '\n';
            }
        }

        return out;
    }
};
} // cli

#endif // COMMAND_LINE_INTERFACE_PARSER_HPP