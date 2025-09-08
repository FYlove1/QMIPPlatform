# 自动扫描所有算法源文件
file(GLOB_RECURSE ALGO_SOURCES "*.cpp")
file(GLOB_RECURSE ALGO_HEADERS "*.h")

# 输出找到的文件（调试用）
message(STATUS "Found algorithm headers: ${ALGO_HEADERS}")
message(STATUS "Found algorithm sources: ${ALGO_SOURCES}")

# 把找到的所有源文件追加到顶层目标
target_sources(QOMIPPlatform PRIVATE
    ${ALGO_SOURCES}
    ${ALGO_HEADERS}
)

# 添加包含路径，使主项目能找到算法头文件
target_include_directories(QOMIPPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})