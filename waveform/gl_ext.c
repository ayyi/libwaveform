#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "waveform/gl_ext.h"
#include "waveform/utils.h"

#define glutGetProcAddress(x) (*glXGetProcAddressARB)((const GLubyte*)x) //TODO check glXGetProcAddress is exported by nvidia
void
get_gl_extensions()
{
   /* OpenGL 2.0 */
   glAttachShader = (PFNGLATTACHSHADERPROC) glutGetProcAddress("glAttachShader");
#if 0
   glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) glutGetProcAddress("glBindAttribLocation");
#endif
   glCompileShader = (PFNGLCOMPILESHADERPROC) glutGetProcAddress("glCompileShader");
   glCreateProgram = (PFNGLCREATEPROGRAMPROC) glutGetProcAddress("glCreateProgram");
   glCreateShader = (PFNGLCREATESHADERPROC) glutGetProcAddress("glCreateShader");
#if 0
   glDeleteProgram = (PFNGLDELETEPROGRAMPROC) glutGetProcAddress("glDeleteProgram");
   glDeleteShader = (PFNGLDELETESHADERPROC) glutGetProcAddress("glDeleteShader");
   glGetActiveAttrib_func = (PFNGLGETACTIVEATTRIBPROC) glutGetProcAddress("glGetActiveAttrib");
   glGetActiveUniform_func = (PFNGLGETACTIVEUNIFORMPROC) glutGetProcAddress("glGetActiveUniform");
   glGetAttachedShaders_func = (PFNGLGETATTACHEDSHADERSPROC) glutGetProcAddress("glGetAttachedShaders");
   glGetAttribLocation_func = (PFNGLGETATTRIBLOCATIONPROC) glutGetProcAddress("glGetAttribLocation");
#endif
   glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) glutGetProcAddress("glGetProgramInfoLog");
   glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) glutGetProcAddress("glGetShaderInfoLog");
   glGetProgramiv = (PFNGLGETPROGRAMIVPROC) glutGetProcAddress("glGetProgramiv");
   glGetShaderiv = (PFNGLGETSHADERIVPROC) glutGetProcAddress("glGetShaderiv");
#if 0
   glGetShaderSource_func = (PFNGLGETSHADERSOURCEPROC) glutGetProcAddress("glGetShaderSource");
#endif
   glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) glutGetProcAddress("glGetUniformLocation");
#if 0
   glGetUniformfv_func = (PFNGLGETUNIFORMFVPROC) glutGetProcAddress("glGetUniformfv");
   glIsProgram_func = (PFNGLISPROGRAMPROC) glutGetProcAddress("glIsProgram");
   glIsShader_func = (PFNGLISSHADERPROC) glutGetProcAddress("glIsShader");
#endif
   glLinkProgram = (PFNGLLINKPROGRAMPROC) glutGetProcAddress("glLinkProgram");
   glShaderSource = (PFNGLSHADERSOURCEPROC) glutGetProcAddress("glShaderSource");
   glUniform1i = (PFNGLUNIFORM1IPROC) glutGetProcAddress("glUniform1i");
   glUniform2i_func = (PFNGLUNIFORM2IPROC) glutGetProcAddress("glUniform2i");
   glUniform3i_func = (PFNGLUNIFORM3IPROC) glutGetProcAddress("glUniform3i");
   glUniform4i_func = (PFNGLUNIFORM4IPROC) glutGetProcAddress("glUniform3i");
   glUniform1f = (PFNGLUNIFORM1FPROC) glutGetProcAddress("glUniform1f");
   glUniform2f_func = (PFNGLUNIFORM2FPROC) glutGetProcAddress("glUniform2f");
   glUniform3f_func = (PFNGLUNIFORM3FPROC) glutGetProcAddress("glUniform3f");
   glUniform4f_func = (PFNGLUNIFORM4FPROC) glutGetProcAddress("glUniform4f");
   glUniform1fv = (PFNGLUNIFORM1FVPROC) glutGetProcAddress("glUniform1fv");
   glUniform2fv = (PFNGLUNIFORM2FVPROC) glutGetProcAddress("glUniform2fv");
   glUniform3fv = (PFNGLUNIFORM3FVPROC) glutGetProcAddress("glUniform3fv");
   glUniform4fv = (PFNGLUNIFORM3FVPROC) glutGetProcAddress("glUniform4fv");
#if 0
   glUniformMatrix2fv_func = (PFNGLUNIFORMMATRIX2FVPROC) glutGetProcAddress("glUniformMatrix2fv");
   glUniformMatrix3fv_func = (PFNGLUNIFORMMATRIX3FVPROC) glutGetProcAddress("glUniformMatrix3fv");
   glUniformMatrix4fv_func = (PFNGLUNIFORMMATRIX4FVPROC) glutGetProcAddress("glUniformMatrix4fv");
#endif
   glUseProgram = (PFNGLUSEPROGRAMPROC) glXGetProcAddress((GLubyte*)"glUseProgram");
#if 0
   glVertexAttrib1f_func = (PFNGLVERTEXATTRIB1FPROC) glutGetProcAddress("glVertexAttrib1f");
   glVertexAttrib2f_func = (PFNGLVERTEXATTRIB2FPROC) glutGetProcAddress("glVertexAttrib2f");
   glVertexAttrib3f_func = (PFNGLVERTEXATTRIB3FPROC) glutGetProcAddress("glVertexAttrib3f");
   glVertexAttrib4f_func = (PFNGLVERTEXATTRIB4FPROC) glutGetProcAddress("glVertexAttrib4f");
   glVertexAttrib1fv_func = (PFNGLVERTEXATTRIB1FVPROC) glutGetProcAddress("glVertexAttrib1fv");
   glVertexAttrib2fv_func = (PFNGLVERTEXATTRIB2FVPROC) glutGetProcAddress("glVertexAttrib2fv");
   glVertexAttrib3fv_func = (PFNGLVERTEXATTRIB3FVPROC) glutGetProcAddress("glVertexAttrib3fv");
   glVertexAttrib4fv_func = (PFNGLVERTEXATTRIB4FVPROC) glutGetProcAddress("glVertexAttrib4fv");

   glVertexAttribPointer_func = (PFNGLVERTEXATTRIBPOINTERPROC) glutGetProcAddress("glVertexAttribPointer");
   glEnableVertexAttribArray_func = (PFNGLENABLEVERTEXATTRIBARRAYPROC) glutGetProcAddress("glEnableVertexAttribArray");
   glDisableVertexAttribArray_func = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) glutGetProcAddress("glDisableVertexAttribArray");

   /* OpenGL 2.1 */
   glUniformMatrix2x3fv_func = (PFNGLUNIFORMMATRIX2X3FVPROC) glutGetProcAddress("glUniformMatrix2x3fv");
   glUniformMatrix3x2fv_func = (PFNGLUNIFORMMATRIX3X2FVPROC) glutGetProcAddress("glUniformMatrix3x2fv");
   glUniformMatrix2x4fv_func = (PFNGLUNIFORMMATRIX2X4FVPROC) glutGetProcAddress("glUniformMatrix2x4fv");
   glUniformMatrix4x2fv_func = (PFNGLUNIFORMMATRIX4X2FVPROC) glutGetProcAddress("glUniformMatrix4x2fv");
   glUniformMatrix3x4fv_func = (PFNGLUNIFORMMATRIX3X4FVPROC) glutGetProcAddress("glUniformMatrix3x4fv");
   glUniformMatrix4x3fv_func = (PFNGLUNIFORMMATRIX4X3FVPROC) glutGetProcAddress("glUniformMatrix4x3fv");

   /* OpenGL 1.4 */
   glPointParameterfv_func = (PFNGLPOINTPARAMETERFVPROC) glutGetProcAddress("glPointParameterfv");
   glSecondaryColor3fv_func = (PFNGLSECONDARYCOLOR3FVPROC) glutGetProcAddress("glSecondaryColor3fv");

   /* GL_ARB_vertex/fragment_program */
   glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC) glutGetProcAddress("glBindProgramARB");
   glDeleteProgramsARB_func = (PFNGLDELETEPROGRAMSARBPROC) glutGetProcAddress("glDeleteProgramsARB");
   glGenProgramsARB_func = (PFNGLGENPROGRAMSARBPROC) glutGetProcAddress("glGenProgramsARB");
   glGetProgramLocalParameterdvARB_func = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) glutGetProcAddress("glGetProgramLocalParameterdvARB");
   glIsProgramARB_func = (PFNGLISPROGRAMARBPROC) glutGetProcAddress("glIsProgramARB");
   glProgramLocalParameter4dARB_func = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC) glutGetProcAddress("glProgramLocalParameter4dARB");
   glProgramLocalParameter4fvARB_func = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) glutGetProcAddress("glProgramLocalParameter4fvARB");
   glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC) glutGetProcAddress("glProgramStringARB");
   glVertexAttrib1fARB_func = (PFNGLVERTEXATTRIB1FARBPROC) glutGetProcAddress("glVertexAttrib1fARB");

   /* GL_APPLE_vertex_array_object */
   glBindVertexArrayAPPLE_func = (PFNGLBINDVERTEXARRAYAPPLEPROC) glutGetProcAddress("glBindVertexArrayAPPLE");
   glDeleteVertexArraysAPPLE_func = (PFNGLDELETEVERTEXARRAYSAPPLEPROC) glutGetProcAddress("glDeleteVertexArraysAPPLE");
   glGenVertexArraysAPPLE_func = (PFNGLGENVERTEXARRAYSAPPLEPROC) glutGetProcAddress("glGenVertexArraysAPPLE");
   glIsVertexArrayAPPLE_func = (PFNGLISVERTEXARRAYAPPLEPROC) glutGetProcAddress("glIsVertexArrayAPPLE");

   /* GL_EXT_stencil_two_side */
   glActiveStencilFaceEXT_func = (PFNGLACTIVESTENCILFACEEXTPROC) glutGetProcAddress("glActiveStencilFaceEXT");

   /* GL_ARB_vertex_buffer_object */
   glGenBuffersARB_func = (PFNGLGENBUFFERSARBPROC) glutGetProcAddress("glGenBuffersARB");
   glDeleteBuffersARB_func = (PFNGLDELETEBUFFERSARBPROC) glutGetProcAddress("glDeleteBuffersARB");
   glBindBufferARB_func = (PFNGLBINDBUFFERARBPROC) glutGetProcAddress("glBindBufferARB");
   glBufferDataARB_func = (PFNGLBUFFERDATAARBPROC) glutGetProcAddress("glBufferDataARB");
   glBufferSubDataARB_func = (PFNGLBUFFERSUBDATAARBPROC) glutGetProcAddress("glBufferSubDataARB");
   glMapBufferARB_func = (PFNGLMAPBUFFERARBPROC) glutGetProcAddress("glMapBufferARB");
   glUnmapBufferARB_func = (PFNGLUNMAPBUFFERARBPROC) glutGetProcAddress("glUnmapBufferARB");

   /* GL_EXT_framebuffer_object */
   glIsRenderbufferEXT_func = (PFNGLISRENDERBUFFEREXTPROC) glutGetProcAddress("glIsRenderbufferEXT");
   glBindRenderbufferEXT_func = (PFNGLBINDRENDERBUFFEREXTPROC) glutGetProcAddress("glBindRenderbufferEXT");
   glDeleteRenderbuffersEXT_func = (PFNGLDELETERENDERBUFFERSEXTPROC) glutGetProcAddress("glDeleteRenderbuffersEXT");
   glGenRenderbuffersEXT_func = (PFNGLGENRENDERBUFFERSEXTPROC) glutGetProcAddress("glGenRenderbuffersEXT");
   glRenderbufferStorageEXT_func = (PFNGLRENDERBUFFERSTORAGEEXTPROC) glutGetProcAddress("glRenderbufferStorageEXT");
   glGetRenderbufferParameterivEXT_func = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) glutGetProcAddress("glGetRenderbufferParameterivEXT");
   glIsFramebufferEXT_func = (PFNGLISFRAMEBUFFEREXTPROC) glutGetProcAddress("glIsFramebufferEXT");
   glBindFramebufferEXT_func = (PFNGLBINDFRAMEBUFFEREXTPROC) glutGetProcAddress("glBindFramebufferEXT");
   glDeleteFramebuffersEXT_func = (PFNGLDELETEFRAMEBUFFERSEXTPROC) glutGetProcAddress("glDeleteFramebuffersEXT");
   glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) glutGetProcAddress("glGenFramebuffersEXT");
   glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) glutGetProcAddress("glCheckFramebufferStatusEXT");
   glFramebufferTexture1DEXT_func = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) glutGetProcAddress("glFramebufferTexture1DEXT");
   glFramebufferTexture2DEXT_func = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) glutGetProcAddress("glFramebufferTexture2DEXT");
   glFramebufferTexture3DEXT_func = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) glutGetProcAddress("glFramebufferTexture3DEXT");
   glFramebufferRenderbufferEXT_func = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) glutGetProcAddress("glFramebufferRenderbufferEXT");
   glGetFramebufferAttachmentParameterivEXT_func = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) glutGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
   glGenerateMipmapEXT_func = (PFNGLGENERATEMIPMAPEXTPROC) glutGetProcAddress("glGenerateMipmapEXT");

   /* GL_ARB_framebuffer_object */
   glIsRenderbuffer_func = (PFNGLISRENDERBUFFERPROC) glutGetProcAddress("glIsRenderbuffer");
   glBindRenderbuffer_func = (PFNGLBINDRENDERBUFFERPROC) glutGetProcAddress("glBindRenderbuffer");
   glDeleteRenderbuffers_func = (PFNGLDELETERENDERBUFFERSPROC) glutGetProcAddress("glDeleteRenderbuffers");
   glGenRenderbuffers_func = (PFNGLGENRENDERBUFFERSPROC) glutGetProcAddress("glGenRenderbuffers");
   glRenderbufferStorage_func = (PFNGLRENDERBUFFERSTORAGEPROC) glutGetProcAddress("glRenderbufferStorage");
   glGetRenderbufferParameteriv_func = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) glutGetProcAddress("glGetRenderbufferParameteriv");
   glIsFramebuffer_func = (PFNGLISFRAMEBUFFERPROC) glutGetProcAddress("glIsFramebuffer");
#endif
   glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) glutGetProcAddress("glBindFramebuffer");
#if 0
   glDeleteFramebuffers_func = (PFNGLDELETEFRAMEBUFFERSPROC) glutGetProcAddress("glDeleteFramebuffers");
#endif
   glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) glutGetProcAddress("glGenFramebuffers");
   glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) glutGetProcAddress("glCheckFramebufferStatus");
#if 0
   glFramebufferTexture1D_func = (PFNGLFRAMEBUFFERTEXTURE1DPROC) glutGetProcAddress("glFramebufferTexture1D");
#endif
   glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) glutGetProcAddress("glFramebufferTexture2D");
#if 0
   glFramebufferTexture3D_func = (PFNGLFRAMEBUFFERTEXTURE3DPROC) glutGetProcAddress("glFramebufferTexture3D");
   glFramebufferRenderbuffer_func = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) glutGetProcAddress("glFramebufferRenderbuffer");
   glGetFramebufferAttachmentParameteriv_func = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) glutGetProcAddress("glGetFramebufferAttachmentParameteriv");
   glGenerateMipmap_func = (PFNGLGENERATEMIPMAPPROC) glutGetProcAddress("glGenerateMipmap");
   glBlitFramebuffer_func = (PFNGLBLITFRAMEBUFFERPROC) glutGetProcAddress("glBlitFramebuffer");
   glRenderbufferStorageMultisample_func = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) glutGetProcAddress("glRenderbufferStorageMultisample");
   glFramebufferTextureLayer_func = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) glutGetProcAddress("glFramebufferTextureLayer");
#endif
}

