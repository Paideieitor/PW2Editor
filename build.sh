#!/usr/bin/env bash

SYSTEM="Linux"
CONFIG="Debug"

GLAD_C="Externals/glad/glad.c"
IMGUI_CPP="Externals/imgui/imgui.cpp Externals/imgui/imgui_demo.cpp Externals/imgui/imgui_draw.cpp Externals/imgui/imgui_impl_glfw.cpp Externals/imgui/imgui_impl_opengl3.cpp Externals/imgui/imgui_stdlib.cpp Externals/imgui/imgui_tables.cpp Externals/imgui/imgui_widgets.cpp"

CORE_CPP="PW2Editor.cpp System.cpp"
UTILS_CPP="Utils/DataUtils.cpp Utils/StringUtils.cpp Utils/FileUtils.cpp Utils/NarcUtils.cpp Utils/Alle5Format.cpp Utils/KlinFormat.cpp Utils/KlangFormat.cpp"
WINDOWS_CPP="Windows/Window.cpp Windows/Wizard.cpp Windows/Engine.cpp"
DATA_CPP="Data/Project.cpp Data/Data.cpp Data/PokemonData.cpp Data/ItemData.cpp Data/MoveData.cpp Data/ZoneData.cpp Data/EncounterData.cpp Data/TrainerData.cpp"
MODULE_CPP=" Modules/Module.cpp Modules/List.cpp Modules/Pokemon.cpp Modules/Item.cpp Modules/Move.cpp Modules/Encounter.cpp Modules/Trainer.cpp"

COMPILE_FILES="${CORE_CPP} ${UTILS_CPP} ${WINDOWS_CPP} ${DATA_CPP} ${MODULE_CPP} ${GLAD_C} ${IMGUI_CPP}"
COMPILE_FLAGS="-Wall -Wextra -pedantic -Werror -std=c++17"

INCLUDE_DIRS="-I. -IExternals"

GLFW_DIR="Externals/glfw3/bin/${SYSTEM}"
GLFW_LIB="glfw3"

LINK_FILES="-L${GLFW_DIR} -l${GLFW_LIB}"

g++ ${COMPILE_FILES} ${INCLUDE_DIRS} ${LINK_FILES} ${COMPILE_FLAGS} -o PW2Editor
