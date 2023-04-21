set(target qtcapp)
add_executable(${target} main.cpp)
target_link_libraries(${target} QtcBase)