/*!
  @file   GLContextIOS.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" GLFW based OpenGL context.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/GLContextIOS.h"

#import <UIKit/UIKit.h>

#include <memory>

AGLET_BEGIN

// Workaround for c++11 targets
template <typename Value, typename... Arguments>
std::unique_ptr<Value> make_unique(Arguments&&... arguments_for_constructor)
{
    return std::unique_ptr<Value>(new Value(std::forward<Arguments>(arguments_for_constructor)...));
}

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
    impl = make_unique<Impl>();
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

