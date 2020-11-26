cmake_minimum_required(VERSION 3.10)

project(qtc-app)

find_package(Qt5Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(sources
 main.cpp
 MainWindow.cpp
)

set(headers
  MainWindow.h
)

add_executable(qtc-app ${sources} ${headers})
target_link_libraries(qtc-app Qt5::Widgets)

install (TARGETS qtc-app DESTINATION bin)
