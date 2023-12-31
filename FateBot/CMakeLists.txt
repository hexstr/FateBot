# CMakeList.txt: FateBot 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 文件写出位置
if(WIN32)
    message(STATUS "Platform: Windows")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/BinOutput/Windows/${CMAKE_BUILD_TYPE}")
elseif(ANDROID)
    message(STATUS "Platform: Android")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/BinOutput/Android/${CMAKE_BUILD_TYPE}")
    add_compile_definitions(ABSL_FLAGS_STRIP_NAMES=0)
    # 添加依赖
    set(CMAKE_PREFIX_PATH "${CMAKE_SYSROOT}/${CMAKE_LIBRARY_ARCHITECTURE}")
    message(STATUS CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH})
elseif(UNIX)
    message(STATUS "Platform: Linux")
    set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/BinOutput/Linux/${CMAKE_BUILD_TYPE}")
else()
    message(FATAL_ERROR "Unknown platform.")
endif()

# 添加头文件
include_directories(
    .
    "third/cpr/include"
    "third/rapidjson"
)

# 添加源文件
aux_source_directory(game game-src)
aux_source_directory(struct struct-src)
aux_source_directory(utils utils-src)
aux_source_directory(third/cpr cpr-src)

add_executable (
    FateBot
    "main.cc"
    ${game-src}
    ${struct-src}
    ${utils-src}
    ${cpr-src}
)

# 添加静态库
set(OPENSSL_USE_STATIC_LIBS TRUE)

find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(FateBot ZLIB::ZLIB)
target_link_libraries(FateBot ${OPENSSL_LIBRARIES})
target_link_libraries(FateBot CURL::libcurl)

find_package(absl REQUIRED)
target_link_libraries(FateBot absl::flat_hash_map absl::btree absl::flags absl::flags_parse)

if(WIN32)
    if(NOT MSVC)
        target_link_libraries(FateBot ws2_32)
        target_link_libraries(FateBot Wldap32)
        target_link_libraries(FateBot crypt32)
        target_link_libraries(FateBot pthread)
    endif()
endif()