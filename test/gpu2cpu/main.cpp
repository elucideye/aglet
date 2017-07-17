#include <aglet/GLContext.h>

#include <gtest/gtest.h>

#include <iostream>

#ifdef ANDROID
#define TEXTURE_FORMAT GL_RGBA
#else
#define TEXTURE_FORMAT GL_BGRA
#endif

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

class GLFrameBufferObject
{
public:
    GLFrameBufferObject()
    {
        glGenFramebuffers(1, &id);
        check_gl_error();
    }

    ~GLFrameBufferObject()
    {
        glDeleteFramebuffers(1, &id);
        check_gl_error();
    }

    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, id);
        check_gl_error();
    }

    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        check_gl_error();
    }

    void attach(GLuint texId, GLenum attachment = GL_COLOR_ATTACHMENT0, GLenum target = GL_TEXTURE_2D)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, target, texId, 0);
        check_gl_error();
        GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        check_gl_error();
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
    : width(width)
    , height(height)
    {
        glGenTextures(1, &texId);
        check_gl_error();
        bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        check_gl_error();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        check_gl_error();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        check_gl_error();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        check_gl_error();
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        check_gl_error();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, texType, GL_UNSIGNED_BYTE, data);
        check_gl_error();
        unbind();
    }
    
    ~GLTexture()
    {
        glDeleteTextures(1, &texId);
        check_gl_error();
    }
    
    void bind()
    {
        glBindTexture(GL_TEXTURE_2D, texId);
        check_gl_error();
    }
    
    void unbind()
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        check_gl_error();
    }
    
    void read(GLubyte *pixels)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        check_gl_error();
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        check_gl_error();
    }
    
    operator GLuint() const
    {
        return texId;
    }
    
protected:
    
    std::size_t width;
    std::size_t height;
    GLuint texId;
};

#if defined(AGLET_OPENGL_ES3)
class GLPixelBufferObjectReader
{
public:
    GLPixelBufferObjectReader(std::size_t width, std::size_t height)
        : width(width)
        , height(height)
    {    
        glGenBuffers(1, &pbo);
        check_gl_error();
        
        std::size_t pbo_size = width * height * 4;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        check_gl_error();
        glBufferData(GL_PIXEL_PACK_BUFFER, pbo_size, 0, GL_DYNAMIC_READ);
        check_gl_error();
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        check_gl_error();
    }

    ~GLPixelBufferObjectReader()
    {
        if (pbo > 0)
        {
            glDeleteBuffers(1, &pbo);
            check_gl_error();
            pbo = 0;
        }
    }

    void bind()
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
        check_gl_error();
    }

    void unbind()
    {
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
        check_gl_error();
    }

    void read(GLubyte *buffer)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        check_gl_error();

        // Note glReadPixels last argument == 0 for PBO reads
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0); // GL_BGRA
        check_gl_error();

        std::size_t pbo_size = width * height * 4;
#if defined(AGLET_OSX)
        // TODO: glMapBufferRange does not seem to work in OS X
        GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
#else
        GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pbo_size, GL_MAP_READ_BIT));
#endif
        check_gl_error();
      
        memcpy(buffer, ptr, pbo_size);
        
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        check_gl_error();
    }

    std::size_t width;
    std::size_t height;
    GLuint pbo;    
};

class GLPixelBufferObjectWriter
{
public:
    GLPixelBufferObjectWriter(std::size_t width, std::size_t height)
        : width(width)
        , height(height)
    {    
        glGenBuffers(1, &pbo);
        check_gl_error();

        std::size_t pbo_size = width * height * 4;
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        check_gl_error();
        glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo_size, 0, GL_STREAM_DRAW);
        check_gl_error();
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        check_gl_error();
    }

    ~GLPixelBufferObjectWriter()
    {
        if (pbo > 0)
        {
            glDeleteBuffers(1, &pbo);
            check_gl_error();
            pbo = 0;
        }
    }

    void bind()
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        check_gl_error();
    }

    void unbind()
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        check_gl_error();
    }

    void write(GLubyte *buffer)
    {
        std::size_t pbo_size = width * height * 4;
        
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
        check_gl_error();
        
        glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo_size, NULL, GL_STREAM_DRAW);
        check_gl_error();
        
        
#if defined(AGLET_OSX)
        // TODO: glMapBufferRange does not seem to work in OS X
        GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
#else
        GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pbo_size, GL_MAP_WRITE_BIT));
#endif
        check_gl_error();
        
        if(ptr)
        {
            memcpy(ptr, buffer, pbo_size);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            check_gl_error();
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, TEXTURE_FORMAT, GL_UNSIGNED_BYTE, 0);
        check_gl_error();
        
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        check_gl_error();
    }

    std::size_t width;
    std::size_t height;
    GLuint pbo;    
};
    
#endif

int gauze_main(int argc, char** argv)
{
    try
    {
        ::testing::InitGoogleTest(&argc, argv);
        auto code = RUN_ALL_TESTS();
        
        std::cout << "CODE: " << code << std::endl;
        return code;
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown exception" << std::endl;
    }
    return -1; // return non-zero
}

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
            std::uint8_t value = (y+1) % 255;
            image[y * cols + x] = { { value, value, value, 255 } };
        }
    }
    return image;
}

TEST(aglet, glReadPixels)
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
    image_rgba_t image1(image0.size());

    GLFrameBufferObject fbo;
    GLTexture texture(width, height, TEXTURE_FORMAT, image0.data()->data());
    fbo.bind();
    fbo.attach(texture);
    texture.read(image1.data()->data());

    ASSERT_TRUE(std::equal(image0.begin(), image0.end(), image1.begin()));
}

#if defined(AGLET_OPENGL_ES3)
TEST(aglet, pbo)
{
    const int width = 640;
    const int height = 480;
    auto gl = aglet::GLContext::create(aglet::GLContext::kAuto, {}, width, height, aglet::GLContext::kGLES30);
    check_gl_error();

    (*gl)();
    check_gl_error();

    ASSERT_TRUE(gl);

    glActiveTexture(GL_TEXTURE0);
    check_gl_error();

    image_rgba_t image0 = make_test_image(height, width);
    image_rgba_t image1(image0.size());
    
    GLTexture texture(width, height, TEXTURE_FORMAT, 0);

    GLFrameBufferObject fbo;
    fbo.bind();
    fbo.attach(texture);
    texture.bind();

    // Write pixels:
    GLPixelBufferObjectWriter pbo_w(width, height);
    pbo_w.bind();
    pbo_w.write(image0.data()->data());
    pbo_w.unbind();
    
    // Read pixels:
    GLPixelBufferObjectReader pbo_r(width, height);
    pbo_r.bind();
    pbo_r.read(image1.data()->data());
    pbo_r.unbind();
    
//    for(int i = 0; i < 4; i++)
//    {
//        std::cout << "value: " << int(image0[0][i]) << std::endl;
//        std::cout << "value: " << int(image1[0][i]) << std::endl;
//    }
    
    ASSERT_TRUE(std::equal(image0.begin(), image0.end(), image1.begin()));
}
#endif
