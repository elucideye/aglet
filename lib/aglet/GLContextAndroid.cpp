/*!
  @file   GLContextAndroid.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" OpenGL context for Android.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/GLContextAndroid.h"
#include "aglet/aglet_assert.h"

#include <iostream>

AGLET_BEGIN

GLContextAndroid::GLContextAndroid(int width, int height)
{
    // EGL config attributes
    const EGLint confAttr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // very important!
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,       // we will create a pixelbuffer surface
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,  // if you need the alpha channel
        EGL_DEPTH_SIZE, 16, // if you need the depth buffer
        EGL_NONE
    };

    // EGL context attributes
    const EGLint ctxAttr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2, // very important!
        EGL_NONE
    };

    // surface attributes
    // the surface size is set to the input frame size
    const EGLint surfaceAttr[] = {
        EGL_WIDTH, width,
        EGL_HEIGHT, height,
        EGL_NONE
    };

    EGLint eglMajVers, eglMinVers;
    EGLint numConfigs;

    eglDisp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    throw_assert((eglGetError() == EGL_SUCCESS), "GLContextAndroid::GLContextAndroid() : eglGetDisplay()");
    throw_assert((eglDisp != EGL_NO_DISPLAY), "GLContextAndroid::GLContextAndroid() : eglGetDisplay()");

    eglInitialize(eglDisp, &eglMajVers, &eglMinVers);
    throw_assert((eglGetError() == EGL_SUCCESS), "GLContextAndroid::GLContextAndroid() : eglInitialize()");

    eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs);
    throw_assert((eglGetError() == EGL_SUCCESS), "GLContextAndroid::GLContextAndroid() : eglChooseConfig()");

    eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);
    throw_assert((eglGetError() == EGL_SUCCESS), "GLContextAndroid::GLContextAndroid() : eglCreatePbufferSurface()");
    throw_assert((eglSurface != EGL_NO_SURFACE), "GLContextAndroid::GLContextAndroid() : eglCreatePbufferSurface()");

    eglCtx = eglCreateContext(eglDisp, eglConf, EGL_NO_CONTEXT, ctxAttr);
    throw_assert((EGL_SUCCESS == eglGetError()), "GLContextAndroid::GLContextAndroid() : eglCreateContext()");
    throw_assert((eglCtx != EGL_NO_CONTEXT), "GLContextAndroid::GLContextAndroid() : eglCreateContext()");

    eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);
    throw_assert((eglGetError() == EGL_SUCCESS), "GLContextAndroid::GLContextAndroid() : eglMakeCurrent()");
}

GLContextAndroid::~GLContextAndroid()
{
    if (eglCtx != EGL_NO_CONTEXT)
    {
        eglDestroyContext(eglDisp, eglCtx);
        eglCtx = EGL_NO_CONTEXT;
    }

    if (eglSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(eglDisp, eglSurface);
        eglSurface = EGL_NO_SURFACE;
    }

    if (eglDisp != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(eglDisp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(eglDisp);
        eglDisp = EGL_NO_DISPLAY;
    }
}

GLContextAndroid::operator bool() const
{
    return (eglCtx != EGL_NO_CONTEXT);
}

void GLContextAndroid::operator()()
{
    auto status = eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);
    throw_assert(status, "GLContextAndroid::operator()() : eglMakeCurrent()");
    throw_assert(eglGetError() == EGL_SUCCESS, "GLContextAndroid::operator()() : eglMakeCurrent()");
}

// Display:
bool GLContextAndroid::hasDisplay() const
{
    return false;
}

void GLContextAndroid::resize(int width, int height)
{
    // noop
}

void GLContextAndroid::operator()(std::function<bool(void)>& f)
{
    while (f())
    {
    }
}

AGLET_END
