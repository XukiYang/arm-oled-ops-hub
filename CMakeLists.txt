# 本cmake使用使用模板构建
cmake_minimum_required(VERSION 3.12) # 最低 CMake 版本
project(arm-oled-ops-hub # 项目名称
    VERSION 1.0.0 # 版本号
    LANGUAGES CXX # 语言：CXX(C++) 或 C
)
set(CMAKE_CXX_STANDARD 17) # C++标准 (11/14/17/20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制使用指定标准
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # 生成 compile_commands.json
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

include_directories(include) # 头文件搜索路径
set(SOURCES src/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logs
    COMMENT "Makeing some directory"
)

# 创建存放目录
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/logs
    COMMENT "Makeing some directory"
)
# 拷贝静态文件
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${PROJECT_SOURCE_DIR}/configs/*
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configs
    COMMENT "Copying configs/*.ini to output/configs directory"
)
# 自动打包依赖库
install(TARGETS arm-oled-ops-hub
    RUNTIME_DEPENDENCIES
    PRE_EXCLUDE_REGEXES ".*system.*"
    DIRECTORIES ${CMAKE_BINARY_DIR}/output/lib
)

install(DIRECTORY include/ DESTINATION include) # 安装头文件
install(TARGETS ${PROJECT_NAME} DESTINATION lib) # 安装库