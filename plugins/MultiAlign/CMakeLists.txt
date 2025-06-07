cmake_minimum_required(VERSION 3.5)
project(MultiAlignPlugin)

# Locate the CloudCompareConfig.cmake in the directory you pass via -DCLOUDCOMPARE_DIR
find_package(CloudCompare REQUIRED
  PATHS "${CLOUDCOMPARE_DIR}"
  NO_DEFAULT_PATH
)

# Qt dependency
find_package(Qt5Widgets REQUIRED)

# Gather sources and headers
file(GLOB SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

# Generate UI wrappers
qt_wrap_ui(UI_FILES
  ui/MultiAlignDlg.ui
)

# Build the plugin as a shared library
add_library(${PROJECT_NAME} SHARED ${SRC} ${UI_FILES})

# Include CloudCompare and Qt headers
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CloudCompare_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

# Link against CloudCompare core libs and Qt
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CloudCompare_LIBRARIES}
    Qt5::Widgets
)

# Set correct suffix for each platform
if(APPLE)
  set(plugin_suffix ".dylib")
elseif(WIN32)
  set(plugin_suffix ".dll")
else()
  set(plugin_suffix ".so")
endif()

# Install the resulting plugin into the standard CC plugin folder
set_target_properties(${PROJECT_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugins"
  SUFFIX "${plugin_suffix}"
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION plugins
)

# Also install the Python helper script
install(PROGRAMS scripts/fgr_multi_align.py
  DESTINATION plugins/MultiAlign/scripts
)
