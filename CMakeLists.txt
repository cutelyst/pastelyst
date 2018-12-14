cmake_minimum_required(VERSION 3.1)

project(Pastelyst LANGUAGES CXX)

if(WIN32)
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif()
endif()

find_package(Qt5 COMPONENTS Core Network Sql Gui REQUIRED)
find_package(Cutelyst2Qt5 2.0.0 REQUIRED)
find_package(KF5SyntaxHighlighting REQUIRED)
find_package(Grantlee5 REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${Cutelyst2Qt5_INCLUDE_DIR}
)

file(GLOB_RECURSE TEMPLATES_SRC root/*)

add_subdirectory(src)
