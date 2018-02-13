# noarch [![Build Status](https://travis-ci.org/RobLoach/noarch.svg?branch=master)](https://travis-ci.org/RobLoach/noarch)

**noarch** is a small [libretro](https://www.libretro.com/) frontend that doesn't provide any video, audio or input. It loads a Core, along with its Content, runs an iteration, and then quits. This is useful for unit testing.

## Building

Use `make`, `pkg-config`, along with a working C99 or C++ compiler.

## Running

    ./noarch <core> <uncompressed content>
