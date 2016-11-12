#ifndef UUID_0042E59A_8219_4278_BF6D_96F6CFCAE63A
#define UUID_0042E59A_8219_4278_BF6D_96F6CFCAE63A

#if defined(__clang__)
#define PLATFORM_COMPILER_CLANG
#elif defined(__GNUC__)
#define PLATFORM_COMPILER_GCC
#elif defined(_MSC_VER)
#define PLATFORM_COMPILER_MSVC
#else
#error "Unsupported compiler. Supported compilers are: MSVC, GCC, Clang."
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if defined(__linux__)
#define PLATFORM_OS_LINUX
#elif defined(_WIN32) || defined(_WIN64)
#define PLATFORM_OS_WINDOWS
#elif defined(__APPLE__) && TARGET_OS_MAC == 1
#define PLATFORM_OS_MAC_OS_X
#else
#error "Unsupported OS. Supported operating systems are: Windows, Linux, Mac OS X."
#endif

#if defined(PLATFORM_OS_WINDOWS) && !defined(PLATFORM_COMPILER_MSVC)
#error "Unsupported combination of compiler and operating system."
#endif

#endif