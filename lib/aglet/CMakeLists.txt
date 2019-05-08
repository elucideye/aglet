set(aglet_libs "")
set(aglet_defs "")
set(aglet_srcs
  aglet.h
  aglet_assert.h
  GLContext.h
  GLContext.cpp
  )

if(ANDROID)

  if(AGLET_OPENGL_ES3)
    find_package(gles3 REQUIRED)
    set(aglet_opengl_lib gles3::gles3)
  else()
    find_package(gles2 REQUIRED)
    set(aglet_opengl_lib gles2::gles2)
  endif()

  find_package(android REQUIRED)
  find_package(android_log REQUIRED)
  find_package(egl REQUIRED)

  list(APPEND aglet_libs android_log::android_log android::android egl::egl ${aglet_opengl_lib})
  list(APPEND aglet_srcs EGLContext.cpp EGLContext.h)
  list(APPEND aglet_defs AGLET_ANDROID=1 AGLET_EGL=1)
endif()

if(IOS)
  list(APPEND aglet_srcs GLContextIOS.mm GLContextIOS.h)
  list(APPEND aglet_defs AGLET_IOS=1)

  find_package(opengles REQUIRED)
  list(APPEND aglet_libs opengles::opengles)

  find_package(foundation REQUIRED)
  list(APPEND aglet_libs foundation::foundation)
endif()

if(MSVC)
  # ogles_gpgpu/platform/opengl/gl_includes.h: #include <gl/glew.h>
  hunter_add_package(glew)
  find_package(glew CONFIG REQUIRED)

  list(APPEND aglet_libs glew::glew)
  list(APPEND aglet_defs PUBLIC NOMINMAX) # avoid std::{min,max} conflicts
  list(APPEND aglet_defs PUBLIC _USE_MATH_DEFINES) # M_PI, etc
endif()

if(NOT ${AGLET_IS_MOBILE})

  if(AGLET_USE_EGL)

    if(AGLET_OPENGL_ES2)
      find_package(gles2 REQUIRED)
      set(aglet_opengl_lib gles2::gles2)
    elseif(AGLET_OPENGL_ES3)
      find_package(gles3 REQUIRED)
      set(aglet_opengl_lib gles3::gles3)
    else()
      find_package(OpenGL REQUIRED)
      set(aglet_opengl_lib OpenGL)
    endif()

    find_package(egl REQUIRED)

    list(APPEND aglet_libs egl::egl ${aglet_opengl_lib})
    list(APPEND aglet_srcs EGLContext.cpp EGLContext.h)
    list(APPEND aglet_defs AGLET_EGL=1)
  else()
    hunter_add_package(glfw)
    find_package(glfw3 REQUIRED)

    list(APPEND aglet_srcs GLFWContext.cpp GLFWContext.h)
    list(APPEND aglet_defs AGLET_HAS_GLFW=1)
    list(APPEND aglet_libs glfw)
  endif()

endif()

if(AGLET_OPENGL_ES2)
  list(APPEND aglet_defs AGLET_OPENGL_ES2=1) # PUBLIC
elseif(AGLET_OPENGL_ES3)
  list(APPEND aglet_defs AGLET_OPENGL_ES3=1) # PUBLIC
endif()

add_library(aglet ${aglet_srcs})
target_link_libraries(aglet PUBLIC ${aglet_libs})
target_compile_definitions(aglet PUBLIC ${aglet_defs})
target_include_directories(aglet PUBLIC "$<BUILD_INTERFACE:${AGLET_INCLUDE_DIRECTORIES}>")
set_property(TARGET aglet PROPERTY FOLDER "libs/aglet")

####
# Installation (https://github.com/forexample/package-example)

set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")
set(include_install_dir "include")

include(CMakePackageConfigHelpers)

# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file(
  "${version_config}" COMPATIBILITY SameMajorVersion
  )

# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}"
  )

install(
  FILES "${project_config}" "${version_config}"
  DESTINATION "${config_install_dir}"
  )

install(
  EXPORT "${TARGETS_EXPORT_NAME}"
  NAMESPACE "${namespace}"
  DESTINATION "${config_install_dir}"
  )

install(
  FILES
  aglet.h
  GLContext.h
  DESTINATION "${include_install_dir}/${PROJECT_NAME}"
)

install(
  TARGETS aglet
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
  RUNTIME DESTINATION "bin"
  INCLUDES DESTINATION "include"
  )
