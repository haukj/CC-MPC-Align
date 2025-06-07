cmake_minimum_required(VERSION 3.1)
project(MultiAlignPlugin)

AddPlugin(
    NAME ${PROJECT_NAME}
    DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )

add_subdirectory(src)
add_subdirectory(ui)
