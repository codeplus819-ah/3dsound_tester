cmake_minimum_required(VERSION 3.15)
project(3dsound_test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")

set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
set(ALSOFT_INSTALL_HRTF_DATA ON CACHE BOOL "" FORCE)
set(LIBTYPE SHARED CACHE STRING "" FORCE)
add_subdirectory("${LIBS_DIR}/openal-soft" EXCLUDE_FROM_ALL)

set(SFML_DIR "${LIBS_DIR}/SFML")
set(SFML_INCLUDE_DIR "${SFML_DIR}/include")
set(SFML_LIB_DIR "${SFML_DIR}/lib")

add_executable(3dsound_test
    src/main.cpp
    src/scanner.cpp
    src/wav_loader.cpp
    src/audio.cpp
    src/ui.cpp
)

target_include_directories(3dsound_test PRIVATE
    src
    "${SFML_INCLUDE_DIR}"
)

target_link_libraries(3dsound_test PRIVATE
    OpenAL
    "${SFML_LIB_DIR}/libsfml-graphics.a"
    "${SFML_LIB_DIR}/libsfml-window.a"
    "${SFML_LIB_DIR}/libsfml-system.a"
)

add_custom_command(TARGET 3dsound_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:OpenAL>
        $<TARGET_FILE_DIR:3dsound_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_DIR}/bin/sfml-graphics-3.dll"
        $<TARGET_FILE_DIR:3dsound_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_DIR}/bin/sfml-window-3.dll"
        $<TARGET_FILE_DIR:3dsound_test>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_DIR}/bin/sfml-system-3.dll"
        $<TARGET_FILE_DIR:3dsound_test>
)

add_custom_command(TARGET 3dsound_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/assets"
        $<TARGET_FILE_DIR:3dsound_test>/assets
)