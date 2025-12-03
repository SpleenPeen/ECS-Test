cmake_minimum_required(VERSION 3.11)
project(physics)
# Require modern C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#### Setup Directories ####
#Main output directory
SET(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
# Ouput all DLLs from all libs into main build folder
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
foreach(cfg Debug Release RelWithDebInfo MinSizeRel)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${cfg} "${OUTPUT_DIRECTORY}/${cfg}")
endforeach()

#### Add External Dependencies ####
add_subdirectory("lib/SFML")
set(SFML_INCS "lib/SFML/include")
link_directories("${CMAKE_BINARY_DIR}/lib/SFML/lib")

add_subdirectory("lib/box2d")
set(B2D_INCS "lib/box2d/include")
link_directories("${CMAKE_BINARY_DIR}/lib/box2d/lib")

# ==== Level system library (build FIRST) ====
add_library(tile_level STATIC
  tile_level_loader/level_system.cpp
)
target_include_directories(tile_level INTERFACE tile_level)
target_link_libraries(tile_level sfml-graphics)

set(SOURCE_FILES
    main.cpp
    gameSys.cpp
    Scenes.cpp
    MouseHelper.cpp
    )

#### Practical 1 ####
add_executable(physics ${SOURCE_FILES})
target_include_directories(physics PRIVATE ${SFML_INCS} ${B2D_INCS} tile_level)
target_link_libraries(physics sfml-graphics box2d tile_level)

# ==== Copy resources ====
add_custom_target(copy_resources ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/res"
          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${PROJECT_SOURCE_DIR}/res"
          "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/res"
  VERBATIM
)
add_dependencies(physics copy_resources)

# ==== VS debugger working dir ====
set_target_properties(physics PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>"
)