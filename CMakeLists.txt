cmake_minimum_required(VERSION 3.10)
project(PW2Editor)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

# Specify the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)

#Specify shared files
set(CORE_SRC System.cpp Windows/Window.cpp)
set(UTILS_SRC Utils/DataUtils.cpp Utils/StringUtils.cpp Utils/FileUtils.cpp)
set(NDS_SRC Utils/NDS/NarcUtils.cpp Utils/NDS/Alle5Format.cpp Utils/NDS/NCLRFormat.cpp Utils/NDS/NCGRFormat.cpp)
set(INTERNAL_SRC Utils/Internal/KlinFormat.cpp Utils/Internal/KlangFormat.cpp Utils/Internal/Image.cpp)
set(DATA_SRC Data/Project.cpp Data/Data.cpp Data/PokemonData.cpp Data/ItemData.cpp Data/MoveData.cpp Data/ZoneData.cpp Data/EncounterData.cpp Data/TrainerData.cpp)
set(MODULES_SRC Modules/Module.cpp Modules/List.cpp Modules/Pokemon.cpp Modules/Item.cpp Modules/Move.cpp Modules/Encounter.cpp Modules/Trainer.cpp)
set(EXTERNALS_SRC Externals/glad/glad.c Externals/imgui/imgui.cpp Externals/imgui/imgui_demo.cpp Externals/imgui/imgui_draw.cpp Externals/imgui/imgui_impl_glfw.cpp Externals/imgui/imgui_impl_opengl3.cpp Externals/imgui/imgui_stdlib.cpp Externals/imgui/imgui_tables.cpp Externals/imgui/imgui_widgets.cpp) 

set(EDITOR_SRC ${CORE_SRC} Windows/Wizard.cpp Windows/Engine.cpp ${UTILS_SRC} ${NDS_SRC} ${INTERNAL_SRC} ${DATA_SRC} ${MODULES_SRC} ${EXTERNALS_SRC})
set(PAINT_SRC ${CORE_SRC} Windows/Paint.cpp ${UTILS_SRC} ${NDS_SRC} ${INTERNAL_SRC} ${EXTERNALS_SRC})

# Add the executable target
add_executable(PW2Editor PW2Editor.cpp ${EDITOR_SRC})
add_executable(PW2Paint PW2Paint.cpp ${PAINT_SRC})

target_include_directories(PW2Editor PRIVATE . Externals) 
target_include_directories(PW2Paint PRIVATE . Externals) 

target_link_directories(PW2Editor PRIVATE Externals/glfw3/bin/Linux)
target_link_directories(PW2Paint PRIVATE Externals/glfw3/bin/Linux)

target_link_libraries(PW2Editor PRIVATE glfw3)
target_link_libraries(PW2Paint PRIVATE glfw3)
