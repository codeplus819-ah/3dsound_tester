cmake_minimum_required(VERSION 3.15)
project(3dsound_tester CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")
    set(MINGW_BIN_DIR "E:/mingw64/bin")

    set(ALSOFT_ENABLE_MODULES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(ALSOFT_UTILS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_TESTS OFF CACHE BOOL "" FORCE)
    set(ALSOFT_INSTALL_HRTF_DATA ON CACHE BOOL "" FORCE)
    set(LIBTYPE SHARED CACHE STRING "" FORCE)

    set(CMAKE_CXX_STANDARD 17)
    add_subdirectory("${LIBS_DIR}/openal-soft" EXCLUDE_FROM_ALL)
    set(CMAKE_CXX_STANDARD 20)

    set(SFML_DIR "${LIBS_DIR}/SFML")
    set(SFML_INCLUDE_DIR "${SFML_DIR}/include")
    set(SFML_LIB_DIR "${SFML_DIR}/lib")

    add_link_options(-static-libgcc -static-libstdc++ -static)

elseif(UNIX AND NOT APPLE)
    find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
    find_package(OpenAL REQUIRED)
endif()

add_executable(3dsound_tester
    src/main.cpp
    src/scanner.cpp
    src/wav_loader.cpp
    src/audio.cpp
    src/ui.cpp
)

target_include_directories(3dsound_tester PRIVATE
    src
)

if(WIN32)
    target_include_directories(3dsound_tester PRIVATE
        "${SFML_INCLUDE_DIR}"
    )

    target_link_libraries(3dsound_tester PRIVATE
        OpenAL
        "${SFML_LIB_DIR}/libsfml-graphics.a"
        "${SFML_LIB_DIR}/libsfml-window.a"
        "${SFML_LIB_DIR}/libsfml-system.a"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(3dsound_tester PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
        OpenAL::OpenAL
    )
endif()

if(WIN32)
    add_custom_command(TARGET 3dsound_tester POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:OpenAL>
            $<TARGET_FILE_DIR:3dsound_tester>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DIR}/bin/sfml-graphics-3.dll"
            $<TARGET_FILE_DIR:3dsound_tester>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DIR}/bin/sfml-window-3.dll"
            $<TARGET_FILE_DIR:3dsound_tester>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SFML_DIR}/bin/sfml-system-3.dll"
            $<TARGET_FILE_DIR:3dsound_tester>
    )

    foreach(RUNTIME_DLL
        libgcc_s_seh-1.dll
        libstdc++-6.dll
        libwinpthread-1.dll
    )
        if(EXISTS "${MINGW_BIN_DIR}/${RUNTIME_DLL}")
            add_custom_command(TARGET 3dsound_tester POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${RUNTIME_DLL}"
                    $<TARGET_FILE_DIR:3dsound_tester>
            )
            message(STATUS "Will copy runtime: ${RUNTIME_DLL}")
        else()
            message(WARNING "Runtime not found: ${MINGW_BIN_DIR}/${RUNTIME_DLL}")
        endif()
    endforeach()
endif()

add_custom_command(TARGET 3dsound_tester POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/assets"
        $<TARGET_FILE_DIR:3dsound_tester>/assets
)