CMAKE_MINIMUM_REQUIRED(VERSION 3.12)


PROJECT(test_config)

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
ADD_DEFINITIONS(-DWIN32)
ELSE()
ADD_DEFINITIONS(-DLinux)
ENDIF()

IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
SET(CMAKE_CXX_FLAGS "-std=c++11 -O0 -g -m64 -fpermissive -w ${CMAKE_CXX_FLAGS}")
ENDIF()

# 设置相对路径
SET (CMAKE_USE_RELATIVE_PATHS ON)

# 头文件路径
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../third_party/libevent/include)
ELSE()
INCLUDE_DIRECTORIES(/usr/local/include)
ENDIF()

# 库文件路径
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/../../third_party/libevent/lib)
ELSE()
LINK_DIRECTORIES(/usr/local/lib)
ENDIF()

# 库文件
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
LINK_LIBRARIES(ws2_32.lib)
LINK_LIBRARIES(libevent.lib)
ELSE()
LINK_LIBRARIES(event.so)
ENDIF()

#设置Debug|Release编译选项
SET(CMAKE_BUILD_TYPE Debug)  
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/../../bin)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/../../bin)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${PROJECT_SOURCE_DIR}/../../lib)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(${PROJECT_SOURCE_DIR} DIR_SRCS)
file(GLOB_RECURSE CURRENT_HEADERS ${PROJECT_SOURCE_DIR}/*.h)
source_group("Header Files" FILES ${CURRENT_HEADERS})

# 指定生成目标，solutin下project名称，生成exe的名称
ADD_EXECUTABLE(test_config ${DIR_SRCS} ${CURRENT_HEADERS})

IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
set_target_properties(test_config
    PROPERTIES
    LINK_FLAGS  /machine:X64
    )
ENDIF()
