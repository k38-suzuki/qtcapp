cmake_minimum_required(VERSION 3.0.2)
project(rqt_netem)

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  std_msgs
  roscpp
  rqt_gui
  rqt_gui_cpp
)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES rqt_netem
  CATKIN_DEPENDS std_msgs roscpp rqt_gui rqt_gui_cpp
#  DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)
link_directories(${catkin_LIBRARY_DIRS})

set(sources
  src/${PROJECT_NAME}/my_plugin.cpp
  src/${PROJECT_NAME}/mainwindow.cpp
  src/${PROJECT_NAME}/bash.cpp
)

set(headers
  include/${PROJECT_NAME}/my_plugin.h
  include/${PROJECT_NAME}/mainwindow.h
  include/${PROJECT_NAME}/bash.h
)

qt5_wrap_cpp(rqt_netem_moc ${headers})

add_library(${PROJECT_NAME} ${sources} ${rqt_netem_moc})

add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} Qt5::Widgets)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)

install(FILES
  plugin.xml
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
