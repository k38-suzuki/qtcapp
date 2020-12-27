QT5_ADD_RESOURCES(RC_SRCS qtc-app.qrc)

set(sources
    main.cpp
    MainWindow.cpp
)

set(headers
    MainWindow.h
)

add_executable(qtc-app ${sources} ${headers} ${RC_SRCS})
target_link_libraries(qtc-app Qt5::Widgets)

install (TARGETS qtc-app DESTINATION bin)
