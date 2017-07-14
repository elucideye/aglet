/*!
  @file   GLContext.h
  @author David Hirvonen
  @brief  Declaration of a cross platform offscreen OpenGL context.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.
  \license{This project is released under the 3 Clause BSD License.}

*/

#ifndef __aglet_GLContext_h__
#define __aglet_GLContext_h__

#include "aglet/aglet.h"
#include <memory>
#include <string>
#include <functional>

AGLET_BEGIN

class GLContext
{
public:
    using GLContextPtr = std::shared_ptr<GLContext>;
    using RenderDelegate = std::function<bool(void)>;
    using CursorDelegate = std::function<void(double xpos, double ypos)>;

    enum ContextKind
    {
        kAuto,    // Select most portable context available:
        kGLFW,    // GLFW based (no mobile support)
        kIOS,     // iOS EAGLContext
        kAndroid, // Android EGL context
        kCount
    };

    enum GLVersion
    {
        kGLES20,
        kGLES30
    };

    struct Geometry
    {
        int width = 0;
        int height = 0;
        float tx = 0.f;
        float ty = 0.f;
        float sx = 1.f;
        float sy = 1.f;
    };

    GLContext() {}
    ~GLContext() {}

    GLContext(const std::string& name, int width, int height) {}

    virtual operator bool() const = 0;
    virtual void operator()() {} // make current

    virtual void setCursorCallback(const CursorDelegate& callback) {}
    virtual void setCursorVisibility(bool flag) {}
    virtual void setCursor(double x, double y) {}
    virtual void getCursor(double& x, double& y) {}
    virtual void setWait(bool flag) {}
    virtual bool hasDisplay() const = 0;
    virtual void resize(int width, int height) {}
    virtual void operator()(RenderDelegate& f){}; // render loop

    Geometry& getGeometry() { return m_geometry; }
    const Geometry& getGeometry() const { return m_geometry; }

    Geometry m_geometry;

    CursorDelegate cursorCallback;

    // Create context (w/ window if name is specified):
    static GLContextPtr create(
        ContextKind kind,
        const std::string& name = {},
        int width = 640,
        int height = 480,
        GLVersion version = kGLES20);
};

AGLET_END

#endif // __aglet_QGLContext_h__
