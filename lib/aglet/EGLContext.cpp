/*!
  @file   EGLContext.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" OpenGL context for Android.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/EGLContext.h"
#include "aglet/aglet_assert.h"

#include <iostream>

AGLET_BEGIN

EGLContextImpl::EGLContextImpl(int width, int height, GLVersion /*kVersion*/)
{
    // EGL config attributes
    const EGLint confAttr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // very important! (ES3 is not defined)
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT, // we will create a pixelbuffer surface
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
    throw_assert((eglGetError() == EGL_SUCCESS), "EGLContextImpl::EGLContextImpl() : eglGetDisplay()");
    throw_assert((eglDisp != EGL_NO_DISPLAY), "EGLContextImpl::EGLContextImpl() : eglGetDisplay()");

    eglInitialize(eglDisp, &eglMajVers, &eglMinVers);
    throw_assert((eglGetError() == EGL_SUCCESS), "EGLContextImpl::EGLContextImpl() : eglInitialize()");

    eglChooseConfig(eglDisp, confAttr, &eglConf, 1, &numConfigs);
    throw_assert((eglGetError() == EGL_SUCCESS), "EGLContextImpl::EGLContextImpl() : eglChooseConfig()");

    eglSurface = eglCreatePbufferSurface(eglDisp, eglConf, surfaceAttr);
    throw_assert((eglGetError() == EGL_SUCCESS), "EGLContextImpl::EGLContextImpl() : eglCreatePbufferSurface()");
    throw_assert((eglSurface != EGL_NO_SURFACE), "EGLContextImpl::EGLContextImpl() : eglCreatePbufferSurface()");

    eglCtx = eglCreateContext(eglDisp, eglConf, EGL_NO_CONTEXT, ctxAttr);
    throw_assert((EGL_SUCCESS == eglGetError()), "EGLContextImpl::EGLContextImpl() : eglCreateContext()");
    throw_assert((eglCtx != EGL_NO_CONTEXT), "EGLContextImpl::EGLContextImpl() : eglCreateContext()");

    eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);
    throw_assert((eglGetError() == EGL_SUCCESS), "EGLContextImpl::EGLContextImpl() : eglMakeCurrent()");
}

EGLContextImpl::~EGLContextImpl()
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

EGLContextImpl::operator bool() const
{
    return (eglCtx != EGL_NO_CONTEXT);
}

void EGLContextImpl::operator()()
{
    auto status = eglMakeCurrent(eglDisp, eglSurface, eglSurface, eglCtx);
    throw_assert(status, "EGLContextImpl::operator()() : eglMakeCurrent()");
    throw_assert(eglGetError() == EGL_SUCCESS, "EGLContextImpl::operator()() : eglMakeCurrent()");
}

// Display:
bool EGLContextImpl::hasDisplay() const
{
    return false;
}

void EGLContextImpl::resize(int width, int height)
{
    // noop
}

void EGLContextImpl::operator()(std::function<bool(void)>& f)
{
    while (f())
    {
    }
}

AGLET_END
