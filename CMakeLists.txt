cmake_minimum_required(VERSION 3.28)
project(mexEdit CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Curses QUIET)

if(NOT Curses_FOUND)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_search_module(CURSES QUIET ncurses)
        if(CURSES_FOUND)
            set(CURSES_INCLUDE_DIRS ${CURSES_INCLUDE_DIRS})
            set(CURSES_LIBRARIES ${CURSES_LINK_LIBRARIES})
            set(Curses_FOUND TRUE)
        endif()
    endif()
endif()

if(NOT Curses_FOUND)
    find_path(CURSES_INCLUDE_DIR ncurses.h
            PATHS /usr/include /usr/local/include /opt/homebrew/include
    )

    find_library(CURSES_LIBRARY
            NAMES ncurses curses ncursesw
            PATHS /usr/lib /usr/local/lib /opt/homebrew/lib
    )

    if(CURSES_INCLUDE_DIR AND CURSES_LIBRARY)
        set(Curses_FOUND TRUE)
    endif()
endif()

if(NOT Curses_FOUND)
    message(FATAL_ERROR "ncurses library not found")
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)
if(CURSES_INCLUDE_DIRS)
    include_directories(${CURSES_INCLUDE_DIRS})
elseif(CURSES_INCLUDE_DIR)
    include_directories(${CURSES_INCLUDE_DIR})
endif()

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

if(TARGET Curses::Curses)
    target_link_libraries(${PROJECT_NAME} PRIVATE Curses::Curses)
elseif(CURSES_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURSES_LIBRARIES})
endif()

if(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-deprecated-declarations")
endif()

