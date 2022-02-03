
set(target qtcapp)

QT5_ADD_RESOURCES(RC_SRCS Qtcapp.qrc)

set(sources
    ConfigDialog.cpp
    qtcapp.cpp
    MainWindow.cpp
)

set(headers
    ConfigDialog.h
    MainWindow.h
)

add_executable(${target} ${sources} ${headers} ${RC_SRCS})
target_link_libraries(${target} Qt5::Widgets)

install(TARGETS ${target} DESTINATION bin)
