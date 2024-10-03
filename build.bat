@echo off

call "V:\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET includes=/ILibraries/Includes /ILibraries/Includes /IC:/VulkanSDK/1.3.290.0/Include
SET links=/link /LIBPATH:Libraries/Lib glfw3dll.lib /LIBPATH:C:/VulkanSDK/1.3.290.0/Lib vulkan-1.lib
SET defines=/D DEBUG

echo "Building main..."

sh shaderCompiler.sh

cl /EHsc /std:c++17 %includes% %defines% src/*.cpp src/Core/*.cpp src/Vulkan/*.cpp %links%
del *.obj

main.exe