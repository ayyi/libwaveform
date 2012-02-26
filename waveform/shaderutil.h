#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

typedef struct uniform_info
{
   const char* name;
   GLuint      size;
   GLenum      type;      // GL_FLOAT or GL_INT
   GLfloat     value[4];
   GLint       location;  // filled in by InitUniforms()
} UniformInfo;

#define END_OF_UNIFORMS   { NULL, 0, GL_NONE, { 0, 0, 0, 0 }, -1 }

typedef struct {
	char*  vertex_file;
	char*  fragment_file;
	GLuint program;       // compiled program
	UniformInfo* uniforms;
} Shader;


extern GLboolean shaders_supported   ();
extern GLuint    compile_shader_text (GLenum shaderType, const char* text);
extern GLuint    compile_shader_file (GLenum shaderType, const char* filename);
extern GLuint    link_shaders        (GLuint vertShader, GLuint fragShader);
extern GLuint    link_shaders2       (GLuint vert_shader_1, GLuint frag_shader_1, GLuint vert_shader_2, GLuint frag_shader_2);

extern void      uniforms_init       (GLuint program, struct uniform_info uniforms[]);


#endif /* SHADER_UTIL_H */
