
set(sources
    App.cpp
    ConfigDialog.cpp
    MainWindow.cpp
)

set(headers
    App.h
    ConfigDialog.h
    MainWindow.h
)

find_package(Qt5Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
QT5_ADD_RESOURCES(RC_SRCS Qtcapp.qrc)

set(target QtcBase)
add_library(${target} ${sources} ${headers} ${RC_SRCS})
target_link_libraries(${target} Qt5::Widgets)
target_include_directories(${target}
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>)

target_compile_definitions(${target} PRIVATE "EXPORTING_QTCBASE")

install(TARGETS ${target}
    EXPORT QtcBaseTargets
    DESTINATION lib)
install(FILES App.h DESTINATION include/qtc)
