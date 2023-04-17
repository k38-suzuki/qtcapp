cmake_minimum_required(VERSION 3.10)
project(qtcapp VERSION 1.2)

add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_QTCAPP_ROS "Building a qtcapp_ros" OFF)
if(BUILD_QTCAPP_ROS)
  find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy
    std_msgs
    rviz
  )

  find_package(Qt5 ${rviz_QT_VERSION} EXACT REQUIRED
    Core
    Widgets
  )
  set(QT_LIBRARIES Qt5::Widgets)
  add_definitions(-DQT_NO_KEYWORDS)

  catkin_package()
  include_directories(${catkin_INCLUDE_DIRS})
endif()

add_subdirectory(src)
