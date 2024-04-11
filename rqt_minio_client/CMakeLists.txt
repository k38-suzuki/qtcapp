cmake_minimum_required(VERSION 3.0.2)
project(rqt_minio_client)

option(BUILD_RQT_MINIO_CLIENT "Building rqt_minio_client" OFF)
if(NOT BUILD_RQT_MINIO_CLIENT)
  return()
endif()

add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rqt_gui
  rqt_gui_cpp
  std_msgs
)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(AWSSDK REQUIRED COMPONENTS s3)
# find_package(AWSSDK REQUIRED COMPONENTS iam)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES rqt_minio_client
  CATKIN_DEPENDS roscpp rqt_gui rqt_gui_cpp std_msgs
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
  src/${PROJECT_NAME}/bucket_item.cpp
  src/${PROJECT_NAME}/json_archive.cpp
)

set(headers
  include/${PROJECT_NAME}/my_plugin.h
  include/${PROJECT_NAME}/mainwindow.h
  include/${PROJECT_NAME}/bucket_item.h
  include/${PROJECT_NAME}/json_archive.h
)

qt5_wrap_cpp(rqt_minio_client_moc ${headers})

add_library(${PROJECT_NAME} ${sources} ${rqt_minio_client_moc})

add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} Qt5::Widgets ${AWSSDK_LINK_LIBRARIES})

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
