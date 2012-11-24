#ifndef __gl_utils_h__
#define __gl_utils_h__

typedef struct _agl               AGl;
typedef struct _uniform_info      AGlUniformInfo;
typedef struct _agl_shader        AGlShader;

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
GLboolean agl_shaders_supported   ();
GLuint    agl_create_program      (AGlShader*, AGlUniformInfo*);
GLuint    agl_compile_shader_text (GLenum shaderType, const char* text);
GLuint    agl_compile_shader_file (GLenum shaderType, const char* filename);
void      agl_uniforms_init       (GLuint program, AGlUniformInfo uniforms[]);
GLuint    agl_link_shaders        (GLuint vertShader, GLuint fragShader);
void      agl_rect                (float x, float y, float w, float h);
void      agl_set_font            (char* font_string);
void      agl_print               (int x, int y, double z, uint32_t colour, const char* fmt, ...);


struct _agl
{
	gboolean        pref_use_shaders;
	gboolean        use_shaders;
};

#define END_OF_UNIFORMS   { NULL, 0, GL_NONE, { 0, 0, 0, 0 }, -1 }

#endif //__gl_utils_h__
