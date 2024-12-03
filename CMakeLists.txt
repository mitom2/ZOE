cmake_minimum_required(VERSION 3.5.0)
project(ZOE VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable(ZOE 
    ZOE.cpp
    gpuModule.cpp
    ioModule.cpp
    memoryModule.cpp
    ioBridge.cpp
)

include_directories(/usr/include/freetype2 -I/usr/local/include/freetype2 -I/usr/include/libpng16)
find_package(Freetype)

target_compile_features(ZOE PRIVATE cxx_std_17)

if(WIN32)
    target_link_libraries(ZOE sfml-graphics)
    add_custom_command(
        TARGET ZOE
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:ZOE>
        VERBATIM
    )
endif()

if(UNIX)
    find_package(Curses REQUIRED)
    include_directories(${CURSES_INCLUDE_DIRS})
    target_link_libraries(ZOE ${CURSES_LIBRARIES} sfml-graphics freetype)
endif(UNIX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)