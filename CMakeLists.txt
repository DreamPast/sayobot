cmake_minimum_required(VERSION 3.9)
project(sayobot-cppcqsdk) # 填写项目名称

set(CMAKE_CXX_STANDARD 17) # 设置 C++ 标准版本为 C++17, 必须

if (MSVC AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
    message(STATUS "project will build in std and dev mode")
else()
    message(AUTHOR_WARNING "project will build in dev mode only")
endif()

if (WIN32)
    set(OS "Win32")
elseif(APPLE)
    set(OS "macOS")
endif()

include_directories(src) # 设置 src 目录为包含目录, 可根据实际情况修改
include_directories(extern/cqcppsdk/include) # 设置 SDK 的 include 目录为包含目录, 必须
include_directories(extern/mysql/include)
include_directories(extern/ImageMagick/include/${OS})
include_directories(extern/nlohmann_json/include)
include_directories(extern/libcurl/include)

link_directories(extern/mysql/lib)
link_directories(extern/ImageMagick/lib)
link_directories(extern/libcurl/lib)

include(extern/cqcppsdk/cqcppsdk.cmake) # 包含 SDK 的 CMake 脚本, 必须
file(GLOB_RECURSE SOURCE_FILES src/*.cpp) # 递归搜索 src 目录中的源文件, 可根据实际情况修改

if (WIN32)
    set(LIBS libmysql mysqlclient CORE_RL_Magick++_ libcurl)
elseif(APPLE)
    set(LIBS mysqlclient
            libMagick++-7.Q16HDRI.4.dylib
            libMagickCore-7.Q16HDRI.7.dylib
            libMagickWand-7.Q16HDRI.7.dylib
            libcurl.4.dylib)
endif()

cq_set_app_id(cn.sayobot.sayobotv2) # 设置 app id, 必须

set(LIB_NAME "app") # 酷Q要求开发阶段的应用名为 app.dll

if (MSVC AND (CMAKE_SIZEOF_VOID_P EQUAL 4)) # MSVC x86
    cq_add_app(${LIB_NAME} ${SOURCE_FILES}) # 添加 std 模式的动态链接库构建目标
    cq_add_install_script(${LIB_NAME} "${PROJECT_SOURCE_DIR}/scripts/install.ps1") # 添加安装脚本(目前只支持 PowerShell 脚本, 建议脚本文件使用 UTF-16 LE 编码)
    target_link_libraries(${LIB_NAME} ${LIBS})
    add_library(${LIB_NAME} SHARED ${SOURCE_FILES}) # 将${SOURCE_FILES}编译到${LIB_NAME}的动态库
endif ()

# 添加 dev 模式的可执行文件构建目标
set(CQCPPSDK_DEV_MODE ON)
cq_add_app(${LIB_NAME}_dev ${SOURCE_FILES})
target_link_libraries(${LIB_NAME}_dev ${LIBS})
add_library(${LIB_NAME}_dev SHARED ${SOURCE_FILES})