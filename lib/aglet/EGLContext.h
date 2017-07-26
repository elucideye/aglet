/*!
  @file   EGLContext
  @author David Hirvonen
  @brief  Declaration of minimal "hidden" OpenGL context for Android.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#ifndef __aglet_EGLContext_h__
#define __aglet_EGLContext_h__

#include "aglet/GLContext.h"

#include <EGL/egl.h>
#include <android/log.h>
#include <android/window.h>
#include <dlfcn.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

AGLET_BEGIN

// NOTE: EGLContext is already a type!
struct EGLContextImpl : public GLContext
{
    EGLContextImpl(int width = 640, int height = 480, GLVersion kVersion = kGLES20);
    ~EGLContextImpl();

    virtual operator bool() const;
    virtual void operator()();

    virtual bool hasDisplay() const;
    virtual void resize(int width, int height);
    virtual void operator()(std::function<bool(void)>& f);

    EGLConfig eglConf;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglCtx = EGL_NO_CONTEXT;
    EGLDisplay eglDisp = EGL_NO_DISPLAY;
};

AGLET_END

#endif // __aglet_EGLContext_h__
