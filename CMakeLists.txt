cmake_minimum_required(VERSION 3.15)
project(libcyn VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CYN_SRC
    src/cyn.c
    src/util.c
    src/user.c
    src/system.c
    src/network.c
    src/memory.c
    src/storage.c
    src/cpu.c
    src/gpu.c
    src/gpu_pci.c
    src/hardware.c
    src/json.c)

set(CYN_PUBLIC_HEADERS include/cyn.h)

add_library(cyn_obj OBJECT ${CYN_SRC})
target_include_directories(cyn_obj PRIVATE include)
target_compile_options(cyn_obj PRIVATE
    -Wall -Wextra -fvisibility=hidden)

# ---- Shared library ----
add_library(cyn SHARED $<TARGET_OBJECTS:cyn_obj>)
target_include_directories(cyn PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(cyn PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME cyn)

# ---- Static library ----
add_library(cyn_static STATIC $<TARGET_OBJECTS:cyn_obj>)
target_include_directories(cyn_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
set_target_properties(cyn_static PROPERTIES OUTPUT_NAME cyn)

# ---- Install ----
include(GNUInstallDirs)
install(TARGETS cyn cyn_static
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${CYN_PUBLIC_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES man/cyn.3
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man3)

# ---- pkg-config ----
configure_file(libcyn.pc.in libcyn.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libcyn.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# ---- Tests ----
enable_testing()
add_executable(test_cyn tests/test_cyn.c)
target_link_libraries(test_cyn PRIVATE cyn)
add_test(NAME test_cyn COMMAND test_cyn)

add_executable(demo examples/demo.c)
target_link_libraries(demo PRIVATE cyn)

# ---- Uninstall target ----
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

  add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()