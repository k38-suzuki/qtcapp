
set(target qtcapp)
add_executable(${target} qtcapp.cpp)
target_link_libraries(${target} PUBLIC QtcBase)
target_include_directories(${target} PUBLIC
    "${PROJECT_BINARY_DIR}")
set_target_properties(${target} PROPERTIES
    PROJECT_LABEL Application
    INSTALL_RPATH "$ORIGIN/../lib")

install(TARGETS ${target} DESTINATION bin)