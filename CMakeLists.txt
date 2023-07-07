cmake_minimum_required(VERSION 3.0.2)
project(qtcapp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
  set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(sources src/${PROJECT_NAME}/NetEmWidget.cpp)
set(headers include/${PROJECT_NAME}/NetEmWidget.h)

option(USE_CATKIN_MAKE "Use catkin_make" ON)
if(NOT USE_CATKIN_MAKE)

  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

  include_directories(${PROJECT_SOURCE_DIR}/include)

  add_library(${PROJECT_NAME} SHARED ${sources} ${headers})
  target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

  install(TARGETS ${PROJECT_NAME}
    EXPORT qtcapp-export
    DESTINATION lib)

  install(EXPORT qtcapp-export
    FILE qtcapp-config.cmake
    DESTINATION lib/cmake/qtcapp)

  add_executable(${PROJECT_NAME}_node src/netem_node.cpp)
  target_link_libraries(${PROJECT_NAME}_node ${PROJECT_NAME})

  install(TARGETS ${PROJECT_NAME}_node
    RUNTIME DESTINATION bin)
    
  add_subdirectory(include)

else()

  find_package(catkin REQUIRED COMPONENTS
    roscpp
    rospy
    std_msgs
  )

  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES qtcapp
    CATKIN_DEPENDS roscpp rospy std_msgs
  #  DEPENDS system_lib
  )

  include_directories(
    include
    ${catkin_INCLUDE_DIRS}
  )

  add_library(${PROJECT_NAME} ${sources} ${headers})
  target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} Qt5::Widgets)

  add_executable(${PROJECT_NAME}_node src/netem_node.cpp)
  target_link_libraries(${PROJECT_NAME}_node ${catkin_LIBRARIES} ${PROJECT_NAME})

  install(TARGETS ${PROJECT_NAME}_node
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  )

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

  # install(FILES
  #   # myfile1
  #   # myfile2
  #   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
  # )

endif()
