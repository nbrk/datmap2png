cmake_minimum_required(VERSION 3.5)

project(datmap2png LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED glib-2.0 cairo)

add_executable(datmap2png main.c internal.h map.c draw_cairo.c)

target_link_directories(datmap2png PRIVATE ${DEPS_LIBRARY_DIRS})
target_link_libraries(datmap2png ${DEPS_LIBRARIES})
target_include_directories(datmap2png PRIVATE ${DEPS_INCLUDE_DIRS})
