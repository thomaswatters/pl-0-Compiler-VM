cmake_minimum_required(VERSION 3.2)
project(Module3)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror ")

set(SOURCE_FILES
src/main.c
src/parser.c
src/scanner.c
src/vm.c
src/errors.c
)

add_executable(compile ${SOURCE_FILES})