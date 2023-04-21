
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(sources
  MainWindow.cpp
)

set(headers
  MainWindow.h
)

set(target QtcBase)
add_library(${target} SHARED ${sources} ${headers})
target_link_libraries(${target} Qt5::Widgets)

set_target_properties(${target} PROPERTIES PUBLIC_HEADER MainWindow.h)

install(TARGETS ${target}
  EXPORT qtcapp-export
  LIBRARY DESTINATION lib
  INCLUDES DESTINATION include
  PUBLIC_HEADER DESTINATION include/qtc
)

install(EXPORT qtcapp-export
  FILE qtcapp-config.cmake
  DESTINATION share/cmake/qtcapp/
)
