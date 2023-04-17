
set(sources
  ConfigDialog.cpp
  MainWindow.cpp
  main.cpp
)

set(headers
  ConfigDialog.h
  MainWindow.h
)

find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
QT5_ADD_RESOURCES(RC_SRCS Qtcapp.qrc)

set(target qtcapp)
add_executable(${target} ${sources} ${headers} ${RC_SRCS})

if(NOT BUILD_QTCAPP_ROS)
  target_link_libraries(${target} Qt5::Widgets)
  install(TARGETS ${target} DESTINATION bin)
elseif(BUILD_QTCAPP_ROS)
  set_target_properties(${target} PROPERTIES OUTPUT_NAME node PREFIX "")
  target_link_libraries(${target} ${catkin_LIBRARIES} ${QT_LIBRARIES})

  install(TARGETS ${target}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
endif()
