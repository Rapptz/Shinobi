#include <sol/sol.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <sstream>
#include "shinobi.hpp"
#include "errors.hpp"

namespace fs = boost::filesystem;

namespace util {
void create_default_file(const std::string& filename) {
    std::ofstream out(filename);
    out << R"delim(-- the default shinobi.lua file

-- the project settings
project.name = "untitled";

-- compiler and linker settings
-- with release and debug configuration
if compiler.name == "g++" or compiler.name == "clang++" then
    compiler.flags = { "-std=c++11", "-pedantic", "-pedantic-errors", "-Wextra", "-Wall", "-O2" };
    linker.flags = { "-static" };

    if config.release then 
        table.insert(compiler.flags, "-DNDEBUG");
    else
        table.insert(compiler.flags, "-g");
    end
elseif compiler.name == "msvc" then
    compiler.flags = { "/Za", "/EHsc", "/W3", "/Gm" };

    if config.release then
        table.insert(compiler.flags, "/DNDEBUG");
        table.insert(compiler.flags, "/O2");
    else
        table.insert(compiler.flags, "/Od");
        table.insert(compiler.flags, "/ZI");
        linker.flags = { "/DEBUG" }
    end
end

-- include paths
compiler.paths = { ".", "libs" };

-- directory information
directory.build = "bin";
directory.object = "obj";

-- files to compile
files = os.glob("*.cpp");
)delim";
}

bool ends_with(const std::string& in, const std::string& other) {
    if(in.length() >= other.length())
        return in.compare(in.length() - other.length(), other.length(), other) == 0;
    else
        return false;
}

bool extension_is(const std::string&) {
    return false;
}

template<typename T, typename... Args>
bool extension_is(const std::string& str, T&& t, Args&&... args) {
    return ends_with(str, std::forward<T>(t)) || extension_is(str, std::forward<Args>(args)...);
}

void replace_all(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string sanitise(const fs::path& p) noexcept {
    auto result = p.string();
    // Remove ./
    auto pos = result.find("./");

    if(pos == std::string::npos) {
        pos = result.find(".\\");
    }

    if(pos != std::string::npos) {
        result.replace(pos, 2, "");
    }

    // Replace all \ with /
    replace_all(result, "\\", "/");

    return result;
}

struct glob_predicate {
private:
    boost::regex filter;

    boost::regex glob_to_regex(const std::string& glob) {
        std::ostringstream ss("^"); // start match
        bool literal = false;

        for(auto&& i : glob) {
            bool escape = false;
            switch(i) {
            // escape character
            case '\\':
                if(literal) {
                    ss << '\\' << '\\';
                }
                escape = !literal;
                break;
            // 0 or more
            case '*':
                ss << (literal ? "\\*" : ".*");
                break;
            // 1 only
            case '?':
                ss << (literal ? "\\?" : ".");
                break;
            // dot
            case '.':
                ss << "\\.";
                break;
            // escape /
            case '/':
                ss << "\\/";
                break;
            default:
                ss << i;
                break;
            };

            literal = escape;
        }

        ss << "$"; // end match

        return boost::regex(ss.str());
    }
public:
    glob_predicate() = default;
    glob_predicate(const std::string& glob): filter(glob_to_regex(glob)) {}

    bool operator()(const std::string& str) const {
        return boost::regex_match(str, filter);
    }
};

std::string table_to_string(const sol::table& t, const std::string& prefix = "") {
    std::ostringstream ss;
    const auto size = t.size();
    unsigned index = 1;
    if(size != 1) {
        ss << prefix << t.get<std::string>(index++);
    }

    for(; index <= size; ++index) {
        ss << ' ' << prefix << t.get<std::string>(index);
    }

    return ss.str();
}

template<typename Cont>
std::string flatten_list(const Cont& c) {
    std::ostringstream ss;
    auto first = c.cbegin();
    auto last = c.cend();

    if(first != last) {
        ss << *first++;
    }

    while(first != last) {
        ss << ' ' << *first++;
    }

    return ss.str();
}

namespace os {
void mkdir(std::string dir) {
    try {
        fs::path p(dir);
        if(!fs::exists(p) && fs::is_directory(p)) {
            fs::create_directory(p);
        }
    }
    catch(const std::exception& e) {
        throw shinobi_error("failed to make directory \"" + dir + '"');
    }
}

void mkdirs(std::string dir) {
    try {
        fs::path p(dir);
        if(!fs::exists(p) && fs::is_directory(p)) {
            fs::create_directories(p);
        }
    }
    catch(const std::exception& e) {
        throw shinobi_error("failed to make any or all directories under \"" + dir + '"');
    }
}

void rmdir(std::string dir) {
    try {
        fs::path p(dir);
        if(fs::exists(p) && fs::is_directory(p)) {
            fs::remove(p);
        }
    }
    catch(const std::exception& e) {
        throw shinobi_error("failed to remove directory \"" + dir + "\" (maybe it's not empty?)");
    }
}

void rmdirs(std::string dir) {
    try {
        fs::path p(dir);
        if(fs::exists(p) && fs::is_directory(p)) {
            fs::remove_all(p);
        }
    }
    catch(const std::exception& e) {
        throw shinobi_error("failed to remove directory " + dir);
    }
}
} // os


shinobi::shinobi(std::ostream& out, const std::string& compiler_name, bool release): lua(new sol::state), file(out) {
    lua->open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::os);
    // inject defaults.
    lua->script(R"delim(project = {}
compiler = {}
linker = {}
directory = {}

function constant(table)
    return setmetatable({}, {
        __index = table,
        __newindex = function(table, key, value)
                        error("Attempt to modify constant table")
                     end,
        __metatable = false
    });
end
)delim");
    lua->get<sol::table>("compiler").set("name", compiler_name);
    fill_config_table(release);
    register_functions();
}

shinobi::~shinobi() = default;

bool is_hidden_directory(const fs::path& p) {
    auto name = p.filename().string();
    return name != ".." && name != "." && name.front() == '.';
}

void shinobi::register_functions() {
    // extend OS functionality
    auto os = lua->get<sol::table>("os");
    os.set_function("mkdir", os::mkdir);
    os.set_function("mkdirs", os::mkdirs);
    os.set_function("rmdir", os::rmdir);
    os.set_function("rmdirs", os::rmdirs);
    os.set_function("glob", [this](std::string pattern) {
        auto t = lua->create_table();
        int index = 1;
        glob_predicate pred(pattern);
        try {
            for(fs::recursive_directory_iterator f("."), l; f != l; ++f) {
                auto p = f->path();
                // check if it's a hidden directory
                if(fs::is_directory(p) && is_hidden_directory(p)) {
                    f.no_push();
                    continue;
                }

                if(fs::is_regular_file(p)) {
                    auto file = sanitise(p);
                    if(pred(file)) {
                        t.set(index++, file);
                    }
                }
            }

            return t;
        }
        catch(const std::exception& e) {
            throw shinobi_error("unable to glob with " + pattern);
        }
    });

    #if SHINOBI_WINDOWS
    os.set("name", "windows");
    #elif SHINOBI_LINUX
    os.set("name", "linux")
    #elif SHINOBI_MACOS
    os.set("name", "osx");
    #else
    os.set("name", "unknown");
    #endif

    // extend table functionality
    lua->script(R"script(
function table.find(t, val)
    local i = 1
    for k, v in pairs(t) do
        if v == val then
            return i, k
        else
           i = i + 1
        end
    end
    return nil
end
function table.erase(t, val)
    local i, k = table.find(t, val)
    if i ~= nil then rawset(t, i, nil) end
end
function table.filter(t, func)
    local result = {}
    for _, v in pairs(t) do
        if func(v) then table.insert(result, v) end
    end
    return result
end
function table.map(t, func)
    local result = {}
    for k, v in pairs(t) do
        table.insert(result, func(v))
    end
    return result
end
)script");
}

void shinobi::fill_config_table(bool release) {
    std::string table("config = constant {\n    release = ");
    table += (release ? "true" : "false");
    table += ",\n    debug = ";
    table += (!release ? "true" : "false");
    table += "\n}\n";
    lua->script(table);
}

void shinobi::open_file(const std::string& filename) {
    // default name is shinobi.lua
    if(!fs::exists(filename)) {
        create_default_file(filename);
    }
    lua->open_file(filename);
}

bool shinobi::compiler_linker_tree() {
    std::string compiler_command("$cxx ");
    std::string linker_command("$cxx ");
    auto cxx = lua->get<sol::table>("compiler");
    auto name = cxx.get<std::string>("name");

    if(name == "msvc") {
        name = "cl";
    }

    file.variable("cxx", name);
    const bool is_gcc_like = name == "g++" || name == "clang++" || name == "gcc" || name == "clang";

    if(is_gcc_like) {
        compiler_command += "-MMD -MF $out.d";
    }
    else if(name == "cl") {
        compiler_command += "/nologo /showIncludes";
    }
    else {
        throw shinobi_fatal_error("unsupported compiler found: \"" + name + '\"');
    }

    auto obj = cxx.get<sol::object>("paths"); // include paths
    if(obj.is<sol::table>()) {
        auto cxxpaths = obj.as<sol::table>();
        if(is_gcc_like) {
            file.variable("cxxpaths", table_to_string(cxxpaths, "-I"));
        }
        else {
            file.variable("cxxpaths", table_to_string(cxxpaths, "/I"));
        }
        compiler_command += " $cxxpaths";
    }

    obj = cxx.get<sol::object>("flags"); // compiler flags
    if(obj.is<sol::table>()) {
        auto cxxflags = obj.as<sol::table>();
        file.variable("cxxflags", table_to_string(cxxflags));
        compiler_command += " $cxxflags";
    }

    if(is_gcc_like) {
        compiler_command += " -c $in -o $out";
        linker_command += " $in -o $out";
    }
    else {
        compiler_command += " /c /Fo$out $in";
        linker_command += " /link /nologo /out:$out";
    }

    auto link = lua->get<sol::table>("linker");
    obj = link.get<sol::object>("flags"); // linker flags
    if(obj.is<sol::table>()) {
        auto linkflags = obj.as<sol::table>();
        file.variable("linkerflags", table_to_string(linkflags));
        linker_command += " $linkerflags";
    }

    obj = link.get<sol::object>("path"); // linker paths
    if(obj.is<sol::table>()) {
        auto linkpaths = obj.as<sol::table>();
        if(is_gcc_like) {
            file.variable("linkerpaths", table_to_string(linkpaths, "-L"));
        }
        else {
            file.variable("linkerpaths", table_to_string(linkpaths, "/libpath:"));
        }
        linker_command += " $linkerpaths";
    }

    obj = link.get<sol::object>("libraries"); // libraries to link
    if(obj.is<sol::table>()) {
        auto libraries = obj.as<sol::table>();
        file.variable("libs", table_to_string(libraries));
        linker_command += " $libs";
    }

    if(is_gcc_like) {
        file.newline();
        file.rule("compile", compiler_command, "deps = gcc", "depfile = $out.d", "description = Compiling $in");
        file.newline();
        file.rule("link", linker_command, "description = Creating $out");
        file.newline();
    }
    else {
        linker_command += " @out.rsp";
        file.newline();
        file.rule("compile", compiler_command, "deps = msvc", "description = Compiling $in");
        file.newline();
        file.rule("link", linker_command, "rspfile = $out.rsp", "rspfile_content = $in", "description = Creating $out");
        file.newline();
    }

    return is_gcc_like;
}

void shinobi::build_sequence(const std::string& dir, const bool is_gcc_like) {
    std::string output_file;
    auto current_directory = fs::current_path();
    for(auto&& p : input) {
        auto directory = current_directory / dir / p;
        if(!fs::exists(directory.parent_path())) {
            fs::create_directories(directory.parent_path());
        }

        if(is_gcc_like) {
            output_file = "$objdir/" + sanitise(fs::path(p).replace_extension(".o"));
        }
        else {
            output_file = "$objdir/" + sanitise(fs::path(p).replace_extension(".obj"));
        }

        output.insert(output_file);
        file.build(output_file, p, "compile");
    }
}

void shinobi::fill_input(const sol::table& t) {
    auto o = t.get<sol::object>("files");
    if(!o.is<sol::table>()) {
        throw shinobi_fatal_error("input files missing (did you forget to set the files table?)");
    }

    auto files = o.as<sol::table>();
    for(size_t i = 1; i <= files.size(); ++i) {
        input.insert(files.get<std::string>(i));
    }
}

std::string shinobi::directory() {
    auto dir = lua->get<sol::table>("directory");
    auto build = dir.get<sol::object>("build");
    auto object = dir.get<sol::object>("object");
    std::string bin = build.is<std::string>() ? build.as<std::string>() : "bin";
    std::string obj = object.is<std::string>() ? object.as<std::string>() : "obj";
    file.variable("builddir", bin);
    file.variable("objdir", obj);
    os::mkdirs(bin);
    return obj;
}

void shinobi::create_executable() {
    auto obj = directory();
    fill_input(lua->global_table());
    const bool is_gcc_like = compiler_linker_tree();
    build_sequence(obj, is_gcc_like);
    file.newline();
    auto proj = lua->get<sol::table>("project").get<sol::object>("name");
    std::string name = proj.is<std::string>() ? proj.as<std::string>() : "untitled";
    file.build("$builddir/" + name, flatten_list(output), "link");
    file.newline();
    file.build("install", "$builddir/" + name, "phony");
    file.newline();
    file.set_default("install");  
}

void shinobi::create() {
    file.comment("This file has been generated by shinobi version " SHINOBI_VERSION);
    file.newline();
    file.variable("ninja_required_version", "1.3");
    create_executable();
}
} // util
