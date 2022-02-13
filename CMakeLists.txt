cmake_minimum_required(VERSION 3.10)

project(Qtcapp VERSION 1.1)

add_library(tutorial_compiler_flags INTERFACE)
target_compile_features(tutorial_compiler_flags INTERFACE cxx_std_11)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(tutorial_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3>>")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

add_subdirectory(src)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${Qtcapp_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Qtcapp_VERSION_MINOR}")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)

install(EXPORT QtcBaseTargets
  FILE QtcBaseTargets.cmake
  DESTINATION lib/cmake/QtcBase)

install(EXPORT QtcBaseTargets
  FILE QtcBaseTargets.cmake
  DESTINATION lib/cmake/QtcBase)

include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/QtcBaseConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/example"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/QtcBaseConfigVersion.cmake"
  VERSION "${Qtcapp_VERSION_MAJOR}.${Qtcapp_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/QtcBaseConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/QtcBaseConfigVersion.cmake
  DESTINATION lib/cmake/QtcBase)

export(EXPORT QtcBaseTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/QtcBaseTargets.cmake")