# suzza-reflect-tool

A simple C++ metadata registration code generation tool based on LLVM & Clang.

## Introduction
suzza-reflect-tool is a standalone reflection generator built on top of LLVM/Clang.
It parses C++ source files via Clang AST, automatically extracts type information,
and generates C++ reflection metadata code for Game Engine.

The source code and reflection code generation tool in this project were independently implemented by myself,
and I have not read or copied the source code of Unreal Engine UHT (Unreal Header Tool).

This project only refers to the architectural ideas of modern game engines and independently implements a simple offline metadata reflection system;

This project adopts the MIT License, as detailed in the LICENSE document.

## Features
- Parse C++ source files using Clang AST
- Generate reflection metadata files to specified output directory
- Consume `compile_commands.json` to reconstruct exact compilation flags
- Support incremental reflection generation
- Standalone executable (`.exe` on Windows)

## Dependencies
- LLVM + Clang (Recommended version: 18 or higher)
- CMake 3.8+
- C++20 or later compiler

## Build
```bash
git clone https://github.com/KomdSpecific/suzza-reflect-tool.git
cd suzza-reflect-tool
mkdir build && cd build
cmake ..
cmake --build . --config Release
