cmake_minimum_required(VERSION 3.16)
project(mexEdit VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

option(ENABLE_MEMORY_DEBUGGING "Enable custom memory debugging" OFF)
option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan in debug mode" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(NCURSES REQUIRED ncurses)

set(CORE_SOURCES
    src/core/Document.cpp
    src/core/Editor.cpp
    src/core/Cursor.cpp
    src/core/CommandManager.cpp
    src/core/EditCommands.cpp
)

set(UI_SOURCES
    src/ui/NCursesRenderer.cpp
)

set(FEATURE_SOURCES
    src/features/SyntaxHighlighter.cpp
    src/features/SearchEngine.cpp
    src/features/FileExplorer.cpp
)

set(UTIL_SOURCES
    src/utils/FileUtils.cpp
    src/utils/MemoryDebugger.cpp
    src/utils/Clipboard.cpp
)

set(APP_SOURCES
    src/main.cpp
    src/EditorApplication.cpp
)

add_library(mexedit_core STATIC ${CORE_SOURCES})
target_include_directories(mexedit_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(mexedit_core PUBLIC cxx_std_17)

add_library(mexedit_ui STATIC ${UI_SOURCES})
target_include_directories(mexedit_ui PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(mexedit_ui PUBLIC ${NCURSES_LIBRARIES})
target_compile_features(mexedit_ui PUBLIC cxx_std_17)

add_library(mexedit_features STATIC ${FEATURE_SOURCES})
target_include_directories(mexedit_features PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(mexedit_features PUBLIC cxx_std_17)

add_library(mexedit_utils STATIC ${UTIL_SOURCES})
target_include_directories(mexedit_utils PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(mexedit_utils PUBLIC cxx_std_17)

add_executable(${PROJECT_NAME} ${APP_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    mexedit_core
    mexedit_ui 
    mexedit_features
    mexedit_utils
    ${NCURSES_LIBRARIES}
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
    if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
    endif()
endif()

if(ENABLE_MEMORY_DEBUGGING)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_MEMORY_DEBUGGING)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

option(ENABLE_TESTING "Enable testing" OFF)
if(ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

