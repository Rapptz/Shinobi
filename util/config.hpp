#ifndef UTIL_CONFIG_HPP
#define UTIL_CONFIG_HPP

#if defined(_WIN32) || defined(__WIN32__)
    #define SHINOBI_WINDOWS

#elif defined(linux) || defined(__linux)
    #define SHINOBI_LINUX

#elif defined(__APPLE__) || defined(MACOSX) || defined(macintosh) || defined(Macintosh)
    #define SHINOBI_MACOS

#endif // System-specific

#endif // UTIL_CONFIG_HPP