#include <aglet/GLContext.h>
#include "aglet/gl_includes.h"
#include <gtest/gtest.h>

#include <iostream>

#if defined(AGLET_ANDROID) || defined(AGLET_LINUX)
#define TEXTURE_FORMAT GL_RGBA
#else
#define TEXTURE_FORMAT GL_BGRA
#endif

#if defined(AGLET_OPENGL_ES2) || defined(AGLET_OPENGL_ES3)
#  define AGLET_OPENGLES
#endif

#if defined(AGLET_OPENGL_ES2)
static const auto glKind = aglet::GLContext::kGLES20;
#elif defined(AGLET_OPENGL_ES3)
static const auto glKind = aglet::GLContext::kGLES30;
#else
static const auto glKind = aglet::GLContext::kGL;
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

    auto gl = aglet::GLContext::create(aglet::GLContext::kAuto, {}, width, height, glKind);
    check_gl_error();
    (*gl)();
    check_gl_error();
    ASSERT_TRUE(gl);

    glActiveTexture(GL_TEXTURE0);
    check_gl_error();

    (*gl)();

    image_rgba_t image0 = make_test_image(height, width);
    image_rgba_t image1(image0.size());

    GLFrameBufferObject fbo;
    GLTexture texture(width, height, TEXTURE_FORMAT, image0.data()->data());
    check_gl_error();
    fbo.bind();
    check_gl_error();

    fbo.attach(texture);
    check_gl_error();
    texture.read(image1.data()->data());

    ASSERT_TRUE(std::equal(image0.begin(), image0.end(), image1.begin()));
}

#define AGLET_TO_STR_(x) #x
#define AGLET_TO_STR(x) AGLET_TO_STR_(x)
#define AGLET_LOGINF(class_tag, fmt, ...)
#define AGLET_LOGERR(class_tag, fmt, ...)                                                                                   \
    do                                                                                                                      \
    {                                                                                                                       \
        fprintf(stderr, "ogles_gpgpu::%s - %s:%d:%s(): " fmt "\n", class_tag, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

// Real-world shader test from here:
// https://github.com/hunter-packages/ogles_gpgpu/blob/hunter/ogles_gpgpu/common/proc/rgb2luv.cpp

// clang-format off
const char* vshaderRgb2LuvSrc = AGLET_TO_STR(
  attribute vec4 aPos;
  attribute vec2 aTexCoord;
  varying vec2 vTexCoord;
  void main() {
    gl_Position = aPos;
    vTexCoord = aTexCoord;
  });

const char* fshaderRgb2LuvSrc = 
#if defined(AGLET_OPENGLES)
AGLET_TO_STR(precision highp float;)
#endif
AGLET_TO_STR(
  varying vec2 vTexCoord;
  uniform sampler2D uInputTex;

  const mat3 RGBtoXYZ = mat3(0.430574, 0.341550, 0.178325, 0.222015, 0.706655, 0.071330, 0.020183, 0.129553, 0.93918);

  const float y0 = 0.00885645167; // pow(6.0/29.0, 3.0)
  const float a = 903.296296296;  // pow(29.0/3.0, 3.0);
  const float un = 0.197833;
  const float vn = 0.468331;
  const float maxi = 0.0037037037;  // 1.0/270.0;
  const float minu = -88.0 * maxi;
  const float minv = -134.0 * maxi;
  const vec3 k = vec3(1.0, 15.0, 3.0);
  const vec3 RGBtoGray = vec3(0.2125, 0.7154, 0.0721);

  void main()
  {
      vec3 rgb = texture2D(uInputTex, vTexCoord).rgb;
      vec3 xyz = (rgb * RGBtoXYZ);
      float z = 1.0/(dot(xyz,k) + 1e-35);

      vec3 luv;
      float y = xyz.y;
      float l = ((y > y0) ? ((116.0 * pow(y, 0.3333333333)) - 16.0) : (y*a)) * maxi;
      luv.x = l;
      luv.y = l * ((52.0 * xyz.x * z) - (13.0*un)) - minu;
      luv.z = l * ((117.0 * xyz.y * z) - (13.0*vn)) - minv;

      gl_FragColor = vec4(vec3(luv.xyz), dot(rgb, RGBtoGray));
  }
);
// clang-format on

// Shader compiler inlined here for test purposes
// Source:
// https://github.com/hunter-packages/ogles_gpgpu/blob/hunter/ogles_gpgpu/common/gl/shader.cpp

typedef enum
{
    ATTR,
    UNIF
} ShaderParamType;

/** 
 * Shader helper class for creating and managing an OpenGL shader.
 */
class Shader
{
public:
    typedef std::pair<int, const char*> Attribute;
    typedef std::vector<Attribute> Attributes;

    /**
     * Constructor.
     */
    Shader()
    {
        programId = 0;
    }

    /**
     * Deconstructor.
     */
    ~Shader()
    {
        if (programId > 0)
        {
            AGLET_LOGINF("Shader", "deleting shader program");
            glDeleteProgram(programId);
        }
    }

    /**
     * Build an OpenGL shader object from vertex and fragment shader source code
     * <vshSrc> and <fshSrc>.
     */
    bool buildFromSrc(const char* vshSrc, const char* fshSrc, const std::vector<Attribute>& attributes = {})
    {
        programId = create(vshSrc, fshSrc, &vshId, &fshId, attributes);
        return (programId > 0);
    }

    /**
     * Use the shader program.
     */
    void use()
    {
        glUseProgram(programId);
    }

    /**
     * Get a shader parameter position for a parameter of type <type> and with
     * <name>.
     */
    GLint getParam(ShaderParamType type, const char* name) const
    {

        // get position according to type and name
        GLint id = (type == ATTR) ? glGetAttribLocation(programId, name) : glGetUniformLocation(programId, name);

        if (id < 0)
        {
            AGLET_LOGERR("Shader", "could not get parameter id for param %s", name);
        }

        return id;
    }

    /**
     * Get a shader parameter position for a parameter of type <type> and with
     * <name>.
     */
    GLuint getProgramId()
    {
        return programId;
    }

private:
    /**
     * Create a shader program from sources <vshSrc> and <fshSrc>. Save shader ids in
     * <vshId> and <fshId>.
     */
    GLuint create(const char* vshSrc, const char* fshSrc, GLuint* vshId, GLuint* fshId, const Attributes& attributes = {})
    {
        *vshId = compile(GL_VERTEX_SHADER, vshSrc);
        *fshId = compile(GL_FRAGMENT_SHADER, fshSrc);

        // create shader program
        GLuint programId = glCreateProgram();

        if (programId == 0)
        {
            AGLET_LOGERR("Shader", "could not create shader program");
            return 0;
        }

        glAttachShader(programId, *vshId); // add the vertex shader to program
        glAttachShader(programId, *fshId); // add the fragment shader to program

        // Bind attribute locations
        // this needs to be done prior to linking
        for (int i = 0; i < attributes.size(); i++)
        {
            glBindAttribLocation(programId, attributes[i].first, attributes[i].second);
        }

        glLinkProgram(programId); // link both shaders to a full program

        // check link status
        GLint linkStatus;
        glGetProgramiv(programId, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            AGLET_LOGERR("Shader", "could not link shader program. error log:");
            GLchar infoLogBuf[1024];
            GLsizei infoLogLen;
            glGetProgramInfoLog(programId, 1024, &infoLogLen, infoLogBuf);
            std::cerr << infoLogBuf << std::endl
                      << std::endl;

            glDeleteProgram(programId);

            return 0;
        }

        return programId;
    }

    /**
     * Compile a shader of type <type> and source <src> and return its id.
     */
    static GLuint compile(GLenum type, const char* src);

    GLuint programId; // full shader program id
    GLuint vshId;     // vertex shader id
    GLuint fshId;     // fragment shader id
};

GLuint Shader::compile(GLenum type, const char* src)
{

    // create a shader
    GLuint shId = glCreateShader(type);
    if (shId == 0)
    {
        AGLET_LOGERR("Shader", "could not create shader");
        return 0;
    }
    // set shader source
    glShaderSource(shId, 1, (const GLchar**)&src, NULL);
    // compile the shader
    glCompileShader(shId);
    // check compile status
    GLint compileStatus;
    glGetShaderiv(shId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE)
    {
        AGLET_LOGERR("Shader", "could not compile shader program. error log:");
        GLchar infoLogBuf[1024];
        GLsizei infoLogLen;
        glGetShaderInfoLog(shId, 1024, &infoLogLen, infoLogBuf);
        std::cerr << infoLogBuf << std::endl
                  << std::endl;
        AGLET_LOGERR("Shader", "could not compile shader program. shader source:");
        std::cerr << src << std::endl
                  << std::endl;

        glDeleteShader(shId);
        return 0;
    }
    return shId;
}

TEST(aglet, glShader)
{
    const int width = 640;
    const int height = 480;
    auto gl = aglet::GLContext::create(aglet::GLContext::kAuto, {}, width, height, glKind);
    check_gl_error();
    (*gl)();
    check_gl_error();
    ASSERT_TRUE(gl);
    Shader shader;
    auto value = shader.buildFromSrc(vshaderRgb2LuvSrc, fshaderRgb2LuvSrc);
    ASSERT_TRUE(value);
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
