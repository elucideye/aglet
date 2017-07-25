/*!
  @file   GLContextIOS
  @author David Hirvonen
  @brief  Declaration of minimal "hidden" OpenGL context for IOS.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#ifndef __aglet_GLContextIOS_h__
#define __aglet_GLContextIOS_h__

#include "aglet/GLContext.h"

#include <memory>

AGLET_BEGIN

class GLContextIOS : public GLContext
{
public:
    GLContextIOS(int width = 640, int height = 480, GLVersion version = kGLES20);
    ~GLContextIOS();

    virtual operator bool() const;
    virtual void operator()(); // make current

    virtual bool hasDisplay() const;
    virtual void resize(int width, int height);
    virtual void operator()(std::function<bool(void)>& f);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

AGLET_END

#endif // __aglet_GLContextIOS_h__
