set(target qtcapp)
add_executable(${target} main.cpp)
target_link_libraries(${target} QtcBase)

install(TARGETS ${target}
  RUNTIME DESTINATION bin)
