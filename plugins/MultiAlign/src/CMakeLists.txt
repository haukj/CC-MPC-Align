add_library(${PROJECT_NAME} SHARED
    MultiAlignPlugin.cpp
    MultiAlignTool.cpp
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    cc_plugin_std
    )
