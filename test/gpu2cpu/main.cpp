#include <aglet/GLContext.h>

#include <gtest/gtest.h>

#include <iostream>

class GLFrameBufferObject
{
public:
    GLFrameBufferObject()
    {
        glGenFramebuffers(1, &id);
    }

    ~GLFrameBufferObject()
    {
        glDeleteFramebuffers(1, &id);
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void attach(GLuint texId, GLenum attachment = GL_COLOR_ATTACHMENT0, GLenum target = GL_TEXTURE_2D)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texId, 0);
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
        {
            std::stringstream ss;
            ss << "Framebuffer incomplete :" << int(fboStatus);
            throw std::runtime_error(ss.str());
        }
    }

protected:
    GLuint id;
};

class GLTexture
{
public:
    GLTexture(std::size_t width, std::size_t height, GLenum texType, void* data)
    {
        glGenTextures(1, &texId);
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, texType, GL_UNSIGNED_BYTE, data);
        unbind();
    }

    ~GLTexture()
    {
        glDeleteTextures(1, &texId);
    }

    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, texId);
    }

    void unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    operator GLuint() const
    {
        return texId;
    }

protected:
    GLuint texId;
};

int gauze_main(int argc, char** argv)
{
    try
    {
        ::testing::InitGoogleTest(&argc, argv);
        auto code = RUN_ALL_TESTS();
        return code;
    }
    catch (...)
    {
    }
    return -1; // return non-zero
}

#ifdef ANDROID
#define TEXTURE_FORMAT GL_RGBA
#else
#define TEXTURE_FORMAT GL_BGRA
#endif

#include <vector>
#include <array>

using rgba_t = std::array<std::uint8_t, 4>;
using image_rgba_t = std::vector<rgba_t>;

static image_rgba_t make_test_image(int rows, int cols)
{
    image_rgba_t image(rows * cols);
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            std::uint8_t value = y % 255;
            image[y * cols + x] = { { value, value, value, 255 } };
        }
    }
    return image;
}

static void check_gl_error()
{
    auto e = glGetError();
    if (e != GL_NO_ERROR)
    {
        std::stringstream ss;
        ss << "OpenGL error: " << int(e);
        throw std::runtime_error(ss.str());
    }
}

TEST(aglet, gpu2cpu)
{
    const int width = 640;
    const int height = 480;
    auto gl = aglet::GLContext::create(aglet::GLContext::kAuto, {}, width, height);
    check_gl_error();

    (*gl)();
    check_gl_error();

    ASSERT_TRUE(gl);

    glActiveTexture(GL_TEXTURE0);
    check_gl_error();

    image_rgba_t image0 = make_test_image(height, width);

    GLFrameBufferObject fbo;
    check_gl_error();

    GLTexture texture(width, height, TEXTURE_FORMAT, image0.data()->data());
    check_gl_error();

    fbo.bind();
    check_gl_error();

    fbo.attach(texture);
    check_gl_error();

    image_rgba_t image1(image0.size());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, image1.data()->data());
    check_gl_error();

    ASSERT_TRUE(std::equal(image0.begin(), image0.end(), image1.begin()));
}
