/*!
  @file   GLContextIOS.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" GLFW based OpenGL context.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/GLContextIOS.h"
#include "aglet/aglet_assert.h"
#include "aglet/gl_includes.h"

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
    Impl(int /*width*/, int /*height*/,  GLContext::GLVersion version)
    {
        switch(version)
        {
            case kGLES20: egl = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]; break;
            case kGLES30: egl = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]; break;
        }
        throw_assert(egl, "EAGLContexfft initWithAPI");

        auto status = [EAGLContext setCurrentContext:egl];
        throw_assert(status, "EAGLContext setCurrentContext");
    }

    void operator()()
    {
        auto status = [EAGLContext setCurrentContext:egl];
        throw_assert(status, "EAGLContext setCurrentContext");        
    }
    
    EAGLContext *egl = nullptr;
};

GLContextIOS::GLContextIOS(int width, int height, GLVersion version)
{
    impl = make_unique<Impl>(width, height, version);
}

GLContextIOS::~GLContextIOS()
{
    
}

GLContextIOS::operator bool() const
{
    return (impl && impl->egl);
}

void GLContextIOS::operator()()
{
    if(impl)
    {
        (*impl)();
    }
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

