cmake_minimum_required(VERSION 3.15)
project(CPP_Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SourceFiles 
    "src/DataUtils.cpp"
    "src/FileUtils.cpp"
    "src/LogUtils.cpp"
    "src/StringUtils.cpp"
    "src/SystemUtils.cpp"

    "src/comps/ActionManager.cpp"
    "src/comps/bitVector.cpp"
    "src/comps/ByteStreamReader.cpp"
    "src/comps/ByteStreamWriter.cpp"
    "src/comps/ThreadPool.cpp"
)

add_library(${PROJECT_NAME} ${SourceFiles})
target_include_directories(${PROJECT_NAME} PUBLIC src)

# https://stackoverflow.com/a/60890947
# /Zc:__cplusplus is required to make __cplusplus accurate
# /Zc:__cplusplus is available starting with Visual Studio 2017 version 15.7
# (according to https://learn.microsoft.com/en-us/cpp/build/reference/zc-cplusplus)
# That version is equivalent to _MSC_VER==1914
# (according to https://learn.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=vs-2019)
# CMake's ${MSVC_VERSION} is equivalent to _MSC_VER
# (according to https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html#variable:MSVC_VERSION)
if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:__cplusplus")
endif()