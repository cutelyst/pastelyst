cmake_minimum_required(VERSION 3.6)

project(Pastelyst VERSION 0.5.0 LANGUAGES CXX)

if(WIN32)
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif()
endif()

include(GNUInstallDirs)
include(FetchContent)

find_package(Qt6 COMPONENTS Core Network Sql Gui REQUIRED)
find_package(Cutelyst3Qt6 3.4.0 REQUIRED)
#find_package(KF5SyntaxHighlighting REQUIRED)
find_package(Cutelee6Qt6 REQUIRED)

FetchContent_Declare(
  KF5SyntaxHighlighting
  GIT_REPOSITORY https://invent.kde.org/frameworks/syntax-highlighting.git
  GIT_TAG        598a9c4b # 5.92.0
)
FetchContent_MakeAvailable(KF5SyntaxHighlighting)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE TEMPLATES_SRC root/*)

add_subdirectory(src)

set(CPACK_PACKAGE_VENDOR "Cutelyst")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Pastelyst.")
set(CPACK_PACKAGE_CONTACT "dantti12@gmail.com.br")

add_definitions(
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_STRICT_ITERATORS
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_USE_QSTRINGBUILDER
    -DQT_USE_FAST_OPERATOR_PLUS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
)

if(UNIX)
  if(NOT CPACK_GENERATOR)
    set(CPACK_GENERATOR "DEB")
  endif()

  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  set(CPACK_STRIP_FILES 1)
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(CPACK_DEBIAN_DEBUGINFO_PACKAGE ON)
  endif()
endif()

include(CPack)
