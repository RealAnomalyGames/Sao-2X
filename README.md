
# Sao-2X

**A Neptune Corporation Game Engine**

Sao-2X is a custom general-purpose 2D and 3D game engine developed by Neptune Corporation.

## Specifications

- Engine programming language: C++
- C++ standard: C++17
- Game scripting language: Lua (planned)
- Build system: CMake
- CI/CD: GitHub Actions
- Distribution: GitHub Releases

## Target Platforms

- iPhone OS 2+
- Android 1+
- Windows 95+

## Development Platforms

Initial automated development builds:

- Windows x64
- Linux x64

## Building

Requirements:
- CMake 3.21+
- C++17 compiler
- Ninja (optional)

### Windows

Run:

    scripts\build.bat

### Linux

Run:

    chmod +x scripts/build.sh
    ./scripts/build.sh

## Project Structure

- engine/ - Engine source code
- editor/ - Visual editor
- builder/ - Game packaging tools
- exporters/ - Platform exporters
- scripting/ - Game scripting integration
- examples/ - Example projects
- tests/ - Engine tests
- docs/ - Documentation

## Version

See VERSION.

## Developer

Neptune Corporation

## Status

Sao-2X is currently in early development.
