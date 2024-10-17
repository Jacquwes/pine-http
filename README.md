# Pine

[![CTest on Windows](https://github.com/Jacquwes/Pine/actions/workflows/cmake.yml/badge.svg?branch=asio)](https://github.com/Jacquwes/Pine/actions/workflows/cmake.yml)

Pine is a modern and lightweight C++20 library for building HTTP servers.

It is multi-threaded and uses asynchronous I/O with coroutines. It is designed 
to be simple to use and easy to integrate into existing 
projects.

The project is still in its early stages and is not yet ready for production 
use. Only Windows is supported at the moment, but Linux support is planned.

No external dependencies are required, except for the standard library and 
Google Test for testing.

## Features

- Asynchronous I/O with coroutines
- Multi-threaded
- HTTP/1.1

## Building

Dependencies: `gtest`

```bash
vcpkg install
```

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path to vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build .
```
