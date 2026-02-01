cmake_minimum_required(VERSION 3.10)
project(PW2Editor)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

# Specify the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

# Add the executable target
add_executable(PW2Editor PW2Editor.cpp System.cpp Utils/DataUtils.cpp Utils/StringUtils.cpp Utils/FileUtils.cpp Utils/NarcUtils.cpp Utils/Alle5Format.cpp Utils/KlinFormat.cpp Utils/KlangFormat.cpp Windows/Window.cpp Windows/Wizard.cpp Windows/Engine.cpp Data/Project.cpp Data/Data.cpp Data/PokemonData.cpp Data/ItemData.cpp Data/MoveData.cpp Data/ZoneData.cpp Data/EncounterData.cpp Data/TrainerData.cpp Modules/Module.cpp Modules/List.cpp Modules/Pokemon.cpp Modules/Item.cpp Modules/Move.cpp Modules/Encounter.cpp Modules/Trainer.cpp Externals/glad/glad.c Externals/imgui/imgui.cpp Externals/imgui/imgui_demo.cpp Externals/imgui/imgui_draw.cpp Externals/imgui/imgui_impl_glfw.cpp Externals/imgui/imgui_impl_opengl3.cpp Externals/imgui/imgui_stdlib.cpp Externals/imgui/imgui_tables.cpp Externals/imgui/imgui_widgets.cpp)

target_include_directories(PW2Editor PRIVATE . Externals) 

target_link_directories(PW2Editor PRIVATE Externals/glfw3/bin/Linux)

target_link_libraries(PW2Editor PRIVATE glfw3)
