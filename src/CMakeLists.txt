
QT5_ADD_RESOURCES(RC_SRCS Qtcapp.qrc)

set(sources
    App.cpp
    ConfigDialog.cpp
    MainWindow.cpp
)

set(headers
    App.h
    ConfigDialog.h
    MainWindow.h
    exportdecl.h
)

add_library(${PROJECT_NAME} SHARED ${sources} ${headers} ${RC_SRCS})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

set(target qtcapp)
set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER App.h)
install(TARGETS ${PROJECT_NAME}
    EXPORT ${target}
    INCLUDES DESTINATION include
    PUBLIC_HEADER DESTINATION include/qtcapp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

install(EXPORT ${target}
    FILE qtcapp-config.cmake
    DESTINATION share/cmake/qtcapp)

add_executable(${target} qtcapp.cpp)
target_link_libraries(${target} Qtcapp)
set_target_properties(${target} PROPERTIES
    PROJECT_LABEL Application
    INSTALL_RPATH "$ORIGIN/../lib")
install(TARGETS ${target} RUNTIME DESTINATION bin)
