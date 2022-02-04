
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

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/qtcapp DESTINATION include)

set(target qtcapp)
add_executable(${target} qtcapp.cpp)
target_link_libraries(${target} Qtcapp)
set_target_properties(${target} PROPERTIES
    PROJECT_LABEL Application
    INSTALL_RPATH "$ORIGIN/../lib")
install(TARGETS ${target} RUNTIME DESTINATION bin)
