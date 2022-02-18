cmake_minimum_required(VERSION 3.20)
project(HTTP_Proxy_Server)

set(CMAKE_CXX_STANDARD 14)

add_executable(HTTP_Proxy_Server main.cpp Server.cpp Client.cpp ProxyService.cpp HTTPRequest.cpp HTTPResponse.cpp Cache.cpp Request.cpp Time.h)
add_subdirectory(./googletest-release-1.11.0)
include_directories(${PROJECT_SOURCE_DIR}/src/include ./googletest-release-1.11.0/include)
link_directories(${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/googletest)
target_link_libraries(${PROJECT_NAME} gtest)