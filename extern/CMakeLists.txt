add_subdirectory( godot-cpp )
add_subdirectory( yaml-cpp )

target_include_directories( ${PROJECT_NAME} PUBLIC libretro-common/include )