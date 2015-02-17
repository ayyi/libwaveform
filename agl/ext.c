#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "agl/ext.h"

#define getProcAddress(x) (*glXGetProcAddressARB)((const GLubyte*)x) //TODO check glXGetProcAddress is exported by nvidia
void
get_gl_extensions()
{
   /* OpenGL 2.0 */
   glAttachShader = (PFNGLATTACHSHADERPROC) getProcAddress("glAttachShader");
#if 0
   glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) getProcAddress("glBindAttribLocation");
#endif
   glCompileShader = (PFNGLCOMPILESHADERPROC) getProcAddress("glCompileShader");
   glCreateProgram = (PFNGLCREATEPROGRAMPROC) getProcAddress("glCreateProgram");
   glCreateShader = (PFNGLCREATESHADERPROC) getProcAddress("glCreateShader");
#if 0
   glDeleteProgram = (PFNGLDELETEPROGRAMPROC) getProcAddress("glDeleteProgram");
   glDeleteShader = (PFNGLDELETESHADERPROC) getProcAddress("glDeleteShader");
   glGetActiveAttrib_func = (PFNGLGETACTIVEATTRIBPROC) getProcAddress("glGetActiveAttrib");
   glGetActiveUniform_func = (PFNGLGETACTIVEUNIFORMPROC) getProcAddress("glGetActiveUniform");
   glGetAttachedShaders_func = (PFNGLGETATTACHEDSHADERSPROC) getProcAddress("glGetAttachedShaders");
   glGetAttribLocation_func = (PFNGLGETATTRIBLOCATIONPROC) getProcAddress("glGetAttribLocation");
#endif
   glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) getProcAddress("glGetProgramInfoLog");
   glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) getProcAddress("glGetShaderInfoLog");
   glGetProgramiv = (PFNGLGETPROGRAMIVPROC) getProcAddress("glGetProgramiv");
   glGetShaderiv = (PFNGLGETSHADERIVPROC) getProcAddress("glGetShaderiv");
#if 0
   glGetShaderSource_func = (PFNGLGETSHADERSOURCEPROC) getProcAddress("glGetShaderSource");
#endif
   glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) getProcAddress("glGetUniformLocation");
#if 0
   glGetUniformfv_func = (PFNGLGETUNIFORMFVPROC) getProcAddress("glGetUniformfv");
   glIsProgram_func = (PFNGLISPROGRAMPROC) getProcAddress("glIsProgram");
   glIsShader_func = (PFNGLISSHADERPROC) getProcAddress("glIsShader");
#endif
   glLinkProgram = (PFNGLLINKPROGRAMPROC) getProcAddress("glLinkProgram");
   glShaderSource = (PFNGLSHADERSOURCEPROC) getProcAddress("glShaderSource");
   glUniform1i = (PFNGLUNIFORM1IPROC) getProcAddress("glUniform1i");
   glUniform2i_func = (PFNGLUNIFORM2IPROC) getProcAddress("glUniform2i");
   glUniform3i_func = (PFNGLUNIFORM3IPROC) getProcAddress("glUniform3i");
   glUniform4i_func = (PFNGLUNIFORM4IPROC) getProcAddress("glUniform3i");
   glUniform1f = (PFNGLUNIFORM1FPROC) getProcAddress("glUniform1f");
   glUniform2f_func = (PFNGLUNIFORM2FPROC) getProcAddress("glUniform2f");
   glUniform3f_func = (PFNGLUNIFORM3FPROC) getProcAddress("glUniform3f");
   glUniform4f = (PFNGLUNIFORM4FPROC) getProcAddress("glUniform4f");
   glUniform1fv = (PFNGLUNIFORM1FVPROC) getProcAddress("glUniform1fv");
   glUniform2fv = (PFNGLUNIFORM2FVPROC) getProcAddress("glUniform2fv");
   glUniform3fv = (PFNGLUNIFORM3FVPROC) getProcAddress("glUniform3fv");
   glUniform4fv = (PFNGLUNIFORM3FVPROC) getProcAddress("glUniform4fv");
   glUniform1iv = (PFNGLUNIFORM1IVPROC) getProcAddress("glUniform1iv");
#if 0
   glUniformMatrix2fv_func = (PFNGLUNIFORMMATRIX2FVPROC) getProcAddress("glUniformMatrix2fv");
   glUniformMatrix3fv_func = (PFNGLUNIFORMMATRIX3FVPROC) getProcAddress("glUniformMatrix3fv");
#endif
   glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) getProcAddress("glUniformMatrix4fv");
   glUseProgram = (PFNGLUSEPROGRAMPROC) glXGetProcAddress((GLubyte*)"glUseProgram");
#if 0
   glVertexAttrib1f_func = (PFNGLVERTEXATTRIB1FPROC) getProcAddress("glVertexAttrib1f");
   glVertexAttrib2f_func = (PFNGLVERTEXATTRIB2FPROC) getProcAddress("glVertexAttrib2f");
   glVertexAttrib3f_func = (PFNGLVERTEXATTRIB3FPROC) getProcAddress("glVertexAttrib3f");
   glVertexAttrib4f_func = (PFNGLVERTEXATTRIB4FPROC) getProcAddress("glVertexAttrib4f");
   glVertexAttrib1fv_func = (PFNGLVERTEXATTRIB1FVPROC) getProcAddress("glVertexAttrib1fv");
   glVertexAttrib2fv_func = (PFNGLVERTEXATTRIB2FVPROC) getProcAddress("glVertexAttrib2fv");
   glVertexAttrib3fv_func = (PFNGLVERTEXATTRIB3FVPROC) getProcAddress("glVertexAttrib3fv");
   glVertexAttrib4fv_func = (PFNGLVERTEXATTRIB4FVPROC) getProcAddress("glVertexAttrib4fv");

   glVertexAttribPointer_func = (PFNGLVERTEXATTRIBPOINTERPROC) getProcAddress("glVertexAttribPointer");
#endif
   glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) getProcAddress("glEnableVertexAttribArray");
#if 0
   glDisableVertexAttribArray_func = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) getProcAddress("glDisableVertexAttribArray");

   /* OpenGL 2.1 */
   glUniformMatrix2x3fv_func = (PFNGLUNIFORMMATRIX2X3FVPROC) getProcAddress("glUniformMatrix2x3fv");
   glUniformMatrix3x2fv_func = (PFNGLUNIFORMMATRIX3X2FVPROC) getProcAddress("glUniformMatrix3x2fv");
   glUniformMatrix2x4fv_func = (PFNGLUNIFORMMATRIX2X4FVPROC) getProcAddress("glUniformMatrix2x4fv");
   glUniformMatrix4x2fv_func = (PFNGLUNIFORMMATRIX4X2FVPROC) getProcAddress("glUniformMatrix4x2fv");
   glUniformMatrix3x4fv_func = (PFNGLUNIFORMMATRIX3X4FVPROC) getProcAddress("glUniformMatrix3x4fv");
   glUniformMatrix4x3fv_func = (PFNGLUNIFORMMATRIX4X3FVPROC) getProcAddress("glUniformMatrix4x3fv");

   /* OpenGL 1.4 */
   glPointParameterfv_func = (PFNGLPOINTPARAMETERFVPROC) getProcAddress("glPointParameterfv");
   glSecondaryColor3fv_func = (PFNGLSECONDARYCOLOR3FVPROC) getProcAddress("glSecondaryColor3fv");

   /* GL_ARB_vertex/fragment_program */
   glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC) getProcAddress("glBindProgramARB");
   glDeleteProgramsARB_func = (PFNGLDELETEPROGRAMSARBPROC) getProcAddress("glDeleteProgramsARB");
   glGenProgramsARB_func = (PFNGLGENPROGRAMSARBPROC) getProcAddress("glGenProgramsARB");
   glGetProgramLocalParameterdvARB_func = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) getProcAddress("glGetProgramLocalParameterdvARB");
   glIsProgramARB_func = (PFNGLISPROGRAMARBPROC) getProcAddress("glIsProgramARB");
   glProgramLocalParameter4dARB_func = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC) getProcAddress("glProgramLocalParameter4dARB");
   glProgramLocalParameter4fvARB_func = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) getProcAddress("glProgramLocalParameter4fvARB");
   glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC) getProcAddress("glProgramStringARB");
   glVertexAttrib1fARB_func = (PFNGLVERTEXATTRIB1FARBPROC) getProcAddress("glVertexAttrib1fARB");

   /* GL_APPLE_vertex_array_object */
#endif
   glBindVertexArrayAPPLE = (PFNGLBINDVERTEXARRAYAPPLEPROC) getProcAddress("glBindVertexArrayAPPLE");
#if 0
   glDeleteVertexArraysAPPLE_func = (PFNGLDELETEVERTEXARRAYSAPPLEPROC) getProcAddress("glDeleteVertexArraysAPPLE");
   glGenVertexArraysAPPLE_func = (PFNGLGENVERTEXARRAYSAPPLEPROC) getProcAddress("glGenVertexArraysAPPLE");
   glIsVertexArrayAPPLE_func = (PFNGLISVERTEXARRAYAPPLEPROC) getProcAddress("glIsVertexArrayAPPLE");

   /* GL_EXT_stencil_two_side */
   glActiveStencilFaceEXT_func = (PFNGLACTIVESTENCILFACEEXTPROC) getProcAddress("glActiveStencilFaceEXT");

   /* GL_ARB_vertex_buffer_object */
   glGenBuffersARB_func = (PFNGLGENBUFFERSARBPROC) getProcAddress("glGenBuffersARB");
   glDeleteBuffersARB_func = (PFNGLDELETEBUFFERSARBPROC) getProcAddress("glDeleteBuffersARB");
   glBindBufferARB_func = (PFNGLBINDBUFFERARBPROC) getProcAddress("glBindBufferARB");
   glBufferDataARB_func = (PFNGLBUFFERDATAARBPROC) getProcAddress("glBufferDataARB");
   glBufferSubDataARB_func = (PFNGLBUFFERSUBDATAARBPROC) getProcAddress("glBufferSubDataARB");
   glMapBufferARB_func = (PFNGLMAPBUFFERARBPROC) getProcAddress("glMapBufferARB");
   glUnmapBufferARB_func = (PFNGLUNMAPBUFFERARBPROC) getProcAddress("glUnmapBufferARB");

   /* GL_EXT_framebuffer_object */
   glIsRenderbufferEXT_func = (PFNGLISRENDERBUFFEREXTPROC) getProcAddress("glIsRenderbufferEXT");
   glBindRenderbufferEXT_func = (PFNGLBINDRENDERBUFFEREXTPROC) getProcAddress("glBindRenderbufferEXT");
   glDeleteRenderbuffersEXT_func = (PFNGLDELETERENDERBUFFERSEXTPROC) getProcAddress("glDeleteRenderbuffersEXT");
   glGenRenderbuffersEXT_func = (PFNGLGENRENDERBUFFERSEXTPROC) getProcAddress("glGenRenderbuffersEXT");
   glRenderbufferStorageEXT_func = (PFNGLRENDERBUFFERSTORAGEEXTPROC) getProcAddress("glRenderbufferStorageEXT");
   glGetRenderbufferParameterivEXT_func = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) getProcAddress("glGetRenderbufferParameterivEXT");
   glIsFramebufferEXT_func = (PFNGLISFRAMEBUFFEREXTPROC) getProcAddress("glIsFramebufferEXT");
   glBindFramebufferEXT_func = (PFNGLBINDFRAMEBUFFEREXTPROC) getProcAddress("glBindFramebufferEXT");
   glDeleteFramebuffersEXT_func = (PFNGLDELETEFRAMEBUFFERSEXTPROC) getProcAddress("glDeleteFramebuffersEXT");
   glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) getProcAddress("glGenFramebuffersEXT");
   glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) getProcAddress("glCheckFramebufferStatusEXT");
   glFramebufferTexture1DEXT_func = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) getProcAddress("glFramebufferTexture1DEXT");
   glFramebufferTexture2DEXT_func = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) getProcAddress("glFramebufferTexture2DEXT");
   glFramebufferTexture3DEXT_func = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) getProcAddress("glFramebufferTexture3DEXT");
   glFramebufferRenderbufferEXT_func = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) getProcAddress("glFramebufferRenderbufferEXT");
   glGetFramebufferAttachmentParameterivEXT_func = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) getProcAddress("glGetFramebufferAttachmentParameterivEXT");
   glGenerateMipmapEXT_func = (PFNGLGENERATEMIPMAPEXTPROC) getProcAddress("glGenerateMipmapEXT");

   /* GL_ARB_framebuffer_object */
   glIsRenderbuffer_func = (PFNGLISRENDERBUFFERPROC) getProcAddress("glIsRenderbuffer");
#endif
   glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) getProcAddress("glBindRenderbuffer");
   glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) getProcAddress("glDeleteRenderbuffers");
   glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) getProcAddress("glGenRenderbuffers");
   glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) getProcAddress("glRenderbufferStorage");
   glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) getProcAddress("glGetRenderbufferParameteriv");
#if 0
   glIsFramebuffer_func = (PFNGLISFRAMEBUFFERPROC) getProcAddress("glIsFramebuffer");
#endif
   glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) getProcAddress("glBindFramebuffer");
   glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) getProcAddress("glDeleteFramebuffers");
   glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) getProcAddress("glGenFramebuffers");
   glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) getProcAddress("glCheckFramebufferStatus");
#if 0
   glFramebufferTexture1D_func = (PFNGLFRAMEBUFFERTEXTURE1DPROC) getProcAddress("glFramebufferTexture1D");
#endif
   glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) getProcAddress("glFramebufferTexture2D");
#if 0
   glFramebufferTexture3D_func = (PFNGLFRAMEBUFFERTEXTURE3DPROC) getProcAddress("glFramebufferTexture3D");
#endif
   glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) getProcAddress("glFramebufferRenderbuffer");
   glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) getProcAddress("glGetFramebufferAttachmentParameteriv");
#if 0
   glGenerateMipmap_func = (PFNGLGENERATEMIPMAPPROC) getProcAddress("glGenerateMipmap");
   glBlitFramebuffer_func = (PFNGLBLITFRAMEBUFFERPROC) getProcAddress("glBlitFramebuffer");
   glRenderbufferStorageMultisample_func = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) getProcAddress("glRenderbufferStorageMultisample");
   glFramebufferTextureLayer_func = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) getProcAddress("glFramebufferTextureLayer");
#endif
   glStringMarkerGREMEDY = (PFNGLSTRINGMARKERGREMEDYPROC) getProcAddress("glStringMarkerGREMEDY");
}

