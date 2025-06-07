cmake_minimum_required(VERSION 3.5)

project(MultiAlignPlugin)

find_package(CloudCompare REQUIRED
  PATHS "${CLOUDCOMPARE_DIR}"
  NO_DEFAULT_PATH
)
find_package(Qt5Widgets REQUIRED)

file(GLOB SRC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
)

qt_wrap_ui(UI_FILES
  ui/MultiAlignDlg.ui
)

add_library(${PROJECT_NAME} SHARED ${SRC} ${UI_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CLOUDCOMPARE_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  ${CLOUDCOMPARE_LIBRARIES}
  Qt5::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugins"
  SUFFIX ".so"
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION plugins
)

install(PROGRAMS scripts/fgr_multi_align.py
  DESTINATION plugins/MultiAlign/scripts
)
