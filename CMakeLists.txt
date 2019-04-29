cmake_minimum_required(VERSION 3.3)

include("cmake/HunterGate.cmake")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.146.tar.gz"
    SHA1 "06f44e121e5ac61c8571203ba47ce0b31187d985"
    LOCAL
)

project(aglet VERSION 1.3.4)

hunter_add_package(check_ci_tag)
find_package(check_ci_tag CONFIG REQUIRED)
check_ci_tag()

if(IOS OR ANDROID)
  set(aglet_opengl_es2_dflt ON)
  set(aglet_opengl_es3_dflt OFF)
else()
  set(aglet_opengl_es2_dflt OFF)
  set(aglet_opengl_es3_dflt OFF)
endif()

option(AGLET_BUILD_TESTS "Build tests" OFF)
option(AGLET_OPENGL_ES2 "Use OpenGL ES 2.0" ${aglet_opengl_es2_dflt})
option(AGLET_OPENGL_ES3 "Use OpenGL ES 3.0" ${aglet_opengl_es3_dflt})

# https://devblogs.nvidia.com/egl-eye-opengl-visualization-without-x-server/
option(AGLET_USE_EGL "Use EGL instead of GLFW for desktop system" OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(AGLET_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/lib")

if(IOS OR ANDROID)
  set(AGLET_IS_MOBILE TRUE)
else()
  set(AGLET_IS_MOBILE FALSE)
endif()

add_subdirectory(lib/aglet)

string(COMPARE EQUAL "$ENV{TRAVIS}" "true" travis_ci)
string(COMPARE EQUAL "$ENV{APPVEYOR}" "True" appveyor_ci)
if(travis_ci OR appveyor_ci)
  set(AGLET_CI TRUE)
else()
  set(AGLET_CI FALSE)
endif()

if(AGLET_BUILD_TESTS)

  hunter_add_package(gauze)
  find_package(gauze CONFIG REQUIRED)

  # We can run CI tests w/ Android Emulator
  if(${AGLET_CI} AND NOT ANDROID)
    # Do not run tests (just build)
  else()
    enable_testing()
  endif()
  add_subdirectory(test)
endif()
