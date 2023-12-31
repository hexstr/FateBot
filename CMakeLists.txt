# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("FateBot")

set(CMAKE_CXX_STANDARD 17)

###############################################################################
set(LINKER_FLAGS "")
set(COMPILER_FLAGS "")
# 添加Debug和Release版本信息
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UP)

if(DEBUG_FLAG)
    add_link_options("LINKER:--verbose")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v")
endif()

if(CMAKE_BUILD_TYPE_UP STREQUAL DEBUG)
    message(STATUS "Debug Version")
    set(LINKER_FLAGS "${LINKER_FLAGS} -g")
    if(WIN32)
        set(CMAKE_PREFIX_PATH "$ENV{appdata}/Program/Library/Debug")
    endif()
else()
    message(STATUS "Release Version")
    if(!MSVC)
        set(COMPILER_FLAGS "${COMPILER_FLAGS} -Oz -flto -fdata-sections -ffunction-sections")
        set(LINKER_FLAGS "${LINKER_FLAGS} -s -flto")
    endif()
endif()

if(!MSVC)
    set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-rtti")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
###############################################################################

# 包含子项目。
add_subdirectory ("FateBot")