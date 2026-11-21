
cmake_minimum_required(VERSION 3.21)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION"
     SAO2X_VERSION
     LIMIT_COUNT 1)

if(NOT SAO2X_VERSION)
    message(FATAL_ERROR "VERSION file is missing or empty.")
endif()

project(Sao2X
    VERSION ${SAO2X_VERSION}
    DESCRIPTION "Sao-2X Game Engine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
include(GNUInstallDirs)

add_library(sao2x
    engine/src/engine.cpp
    engine/src/config.cpp
    engine/src/logger.cpp
    engine/src/application.cpp
)

add_library(Sao2X::Engine ALIAS sao2x)

target_include_directories(sao2x
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/engine/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(sao2x
    PUBLIC SAO2X_VERSION="${PROJECT_VERSION}"
)

add_executable(sao2x_app
    engine/src/main.cpp
)

set_target_properties(sao2x_app PROPERTIES
    OUTPUT_NAME "sao2x"
)

target_link_libraries(sao2x_app PRIVATE Sao2X::Engine)

if(MINGW)
    target_link_options(sao2x_app PRIVATE
        -static
        -static-libgcc
        -static-libstdc++
    )
endif()

if(BUILD_TESTING)
    add_executable(sao2x_tests
        tests/smoke_test.cpp
    )

    target_link_libraries(sao2x_tests PRIVATE Sao2X::Engine)

    if(MINGW)
        target_link_options(sao2x_tests PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
        )
    endif()

    add_test(
        NAME Sao2X.CoreTests
        COMMAND sao2x_tests
    )
endif()

install(
    TARGETS sao2x sao2x_app
    EXPORT Sao2XTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY engine/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
