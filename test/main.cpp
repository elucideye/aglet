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

    void read(GLubyte* pixels)
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

namespace Tools
{
void checkGLErr(const char* cls, const char* msg)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        std::cerr << cls << " " << msg << " - GL error " << err << " occurred" << std::endl;
    }
}
}

class IPBO
{
public:
    /**
     * Constructor.
     */
    IPBO(std::size_t width, std::size_t height);

    /**
     * Destructor.
     */
    ~IPBO();

    /**
     * Bind the PBO (GL_PIXEL_PACK_BUFFER)
     */
    void bind();

    /**
     * Unbind the PBO (GL_PIXEL_PACK_BUFFER).
     */
    void unbind();

    /**
     * Start read operation (asynchronous/non-blocking).
     */
    void start(); // asynchronous

    /**
     * Pack/read pixels to <buffer> (blocking call) after a call to start().
     */
    void finish(GLubyte* buffer); // asynchronous

    /**
     * Perform a blocking pack from the PBO.
     */
    void read(GLubyte* buffer); // synchronous (start, finish)

    /**
     * Returns current processing state for asynchronous read.
     */
    bool isReadingAsynchronously() const;

protected:
    bool isReadingAsynchronously_ = false; // read state (async API)
    std::size_t width;                     // width of PBO
    std::size_t height;                    // head of PBO
    GLuint pbo;                            // ID of created PBO
};

class OPBO
{
public:
    /**
     * Constructor.
     */
    OPBO(std::size_t width, std::size_t height);

    /**
     * Destructor.
     */
    ~OPBO();

    /**
     * Bind the PBO (GL_PIXEL_UNPACK_BUFFER)
     */
    void bind();

    /**
     * Unbind the PBO (GL_PIXEL_UNPACK_BUFFER)
     */
    void unbind();

    /**
     * Unpack/write pixels in <buffer> to texture <texId>.
     */
    void write(const GLubyte* buffer, GLuint texId = 0);

protected:
    std::size_t width;  // width of PBO
    std::size_t height; // head of PBO
    GLuint pbo;         // ID of created PBO
};

IPBO::IPBO(std::size_t width, std::size_t height)
    : width(width)
    , height(height)
    , isReadingAsynchronously_(false)
{

    glGenBuffers(1, &pbo);
    Tools::checkGLErr("IPBO::IPBO", "glGenBuffers()");

    std::size_t pbo_size = width * height * 4;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    Tools::checkGLErr("IPBO::IPBO", "glBindBuffer()");

    glBufferData(GL_PIXEL_PACK_BUFFER, pbo_size, 0, GL_DYNAMIC_READ);
    Tools::checkGLErr("IPBO::IPBO", "glBufferData()");

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    Tools::checkGLErr("IPBO::IPBO", "glBindBuffer()");
}

IPBO::~IPBO()
{
    if (pbo > 0)
    {
        glDeleteBuffers(1, &pbo);
        Tools::checkGLErr("IPBO::~IPBO", "glDeleteBuffers()");
        pbo = 0;
    }
}

bool IPBO::isReadingAsynchronously() const
{
    return isReadingAsynchronously_;
}

void IPBO::bind()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    Tools::checkGLErr("IPBO::bind", "glBindBuffer()");
}

void IPBO::unbind()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    Tools::checkGLErr("IPBO::unbind", "glBindBuffer()");
}

void IPBO::start()
{

    if (!isReadingAsynchronously_)
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        Tools::checkGLErr("IPBO::start", "glReadBuffer()");

        // Note glReadPixels last argument == 0 for PBO reads
        glReadPixels(0, 0, width, height, TEXTURE_FORMAT, GL_UNSIGNED_BYTE, 0);
        Tools::checkGLErr("IPBO::start", "glReadPixels()");

        isReadingAsynchronously_ = true;
    }
}

void IPBO::finish(GLubyte* buffer)
{

    if (isReadingAsynchronously_)
    {
        std::size_t pbo_size = width * height * 4;
#if defined(AGLET_OSX)
        // Note: glMapBufferRange does not seem to work in OS X
        GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
#else
        GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pbo_size, GL_MAP_READ_BIT));
#endif
        Tools::checkGLErr("IPBO::finish", "glMapBufferRange()");

        if (ptr)
        {
            memcpy(buffer, ptr, pbo_size);

            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            Tools::checkGLErr("IPBO::finish", "glUnmapBuffer()");
        }

        isReadingAsynchronously_ = false;
    }
}

void IPBO::read(GLubyte* buffer)
{
    start();        // Use start() and
    finish(buffer); // finish() pair for consistent internal state
}

// ::: output/write  :::

OPBO::OPBO(std::size_t width, std::size_t height)
    : width(width)
    , height(height)
{

    glGenBuffers(1, &pbo);
    Tools::checkGLErr("OPBO::OPBO", "glGenBuffers()");

    std::size_t pbo_size = width * height * 4;
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    Tools::checkGLErr("OPBO::OPBO", "glBindBuffer()");

    glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo_size, 0, GL_STREAM_DRAW);
    Tools::checkGLErr("OPBO::OPBO", "glBufferData()");

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    Tools::checkGLErr("OPBO::OPBO", "glBindBuffer()");
}

OPBO::~OPBO()
{
    if (pbo > 0)
    {
        glDeleteBuffers(1, &pbo);
        Tools::checkGLErr("OPBO::~OPBO", "glDeleteBuffers()");
        pbo = 0;
    }
}

void OPBO::bind()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    Tools::checkGLErr("OPBO::bind", "glBindBuffer()");
}

void OPBO::unbind()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    Tools::checkGLErr("OPBO::unbind", "glBindBuffer()");
}

void OPBO::write(const GLubyte* buffer, GLuint texId)
{
    std::size_t pbo_size = width * height * 4;

    glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo_size, NULL, GL_STREAM_DRAW);
    Tools::checkGLErr("OPBO::write", "glBufferData()");

#if defined(AGLET_OSX)
    // TODO: glMapBufferRange does not seem to work in OS X
    GLubyte* ptr = static_cast<GLubyte*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));
    Tools::checkGLErr("OPBO::write", "glMapBuffer()");
#else
    GLubyte* ptr = static_cast<GLubyte*>(glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, pbo_size, GL_MAP_WRITE_BIT));
    Tools::checkGLErr("OPBO::write", "glMapBufferRange()");
#endif

    if (ptr)
    {
        memcpy(ptr, buffer, pbo_size);

        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        Tools::checkGLErr("OPBO::write", "glUnmapBuffer()");

        glBindTexture(GL_TEXTURE_2D, texId);
        Tools::checkGLErr("OPBO::write", "glBindTexture()");

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, TEXTURE_FORMAT, GL_UNSIGNED_BYTE, 0);
        Tools::checkGLErr("OPBO::write", "glTexSubImage2D()");
    }
}

#endif

int gauze_main(int argc, char** argv)
{
    try
    {
        ::testing::InitGoogleTest(&argc, argv);
        auto code = RUN_ALL_TESTS();
        return code;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Unknown exception" << std::endl;
    }
    return 1; // return non-zero
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
            std::uint8_t value = (y + 1) % 255;
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

    { // Write pixels:
        OPBO pbo(width, height);
        pbo.bind();
        pbo.write(image0.data()->data(), texture);
        pbo.unbind();
    }

    { // Read pixels:
        GLFrameBufferObject fbo;
        fbo.bind();
        fbo.attach(texture);

        IPBO pbo(width, height);
        pbo.bind();
        pbo.read(image1.data()->data());
        pbo.unbind();

        fbo.unbind();
    }

    ASSERT_TRUE(std::equal(image0.begin(), image0.end(), image1.begin()));
}
#endif
