import os
import platform

########################################################################################################################
# setup vs code
########################################################################################################################

if not os.path.isdir('.vscode'):
    os.mkdir('.vscode')

if(platform.system() == "Windows"):

    with open('.vscode/launch.json', 'w') as file:
        file.write(
"""{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(Windows) Launch",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/out/sandbox.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/out/",
            "environment": [],
            "console": "integratedTerminal"
        }
    ]
}
""")

    with open('.vscode/c_cpp_properties.json', 'w') as file:
        file.write(
"""{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/dependencies/imgui",
                "${workspaceFolder}/dependencies/pilotlight",
                "${workspaceFolder}/dependencies/glfw/include",
                "${env:VK_SDK_PATH}/Include"
            ],
            "defines": [
                "_DEBUG"
            ],
            "windowsSdkVersion": "10.0.19041.0",
            "cStandard": "c99",
            "intelliSenseMode": "windows-msvc-x64"
        }
    ],
    "version": 4
}
""")

elif(platform.system() == "Darwin"):

    with open('.vscode/launch.json', 'w') as file:
        file.write(
"""{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(lldb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/out/sandbox",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/out/",
            "environment": [],
            "externalConsole": false,
            "MIMode": "lldb"
        }
    ]
}
""")

    with open('.vscode/c_cpp_properties.json', 'w') as file:
        file.write(
"""{
    "configurations": [
        {
            "name": "Apple",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/dependencies/imgui",
                "${workspaceFolder}/dependencies/pilotlight",
                "${workspaceFolder}/dependencies/glfw/include"
            ],
            "defines": [
                "_DEBUG"
            ],
            "cStandard": "c99",
            "intelliSenseMode": "macos-clang-arm64"
        }
    ],
    "version": 4
}
""")


elif(platform.system() == "Linux"):

    with open('.vscode/launch.json', 'w') as file:
        file.write(
"""{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(lldb) Launch",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/out/sandbox",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}/out/",
            "environment": []
        }
    ]
}
""")

    with open('.vscode/c_cpp_properties.json', 'w') as file:
        file.write(
"""{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/dependencies/imgui",
                "${workspaceFolder}/dependencies/pilotlight",
                "${workspaceFolder}/dependencies/glfw/include"
            ],
            "defines": [
                "_DEBUG"
            ],
            "cStandard": "c99",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
""")