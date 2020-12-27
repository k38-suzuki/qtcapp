
set(target qtc-app)

QT5_ADD_RESOURCES(RC_SRCS qtc-app.qrc)

set(sources
    main.cpp
    MainWindow.cpp
)

set(headers
    MainWindow.h
)

add_executable(${target} ${sources} ${headers} ${RC_SRCS})
target_link_libraries(${target} Qt5::Widgets)

install(TARGETS qtc-app DESTINATION bin)
