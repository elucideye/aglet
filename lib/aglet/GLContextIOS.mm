/*!
  @file   GLContextIOS.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" GLFW based OpenGL context.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/GLContextIOS.h"
#include "aglet/core/make_unique.h"

#import <UIKit/UIKit.h>

AGLET_BEGIN

struct GLContextIOS::Impl
{
    Impl()
    {
        egl = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        [EAGLContext setCurrentContext:egl];
    }
    
    EAGLContext *egl = nullptr;
};

GLContextIOS::GLContextIOS()
{
    impl = aglet::core::make_unique<Impl>();
}

GLContextIOS::~GLContextIOS()
{
    
}

GLContextIOS::operator bool() const
{
    return (impl && impl->egl);
}

// Display:
bool GLContextIOS::hasDisplay() const
{
    return false;
}

void GLContextIOS::resize(int width, int height)
{
    // noop
}

void GLContextIOS::operator()(std::function<bool(void)> &f)
{
    while(f()) {}
}

AGLET_END

