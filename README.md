# hwtrycatch

[![Build status](https://ci.appveyor.com/api/projects/status/cvvfms6c48opx2j0?svg=true)](https://ci.appveyor.com/project/kutelev/hwtrycatch)
[![Build Status](https://travis-ci.org/kutelev/hwtrycatch.svg?branch=master)](https://travis-ci.org/kutelev/hwtrycatch)
[![pipeline status](https://gitlab.com/kutelev/hwtrycatch/badges/master/pipeline.svg)](https://gitlab.com/kutelev/hwtrycatch/pipelines/master/latest)

## Description ##

**hwtrycatch** provides cross-platform way of catching hardware exceptions (like access violation or division by zero) using standard C++ try/catch blocks. After catching exception application can exit gracefully or continue execution.

Supports single-threaded and multi-threaded environments.

## Supported platforms ##

All major desktop operating systems are supported:

* Linux
* Windows
* Mac OS X

Following mobile platforms are supposed to work but tests are not run:

* Android
* iOS

## Automatically tested on ##

Linux:
* GCC 4.8.x on Ubuntu 14.04 LTS
* GCC 5.4.x on Ubuntu 16.04 LTS

Windows:
* Visual Studio 2015
* Visual Studio 2017

macOS:
* Xcode 8
* Xcode 8.3
* Xcode 9
* Xcode 9.4
* Xcode 10
* Xcode 10.1

## Exceptions which can be caught ##

On Linux and Mac OS X following POSIX signals are caught: SIGBUS, SIGFPE, SIGILL, SIGSEGV.

On Windows similar set is caught. Examples: EXCEPTION_ACCESS_VIOLATION, EXCEPTION_ILLEGAL_INSTRUCTION, EXCEPTION_INT_DIVIDE_BY_ZERO.

## Implementation details ##

Although **hwtrycatch** is written using C++11 under the hood it uses setjmp/longjmp and thread-local storage which are available in pure C.

On non-Windows systems it uses signal handler for catching exceptions, on Windows vectored exception handler is used for similar purposes.
