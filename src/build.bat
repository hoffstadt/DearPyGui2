
@rem without this, PATH will not reset when called within same session
@setlocal 

@rem make current directory the same as batch file
@pushd %~dp0 
@set dir=%~dp0

@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@rem |                            Setup                                       |
@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@rem create output directory
@if not exist ..\out @mkdir ..\out

@rem cleanup temp files
@del ..\out\*.pdb > NUL 2> NUL

@rem -------------------Setup development environment--------------------------
@set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build;%PATH%
@set PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build;%PATH%
@set PATH=%dir%..\out;%PATH%

@rem setup environment for msvc
@call vcvarsall.bat amd64 > nul

@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@rem |                          Common Settings                               |
@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@rem build config: Debug or Release
@set DPG_CONFIG=Debug

@rem include directories
@set DPG_INCLUDE_DIRECTORIES=-I"%WindowsSdkDir%Include\um" 
@set DPG_INCLUDE_DIRECTORIES=-I"%WindowsSdkDir%Include\shared" %DPG_INCLUDE_DIRECTORIES%
@set DPG_INCLUDE_DIRECTORIES=-I%VULKAN_SDK%/Include            %DPG_INCLUDE_DIRECTORIES%
@set DPG_INCLUDE_DIRECTORIES=-I..\dependencies\imgui           %DPG_INCLUDE_DIRECTORIES%
@set DPG_INCLUDE_DIRECTORIES=-I..\dependencies\imgui\backends  %DPG_INCLUDE_DIRECTORIES%
@set DPG_INCLUDE_DIRECTORIES=-I..\dependencies\glfw\include    %DPG_INCLUDE_DIRECTORIES%

@rem link directories
@set DPG_LINK_DIRECTORIES=-LIBPATH:"%VULKAN_SDK%\Lib" -LIBPATH:"..\out"

@rem common defines
@set DPG_DEFINES=

@rem common compiler flags
@set DPG_COMPILER_FLAGS=-nologo -std:c11 -W3 -WX -wd4201 -wd4100 -wd4996 -wd4505 -wd4189 -wd5105 -wd4115

@rem common libraries
@set DPG_LINK_LIBRARIES=user32.lib Shell32.lib Ole32.lib gdi32.lib

@rem release specific
@if "%DPG_CONFIG%" equ "Release" (

    @set DPG_LINK_LIBRARIES=ucrt.lib glfw.lib imgui.lib %DPG_LINK_LIBRARIES%

    @rem release specific defines
    @set DPG_DEFINES=%DPG_DEFINES%

    @rem release specific compiler flags
    @set DPG_COMPILER_FLAGS=-O2 -MD %DPG_COMPILER_FLAGS%
)

@rem debug specific
@if "%DPG_CONFIG%" equ "Debug" (

    @set DPG_LINK_LIBRARIES=ucrtd.lib glfwd.lib imguid.lib %DPG_LINK_LIBRARIES%

    @rem debug specific defines
    @set DPG_DEFINES=-D_DEBUG %DPG_DEFINES%
   
    @rem debug specific compiler flags
    @set DPG_COMPILER_FLAGS=-Od -MDd -Zi %DPG_COMPILER_FLAGS%
)

@set DPG_RESULT=[1m[92mSuccessful.[0m

@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@rem |                             GLFW                                       |
@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

@set GLFW_DEFINES=-DUNICODE -D_UNICODE -D_CRT_SECURE_NO_WARNINGS -D_GLFW_VULKAN_STATIC -D_GLFW_WIN32
@set GLFW_COMPILER_FLAGS=-nologo -std:c11 -W3 -wd5105

@rem release specific
@if "%DPG_CONFIG%" equ "Release" (
    @set GLFW_BIN=glfw.lib
    @if exist ..\out\glfw.lib @goto ImGuiBuild
)
@rem debug specific
@if "%DPG_CONFIG%" equ "Debug" (
    @set GLFW_BIN=glfwd.lib
    @set GLFW_DEFINES=-D_DEBUG %GLFW_DEFINES%
    @set GLFW_COMPILER_FLAGS=-Od -MDd -Zi %GLFW_COMPILER_FLAGS%
    @if exist ..\out\glfwd.lib @goto ImGuiBuild
)

@set GLFW_SOURCES="../dependencies/glfw/src/context.c"
@set GLFW_SOURCES="../dependencies/glfw/src/init.c"           %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/input.c"          %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/monitor.c"        %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/vulkan.c"         %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/window.c"         %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/context.c"        %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/egl_context.c"    %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/wgl_context.c"    %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/osmesa_context.c" %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_time.c"     %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_thread.c"   %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_init.c"     %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_joystick.c" %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_monitor.c"  %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/win32_window.c"   %GLFW_SOURCES%
@set GLFW_SOURCES="../dependencies/glfw/src/wgl_context.c"    %GLFW_SOURCES%

@rem run compiler
@echo.
@echo [1m[93mStep: glfw.lib[0m
@echo [1m[93m~~~~~~~~~~~~~~~~[0m
@echo [1m[36mCompiling...[0m
cl %GLFW_COMPILER_FLAGS% %GLFW_DEFINES% -c -permissive- %GLFW_SOURCES% -Fo..\out\

@rem check build status
@set DPG_BUILD_STATUS=%ERRORLEVEL%
@if %DPG_BUILD_STATUS% NEQ 0 (
    echo [1m[91mCompilation Failed with error code[0m: %DPG_BUILD_STATUS%
    @set DPG_RESULT=[1m[91mFailed.[0m
    goto CleanupGLFW
)

lib -nologo -OUT:..\out\%GLFW_BIN% ..\out\*.obj

:CleanupGLFW
    @echo [1m[36mCleaning...[0m
    @del ..\out\*.obj

@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@rem |                          Dear ImGui                                    |
@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:ImGuiBuild

@set IMGUI_DEFINES=-DUNICODE -D_UNICODE
@set IMGUI_COMPILER_FLAGS=-nologo -std:c++14

@rem release specific
@if "%DPG_CONFIG%" equ "Release" (
    @set IMGUI_BIN=imgui.lib
    @if exist ..\out\imgui.lib @goto MainBuild
)
@rem debug specific
@if "%DPG_CONFIG%" equ "Debug" (
    @set IMGUI_BIN=imguid.lib
    @set IMGUI_DEFINES=-D_DEBUG %IMGUI_DEFINES%
    @set IMGUI_COMPILER_FLAGS=-Od -MDd -Zi %IMGUI_COMPILER_FLAGS%
    @if exist ..\out\imguid.lib @goto MainBuild
)

@set IMGUI_SOURCES="../dependencies/imgui/imgui_demo.cpp"
@set IMGUI_SOURCES="../dependencies/imgui/imgui_draw.cpp"                 %IMGUI_SOURCES%
@set IMGUI_SOURCES="../dependencies/imgui/imgui_tables.cpp"               %IMGUI_SOURCES%
@set IMGUI_SOURCES="../dependencies/imgui/imgui_widgets.cpp"              %IMGUI_SOURCES%
@set IMGUI_SOURCES="../dependencies/imgui/imgui.cpp"                      %IMGUI_SOURCES%
@set IMGUI_SOURCES="../dependencies/imgui/backends/imgui_impl_vulkan.cpp" %IMGUI_SOURCES%
@set IMGUI_SOURCES="../dependencies/imgui/backends/imgui_impl_glfw.cpp"   %IMGUI_SOURCES%

@rem run compiler
@echo.
@echo [1m[93mStep: imgui.lib[0m
@echo [1m[93m~~~~~~~~~~~~~~~~[0m
@echo [1m[36mCompiling...[0m
cl %IMGUI_COMPILER_FLAGS% %IMGUI_DEFINES% %DPG_INCLUDE_DIRECTORIES% -c -permissive- %IMGUI_SOURCES% -Fo..\out\

@rem check build status
@set DPG_BUILD_STATUS=%ERRORLEVEL%
@if %DPG_BUILD_STATUS% NEQ 0 (
    echo [1m[91mCompilation Failed with error code[0m: %DPG_BUILD_STATUS%
    @set DPG_RESULT=[1m[91mFailed.[0m
    goto CleanupImGui
)

lib -nologo -OUT:..\out\%IMGUI_BIN% ..\out\*.obj

:CleanupImGui
    @echo [1m[36mCleaning...[0m
    @del ..\out\*.obj

@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@rem |                          Executable                                    |
@rem ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
:MainBuild

@set DPG_SOURCES=dpg_main.cpp dearpygui.c

@rem run compiler
@echo.
@echo [1m[93mStep: sandbox.exe[0m
@echo [1m[93m~~~~~~~~~~~~~~~~~~~~~~~~[0m
@echo [1m[36mCompiling and Linking...[0m
@cl %DPG_INCLUDE_DIRECTORIES% %DPG_DEFINES% %DPG_COMPILER_FLAGS% -permissive- %DPG_SOURCES% -Fe..\out\sandbox.exe -Fo..\out\ -link -incremental:no %DPG_LINKER_FLAGS% %DPG_LINK_DIRECTORIES% %DPG_LINK_LIBRARIES%

@rem check build status
@set DPG_BUILD_STATUS=%ERRORLEVEL%

@if %DPG_BUILD_STATUS% neq 0 (
    echo [1m[91mCompilation Failed with error code[0m: %DPG_BUILD_STATUS%
    @set DPG_RESULT=[1m[91mFailed.[0m
    goto CleanupExe
)

:CleanupExe
    @echo [1m[36mCleaning...[0m
    @del ..\out\*.obj

@rem --------------------------------------------------------------------------
@rem Information Output
@rem --------------------------------------------------------------------------
:PrintInfo
@echo.
@echo [36m--------------------------------------------------------------------------[0m
@echo [1m[93m                        Build Information [0m
@echo [36mResults:             [0m %DPG_RESULT%
@echo [36mConfiguration:       [0m [35m%DPG_CONFIG%[0m
@echo [36mWorking directory:   [0m [35m%dir%[0m
@echo [36mOutput directory:    [0m [35m..\out[0m
@echo [36mOutput binary:       [0m [33msandbox.exe[0m
@echo [36m--------------------------------------------------------------------------[0m

@popd

@rem keep terminal open if clicked from explorer
@echo off
for %%x in (%cmdcmdline%) do if /i "%%~x"=="/c" set DOUBLECLICKED=1
if defined DOUBLECLICKED pause
