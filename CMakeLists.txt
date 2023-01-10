cmake_minimum_required(VERSION 3.25)

project(genetic_programming C)
set(CMAKE_C_STANDARD 11)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

include(cmake/Cache.cmake)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

include(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2_GFX REQUIRED SDL2_gfx)

include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_GFX_INCLUDE_DIRS})

set(SOURCES
        src/game.h
        src/game.c
        src/rendering.h
        src/rendering.c
)

add_executable(simulation src/simulation.c ${SOURCES})
target_link_libraries(simulation PRIVATE project_warnings project_options m ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES}) 

add_executable(trainer src/trainer.c ${SOURCES})
target_link_libraries(trainer PRIVATE project_warnings project_options m ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES}) 
