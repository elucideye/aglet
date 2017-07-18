cmake_minimum_required(VERSION 3.3) # IN_LIST

include("cmake/HunterGate.cmake")
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.10.tar.gz"
    SHA1 "73b5a5253b75d5ed79c89242c167ca1f94386621"
)

project(aglet VERSION 0.2)

option(AGLET_BUILD_TESTS "Build tests" OFF)
option(AGLET_OPENGL_ES3 "Use OpenGL ES 3.0" OFF) # default 2.0

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

set(AGLET_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/lib")

if(IOS OR ANDROID)
  set(AGLET_IS_MOBILE TRUE)
else()
  set(AGLET_IS_MOBILE FALSE)
endif()

add_subdirectory(lib/aglet)

if(AGLET_BUILD_TESTS)

  hunter_add_package(gauze)
  find_package(gauze CONFIG REQUIRED)
  
  if("$ENV{TRAVIS}" STREQUAL "true" AND IOS)
    # Do not run tests (just build)
  else()
    enable_testing()
  endif()
  add_subdirectory(test)
endif()
