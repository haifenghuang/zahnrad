/*
    Copyright (c) 2016 Micha Mettke

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1.  The origin of this software must not be misrepresented; you must not
        claim that you wrote the original software. If you use this software
        in a product, an acknowledgment in the product documentation would be
        appreciated but is not required.
    2.  Altered source versions must be plainly marked as such, and must not be
        misrepresented as being the original software.
    3.  This notice may not be removed or altered from any source distribution.
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glxext.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include "../../zahnrad.h"
#include "../demo.c"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wfloat-equal"
#pragma clang diagnostic ignored "-Wbad-function-cast"
#pragma clang diagnostic ignored "-Wcast-qual"
#pragma clang diagnostic ignored "-Wshadow"
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wdeclaration-after-statement"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wbad-function-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wunused-function"
#elif _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif _MSC_VER
#pragma warning (pop)
#endif

/* prefered OpenGL version */
#define OGL_MAJOR_VERSION 3
#define OGL_MINOR_VERSION 0

#define MAX_VERTEX_MEMORY 512 * 1024
#define MAX_ELEMENT_MEMORY 128 * 1024
#define UNUSED(a)   ((void)(a))

typedef GLXContext(*glxCreateContext)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
/* GL_ARB_vertex_buffer_object */
typedef void(*qglGenBuffers)(GLsizei, GLuint*);
typedef void(*qglBindBuffer)(GLenum, GLuint);
typedef void(*qglBufferData)(GLenum, GLsizeiptr, const GLvoid*, GLenum);
typedef void(*qglBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid*);
typedef void*(*qglMapBuffer)(GLenum, GLenum);
typedef GLboolean(*qglUnmapBuffer)(GLenum);
typedef void(*qglDeleteBuffers)(GLsizei, GLuint*);
/* GL_ARB_vertex_array_object */
typedef void (*qglGenVertexArrays)(GLsizei, GLuint*);
typedef void (*qglBindVertexArray)(GLuint);
typedef void (*qglDeleteVertexArrays)(GLsizei, const GLuint*);
/* GL_ARB_vertex_program / GL_ARB_fragment_program */
typedef void(*qglVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*);
typedef void(*qglEnableVertexAttribArray)(GLuint);
typedef void(*qglDisableVertexAttribArray)(GLuint);
/* GL_ARB_framebuffer_object */
typedef void(*qglGenerateMipmap)(GLenum target);
/* GLSL/OpenGL 2.0 core */
typedef GLuint(*qglCreateShader)(GLenum);
typedef void(*qglShaderSource)(GLuint, GLsizei, const GLchar**, const GLint*);
typedef void(*qglCompileShader)(GLuint);
typedef void(*qglGetShaderiv)(GLuint, GLenum, GLint*);
typedef void(*qglGetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void(*qglDeleteShader)(GLuint);
typedef GLuint(*qglCreateProgram)(void);
typedef void(*qglAttachShader)(GLuint, GLuint);
typedef void(*qglDetachShader)(GLuint, GLuint);
typedef void(*qglLinkProgram)(GLuint);
typedef void(*qglUseProgram)(GLuint);
typedef void(*qglGetProgramiv)(GLuint, GLenum, GLint*);
typedef void(*qglGetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*);
typedef void(*qglDeleteProgram)(GLuint);
typedef GLint(*qglGetUniformLocation)(GLuint, const GLchar*);
typedef GLint(*qglGetAttribLocation)(GLuint, const GLchar*);
typedef void(*qglUniform1i)(GLint, GLint);
typedef void(*qglUniform1f)(GLint, GLfloat);
typedef void(*qglUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat*);
typedef void(*qglUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*);

static qglGenBuffers glGenBuffers;
static qglBindBuffer glBindBuffer;
static qglBufferData glBufferData;
static qglBufferSubData glBufferSubData;
static qglMapBuffer glMapBuffer;
static qglUnmapBuffer glUnmapBuffer;
static qglDeleteBuffers glDeleteBuffers;
static qglGenVertexArrays glGenVertexArrays;
static qglBindVertexArray glBindVertexArray;
static qglDeleteVertexArrays glDeleteVertexArrays;
static qglVertexAttribPointer glVertexAttribPointer;
static qglEnableVertexAttribArray glEnableVertexAttribArray;
static qglDisableVertexAttribArray glDisableVertexAttribArray;
static qglGenerateMipmap glGenerateMipmap;
static qglCreateShader glCreateShader;
static qglShaderSource glShaderSource;
static qglCompileShader glCompileShader;
static qglGetShaderiv glGetShaderiv;
static qglGetShaderInfoLog glGetShaderInfoLog;
static qglDeleteShader glDeleteShader;
static qglCreateProgram glCreateProgram;
static qglAttachShader glAttachShader;
static qglDetachShader glDetachShader;
static qglLinkProgram glLinkProgram;
static qglUseProgram glUseProgram;
static qglGetProgramiv glGetProgramiv;
static qglGetProgramInfoLog glGetProgramInfoLog;
static qglDeleteProgram glDeleteProgram;
static qglGetUniformLocation glGetUniformLocation;
static qglGetAttribLocation glGetAttribLocation;
static qglUniform1i glUniform1i;
static qglUniform1f glUniform1f;
static qglUniformMatrix3fv glUniformMatrix3fv;
static qglUniformMatrix4fv glUniformMatrix4fv;

enum graphics_card_vendors {
    VENDOR_UNKNOWN,
    VENDOR_NVIDIA,
    VENDOR_AMD,
    VENDOR_INTEL
};

struct opengl {
    /* context */
    GLXContext ctx;
    glxCreateContext create_context;
    /* info */
    const char *vendor_str;
    const char *version_str;
    const char *extensions_str;
    const char *renderer_str;
    const char *glsl_version_str;
    enum graphics_card_vendors vendor;
    /* version */
    float version;
    int major_version;
    int minor_version;
    /* extensions */
    int glsl_available;
    int vertex_buffer_obj_available;
    int vertex_array_obj_available;
    int map_buffer_range_available;
    int fragment_program_available;
    int frame_buffer_object_available;
};

struct device {
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
    struct zr_draw_null_texture null;
    struct zr_buffer cmds;
};

struct XWindow {
    Display *dpy;
    Window win;
    XVisualInfo *vis;
    Colormap cmap;
    XSetWindowAttributes swa;
    XWindowAttributes attr;
    GLXFBConfig fbc;
    int width, height;
};
static int gl_err = FALSE;

static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputs("\n", stderr);
    exit(EXIT_FAILURE);
}

static struct zr_image
icon_load(const char *filename)
{
    int x,y,n;
    GLuint tex;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    if (!data) die("[SDL]: failed to load image: %s", filename);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return zr_image_id((int)tex);
}

static int
gl_check_extension(struct opengl *GL, const char *ext)
{
    const char *start, *where, *term;
    where = strchr(ext, ' ');
    if (where || *ext == '\0')
        return FALSE;

    for (start = GL->extensions_str;;) {
        where = strstr((const char*)start, ext);
        if (!where) break;
        term = where + strlen(ext);
        if (where == start || *(where - 1) == ' ') {
            if (*term == ' ' || *term == '\0')
                return TRUE;
        }
        start = term;
    }
    return FALSE;
}

#define GL_EXT(name) (q##name)gl_ext(#name)
static __GLXextFuncPtr
gl_ext(const char *name)
{
    __GLXextFuncPtr func;
    func = glXGetProcAddress((const GLubyte*)name);
    if (!func) {
        fprintf(stdout, "[GL]: failed to load extension: %s", name);
        return NULL;
    }
    return func;
}

static int
gl_error_handler(Display *dpy, XErrorEvent *ev)
{
    UNUSED((dpy, ev));
    gl_err = TRUE;
    return 0;
}

static int
stricmpn(const char *a, const char *b, int len)
{
    int i = 0;
    for (i = 0; i < len && a[i] && b[i]; ++i)
        if (a[i] != b[i]) return 1;
    if (i != len) return 1;
    return 0;
}

static void
device_init(struct device *dev)
{
    GLint status;
    static const GLchar *vertex_shader =
        "#version 300 es\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        "#version 300 es\n"
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);
    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct zr_draw_vertex);
        size_t vp = offsetof(struct zr_draw_vertex, position);
        size_t vt = offsetof(struct zr_draw_vertex, uv);
        size_t vc = offsetof(struct zr_draw_vertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);

        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void
device_upload_atlas(struct device *dev, const void *image, int width, int height)
{
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

static void
device_shutdown(struct device *dev)
{
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    glDeleteVertexArrays(1, &dev->vao);
}

static void
device_draw(struct device *dev, struct zr_context *ctx, int width, int height,
    enum zr_anti_aliasing AA)
{
    GLint last_prog, last_tex;
    GLint last_ebo, last_vbo, last_vao;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)width;
    ortho[1][1] /= (GLfloat)height;

    /* save previous opengl state */
    glGetIntegerv(GL_CURRENT_PROGRAM, &last_prog);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_tex);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_vao);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_ebo);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vbo);

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);

    {
        /* convert from command queue into draw list and draw to screen */
        void *vertices, *elements;
        const struct zr_draw_command *cmd;
        const zr_draw_index *offset = NULL;

        /* allocate vertex and element buffer */
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_MEMORY, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_MEMORY, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            struct zr_buffer vbuf, ebuf;

            /* fill converting configuration */
            struct zr_convert_config config;
            memset(&config, 0, sizeof(config));
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;
            config.circle_segment_count = 22;
            config.arc_segment_count = 22;
            config.curve_segment_count = 22;
            config.null = dev->null;

            /* setup buffers to load vertices and elements */
            zr_buffer_init_fixed(&vbuf, vertices, MAX_VERTEX_MEMORY);
            zr_buffer_init_fixed(&ebuf, elements, MAX_ELEMENT_MEMORY);
            zr_convert(ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        zr_draw_foreach(cmd, ctx, &dev->cmds) {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor((GLint)cmd->clip_rect.x,
                height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h),
                (GLint)cmd->clip_rect.w, (GLint)cmd->clip_rect.h);
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        zr_clear(ctx);
    }

    /* restore old state */
    glUseProgram((GLuint)last_prog);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_tex);
    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)last_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (GLuint)last_ebo);
    glBindVertexArray((GLuint)last_vao);
    glDisable(GL_SCISSOR_TEST);
}


static void
input_key(struct XWindow *xw, struct zr_context *ctx, XEvent *evt, int down)
{
    int ret;
    KeySym *code = XGetKeyboardMapping(xw->dpy, (KeyCode)evt->xkey.keycode, 1, &ret);
    if (*code == XK_Shift_L || *code == XK_Shift_R)
        zr_input_key(ctx, ZR_KEY_SHIFT, down);
    else if (*code == XK_Delete)
        zr_input_key(ctx, ZR_KEY_DEL, down);
    else if (*code == XK_Return)
        zr_input_key(ctx, ZR_KEY_ENTER, down);
    else if (*code == XK_Tab)
        zr_input_key(ctx, ZR_KEY_TAB, down);
    else if (*code == XK_space && !down)
        zr_input_char(ctx, ' ');
    else if (*code == XK_Left)
        zr_input_key(ctx, ZR_KEY_LEFT, down);
    else if (*code == XK_Right)
        zr_input_key(ctx, ZR_KEY_RIGHT, down);
    else if (*code == XK_BackSpace)
        zr_input_key(ctx, ZR_KEY_BACKSPACE, down);
    else if (*code > 32 && *code < 128) {
        if (*code == 'c')
            zr_input_key(ctx, ZR_KEY_COPY, down && (evt->xkey.state & ControlMask));
        else if (*code == 'v')
            zr_input_key(ctx, ZR_KEY_PASTE, down && (evt->xkey.state & ControlMask));
        else if (*code == 'x')
            zr_input_key(ctx, ZR_KEY_CUT, down && (evt->xkey.state & ControlMask));
        if (!down)
            zr_input_unicode(ctx, (zr_rune)*code);
    }
    XFree(code);
}

static void
input_motion(struct zr_context *ctx, XEvent *evt)
{
    const int x = evt->xmotion.x;
    const int y = evt->xmotion.y;
    zr_input_motion(ctx, x, y);
}

static void
input_button(struct zr_context *ctx, XEvent *evt, int down)
{
    const int x = evt->xbutton.x;
    const int y = evt->xbutton.y;
    if (evt->xbutton.button == Button1)
        zr_input_button(ctx, ZR_BUTTON_LEFT, x, y, down);
    else if (evt->xbutton.button == Button2)
        zr_input_button(ctx, ZR_BUTTON_MIDDLE, x, y, down);
    else if (evt->xbutton.button == Button3)
        zr_input_button(ctx, ZR_BUTTON_RIGHT, x, y, down);
    else if (evt->xbutton.button == Button4)
        zr_input_scroll(ctx, 1.0f);
    else if (evt->xbutton.button == Button5)
        zr_input_scroll(ctx, -1.0f);
}

int main(int argc, char **argv)
{
    int i;
    int running = 1;
    const char *font_path;
    struct opengl gl;
    struct device device;
    struct demo gui;
    struct zr_font *font;
    struct zr_font_atlas atlas;
    struct XWindow win;

    memset(&gl, 0, sizeof(gl));
    memset(&win, 0, sizeof(win));
    memset(&gui, 0, sizeof(gui));
    font_path = (argc > 1) ? argv[1]: 0;

    win.dpy = XOpenDisplay(NULL);
    if (!win.dpy) die("Failed to open X display\n");
    {
        /* check glx version */
        int glx_major, glx_minor;
        if (!glXQueryVersion(win.dpy, &glx_major, &glx_minor))
            die("[X11]: Error: Failed to query OpenGL version\n");
        if ((glx_major == 1 && glx_minor < 3) || (glx_major < 1))
            die("[X11]: Error: Invalid GLX version!\n");
        fprintf(stdout, "[X11]: OpenGL version %d.%d\n", glx_major, glx_minor);
    }
    {
        /* find and pick matching framebuffer visual */
        int fb_count;
        static GLint attr[] = {
            GLX_X_RENDERABLE,   True,
            GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
            GLX_RENDER_TYPE,    GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
            GLX_RED_SIZE,       8,
            GLX_GREEN_SIZE,     8,
            GLX_BLUE_SIZE,      8,
            GLX_ALPHA_SIZE,     8,
            GLX_DEPTH_SIZE,     24,
            GLX_STENCIL_SIZE,   8,
            GLX_DOUBLEBUFFER,   True,
            None
        };
        GLXFBConfig *fbc;
        fprintf(stdout, "[X11]: Query matching framebuffer configurations\n");
        fbc = glXChooseFBConfig(win.dpy, DefaultScreen(win.dpy), attr, &fb_count);
        if (!fbc) die("[X11]: Error: failed to retrieve framebuffer configuration\n");
        fprintf(stdout, "[X11]: Found %d matching framebuffer configurations\n", fb_count);
        {
            /* pick framebuffer with most samples per pixel */
            int fb_best = -1, best_num_samples = -1;
            for (i = 0; i < fb_count; ++i) {
                XVisualInfo *vi = glXGetVisualFromFBConfig(win.dpy, fbc[i]);
                if (vi) {
                    int sample_buffer, samples;
                    glXGetFBConfigAttrib(win.dpy, fbc[i], GLX_SAMPLE_BUFFERS, &sample_buffer);
                    glXGetFBConfigAttrib(win.dpy, fbc[i], GLX_SAMPLES, &samples);
                    fprintf(stdout, "\tFramebuffer Config %d: Visual ID 0x%2x: "
                        "(SAMPLE_BUFFER: %d, SAMPLES: %d)\n", i, (unsigned int)vi->visualid,
                        sample_buffer, samples);
                    if ((fb_best < 0) || (sample_buffer && samples > best_num_samples))
                        fb_best = i; best_num_samples = samples;
                }
            }
            win.fbc = fbc[fb_best];
            XFree(fbc);
            win.vis = glXGetVisualFromFBConfig(win.dpy, win.fbc);
            fprintf(stdout, "[X11]: Chosen visual id: 0x%x\n", (unsigned)win.vis->visualid);
        }
    }
    {
        /* create window */
        fprintf(stdout, "[X11]: Creating colormap\n");
        win.cmap = XCreateColormap(win.dpy, RootWindow(win.dpy, win.vis->screen), win.vis->visual, AllocNone);
        win.swa.colormap =  win.cmap;
        win.swa.background_pixmap = None;
        win.swa.border_pixel = 0;
        win.swa.event_mask =
            ExposureMask | KeyPressMask | KeyReleaseMask |
            ButtonPress | ButtonReleaseMask| ButtonMotionMask |
            Button1MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask|
            PointerMotionMask| StructureNotifyMask;
        fprintf(stdout, "[X11]: Creating window\n");
        win.win = XCreateWindow(win.dpy, RootWindow(win.dpy, win.vis->screen), 0, 0,
            WINDOW_WIDTH, WINDOW_HEIGHT, 0, win.vis->depth, InputOutput,
            win.vis->visual, CWBorderPixel|CWColormap|CWEventMask, &win.swa);
        if (!win.win) die("[X11]: Failed to create window\n");
        XFree(win.vis);
        XStoreName(win.dpy, win.win, "Zahnrad");
        fprintf(stdout, "[X11]: Mapping window\n");
        XMapWindow(win.dpy, win.win);
    }
    {
        /* create opengl context */
        int(*old_handler)(Display*, XErrorEvent*) = XSetErrorHandler(gl_error_handler);
        gl.extensions_str = glXQueryExtensionsString(win.dpy, DefaultScreen(win.dpy));
        gl.create_context = (glxCreateContext)
            glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

        gl_err = FALSE;
        if (!gl_check_extension(&gl, "GLX_ARB_create_context") || !gl.create_context) {
            fprintf(stdout, "[X11]: glXCreateContextAttribARB() not found...\n");
            fprintf(stdout, "[X11]: ... using old-style GLX context\n");
            gl.ctx = glXCreateNewContext(win.dpy, win.fbc, GLX_RGBA_TYPE, 0, True);
        } else {
            GLint attr[] = {
                GLX_CONTEXT_MAJOR_VERSION_ARB, OGL_MAJOR_VERSION,
                GLX_CONTEXT_MINOR_VERSION_ARB, OGL_MINOR_VERSION,
                None
            };
            fprintf(stdout, "[X11]: Creating Context...\n");
            gl.ctx = gl.create_context(win.dpy, win.fbc, 0, True, attr);
            XSync(win.dpy, False);
            if (gl_err || !gl.ctx) {
                /* Could not create GL 3.0 context. Fallback to old 2.x context.
                 * If a version below 3.0 is requested, implementations will
                 * return the newest context version compatible with OpenGL
                 * version less than version 3.0.*/
                attr[1] = 1; attr[3] = 0;
                gl_err = FALSE;
                fprintf(stdout, "[X11] Failed to create OpenGL 3.0 context\n");
                fprintf(stdout, "[X11] ... using old-style GLX context!\n");
                gl.ctx = gl.create_context(win.dpy, win.fbc, 0, True, attr);
            } else fprintf(stdout, "[X11] OpenGL 3.0 Context created\n");
        }
        XSync(win.dpy, False);
        XSetErrorHandler(old_handler);
        if (gl_err || !gl.ctx)
            die("[X11]: Failed to create an OpenGL context\n");

        if (!glXIsDirect(win.dpy, gl.ctx))
            fprintf(stdout, "[X11] Optained indirect GLX rendering context\n");
        else fprintf(stdout, "[X11] Optained direct GLX rendering context\n");
        glXMakeCurrent(win.dpy, win.win, gl.ctx);
    }
    {
        int failed = FALSE;
        gl.version_str = (const char*)glGetString(GL_VERSION);
        glGetIntegerv(GL_MAJOR_VERSION, &gl.major_version);
        glGetIntegerv(GL_MINOR_VERSION, &gl.minor_version);
        if (gl.major_version < 2)
            die("[GL]: Graphics card does not fullfill minimum OpenGL 2.0 support\n");
        gl.version = (float)gl.major_version + (float)gl.minor_version * 0.1f;

        gl.renderer_str = (const char*)glGetString(GL_RENDERER);
        gl.extensions_str = (const char*)glGetString(GL_EXTENSIONS);
        gl.glsl_version_str = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

        gl.vendor_str = (const char*)glGetString(GL_VENDOR);
        if (!stricmpn(gl.vendor_str, "ATI", 4) ||
            !stricmpn(gl.vendor_str, "AMD", 4))
            gl.vendor = VENDOR_AMD;
        else if (!stricmpn(gl.vendor_str, "NVIDIA", 6))
            gl.vendor = VENDOR_NVIDIA;
        else if (!stricmpn(gl.vendor_str, "Intel", 5))
            gl.vendor = VENDOR_INTEL;
        else gl.vendor = VENDOR_UNKNOWN;

        fprintf(stdout, "[GL] OpenGL\n");
        fprintf(stdout, "\tVersion: %d.%d\n", gl.major_version, gl.minor_version);
        fprintf(stdout, "\tVendor: %s\n", gl.vendor_str);
        fprintf(stdout, "\tRenderer: %s\n", gl.renderer_str);
        fprintf(stdout, "\tGLSL: %s\n\n", gl.glsl_version_str);

        /* Extensions */
        fprintf(stdout, "[GL] Loading extensions...\n");
        gl.glsl_available = (gl.version >= 2.0f);
        if (gl.glsl_available) {
            /* GLSL core in OpenGL > 2 */
            glCreateShader = GL_EXT(glCreateShader);
            glShaderSource = GL_EXT(glShaderSource);
            glCompileShader = GL_EXT(glCompileShader);
            glGetShaderiv = GL_EXT(glGetShaderiv);
            glGetShaderInfoLog = GL_EXT(glGetShaderInfoLog);
            glDeleteShader = GL_EXT(glDeleteShader);
            glCreateProgram = GL_EXT(glCreateProgram);
            glAttachShader = GL_EXT(glAttachShader);
            glDetachShader = GL_EXT(glDetachShader);
            glLinkProgram = GL_EXT(glLinkProgram);
            glUseProgram = GL_EXT(glUseProgram);
            glGetProgramiv = GL_EXT(glGetProgramiv);
            glGetProgramInfoLog = GL_EXT(glGetProgramInfoLog);
            glDeleteProgram = GL_EXT(glDeleteProgram);
            glGetUniformLocation = GL_EXT(glGetUniformLocation);
            glGetAttribLocation = GL_EXT(glGetAttribLocation);
            glUniform1i = GL_EXT(glUniform1i);
            glUniform1f = GL_EXT(glUniform1f);
            glUniformMatrix3fv = GL_EXT(glUniformMatrix3fv);
            glUniformMatrix4fv = GL_EXT(glUniformMatrix4fv);
        }
        gl.vertex_buffer_obj_available = gl_check_extension(&gl, "GL_ARB_vertex_buffer_object");
        if (gl.vertex_buffer_obj_available) {
            /* GL_ARB_vertex_buffer_object */
            glGenBuffers = GL_EXT(glGenBuffers);
            glBindBuffer = GL_EXT(glBindBuffer);
            glBufferData = GL_EXT(glBufferData);
            glBufferSubData = GL_EXT(glBufferSubData);
            glMapBuffer = GL_EXT(glMapBuffer);
            glUnmapBuffer = GL_EXT(glUnmapBuffer);
            glDeleteBuffers = GL_EXT(glDeleteBuffers);
        }
        gl.fragment_program_available = gl_check_extension(&gl, "GL_ARB_fragment_program");
        if (gl.fragment_program_available) {
            /* GL_ARB_vertex_program / GL_ARB_fragment_program  */
            glVertexAttribPointer = GL_EXT(glVertexAttribPointer);
            glEnableVertexAttribArray = GL_EXT(glEnableVertexAttribArray);
            glDisableVertexAttribArray = GL_EXT(glDisableVertexAttribArray);
        }
        gl.vertex_array_obj_available = gl_check_extension(&gl, "GL_ARB_vertex_array_object");
        if (gl.vertex_array_obj_available) {
            /* GL_ARB_vertex_array_object */
            glGenVertexArrays = GL_EXT(glGenVertexArrays);
            glBindVertexArray = GL_EXT(glBindVertexArray);
            glDeleteVertexArrays = GL_EXT(glDeleteVertexArrays);
        }
        gl.frame_buffer_object_available = gl_check_extension(&gl, "GL_ARB_framebuffer_object");
        if (gl.frame_buffer_object_available) {
            /* GL_ARB_framebuffer_object */
            glGenerateMipmap = GL_EXT(glGenerateMipmap);
        }
        if (!gl.vertex_buffer_obj_available) {
            fprintf(stdout, "[GL] Error: GL_ARB_vertex_buffer_object is not available!\n");
            failed = TRUE;
        }
        if (!gl.fragment_program_available) {
            fprintf(stdout, "[GL] Error: GL_ARB_fragment_program is not available!\n");
            failed = TRUE;
        }
        if (!gl.vertex_array_obj_available) {
            fprintf(stdout, "[GL] Error: GL_ARB_vertex_array_object is not available!\n");
            failed = TRUE;
        }
        if (!gl.frame_buffer_object_available) {
            fprintf(stdout, "[GL] Error: GL_ARB_framebuffer_object is not available!\n");
            failed = TRUE;
        }
        if (failed) goto cleanup;
        fprintf(stdout, "[GL] Extensions successfully loaded\n");
    }

    /* screen */
    XGetWindowAttributes(win.dpy, win.win, &win.attr);
    win.width = win.attr.width;
    win.height = win.attr.height;

    device_init(&device);
    {
        /* Font */
        const void *image;
        int width, height;

        zr_font_atlas_init_default(&atlas);
        zr_font_atlas_begin(&atlas);
        if (font_path) font = zr_font_atlas_add_from_file(&atlas, font_path, 14.0f, NULL);
        else font = zr_font_atlas_add_default(&atlas, 14.0f, NULL);
        image = zr_font_atlas_bake(&atlas, &width, &height, ZR_FONT_ATLAS_RGBA32);
        device_upload_atlas(&device, image, width, height);
        zr_font_atlas_end(&atlas, zr_handle_id((int)device.font_tex), &device.null);

        /* GUI */
        memset(&gui, 0, sizeof(gui));
        zr_buffer_init_default(&device.cmds);
        zr_init_default(&gui.ctx, &font->handle);
    }


    device_init(&device);
    glEnable(GL_TEXTURE_2D);
    gui.icons.unchecked = icon_load("../../icon/unchecked.png");
    gui.icons.checked = icon_load("../../icon/checked.png");
    gui.icons.rocket = icon_load("../../icon/rocket.png");
    gui.icons.cloud = icon_load("../../icon/cloud.png");
    gui.icons.pen = icon_load("../../icon/pen.png");
    gui.icons.play = icon_load("../../icon/play.png");
    gui.icons.pause = icon_load("../../icon/pause.png");
    gui.icons.stop = icon_load("../../icon/stop.png");
    gui.icons.next =  icon_load("../../icon/next.png");
    gui.icons.prev =  icon_load("../../icon/prev.png");
    gui.icons.tools = icon_load("../../icon/tools.png");
    gui.icons.dir = icon_load("../../icon/directory.png");
    gui.icons.copy = icon_load("../../icon/copy.png");
    gui.icons.convert = icon_load("../../icon/export.png");
    gui.icons.del = icon_load("../../icon/delete.png");
    gui.icons.edit = icon_load("../../icon/edit.png");
    gui.icons.menu[0] = icon_load("../../icon/home.png");
    gui.icons.menu[1] = icon_load("../../icon/phone.png");
    gui.icons.menu[2] = icon_load("../../icon/plane.png");
    gui.icons.menu[3] = icon_load("../../icon/wifi.png");
    gui.icons.menu[4] = icon_load("../../icon/settings.png");
    gui.icons.menu[5] = icon_load("../../icon/volume.png");

    gui.icons.home = icon_load("../../icon/home.png");
    gui.icons.directory = icon_load("../../icon/directory.png");
    gui.icons.computer = icon_load("../../icon/computer.png");
    gui.icons.desktop = icon_load("../../icon/desktop.png");
    gui.icons.default_file = icon_load("../../icon/default.png");
    gui.icons.text_file = icon_load("../../icon/text.png");
    gui.icons.music_file = icon_load("../../icon/music.png");
    gui.icons.font_file =  icon_load("../../icon/font.png");
    gui.icons.img_file = icon_load("../../icon/img.png");
    gui.icons.movie_file = icon_load("../../icon/movie.png");

    for (i = 0; i < 9; ++i) {
        char buffer[256];
        sprintf(buffer, "../../images/image%d.png", (i+1));
        gui.icons.images[i] = icon_load(buffer);
    }

    while (running) {
        /* input */
        XEvent evt;
        zr_input_begin(&gui.ctx);
        while (XCheckWindowEvent(win.dpy, win.win, win.swa.event_mask, &evt)) {
            if (evt.type == KeyPress)
                input_key(&win, &gui.ctx, &evt, zr_true);
            else if (evt.type == KeyRelease)
                input_key(&win, &gui.ctx, &evt, zr_false);
            else if (evt.type == ButtonPress)
                input_button(&gui.ctx, &evt, zr_true);
            else if (evt.type == ButtonRelease)
                input_button(&gui.ctx, &evt, zr_false);
            else if (evt.type == MotionNotify)
                input_motion(&gui.ctx, &evt);
            else if (evt.type == Expose || evt.type == ConfigureNotify) {
                XGetWindowAttributes(win.dpy, win.win, &win.attr);
                win.width = win.attr.width;
                win.height = win.attr.height;
            }
        }
        zr_input_end(&gui.ctx);

        /* GUI */
        XGetWindowAttributes(win.dpy, win.win, &win.attr);
        running = run_demo(&gui);

        /* Draw */
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glViewport(0, 0, win.width, win.height);
        device_draw(&device, &gui.ctx, win.width, win.height, ZR_ANTI_ALIASING_ON);
        glXSwapBuffers(win.dpy, win.win);
    }

cleanup:
    zr_font_atlas_clear(&atlas);
    zr_free(&gui.ctx);
    zr_buffer_free(&device.cmds);
    device_shutdown(&device);

    glDeleteTextures(1,(const GLuint*)&gui.icons.unchecked.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.checked.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.rocket.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.cloud.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.pen.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.play.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.pause.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.stop.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.next.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.prev.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.tools.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.dir.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.del.handle.id);

    glDeleteTextures(1,(const GLuint*)&gui.icons.home.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.directory.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.computer.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.desktop.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.default_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.text_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.music_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.font_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.img_file.handle.id);
    glDeleteTextures(1,(const GLuint*)&gui.icons.movie_file.handle.id);

    for (i = 0; i < 9; ++i)
        glDeleteTextures(1, (const GLuint*)&gui.icons.images[i].handle.id);
    for (i = 0; i < 6; ++i)
        glDeleteTextures(1, (const GLuint*)&gui.icons.menu[i].handle.id);

    glXMakeCurrent(win.dpy, 0, 0);
    glXDestroyContext(win.dpy, gl.ctx);
    XUnmapWindow(win.dpy, win.win);
    XFreeColormap(win.dpy, win.cmap);
    XDestroyWindow(win.dpy, win.win);
    XCloseDisplay(win.dpy);
    return 0;
}

