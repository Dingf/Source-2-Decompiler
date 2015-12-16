cmake_minimum_required (VERSION 3.4.1)

set (PROJECT_NAME "s2dc")
project (${PROJECT_NAME})

set (Boost_USE_STATIC_LIBS ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

find_package(Boost 1.58.0 REQUIRED COMPONENTS filesystem system)
include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories (include)
file (GLOB SOURCE_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries (${PROJECT_NAME} ${Boost_LIBRARIES})