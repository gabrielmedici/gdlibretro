# SPDX-License-Identifier: Unlicense

target_sources( ${PROJECT_NAME}
    PRIVATE
        LibRetroHost.h
        LibRetroHost.cpp
        RegisterExtension.cpp
        RetroHost.hpp
        RetroHost.cpp
        CoreEnvironment.cpp
        Audio.cpp
        Input.cpp
        Video.cpp
        noarch.c
)

target_include_directories( ${PROJECT_NAME}
    PRIVATE
        "src"
)
