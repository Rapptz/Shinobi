#ifndef SHINOBI_UTIL_CONFIG_HPP
#define SHINOBI_UTIL_CONFIG_HPP

#define SHINOBI_VERSION_MAJOR 0
#define SHINOBI_VERSION_MINOR 9
#define SHINOBI_VERSION_PATCH 5
#define SHINOBI_VERSION "0.9.5"

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#define SHINOBI_WINDOWS

#elif defined(linux) || defined(__linux)
#define SHINOBI_LINUX

#elif defined(__APPLE__) || defined(__MACH__) || defined(macintosh) || defined(Macintosh)
#define SHINOBI_MACOS

#endif // System-specific

#endif // SHINOBI_UTIL_CONFIG_HPP