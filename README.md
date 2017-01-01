# hwtrycatch

[![Build status](https://ci.appveyor.com/api/projects/status/t1h7qk5g2kvc9xwp/branch/master?svg=true)](https://ci.appveyor.com/project/kutelev/hwtrycatch/branch/master)
[![Build Status](https://travis-ci.org/kutelev/hwtrycatch.svg?branch=master)](https://travis-ci.org/kutelev/hwtrycatch)

## Description ##

**hwtrycatch** provides cross-platform way of catching hardware exceptions (like access violation or division by zero) using standard C++ try/catch blocks. After catching exception application can exit gracefully or continue execution.

Supports single-threaded and multi-threaded environments.

## Supported platforms ##

All major desktop operating systems are supported:

* Linux
* Windows
* Mac OS X

Supported mobile platforms:

* Android
* iOS

## Automatically tested with ##

* GCC 4.8 on Ubuntu 14.04 LTS (Trusty Tahr)
* Visual Studio 2015 on Windows
* Xcode 8 on Mac OS X 10.11 (El Capitan)

## Exceptions which can be caught ##

On Linux and Mac OS X following POSIX signals are caught: SIGBUS, SIGFPE, SIGILL, SIGSEGV.

On Windows similar set is caught. Examples: EXCEPTION_ACCESS_VIOLATION, EXCEPTION_ILLEGAL_INSTRUCTION, EXCEPTION_INT_DIVIDE_BY_ZERO.

## Implementation details ##

Although **hwtrycatch** is written using C++11 under the hood it uses setjmp/longjmp and thread-local storage which are available in pure C.

On non-Windows systems it uses signal handler for catching exceptions, on Windows vectored exception handler is used for similar purposes.
