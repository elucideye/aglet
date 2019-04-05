/*!
  @file   GLFWContext.cpp
  @author David Hirvonen
  @brief  Implementation of minimal "hidden" GLFW based OpenGL context.

  \copyright Copyright 2017 Elucideye, Inc. All rights reserved.

*/

#include "aglet/GLFWContext.h"
#include "aglet/aglet_assert.h"
#include "aglet/gl_includes.h"

#include <map>
#include <mutex>
#include <cmath>

#include <assert.h>

AGLET_BEGIN

static void GLFWContextError(int code, const char* text)
{
    throw_assert(code != 0, text);
}

struct GLFWContextPool
{
    std::recursive_mutex mutex;

    void alloc(GLFWContext* src, const std::string& name, int width, int height)
    {
        std::unique_lock<decltype(mutex)> lock(mutex);
        if (pool.empty())
        {
	    if (!glfwInit())
	    {
	        glfwTerminate();
		throw_assert(false, "glfwInit()");
	    }
        }
	
        glfwSetErrorCallback(GLFWContextError);

        src->alloc(name, width, height);
        auto* context = src->getContext();
        if (!context)
        {
            glfwTerminate();
            throw_assert(context, "getContext()");
        }

        pool[context] = src;
    }

    void erase(GLFWContext* context)
    {
        std::unique_lock<decltype(mutex)> lock(mutex);
        auto iter = pool.find(context->getContext());
        assert(iter != pool.end());
        glfwDestroyWindow(iter->first);
        pool.erase(iter);
        if (pool.empty())
	{
            glfwTerminate();
        }
    }

    // Will not add if missing
    GLFWContext* operator[](GLFWwindow* window)
    {
        std::unique_lock<decltype(mutex)> lock(mutex);
        auto iter = pool.find(window);
        assert(iter != pool.end());
        return iter->second;
    }

    std::map<GLFWwindow*, GLFWContext*> pool;
};

static GLFWContextPool glfwPool;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void GLFWContext::alloc(const std::string& name, int width, int height)
{
    if (name.empty())
    {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    m_context = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (!m_context)
    {
        glfwTerminate();
        throw_assert(m_context, "glfwCreateWindow()");
    }
    m_geometry.width = width;
    m_geometry.height = height;

    glfwSetFramebufferSizeCallback(m_context, framebuffer_size_callback);
    glfwMakeContextCurrent(m_context);

#if defined(AGLET_HAS_GLEW)
    throw_assert(!glewInit(), "glewInit()")
#endif

    glfwGetFramebufferSize(m_context, &width, &height);

    framebufferSizeCallback(width, height);

    if (!name.empty())
    {
        glfwShowWindow(m_context);
        m_visible = true;
    }
    else
    {
        m_visible = false;
    }
}

GLFWContext::GLFWContext(const std::string& name, int width, int height)
{
    // forcing centralized allocation ensures proper reference counting
    glfwPool.alloc(this, name, width, height);
}

GLFWContext::~GLFWContext()
{
    glfwPool.erase(this);
}

void GLFWContext::operator()()
{
    glfwMakeContextCurrent(m_context);
}

GLFWContext::operator bool() const
{
    return (m_context != nullptr);
}

void GLFWContext::getCursor(double& x, double& y)
{
    glfwGetCursorPos(m_context, &x, &y);
}

void GLFWContext::setCursor(double x, double y)
{
    glfwSetCursorPos(m_context, x, y);
}

void GLFWContext::setCursorVisibility(bool flag)
{
    glfwSetInputMode(m_context, GLFW_CURSOR, flag ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto* context = glfwPool[window];
    assert(context);
    if (context->cursorCallback)
    {
        context->cursorCallback(xpos, ypos);
    }
}

void GLFWContext::setCursorCallback(const CursorDelegate& delegate)
{
    cursorCallback = delegate;
    glfwSetCursorPosCallback(m_context, mouse_callback);
}

// ::: display :::

bool GLFWContext::hasDisplay() const
{
    return m_visible;
}

void GLFWContext::resize(int width, int height)
{
    m_geometry.width = width;
    m_geometry.height = height;
    glfwSetWindowSize(m_context, width, height);
}

void GLFWContext::operator()(std::function<bool(void)>& f)
{
    bool okay = true;
    while (!glfwWindowShouldClose(m_context) && okay)
    {
        if (m_wait)
        {
            glfwWaitEvents();
        }
        else
        {
            glfwPollEvents();
        }
        okay = f(); // <== callback
        glfwSwapBuffers(m_context);
    }
    glfwTerminate();
}

void GLFWContext::framebufferSizeCallback(int width, int height)
{
    const float wScale = static_cast<float>(width) / static_cast<float>(m_geometry.width);
    const float hScale = static_cast<float>(height) / static_cast<float>(m_geometry.height);
    const float minScale = (wScale < hScale) ? wScale : hScale;
    const float winWidth = minScale * m_geometry.width;
    const float winHeight = minScale * m_geometry.height;
    const int wShift = static_cast<int>(std::nearbyint((width - winWidth) / 2.0f));
    const int hShift = static_cast<int>(std::nearbyint((height - winHeight) / 2.0f));

    m_geometry.sx = minScale;
    m_geometry.sy = minScale;
    m_geometry.tx = wShift;
    m_geometry.ty = hShift;

    glfwMakeContextCurrent(m_context);
    glViewport(wShift, hShift, std::nearbyint(winWidth), std::nearbyint(winHeight));
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto* context = glfwPool[window];
    assert(context);
    context->framebufferSizeCallback(width, height);
}

AGLET_END
