#include "aglet/GLContext.h"
#include "aglet/gl_includes.h"

#include <assert.h>

// clang-format off
#if defined(AGLET_HAS_GLFW)
#  include "aglet/GLFWContext.h"
#endif
// clang-format on

// clang-format off
#if defined(AGLET_IOS)
#  include "aglet/GLContextIOS.h"
#endif
// clang-format on

// clang-format off
#if defined(AGLET_EGL)
#  include "aglet/EGLContext.h"
#endif
// clang-format on

#include <memory>

AGLET_BEGIN

auto GLContext::create(ContextKind kind, const std::string& name, int width, int height, GLVersion version) -> GLContextPtr
{
    switch (kind)
    {
        case kAuto:

#if defined(AGLET_IOS)
        case kIOS:
            return std::make_shared<aglet::GLContextIOS>(width, height, version);
#endif

#if defined(AGLET_EGL)
        case kEGL:
            return std::make_shared<aglet::EGLContextImpl>(width, height, version);
#endif

#if defined(AGLET_HAS_GLFW)
        case kGLFW:
            return std::make_shared<aglet::GLFWContext>(name, width, height);
#endif

        default:
            assert(false);
            break;
    }

    return nullptr;
}

AGLET_END
