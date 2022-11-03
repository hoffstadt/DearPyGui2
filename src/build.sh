#!/bin/bash

###############################################################################
#                                 Setup                                       #
###############################################################################

# colors
BOLD=$'\e[0;1m'
RED=$'\e[0;31m'
RED_BG=$'\e[0;41m'
GREEN=$'\e[0;32m'
GREEN_BG=$'\e[0;42m'
CYAN=$'\e[0;36m'
MAGENTA=$'\e[0;35m'
YELLOW=$'\e[0;33m'
WHITE=$'\e[0;97m'
NC=$'\e[0m'

DPG_RESULT=${BOLD}${GREEN}Successful.${NC}

# find directory of this script
SOURCE=${BASH_SOURCE[0]}
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

# make current directory the same as this script
pushd $DIR >/dev/null

# create output directory
if ! [[ -d ../out ]]; then
    mkdir ../out
fi

# get platform & architecture
PLAT="$(uname)"
ARCH="$(uname -m)"

if [ -f ../out/sandbox ]; then
    rm -f ../out/sandbox
fi

###############################################################################
#                           Common Settings                                   #
###############################################################################

# build config: Debug or Release
DPG_CONFIG=Debug

# common include directories
DPG_INCLUDE_DIRECTORIES="-I../dependencies/glfw/include -I../dependencies/imgui -I../dependencies/imgui/backends"

# common link directories
DPG_LINK_DIRECTORIES="-L../out"

###############################################################################
###############################################################################
###############################################################################
#                                Apple                                        #
###############################################################################
###############################################################################
###############################################################################

if [[ "$PLAT" == "Darwin" ]]; then

DPG_INCLUDE_DIRECTORIES+=" -I$VULKAN_SDK/include"
DPG_LINK_DIRECTORIES+=" -L$VULKAN_SDK/lib/"

###############################################################################
#                                GLFW                                         #
###############################################################################

GLFW_DEFINES="-D_GLFW_VULKAN_STATIC -D_GLFW_COCOA"
GLFW_COMPILER_FLAGS=-std=c99
GLFW_LINK_LIBRARIES="-framework Cocoa -framework IOKit -framework CoreFoundation"

# debug specific
if [[ "$DPG_CONFIG" == "Debug" ]]; then

    GLFW_OUT_BIN="glfw3d.a"

    # debug specific compiler flags
    GLFW_COMPILER_FLAGS+=" --debug -g"

# release specific
elif [[ "$DPG_CONFIG" == "Release" ]]; then
    GLFW_OUT_BIN="glfw3.a"
fi

# arm64 specifics
if [[ "$ARCH" == "arm64" ]]; then

  # arm64 specific compiler flags
  GLFW_COMPILER_FLAGS+=" -arch arm64"

# x64 specifcis
else

  # x64 specific compiler flags
  GLFW_COMPILER_FLAGS+=" -arch x86_64"

fi

if ! [[ -f ../out/${GLFW_OUT_BIN} ]]; then
    echo
    echo ${YELLOW}Step 0: glfw.o${NC}
    echo ${YELLOW}~~~~~~~~~~~~~~${NC}
    echo ${CYAN}Compiling...${NC}
    clang -c -fPIC ../dependencies/glfw/src/context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/context.o
    clang -c -fPIC ../dependencies/glfw/src/init.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/init.o
    clang -c -fPIC ../dependencies/glfw/src/input.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/input.o
    clang -c -fPIC ../dependencies/glfw/src/monitor.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/monitor.o
    clang -c -fPIC ../dependencies/glfw/src/vulkan.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/vulkan.o
    clang -c -fPIC ../dependencies/glfw/src/window.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/window.o
    clang -c -fPIC ../dependencies/glfw/src/egl_context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/egl_context.o
    clang -c -fPIC ../dependencies/glfw/src/osmesa_context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/osmesa_context.o
    clang -c -fPIC ../dependencies/glfw/src/cocoa_time.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/cocoa_time.o
    clang -c -fPIC ../dependencies/glfw/src/posix_thread.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/posix_thread.o
    clang -c -fPIC ../dependencies/glfw/src/cocoa_init.m $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/cocoa_init.o
    clang -c -fPIC ../dependencies/glfw/src/cocoa_joystick.m $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/cocoa_joystick.o
    clang -c -fPIC ../dependencies/glfw/src/cocoa_monitor.m $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/cocoa_monitor.o
    clang -c -fPIC ../dependencies/glfw/src/cocoa_window.m $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/cocoa_window.o
    clang -c -fPIC ../dependencies/glfw/src/nsgl_context.m $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/nsgl_context.o

    ar rcs ../out/${GLFW_OUT_BIN} ../out/*.o
    rm ../out/*.o

    if [ $? -ne 0 ]
    then
        DPG_RESULT=${BOLD}${RED}Failed.${NC}
    fi
fi

###############################################################################
#                              apple executable                               #
###############################################################################

DPG_SOURCES="dpg_main.c dearpygui.c ../out/${GLFW_OUT_BIN}"
DPG_LINK_LIBRARIES="-lvulkan -framework Cocoa -framework IOKit -framework CoreFoundation"

# arm64 specifics
if [[ "$ARCH" == "arm64" ]]; then

  # arm64 specific compiler flags
  DPG_COMPILER_FLAGS+=" -arch arm64"

# x64 specifcis
else

  # x64 specific compiler flags
  DPG_COMPILER_FLAGS+=" -arch x86_64"

fi

echo
echo ${YELLOW}Step 1: sandbox${NC}
echo ${YELLOW}~~~~~~~~~~~~~~~~~~~${NC}
echo ${CYAN}Compiling and Linking...${NC}
clang $DPG_SOURCES $DOG_DEFINES $DPG_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES $DPG_LINK_DIRECTORIES $DPG_LINK_LIBRARIES -o ../out/sandbox

if [ $? -ne 0 ]
then
    DPG_RESULT=${BOLD}${RED}Failed.${NC}
fi

###############################################################################
###############################################################################
###############################################################################
#                                Linux                                        #
###############################################################################
###############################################################################
###############################################################################

else

###############################################################################
#                           Linux Common Settings                             #
###############################################################################

# common defines
DPG_DEFINES="-D_DEBUG"

# additional include directories
if [ -z "$VULKAN_SDK" ]
then
  echo "VULKAN_SDK env var not set"
else
  DPG_INCLUDE_DIRECTORIES+=" -I$VULKAN_SDK/include"
  DPG_LINK_DIRECTORIES+=" -L$VULKAN_SDK/lib"
fi

if [ -d /usr/include/vulkan ]; then
  DPG_INCLUDE_DIRECTORIES+=" -I/usr/include/vulkan"
fi

# common libraries & frameworks
DPG_LINK_LIBRARIES="-lvulkan -lxcb -lX11 -lX11-xcb -lxkbcommon -lpthread"

# additional link directories
DPG_LINK_DIRECTORIES+=" -L$VULKAN_SDK/lib"
DPG_LINK_DIRECTORIES+=" -L/usr/lib/x86_64-linux-gnu"

# common compiler flags
DPG_COMPILER_FLAGS=

# common linker flags
DPG_LINK_FLAGS="-ldl -lm"

# debug specific
if [[ "$DPG_CONFIG" == "Debug" ]]; then

  # debug specific compiler flags
  DPG_COMPILER_FLAGS+=" --debug -g"

# release specific
elif [[ "$DPG_CONFIG" == "Release" ]]; then
  echo "No release specifics yet"
fi

###############################################################################
#                                GLFW                                         #
###############################################################################

GLFW_DEFINES="-D_GLFW_VULKAN_STATIC -D_GLFW_X11"
GLFW_COMPILER_FLAGS=-std=c99

# debug specific
if [[ "$DPG_CONFIG" == "Debug" ]]; then

  # debug specific compiler flags
  GLFW_COMPILER_FLAGS+=" --debug -g"

# release specific
elif [[ "$DPG_CONFIG" == "Release" ]]; then
  echo "No release specifics yet"
fi

echo
echo ${YELLOW}Step: glfw.o${NC}
echo ${YELLOW}~~~~~~~~~~~~~~${NC}
echo ${CYAN}Compiling...${NC}
gcc -c -fPIC ../dependencies/glfw/src/context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/context.o
gcc -c -fPIC ../dependencies/glfw/src/init.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/init.o
gcc -c -fPIC ../dependencies/glfw/src/input.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/input.o
gcc -c -fPIC ../dependencies/glfw/src/monitor.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/monitor.o
gcc -c -fPIC ../dependencies/glfw/src/vulkan.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/vulkan.o
gcc -c -fPIC ../dependencies/glfw/src/window.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/window.o
gcc -c -fPIC ../dependencies/glfw/src/egl_context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/egl_context.o
gcc -c -fPIC ../dependencies/glfw/src/osmesa_context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/osmesa_context.o
gcc -c -fPIC ../dependencies/glfw/src/posix_time.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/posix_time.o
gcc -c -fPIC ../dependencies/glfw/src/posix_thread.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/posix_thread.o
gcc -c -fPIC ../dependencies/glfw/src/x11_init.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/x11_init.o
gcc -c -fPIC ../dependencies/glfw/src/x11_monitor.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/x11_monitor.o
gcc -c -fPIC ../dependencies/glfw/src/x11_window.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/x11_window.o
gcc -c -fPIC ../dependencies/glfw/src/xkb_unicode.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/xkb_unicode.o
gcc -c -fPIC ../dependencies/glfw/src/glx_context.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/glx_context.o
gcc -c -fPIC ../dependencies/glfw/src/linux_joystick.c $GLFW_DEFINES $GLFW_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/linux_joystick.o

ar rcs ../out/glfw3d.a ../out/*.o
rm ../out/*.o

if [ $? -ne 0 ]
then
    DPG_RESULT=${BOLD}${RED}Failed.${NC}
fi

###############################################################################
#                                Dear ImGui                                   #
###############################################################################

IMGUI_DEFINES=
IMGUI_COMPILER_FLAGS=-std=c++14

# debug specific
if [[ "$DPG_CONFIG" == "Debug" ]]; then

  # debug specific compiler flags
  DPG_COMPILER_FLAGS+=" --debug -g"

# release specific
elif [[ "$DPG_CONFIG" == "Release" ]]; then
  echo "No release specifics yet"
fi

echo
echo ${YELLOW}Step: imgui.o${NC}
echo ${YELLOW}~~~~~~~~~~~~~~${NC}
echo ${CYAN}Compiling...${NC}
gcc -c -fPIC ../dependencies/imgui/imgui_demo.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_demo.o
gcc -c -fPIC ../dependencies/imgui/imgui_draw.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_draw.o
gcc -c -fPIC ../dependencies/imgui/imgui_tables.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_tables.o
gcc -c -fPIC ../dependencies/imgui/imgui_widgets.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_widgets.o
gcc -c -fPIC ../dependencies/imgui/imgui.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui.o
gcc -c -fPIC ../dependencies/imgui/backends/imgui_impl_vulkan.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_impl_vulkan.o
gcc -c -fPIC ../dependencies/imgui/backends/imgui_impl_glfw.cpp $IMGUI_DEFINES $IMGUI_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/imgui_impl_glfw.o

ar rcs ../out/imguid.a ../out/*.o
rm ../out/*.o

if [ $? -ne 0 ]
then
    DPG_RESULT=${BOLD}${RED}Failed.${NC}
fi

###############################################################################
#                              linux executable                               #
###############################################################################

DPG_SOURCES="dearpygui.c"

echo
echo ${YELLOW}Step: sandbox${NC}
echo ${YELLOW}~~~~~~~~~~~~~~~~~~~${NC}
echo ${CYAN}Compiling and Linking...${NC}
gcc -c -fPIC $DPG_SOURCES $DPG_DEFINES -std=c99 $DPG_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES -o ../out/dearpygui.o
g++ dpg_main.cpp ../out/glfw3d.a ../out/imguid.a ../out/dearpygui.o $DPG_DEFINES -std=c++14 $DPG_COMPILER_FLAGS $DPG_INCLUDE_DIRECTORIES $DPG_LINK_DIRECTORIES $DPG_LINK_FLAGS $DPG_LINK_LIBRARIES -o ../out/sandbox

if [ $? -ne 0 ]
then
    DPG_RESULT=${BOLD}${RED}Failed.${NC}
fi

fi


###############################################################################
#                          Information Output                                 #
###############################################################################
echo
echo ${CYAN}-------------------------------------------------------------------------${NC}
echo ${YELLOW}                      Build Information ${NC}
echo ${CYAN}Results:             ${NC} ${DPG_RESULT}
echo ${CYAN}Configuration:       ${NC} ${MAGENTA}${PL_CONFIG}${NC}
echo ${CYAN}Working directory:   ${NC} ${MAGENTA}${DIR}${NC}
echo ${CYAN}Output directory:    ${NC} ${MAGENTA}../out${NC}
echo ${CYAN}Output binary:       ${NC} ${YELLOW}sandbox${NC}
echo ${CYAN}--------------------------------------------------------------------------${NC}

popd >/dev/null
