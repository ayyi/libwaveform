#ifndef __agl_utils_h__
#define __agl_utils_h__
#include "agl/typedefs.h"
//#include "agl/shader.h"

typedef struct _agl               AGl;
typedef struct _uniform_info      AGlUniformInfo;

typedef struct _agl_shader_text
{
	char*        vert;
	char*        frag;
} AGlShaderText;

struct _agl_shader
{
	char*           vertex_file;
	char*           fragment_file;
	uint32_t        program;       // compiled program
	AGlUniformInfo* uniforms;
	void            (*set_uniforms_)();
	AGlShaderText*  text;
};

struct _uniform_info
{
   const char* name;
   GLuint      size;
   GLenum      type;      // GL_FLOAT or GL_INT
   GLfloat     value[4];
   GLint       location;  // filled in by agl_uniforms_init()
};

AGl*      agl_get_instance        ();
void      agl_enable              (gulong flags);
GLboolean agl_shaders_supported   ();
void      agl_shaders_init        ();
GLuint    agl_create_program      (AGlShader*);
GLuint    agl_compile_shader_text (GLenum shaderType, const char* text);
GLuint    agl_compile_shader_file (GLenum shaderType, const char* filename);
void      agl_uniforms_init       (GLuint program, AGlUniformInfo uniforms[]);
GLuint    agl_link_shaders        (GLuint vertShader, GLuint fragShader);
void      wf_canvas_use_program   (int);
void      agl_use_program         (AGlShader*);
void      agl_rect                (float x, float y, float w, float h);
void      agl_enable_stencil      (float x, float y, float w, float h);
void      agl_disable_stencil     ();
void      agl_print_error         (const char* func, int err, const char* format, ...);

void      agl_set_font            (char* font_string);
void      agl_print               (int x, int y, double z, uint32_t colour, const char* fmt, ...);

int       agl_power_of_two        (guint);


struct _agl
{
	gboolean        pref_use_shaders;
	gboolean        use_shaders;
	AlphaMapShader* text_shader;
};

#define END_OF_UNIFORMS   { NULL, 0, GL_NONE, { 0, 0, 0, 0 }, -1 }

extern GLenum _wf_ge;
#define gl_error ((_wf_ge = glGetError()) != GL_NO_ERROR)
#define gl_warn(A, ...) { \
		if(gl_error){ \
		agl_print_error(__func__, _wf_ge, A, ##__VA_ARGS__); \
	}}

#endif //__agl_utils_h__
