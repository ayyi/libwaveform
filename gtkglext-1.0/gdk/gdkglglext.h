/* GdkGLExt - OpenGL Extension to GDK
 * Copyright (C) 2002-2004  Naofumi Yasufuku
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
 */

/*
 * This is a generated file.  Please modify "gen-gdkglglext-h.pl".
 */

#ifndef __GDK_GL_GLEXT_H__
#define __GDK_GL_GLEXT_H__

#include <glib.h>

#include <gdk/gdkglquery.h>

G_BEGIN_DECLS

#ifndef HAVE_GLHALFNV
#if defined(GL_NV_half_float) && defined(GDKGLEXT_NEED_GLHALFNV_TYPEDEF)
typedef unsigned short GLhalfNV;
#endif
#endif

/* Avoid old glext.h bug. */
#if !defined(GL_SGIS_point_parameters) && defined(GL_POINT_SIZE_MIN_SGIS)
#define GL_SGIS_point_parameters 1
#endif

#undef __glext_h_
#undef GL_GLEXT_VERSION
#include <gdk/glext/glext.h>
#include <gdk/glext/glext-extra.h>

/*
 * GL_VERSION_1_2
 */

/* glBlendColor */
typedef void (APIENTRYP GdkGLProc_glBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GdkGLProc    gdk_gl_get_glBlendColor (void);
#define      gdk_gl_glBlendColor(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glBlendColor) (proc)) (red, green, blue, alpha) )

/* glBlendEquation */
typedef void (APIENTRYP GdkGLProc_glBlendEquation) (GLenum mode);
GdkGLProc    gdk_gl_get_glBlendEquation (void);
#define      gdk_gl_glBlendEquation(proc, mode) \
  ( ((GdkGLProc_glBlendEquation) (proc)) (mode) )

/* glDrawRangeElements */
typedef void (APIENTRYP GdkGLProc_glDrawRangeElements) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
GdkGLProc    gdk_gl_get_glDrawRangeElements (void);
#define      gdk_gl_glDrawRangeElements(proc, mode, start, end, count, type, indices) \
  ( ((GdkGLProc_glDrawRangeElements) (proc)) (mode, start, end, count, type, indices) )

/* glTexImage3D */
typedef void (APIENTRYP GdkGLProc_glTexImage3D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexImage3D (void);
#define      gdk_gl_glTexImage3D(proc, target, level, internalformat, width, height, depth, border, format, type, pixels) \
  ( ((GdkGLProc_glTexImage3D) (proc)) (target, level, internalformat, width, height, depth, border, format, type, pixels) )

/* glTexSubImage3D */
typedef void (APIENTRYP GdkGLProc_glTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexSubImage3D (void);
#define      gdk_gl_glTexSubImage3D(proc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) \
  ( ((GdkGLProc_glTexSubImage3D) (proc)) (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) )

/* glCopyTexSubImage3D */
typedef void (APIENTRYP GdkGLProc_glCopyTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyTexSubImage3D (void);
#define      gdk_gl_glCopyTexSubImage3D(proc, target, level, xoffset, yoffset, zoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyTexSubImage3D) (proc)) (target, level, xoffset, yoffset, zoffset, x, y, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_2 GdkGL_GL_VERSION_1_2;

struct _GdkGL_GL_VERSION_1_2
{
  GdkGLProc_glBlendColor glBlendColor;
  GdkGLProc_glBlendEquation glBlendEquation;
  GdkGLProc_glDrawRangeElements glDrawRangeElements;
  GdkGLProc_glTexImage3D glTexImage3D;
  GdkGLProc_glTexSubImage3D glTexSubImage3D;
  GdkGLProc_glCopyTexSubImage3D glCopyTexSubImage3D;
};

GdkGL_GL_VERSION_1_2 *gdk_gl_get_GL_VERSION_1_2 (void);

/*
 * GL_VERSION_1_2_DEPRECATED
 */

/* glColorTable */
typedef void (APIENTRYP GdkGLProc_glColorTable) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
GdkGLProc    gdk_gl_get_glColorTable (void);
#define      gdk_gl_glColorTable(proc, target, internalformat, width, format, type, table) \
  ( ((GdkGLProc_glColorTable) (proc)) (target, internalformat, width, format, type, table) )

/* glColorTableParameterfv */
typedef void (APIENTRYP GdkGLProc_glColorTableParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glColorTableParameterfv (void);
#define      gdk_gl_glColorTableParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glColorTableParameterfv) (proc)) (target, pname, params) )

/* glColorTableParameteriv */
typedef void (APIENTRYP GdkGLProc_glColorTableParameteriv) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glColorTableParameteriv (void);
#define      gdk_gl_glColorTableParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glColorTableParameteriv) (proc)) (target, pname, params) )

/* glCopyColorTable */
typedef void (APIENTRYP GdkGLProc_glCopyColorTable) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyColorTable (void);
#define      gdk_gl_glCopyColorTable(proc, target, internalformat, x, y, width) \
  ( ((GdkGLProc_glCopyColorTable) (proc)) (target, internalformat, x, y, width) )

/* glGetColorTable */
typedef void (APIENTRYP GdkGLProc_glGetColorTable) (GLenum target, GLenum format, GLenum type, GLvoid *table);
GdkGLProc    gdk_gl_get_glGetColorTable (void);
#define      gdk_gl_glGetColorTable(proc, target, format, type, table) \
  ( ((GdkGLProc_glGetColorTable) (proc)) (target, format, type, table) )

/* glGetColorTableParameterfv */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameterfv) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameterfv (void);
#define      gdk_gl_glGetColorTableParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameterfv) (proc)) (target, pname, params) )

/* glGetColorTableParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameteriv (void);
#define      gdk_gl_glGetColorTableParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameteriv) (proc)) (target, pname, params) )

/* glColorSubTable */
typedef void (APIENTRYP GdkGLProc_glColorSubTable) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
GdkGLProc    gdk_gl_get_glColorSubTable (void);
#define      gdk_gl_glColorSubTable(proc, target, start, count, format, type, data) \
  ( ((GdkGLProc_glColorSubTable) (proc)) (target, start, count, format, type, data) )

/* glCopyColorSubTable */
typedef void (APIENTRYP GdkGLProc_glCopyColorSubTable) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyColorSubTable (void);
#define      gdk_gl_glCopyColorSubTable(proc, target, start, x, y, width) \
  ( ((GdkGLProc_glCopyColorSubTable) (proc)) (target, start, x, y, width) )

/* glConvolutionFilter1D */
typedef void (APIENTRYP GdkGLProc_glConvolutionFilter1D) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
GdkGLProc    gdk_gl_get_glConvolutionFilter1D (void);
#define      gdk_gl_glConvolutionFilter1D(proc, target, internalformat, width, format, type, image) \
  ( ((GdkGLProc_glConvolutionFilter1D) (proc)) (target, internalformat, width, format, type, image) )

/* glConvolutionFilter2D */
typedef void (APIENTRYP GdkGLProc_glConvolutionFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
GdkGLProc    gdk_gl_get_glConvolutionFilter2D (void);
#define      gdk_gl_glConvolutionFilter2D(proc, target, internalformat, width, height, format, type, image) \
  ( ((GdkGLProc_glConvolutionFilter2D) (proc)) (target, internalformat, width, height, format, type, image) )

/* glConvolutionParameterf */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameterf) (GLenum target, GLenum pname, GLfloat params);
GdkGLProc    gdk_gl_get_glConvolutionParameterf (void);
#define      gdk_gl_glConvolutionParameterf(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameterf) (proc)) (target, pname, params) )

/* glConvolutionParameterfv */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameterfv) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glConvolutionParameterfv (void);
#define      gdk_gl_glConvolutionParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameterfv) (proc)) (target, pname, params) )

/* glConvolutionParameteri */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameteri) (GLenum target, GLenum pname, GLint params);
GdkGLProc    gdk_gl_get_glConvolutionParameteri (void);
#define      gdk_gl_glConvolutionParameteri(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameteri) (proc)) (target, pname, params) )

/* glConvolutionParameteriv */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameteriv) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glConvolutionParameteriv (void);
#define      gdk_gl_glConvolutionParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameteriv) (proc)) (target, pname, params) )

/* glCopyConvolutionFilter1D */
typedef void (APIENTRYP GdkGLProc_glCopyConvolutionFilter1D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyConvolutionFilter1D (void);
#define      gdk_gl_glCopyConvolutionFilter1D(proc, target, internalformat, x, y, width) \
  ( ((GdkGLProc_glCopyConvolutionFilter1D) (proc)) (target, internalformat, x, y, width) )

/* glCopyConvolutionFilter2D */
typedef void (APIENTRYP GdkGLProc_glCopyConvolutionFilter2D) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyConvolutionFilter2D (void);
#define      gdk_gl_glCopyConvolutionFilter2D(proc, target, internalformat, x, y, width, height) \
  ( ((GdkGLProc_glCopyConvolutionFilter2D) (proc)) (target, internalformat, x, y, width, height) )

/* glGetConvolutionFilter */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionFilter) (GLenum target, GLenum format, GLenum type, GLvoid *image);
GdkGLProc    gdk_gl_get_glGetConvolutionFilter (void);
#define      gdk_gl_glGetConvolutionFilter(proc, target, format, type, image) \
  ( ((GdkGLProc_glGetConvolutionFilter) (proc)) (target, format, type, image) )

/* glGetConvolutionParameterfv */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionParameterfv) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetConvolutionParameterfv (void);
#define      gdk_gl_glGetConvolutionParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetConvolutionParameterfv) (proc)) (target, pname, params) )

/* glGetConvolutionParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetConvolutionParameteriv (void);
#define      gdk_gl_glGetConvolutionParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetConvolutionParameteriv) (proc)) (target, pname, params) )

/* glGetSeparableFilter */
typedef void (APIENTRYP GdkGLProc_glGetSeparableFilter) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
GdkGLProc    gdk_gl_get_glGetSeparableFilter (void);
#define      gdk_gl_glGetSeparableFilter(proc, target, format, type, row, column, span) \
  ( ((GdkGLProc_glGetSeparableFilter) (proc)) (target, format, type, row, column, span) )

/* glSeparableFilter2D */
typedef void (APIENTRYP GdkGLProc_glSeparableFilter2D) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
GdkGLProc    gdk_gl_get_glSeparableFilter2D (void);
#define      gdk_gl_glSeparableFilter2D(proc, target, internalformat, width, height, format, type, row, column) \
  ( ((GdkGLProc_glSeparableFilter2D) (proc)) (target, internalformat, width, height, format, type, row, column) )

/* glGetHistogram */
typedef void (APIENTRYP GdkGLProc_glGetHistogram) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GdkGLProc    gdk_gl_get_glGetHistogram (void);
#define      gdk_gl_glGetHistogram(proc, target, reset, format, type, values) \
  ( ((GdkGLProc_glGetHistogram) (proc)) (target, reset, format, type, values) )

/* glGetHistogramParameterfv */
typedef void (APIENTRYP GdkGLProc_glGetHistogramParameterfv) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetHistogramParameterfv (void);
#define      gdk_gl_glGetHistogramParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetHistogramParameterfv) (proc)) (target, pname, params) )

/* glGetHistogramParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetHistogramParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetHistogramParameteriv (void);
#define      gdk_gl_glGetHistogramParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetHistogramParameteriv) (proc)) (target, pname, params) )

/* glGetMinmax */
typedef void (APIENTRYP GdkGLProc_glGetMinmax) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GdkGLProc    gdk_gl_get_glGetMinmax (void);
#define      gdk_gl_glGetMinmax(proc, target, reset, format, type, values) \
  ( ((GdkGLProc_glGetMinmax) (proc)) (target, reset, format, type, values) )

/* glGetMinmaxParameterfv */
typedef void (APIENTRYP GdkGLProc_glGetMinmaxParameterfv) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMinmaxParameterfv (void);
#define      gdk_gl_glGetMinmaxParameterfv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMinmaxParameterfv) (proc)) (target, pname, params) )

/* glGetMinmaxParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetMinmaxParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMinmaxParameteriv (void);
#define      gdk_gl_glGetMinmaxParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMinmaxParameteriv) (proc)) (target, pname, params) )

/* glHistogram */
typedef void (APIENTRYP GdkGLProc_glHistogram) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
GdkGLProc    gdk_gl_get_glHistogram (void);
#define      gdk_gl_glHistogram(proc, target, width, internalformat, sink) \
  ( ((GdkGLProc_glHistogram) (proc)) (target, width, internalformat, sink) )

/* glMinmax */
typedef void (APIENTRYP GdkGLProc_glMinmax) (GLenum target, GLenum internalformat, GLboolean sink);
GdkGLProc    gdk_gl_get_glMinmax (void);
#define      gdk_gl_glMinmax(proc, target, internalformat, sink) \
  ( ((GdkGLProc_glMinmax) (proc)) (target, internalformat, sink) )

/* glResetHistogram */
typedef void (APIENTRYP GdkGLProc_glResetHistogram) (GLenum target);
GdkGLProc    gdk_gl_get_glResetHistogram (void);
#define      gdk_gl_glResetHistogram(proc, target) \
  ( ((GdkGLProc_glResetHistogram) (proc)) (target) )

/* glResetMinmax */
typedef void (APIENTRYP GdkGLProc_glResetMinmax) (GLenum target);
GdkGLProc    gdk_gl_get_glResetMinmax (void);
#define      gdk_gl_glResetMinmax(proc, target) \
  ( ((GdkGLProc_glResetMinmax) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_2_DEPRECATED GdkGL_GL_VERSION_1_2_DEPRECATED;

struct _GdkGL_GL_VERSION_1_2_DEPRECATED
{
  GdkGLProc_glColorTable glColorTable;
  GdkGLProc_glColorTableParameterfv glColorTableParameterfv;
  GdkGLProc_glColorTableParameteriv glColorTableParameteriv;
  GdkGLProc_glCopyColorTable glCopyColorTable;
  GdkGLProc_glGetColorTable glGetColorTable;
  GdkGLProc_glGetColorTableParameterfv glGetColorTableParameterfv;
  GdkGLProc_glGetColorTableParameteriv glGetColorTableParameteriv;
  GdkGLProc_glColorSubTable glColorSubTable;
  GdkGLProc_glCopyColorSubTable glCopyColorSubTable;
  GdkGLProc_glConvolutionFilter1D glConvolutionFilter1D;
  GdkGLProc_glConvolutionFilter2D glConvolutionFilter2D;
  GdkGLProc_glConvolutionParameterf glConvolutionParameterf;
  GdkGLProc_glConvolutionParameterfv glConvolutionParameterfv;
  GdkGLProc_glConvolutionParameteri glConvolutionParameteri;
  GdkGLProc_glConvolutionParameteriv glConvolutionParameteriv;
  GdkGLProc_glCopyConvolutionFilter1D glCopyConvolutionFilter1D;
  GdkGLProc_glCopyConvolutionFilter2D glCopyConvolutionFilter2D;
  GdkGLProc_glGetConvolutionFilter glGetConvolutionFilter;
  GdkGLProc_glGetConvolutionParameterfv glGetConvolutionParameterfv;
  GdkGLProc_glGetConvolutionParameteriv glGetConvolutionParameteriv;
  GdkGLProc_glGetSeparableFilter glGetSeparableFilter;
  GdkGLProc_glSeparableFilter2D glSeparableFilter2D;
  GdkGLProc_glGetHistogram glGetHistogram;
  GdkGLProc_glGetHistogramParameterfv glGetHistogramParameterfv;
  GdkGLProc_glGetHistogramParameteriv glGetHistogramParameteriv;
  GdkGLProc_glGetMinmax glGetMinmax;
  GdkGLProc_glGetMinmaxParameterfv glGetMinmaxParameterfv;
  GdkGLProc_glGetMinmaxParameteriv glGetMinmaxParameteriv;
  GdkGLProc_glHistogram glHistogram;
  GdkGLProc_glMinmax glMinmax;
  GdkGLProc_glResetHistogram glResetHistogram;
  GdkGLProc_glResetMinmax glResetMinmax;
};

GdkGL_GL_VERSION_1_2_DEPRECATED *gdk_gl_get_GL_VERSION_1_2_DEPRECATED (void);

/*
 * GL_VERSION_1_3
 */

/* glActiveTexture */
typedef void (APIENTRYP GdkGLProc_glActiveTexture) (GLenum texture);
GdkGLProc    gdk_gl_get_glActiveTexture (void);
#define      gdk_gl_glActiveTexture(proc, texture) \
  ( ((GdkGLProc_glActiveTexture) (proc)) (texture) )

/* glSampleCoverage */
typedef void (APIENTRYP GdkGLProc_glSampleCoverage) (GLclampf value, GLboolean invert);
GdkGLProc    gdk_gl_get_glSampleCoverage (void);
#define      gdk_gl_glSampleCoverage(proc, value, invert) \
  ( ((GdkGLProc_glSampleCoverage) (proc)) (value, invert) )

/* glCompressedTexImage3D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage3D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage3D (void);
#define      gdk_gl_glCompressedTexImage3D(proc, target, level, internalformat, width, height, depth, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage3D) (proc)) (target, level, internalformat, width, height, depth, border, imageSize, data) )

/* glCompressedTexImage2D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage2D (void);
#define      gdk_gl_glCompressedTexImage2D(proc, target, level, internalformat, width, height, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage2D) (proc)) (target, level, internalformat, width, height, border, imageSize, data) )

/* glCompressedTexImage1D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage1D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage1D (void);
#define      gdk_gl_glCompressedTexImage1D(proc, target, level, internalformat, width, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage1D) (proc)) (target, level, internalformat, width, border, imageSize, data) )

/* glCompressedTexSubImage3D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage3D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage3D (void);
#define      gdk_gl_glCompressedTexSubImage3D(proc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage3D) (proc)) (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) )

/* glCompressedTexSubImage2D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage2D (void);
#define      gdk_gl_glCompressedTexSubImage2D(proc, target, level, xoffset, yoffset, width, height, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage2D) (proc)) (target, level, xoffset, yoffset, width, height, format, imageSize, data) )

/* glCompressedTexSubImage1D */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage1D) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage1D (void);
#define      gdk_gl_glCompressedTexSubImage1D(proc, target, level, xoffset, width, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage1D) (proc)) (target, level, xoffset, width, format, imageSize, data) )

/* glGetCompressedTexImage */
typedef void (APIENTRYP GdkGLProc_glGetCompressedTexImage) (GLenum target, GLint level, GLvoid *img);
GdkGLProc    gdk_gl_get_glGetCompressedTexImage (void);
#define      gdk_gl_glGetCompressedTexImage(proc, target, level, img) \
  ( ((GdkGLProc_glGetCompressedTexImage) (proc)) (target, level, img) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_3 GdkGL_GL_VERSION_1_3;

struct _GdkGL_GL_VERSION_1_3
{
  GdkGLProc_glActiveTexture glActiveTexture;
  GdkGLProc_glSampleCoverage glSampleCoverage;
  GdkGLProc_glCompressedTexImage3D glCompressedTexImage3D;
  GdkGLProc_glCompressedTexImage2D glCompressedTexImage2D;
  GdkGLProc_glCompressedTexImage1D glCompressedTexImage1D;
  GdkGLProc_glCompressedTexSubImage3D glCompressedTexSubImage3D;
  GdkGLProc_glCompressedTexSubImage2D glCompressedTexSubImage2D;
  GdkGLProc_glCompressedTexSubImage1D glCompressedTexSubImage1D;
  GdkGLProc_glGetCompressedTexImage glGetCompressedTexImage;
};

GdkGL_GL_VERSION_1_3 *gdk_gl_get_GL_VERSION_1_3 (void);

/*
 * GL_VERSION_1_3_DEPRECATED
 */

/* glClientActiveTexture */
typedef void (APIENTRYP GdkGLProc_glClientActiveTexture) (GLenum texture);
GdkGLProc    gdk_gl_get_glClientActiveTexture (void);
#define      gdk_gl_glClientActiveTexture(proc, texture) \
  ( ((GdkGLProc_glClientActiveTexture) (proc)) (texture) )

/* glMultiTexCoord1d */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1d) (GLenum target, GLdouble s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1d (void);
#define      gdk_gl_glMultiTexCoord1d(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1d) (proc)) (target, s) )

/* glMultiTexCoord1dv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1dv) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dv (void);
#define      gdk_gl_glMultiTexCoord1dv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1dv) (proc)) (target, v) )

/* glMultiTexCoord1f */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1f) (GLenum target, GLfloat s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1f (void);
#define      gdk_gl_glMultiTexCoord1f(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1f) (proc)) (target, s) )

/* glMultiTexCoord1fv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1fv) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fv (void);
#define      gdk_gl_glMultiTexCoord1fv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1fv) (proc)) (target, v) )

/* glMultiTexCoord1i */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1i) (GLenum target, GLint s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1i (void);
#define      gdk_gl_glMultiTexCoord1i(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1i) (proc)) (target, s) )

/* glMultiTexCoord1iv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1iv) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1iv (void);
#define      gdk_gl_glMultiTexCoord1iv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1iv) (proc)) (target, v) )

/* glMultiTexCoord1s */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1s) (GLenum target, GLshort s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1s (void);
#define      gdk_gl_glMultiTexCoord1s(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1s) (proc)) (target, s) )

/* glMultiTexCoord1sv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1sv) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1sv (void);
#define      gdk_gl_glMultiTexCoord1sv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1sv) (proc)) (target, v) )

/* glMultiTexCoord2d */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2d) (GLenum target, GLdouble s, GLdouble t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2d (void);
#define      gdk_gl_glMultiTexCoord2d(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2d) (proc)) (target, s, t) )

/* glMultiTexCoord2dv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2dv) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dv (void);
#define      gdk_gl_glMultiTexCoord2dv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2dv) (proc)) (target, v) )

/* glMultiTexCoord2f */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2f) (GLenum target, GLfloat s, GLfloat t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2f (void);
#define      gdk_gl_glMultiTexCoord2f(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2f) (proc)) (target, s, t) )

/* glMultiTexCoord2fv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2fv) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fv (void);
#define      gdk_gl_glMultiTexCoord2fv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2fv) (proc)) (target, v) )

/* glMultiTexCoord2i */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2i) (GLenum target, GLint s, GLint t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2i (void);
#define      gdk_gl_glMultiTexCoord2i(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2i) (proc)) (target, s, t) )

/* glMultiTexCoord2iv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2iv) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2iv (void);
#define      gdk_gl_glMultiTexCoord2iv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2iv) (proc)) (target, v) )

/* glMultiTexCoord2s */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2s) (GLenum target, GLshort s, GLshort t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2s (void);
#define      gdk_gl_glMultiTexCoord2s(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2s) (proc)) (target, s, t) )

/* glMultiTexCoord2sv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2sv) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2sv (void);
#define      gdk_gl_glMultiTexCoord2sv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2sv) (proc)) (target, v) )

/* glMultiTexCoord3d */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3d) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3d (void);
#define      gdk_gl_glMultiTexCoord3d(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3d) (proc)) (target, s, t, r) )

/* glMultiTexCoord3dv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3dv) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dv (void);
#define      gdk_gl_glMultiTexCoord3dv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3dv) (proc)) (target, v) )

/* glMultiTexCoord3f */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3f) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3f (void);
#define      gdk_gl_glMultiTexCoord3f(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3f) (proc)) (target, s, t, r) )

/* glMultiTexCoord3fv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3fv) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fv (void);
#define      gdk_gl_glMultiTexCoord3fv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3fv) (proc)) (target, v) )

/* glMultiTexCoord3i */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3i) (GLenum target, GLint s, GLint t, GLint r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3i (void);
#define      gdk_gl_glMultiTexCoord3i(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3i) (proc)) (target, s, t, r) )

/* glMultiTexCoord3iv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3iv) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3iv (void);
#define      gdk_gl_glMultiTexCoord3iv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3iv) (proc)) (target, v) )

/* glMultiTexCoord3s */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3s) (GLenum target, GLshort s, GLshort t, GLshort r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3s (void);
#define      gdk_gl_glMultiTexCoord3s(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3s) (proc)) (target, s, t, r) )

/* glMultiTexCoord3sv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3sv) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3sv (void);
#define      gdk_gl_glMultiTexCoord3sv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3sv) (proc)) (target, v) )

/* glMultiTexCoord4d */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4d) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4d (void);
#define      gdk_gl_glMultiTexCoord4d(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4d) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4dv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4dv) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dv (void);
#define      gdk_gl_glMultiTexCoord4dv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4dv) (proc)) (target, v) )

/* glMultiTexCoord4f */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4f) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4f (void);
#define      gdk_gl_glMultiTexCoord4f(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4f) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4fv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4fv) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fv (void);
#define      gdk_gl_glMultiTexCoord4fv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4fv) (proc)) (target, v) )

/* glMultiTexCoord4i */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4i) (GLenum target, GLint s, GLint t, GLint r, GLint q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4i (void);
#define      gdk_gl_glMultiTexCoord4i(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4i) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4iv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4iv) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4iv (void);
#define      gdk_gl_glMultiTexCoord4iv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4iv) (proc)) (target, v) )

/* glMultiTexCoord4s */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4s) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4s (void);
#define      gdk_gl_glMultiTexCoord4s(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4s) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4sv */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4sv) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4sv (void);
#define      gdk_gl_glMultiTexCoord4sv(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4sv) (proc)) (target, v) )

/* glLoadTransposeMatrixf */
typedef void (APIENTRYP GdkGLProc_glLoadTransposeMatrixf) (const GLfloat *m);
GdkGLProc    gdk_gl_get_glLoadTransposeMatrixf (void);
#define      gdk_gl_glLoadTransposeMatrixf(proc, m) \
  ( ((GdkGLProc_glLoadTransposeMatrixf) (proc)) (m) )

/* glLoadTransposeMatrixd */
typedef void (APIENTRYP GdkGLProc_glLoadTransposeMatrixd) (const GLdouble *m);
GdkGLProc    gdk_gl_get_glLoadTransposeMatrixd (void);
#define      gdk_gl_glLoadTransposeMatrixd(proc, m) \
  ( ((GdkGLProc_glLoadTransposeMatrixd) (proc)) (m) )

/* glMultTransposeMatrixf */
typedef void (APIENTRYP GdkGLProc_glMultTransposeMatrixf) (const GLfloat *m);
GdkGLProc    gdk_gl_get_glMultTransposeMatrixf (void);
#define      gdk_gl_glMultTransposeMatrixf(proc, m) \
  ( ((GdkGLProc_glMultTransposeMatrixf) (proc)) (m) )

/* glMultTransposeMatrixd */
typedef void (APIENTRYP GdkGLProc_glMultTransposeMatrixd) (const GLdouble *m);
GdkGLProc    gdk_gl_get_glMultTransposeMatrixd (void);
#define      gdk_gl_glMultTransposeMatrixd(proc, m) \
  ( ((GdkGLProc_glMultTransposeMatrixd) (proc)) (m) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_3_DEPRECATED GdkGL_GL_VERSION_1_3_DEPRECATED;

struct _GdkGL_GL_VERSION_1_3_DEPRECATED
{
  GdkGLProc_glClientActiveTexture glClientActiveTexture;
  GdkGLProc_glMultiTexCoord1d glMultiTexCoord1d;
  GdkGLProc_glMultiTexCoord1dv glMultiTexCoord1dv;
  GdkGLProc_glMultiTexCoord1f glMultiTexCoord1f;
  GdkGLProc_glMultiTexCoord1fv glMultiTexCoord1fv;
  GdkGLProc_glMultiTexCoord1i glMultiTexCoord1i;
  GdkGLProc_glMultiTexCoord1iv glMultiTexCoord1iv;
  GdkGLProc_glMultiTexCoord1s glMultiTexCoord1s;
  GdkGLProc_glMultiTexCoord1sv glMultiTexCoord1sv;
  GdkGLProc_glMultiTexCoord2d glMultiTexCoord2d;
  GdkGLProc_glMultiTexCoord2dv glMultiTexCoord2dv;
  GdkGLProc_glMultiTexCoord2f glMultiTexCoord2f;
  GdkGLProc_glMultiTexCoord2fv glMultiTexCoord2fv;
  GdkGLProc_glMultiTexCoord2i glMultiTexCoord2i;
  GdkGLProc_glMultiTexCoord2iv glMultiTexCoord2iv;
  GdkGLProc_glMultiTexCoord2s glMultiTexCoord2s;
  GdkGLProc_glMultiTexCoord2sv glMultiTexCoord2sv;
  GdkGLProc_glMultiTexCoord3d glMultiTexCoord3d;
  GdkGLProc_glMultiTexCoord3dv glMultiTexCoord3dv;
  GdkGLProc_glMultiTexCoord3f glMultiTexCoord3f;
  GdkGLProc_glMultiTexCoord3fv glMultiTexCoord3fv;
  GdkGLProc_glMultiTexCoord3i glMultiTexCoord3i;
  GdkGLProc_glMultiTexCoord3iv glMultiTexCoord3iv;
  GdkGLProc_glMultiTexCoord3s glMultiTexCoord3s;
  GdkGLProc_glMultiTexCoord3sv glMultiTexCoord3sv;
  GdkGLProc_glMultiTexCoord4d glMultiTexCoord4d;
  GdkGLProc_glMultiTexCoord4dv glMultiTexCoord4dv;
  GdkGLProc_glMultiTexCoord4f glMultiTexCoord4f;
  GdkGLProc_glMultiTexCoord4fv glMultiTexCoord4fv;
  GdkGLProc_glMultiTexCoord4i glMultiTexCoord4i;
  GdkGLProc_glMultiTexCoord4iv glMultiTexCoord4iv;
  GdkGLProc_glMultiTexCoord4s glMultiTexCoord4s;
  GdkGLProc_glMultiTexCoord4sv glMultiTexCoord4sv;
  GdkGLProc_glLoadTransposeMatrixf glLoadTransposeMatrixf;
  GdkGLProc_glLoadTransposeMatrixd glLoadTransposeMatrixd;
  GdkGLProc_glMultTransposeMatrixf glMultTransposeMatrixf;
  GdkGLProc_glMultTransposeMatrixd glMultTransposeMatrixd;
};

GdkGL_GL_VERSION_1_3_DEPRECATED *gdk_gl_get_GL_VERSION_1_3_DEPRECATED (void);

/*
 * GL_VERSION_1_4
 */

/* glBlendFuncSeparate */
typedef void (APIENTRYP GdkGLProc_glBlendFuncSeparate) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GdkGLProc    gdk_gl_get_glBlendFuncSeparate (void);
#define      gdk_gl_glBlendFuncSeparate(proc, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) \
  ( ((GdkGLProc_glBlendFuncSeparate) (proc)) (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) )

/* glMultiDrawArrays */
typedef void (APIENTRYP GdkGLProc_glMultiDrawArrays) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawArrays (void);
#define      gdk_gl_glMultiDrawArrays(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawArrays) (proc)) (mode, first, count, primcount) )

/* glMultiDrawElements */
typedef void (APIENTRYP GdkGLProc_glMultiDrawElements) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawElements (void);
#define      gdk_gl_glMultiDrawElements(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glMultiDrawElements) (proc)) (mode, count, type, indices, primcount) )

/* glPointParameterf */
typedef void (APIENTRYP GdkGLProc_glPointParameterf) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPointParameterf (void);
#define      gdk_gl_glPointParameterf(proc, pname, param) \
  ( ((GdkGLProc_glPointParameterf) (proc)) (pname, param) )

/* glPointParameterfv */
typedef void (APIENTRYP GdkGLProc_glPointParameterfv) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPointParameterfv (void);
#define      gdk_gl_glPointParameterfv(proc, pname, params) \
  ( ((GdkGLProc_glPointParameterfv) (proc)) (pname, params) )

/* glPointParameteri */
typedef void (APIENTRYP GdkGLProc_glPointParameteri) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPointParameteri (void);
#define      gdk_gl_glPointParameteri(proc, pname, param) \
  ( ((GdkGLProc_glPointParameteri) (proc)) (pname, param) )

/* glPointParameteriv */
typedef void (APIENTRYP GdkGLProc_glPointParameteriv) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glPointParameteriv (void);
#define      gdk_gl_glPointParameteriv(proc, pname, params) \
  ( ((GdkGLProc_glPointParameteriv) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_4 GdkGL_GL_VERSION_1_4;

struct _GdkGL_GL_VERSION_1_4
{
  GdkGLProc_glBlendFuncSeparate glBlendFuncSeparate;
  GdkGLProc_glMultiDrawArrays glMultiDrawArrays;
  GdkGLProc_glMultiDrawElements glMultiDrawElements;
  GdkGLProc_glPointParameterf glPointParameterf;
  GdkGLProc_glPointParameterfv glPointParameterfv;
  GdkGLProc_glPointParameteri glPointParameteri;
  GdkGLProc_glPointParameteriv glPointParameteriv;
};

GdkGL_GL_VERSION_1_4 *gdk_gl_get_GL_VERSION_1_4 (void);

/*
 * GL_VERSION_1_4_DEPRECATED
 */

/* glFogCoordf */
typedef void (APIENTRYP GdkGLProc_glFogCoordf) (GLfloat coord);
GdkGLProc    gdk_gl_get_glFogCoordf (void);
#define      gdk_gl_glFogCoordf(proc, coord) \
  ( ((GdkGLProc_glFogCoordf) (proc)) (coord) )

/* glFogCoordfv */
typedef void (APIENTRYP GdkGLProc_glFogCoordfv) (const GLfloat *coord);
GdkGLProc    gdk_gl_get_glFogCoordfv (void);
#define      gdk_gl_glFogCoordfv(proc, coord) \
  ( ((GdkGLProc_glFogCoordfv) (proc)) (coord) )

/* glFogCoordd */
typedef void (APIENTRYP GdkGLProc_glFogCoordd) (GLdouble coord);
GdkGLProc    gdk_gl_get_glFogCoordd (void);
#define      gdk_gl_glFogCoordd(proc, coord) \
  ( ((GdkGLProc_glFogCoordd) (proc)) (coord) )

/* glFogCoorddv */
typedef void (APIENTRYP GdkGLProc_glFogCoorddv) (const GLdouble *coord);
GdkGLProc    gdk_gl_get_glFogCoorddv (void);
#define      gdk_gl_glFogCoorddv(proc, coord) \
  ( ((GdkGLProc_glFogCoorddv) (proc)) (coord) )

/* glFogCoordPointer */
typedef void (APIENTRYP GdkGLProc_glFogCoordPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glFogCoordPointer (void);
#define      gdk_gl_glFogCoordPointer(proc, type, stride, pointer) \
  ( ((GdkGLProc_glFogCoordPointer) (proc)) (type, stride, pointer) )

/* glSecondaryColor3b */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3b) (GLbyte red, GLbyte green, GLbyte blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3b (void);
#define      gdk_gl_glSecondaryColor3b(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3b) (proc)) (red, green, blue) )

/* glSecondaryColor3bv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3bv) (const GLbyte *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3bv (void);
#define      gdk_gl_glSecondaryColor3bv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3bv) (proc)) (v) )

/* glSecondaryColor3d */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3d) (GLdouble red, GLdouble green, GLdouble blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3d (void);
#define      gdk_gl_glSecondaryColor3d(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3d) (proc)) (red, green, blue) )

/* glSecondaryColor3dv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3dv) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3dv (void);
#define      gdk_gl_glSecondaryColor3dv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3dv) (proc)) (v) )

/* glSecondaryColor3f */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3f) (GLfloat red, GLfloat green, GLfloat blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3f (void);
#define      gdk_gl_glSecondaryColor3f(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3f) (proc)) (red, green, blue) )

/* glSecondaryColor3fv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3fv) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3fv (void);
#define      gdk_gl_glSecondaryColor3fv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3fv) (proc)) (v) )

/* glSecondaryColor3i */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3i) (GLint red, GLint green, GLint blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3i (void);
#define      gdk_gl_glSecondaryColor3i(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3i) (proc)) (red, green, blue) )

/* glSecondaryColor3iv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3iv) (const GLint *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3iv (void);
#define      gdk_gl_glSecondaryColor3iv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3iv) (proc)) (v) )

/* glSecondaryColor3s */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3s) (GLshort red, GLshort green, GLshort blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3s (void);
#define      gdk_gl_glSecondaryColor3s(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3s) (proc)) (red, green, blue) )

/* glSecondaryColor3sv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3sv) (const GLshort *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3sv (void);
#define      gdk_gl_glSecondaryColor3sv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3sv) (proc)) (v) )

/* glSecondaryColor3ub */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ub) (GLubyte red, GLubyte green, GLubyte blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3ub (void);
#define      gdk_gl_glSecondaryColor3ub(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3ub) (proc)) (red, green, blue) )

/* glSecondaryColor3ubv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ubv) (const GLubyte *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3ubv (void);
#define      gdk_gl_glSecondaryColor3ubv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3ubv) (proc)) (v) )

/* glSecondaryColor3ui */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ui) (GLuint red, GLuint green, GLuint blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3ui (void);
#define      gdk_gl_glSecondaryColor3ui(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3ui) (proc)) (red, green, blue) )

/* glSecondaryColor3uiv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3uiv) (const GLuint *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3uiv (void);
#define      gdk_gl_glSecondaryColor3uiv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3uiv) (proc)) (v) )

/* glSecondaryColor3us */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3us) (GLushort red, GLushort green, GLushort blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3us (void);
#define      gdk_gl_glSecondaryColor3us(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3us) (proc)) (red, green, blue) )

/* glSecondaryColor3usv */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3usv) (const GLushort *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3usv (void);
#define      gdk_gl_glSecondaryColor3usv(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3usv) (proc)) (v) )

/* glSecondaryColorPointer */
typedef void (APIENTRYP GdkGLProc_glSecondaryColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glSecondaryColorPointer (void);
#define      gdk_gl_glSecondaryColorPointer(proc, size, type, stride, pointer) \
  ( ((GdkGLProc_glSecondaryColorPointer) (proc)) (size, type, stride, pointer) )

/* glWindowPos2d */
typedef void (APIENTRYP GdkGLProc_glWindowPos2d) (GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glWindowPos2d (void);
#define      gdk_gl_glWindowPos2d(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2d) (proc)) (x, y) )

/* glWindowPos2dv */
typedef void (APIENTRYP GdkGLProc_glWindowPos2dv) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos2dv (void);
#define      gdk_gl_glWindowPos2dv(proc, v) \
  ( ((GdkGLProc_glWindowPos2dv) (proc)) (v) )

/* glWindowPos2f */
typedef void (APIENTRYP GdkGLProc_glWindowPos2f) (GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glWindowPos2f (void);
#define      gdk_gl_glWindowPos2f(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2f) (proc)) (x, y) )

/* glWindowPos2fv */
typedef void (APIENTRYP GdkGLProc_glWindowPos2fv) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos2fv (void);
#define      gdk_gl_glWindowPos2fv(proc, v) \
  ( ((GdkGLProc_glWindowPos2fv) (proc)) (v) )

/* glWindowPos2i */
typedef void (APIENTRYP GdkGLProc_glWindowPos2i) (GLint x, GLint y);
GdkGLProc    gdk_gl_get_glWindowPos2i (void);
#define      gdk_gl_glWindowPos2i(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2i) (proc)) (x, y) )

/* glWindowPos2iv */
typedef void (APIENTRYP GdkGLProc_glWindowPos2iv) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos2iv (void);
#define      gdk_gl_glWindowPos2iv(proc, v) \
  ( ((GdkGLProc_glWindowPos2iv) (proc)) (v) )

/* glWindowPos2s */
typedef void (APIENTRYP GdkGLProc_glWindowPos2s) (GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glWindowPos2s (void);
#define      gdk_gl_glWindowPos2s(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2s) (proc)) (x, y) )

/* glWindowPos2sv */
typedef void (APIENTRYP GdkGLProc_glWindowPos2sv) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos2sv (void);
#define      gdk_gl_glWindowPos2sv(proc, v) \
  ( ((GdkGLProc_glWindowPos2sv) (proc)) (v) )

/* glWindowPos3d */
typedef void (APIENTRYP GdkGLProc_glWindowPos3d) (GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glWindowPos3d (void);
#define      gdk_gl_glWindowPos3d(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3d) (proc)) (x, y, z) )

/* glWindowPos3dv */
typedef void (APIENTRYP GdkGLProc_glWindowPos3dv) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos3dv (void);
#define      gdk_gl_glWindowPos3dv(proc, v) \
  ( ((GdkGLProc_glWindowPos3dv) (proc)) (v) )

/* glWindowPos3f */
typedef void (APIENTRYP GdkGLProc_glWindowPos3f) (GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glWindowPos3f (void);
#define      gdk_gl_glWindowPos3f(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3f) (proc)) (x, y, z) )

/* glWindowPos3fv */
typedef void (APIENTRYP GdkGLProc_glWindowPos3fv) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos3fv (void);
#define      gdk_gl_glWindowPos3fv(proc, v) \
  ( ((GdkGLProc_glWindowPos3fv) (proc)) (v) )

/* glWindowPos3i */
typedef void (APIENTRYP GdkGLProc_glWindowPos3i) (GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glWindowPos3i (void);
#define      gdk_gl_glWindowPos3i(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3i) (proc)) (x, y, z) )

/* glWindowPos3iv */
typedef void (APIENTRYP GdkGLProc_glWindowPos3iv) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos3iv (void);
#define      gdk_gl_glWindowPos3iv(proc, v) \
  ( ((GdkGLProc_glWindowPos3iv) (proc)) (v) )

/* glWindowPos3s */
typedef void (APIENTRYP GdkGLProc_glWindowPos3s) (GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glWindowPos3s (void);
#define      gdk_gl_glWindowPos3s(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3s) (proc)) (x, y, z) )

/* glWindowPos3sv */
typedef void (APIENTRYP GdkGLProc_glWindowPos3sv) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos3sv (void);
#define      gdk_gl_glWindowPos3sv(proc, v) \
  ( ((GdkGLProc_glWindowPos3sv) (proc)) (v) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_4_DEPRECATED GdkGL_GL_VERSION_1_4_DEPRECATED;

struct _GdkGL_GL_VERSION_1_4_DEPRECATED
{
  GdkGLProc_glFogCoordf glFogCoordf;
  GdkGLProc_glFogCoordfv glFogCoordfv;
  GdkGLProc_glFogCoordd glFogCoordd;
  GdkGLProc_glFogCoorddv glFogCoorddv;
  GdkGLProc_glFogCoordPointer glFogCoordPointer;
  GdkGLProc_glSecondaryColor3b glSecondaryColor3b;
  GdkGLProc_glSecondaryColor3bv glSecondaryColor3bv;
  GdkGLProc_glSecondaryColor3d glSecondaryColor3d;
  GdkGLProc_glSecondaryColor3dv glSecondaryColor3dv;
  GdkGLProc_glSecondaryColor3f glSecondaryColor3f;
  GdkGLProc_glSecondaryColor3fv glSecondaryColor3fv;
  GdkGLProc_glSecondaryColor3i glSecondaryColor3i;
  GdkGLProc_glSecondaryColor3iv glSecondaryColor3iv;
  GdkGLProc_glSecondaryColor3s glSecondaryColor3s;
  GdkGLProc_glSecondaryColor3sv glSecondaryColor3sv;
  GdkGLProc_glSecondaryColor3ub glSecondaryColor3ub;
  GdkGLProc_glSecondaryColor3ubv glSecondaryColor3ubv;
  GdkGLProc_glSecondaryColor3ui glSecondaryColor3ui;
  GdkGLProc_glSecondaryColor3uiv glSecondaryColor3uiv;
  GdkGLProc_glSecondaryColor3us glSecondaryColor3us;
  GdkGLProc_glSecondaryColor3usv glSecondaryColor3usv;
  GdkGLProc_glSecondaryColorPointer glSecondaryColorPointer;
  GdkGLProc_glWindowPos2d glWindowPos2d;
  GdkGLProc_glWindowPos2dv glWindowPos2dv;
  GdkGLProc_glWindowPos2f glWindowPos2f;
  GdkGLProc_glWindowPos2fv glWindowPos2fv;
  GdkGLProc_glWindowPos2i glWindowPos2i;
  GdkGLProc_glWindowPos2iv glWindowPos2iv;
  GdkGLProc_glWindowPos2s glWindowPos2s;
  GdkGLProc_glWindowPos2sv glWindowPos2sv;
  GdkGLProc_glWindowPos3d glWindowPos3d;
  GdkGLProc_glWindowPos3dv glWindowPos3dv;
  GdkGLProc_glWindowPos3f glWindowPos3f;
  GdkGLProc_glWindowPos3fv glWindowPos3fv;
  GdkGLProc_glWindowPos3i glWindowPos3i;
  GdkGLProc_glWindowPos3iv glWindowPos3iv;
  GdkGLProc_glWindowPos3s glWindowPos3s;
  GdkGLProc_glWindowPos3sv glWindowPos3sv;
};

GdkGL_GL_VERSION_1_4_DEPRECATED *gdk_gl_get_GL_VERSION_1_4_DEPRECATED (void);

/*
 * GL_VERSION_1_5
 */

/* glGenQueries */
typedef void (APIENTRYP GdkGLProc_glGenQueries) (GLsizei n, GLuint *ids);
GdkGLProc    gdk_gl_get_glGenQueries (void);
#define      gdk_gl_glGenQueries(proc, n, ids) \
  ( ((GdkGLProc_glGenQueries) (proc)) (n, ids) )

/* glDeleteQueries */
typedef void (APIENTRYP GdkGLProc_glDeleteQueries) (GLsizei n, const GLuint *ids);
GdkGLProc    gdk_gl_get_glDeleteQueries (void);
#define      gdk_gl_glDeleteQueries(proc, n, ids) \
  ( ((GdkGLProc_glDeleteQueries) (proc)) (n, ids) )

/* glIsQuery */
typedef GLboolean (APIENTRYP GdkGLProc_glIsQuery) (GLuint id);
GdkGLProc    gdk_gl_get_glIsQuery (void);
#define      gdk_gl_glIsQuery(proc, id) \
  ( ((GdkGLProc_glIsQuery) (proc)) (id) )

/* glBeginQuery */
typedef void (APIENTRYP GdkGLProc_glBeginQuery) (GLenum target, GLuint id);
GdkGLProc    gdk_gl_get_glBeginQuery (void);
#define      gdk_gl_glBeginQuery(proc, target, id) \
  ( ((GdkGLProc_glBeginQuery) (proc)) (target, id) )

/* glEndQuery */
typedef void (APIENTRYP GdkGLProc_glEndQuery) (GLenum target);
GdkGLProc    gdk_gl_get_glEndQuery (void);
#define      gdk_gl_glEndQuery(proc, target) \
  ( ((GdkGLProc_glEndQuery) (proc)) (target) )

/* glGetQueryiv */
typedef void (APIENTRYP GdkGLProc_glGetQueryiv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetQueryiv (void);
#define      gdk_gl_glGetQueryiv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetQueryiv) (proc)) (target, pname, params) )

/* glGetQueryObjectiv */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjectiv) (GLuint id, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetQueryObjectiv (void);
#define      gdk_gl_glGetQueryObjectiv(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjectiv) (proc)) (id, pname, params) )

/* glGetQueryObjectuiv */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjectuiv) (GLuint id, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetQueryObjectuiv (void);
#define      gdk_gl_glGetQueryObjectuiv(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjectuiv) (proc)) (id, pname, params) )

/* glBindBuffer */
typedef void (APIENTRYP GdkGLProc_glBindBuffer) (GLenum target, GLuint buffer);
GdkGLProc    gdk_gl_get_glBindBuffer (void);
#define      gdk_gl_glBindBuffer(proc, target, buffer) \
  ( ((GdkGLProc_glBindBuffer) (proc)) (target, buffer) )

/* glDeleteBuffers */
typedef void (APIENTRYP GdkGLProc_glDeleteBuffers) (GLsizei n, const GLuint *buffers);
GdkGLProc    gdk_gl_get_glDeleteBuffers (void);
#define      gdk_gl_glDeleteBuffers(proc, n, buffers) \
  ( ((GdkGLProc_glDeleteBuffers) (proc)) (n, buffers) )

/* glGenBuffers */
typedef void (APIENTRYP GdkGLProc_glGenBuffers) (GLsizei n, GLuint *buffers);
GdkGLProc    gdk_gl_get_glGenBuffers (void);
#define      gdk_gl_glGenBuffers(proc, n, buffers) \
  ( ((GdkGLProc_glGenBuffers) (proc)) (n, buffers) )

/* glIsBuffer */
typedef GLboolean (APIENTRYP GdkGLProc_glIsBuffer) (GLuint buffer);
GdkGLProc    gdk_gl_get_glIsBuffer (void);
#define      gdk_gl_glIsBuffer(proc, buffer) \
  ( ((GdkGLProc_glIsBuffer) (proc)) (buffer) )

/* glBufferData */
typedef void (APIENTRYP GdkGLProc_glBufferData) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
GdkGLProc    gdk_gl_get_glBufferData (void);
#define      gdk_gl_glBufferData(proc, target, size, data, usage) \
  ( ((GdkGLProc_glBufferData) (proc)) (target, size, data, usage) )

/* glBufferSubData */
typedef void (APIENTRYP GdkGLProc_glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GdkGLProc    gdk_gl_get_glBufferSubData (void);
#define      gdk_gl_glBufferSubData(proc, target, offset, size, data) \
  ( ((GdkGLProc_glBufferSubData) (proc)) (target, offset, size, data) )

/* glGetBufferSubData */
typedef void (APIENTRYP GdkGLProc_glGetBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
GdkGLProc    gdk_gl_get_glGetBufferSubData (void);
#define      gdk_gl_glGetBufferSubData(proc, target, offset, size, data) \
  ( ((GdkGLProc_glGetBufferSubData) (proc)) (target, offset, size, data) )

/* glMapBuffer */
typedef GLvoid* (APIENTRYP GdkGLProc_glMapBuffer) (GLenum target, GLenum access);
GdkGLProc    gdk_gl_get_glMapBuffer (void);
#define      gdk_gl_glMapBuffer(proc, target, access) \
  ( ((GdkGLProc_glMapBuffer) (proc)) (target, access) )

/* glUnmapBuffer */
typedef GLboolean (APIENTRYP GdkGLProc_glUnmapBuffer) (GLenum target);
GdkGLProc    gdk_gl_get_glUnmapBuffer (void);
#define      gdk_gl_glUnmapBuffer(proc, target) \
  ( ((GdkGLProc_glUnmapBuffer) (proc)) (target) )

/* glGetBufferParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetBufferParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetBufferParameteriv (void);
#define      gdk_gl_glGetBufferParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferParameteriv) (proc)) (target, pname, params) )

/* glGetBufferPointerv */
typedef void (APIENTRYP GdkGLProc_glGetBufferPointerv) (GLenum target, GLenum pname, GLvoid* *params);
GdkGLProc    gdk_gl_get_glGetBufferPointerv (void);
#define      gdk_gl_glGetBufferPointerv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferPointerv) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_1_5 GdkGL_GL_VERSION_1_5;

struct _GdkGL_GL_VERSION_1_5
{
  GdkGLProc_glGenQueries glGenQueries;
  GdkGLProc_glDeleteQueries glDeleteQueries;
  GdkGLProc_glIsQuery glIsQuery;
  GdkGLProc_glBeginQuery glBeginQuery;
  GdkGLProc_glEndQuery glEndQuery;
  GdkGLProc_glGetQueryiv glGetQueryiv;
  GdkGLProc_glGetQueryObjectiv glGetQueryObjectiv;
  GdkGLProc_glGetQueryObjectuiv glGetQueryObjectuiv;
  GdkGLProc_glBindBuffer glBindBuffer;
  GdkGLProc_glDeleteBuffers glDeleteBuffers;
  GdkGLProc_glGenBuffers glGenBuffers;
  GdkGLProc_glIsBuffer glIsBuffer;
  GdkGLProc_glBufferData glBufferData;
  GdkGLProc_glBufferSubData glBufferSubData;
  GdkGLProc_glGetBufferSubData glGetBufferSubData;
  GdkGLProc_glMapBuffer glMapBuffer;
  GdkGLProc_glUnmapBuffer glUnmapBuffer;
  GdkGLProc_glGetBufferParameteriv glGetBufferParameteriv;
  GdkGLProc_glGetBufferPointerv glGetBufferPointerv;
};

GdkGL_GL_VERSION_1_5 *gdk_gl_get_GL_VERSION_1_5 (void);

/*
 * GL_VERSION_2_0
 */

/* glBlendEquationSeparate */
typedef void (APIENTRYP GdkGLProc_glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
GdkGLProc    gdk_gl_get_glBlendEquationSeparate (void);
#define      gdk_gl_glBlendEquationSeparate(proc, modeRGB, modeAlpha) \
  ( ((GdkGLProc_glBlendEquationSeparate) (proc)) (modeRGB, modeAlpha) )

/* glDrawBuffers */
typedef void (APIENTRYP GdkGLProc_glDrawBuffers) (GLsizei n, const GLenum *bufs);
GdkGLProc    gdk_gl_get_glDrawBuffers (void);
#define      gdk_gl_glDrawBuffers(proc, n, bufs) \
  ( ((GdkGLProc_glDrawBuffers) (proc)) (n, bufs) )

/* glStencilOpSeparate */
typedef void (APIENTRYP GdkGLProc_glStencilOpSeparate) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GdkGLProc    gdk_gl_get_glStencilOpSeparate (void);
#define      gdk_gl_glStencilOpSeparate(proc, face, sfail, dpfail, dppass) \
  ( ((GdkGLProc_glStencilOpSeparate) (proc)) (face, sfail, dpfail, dppass) )

/* glStencilFuncSeparate */
typedef void (APIENTRYP GdkGLProc_glStencilFuncSeparate) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
GdkGLProc    gdk_gl_get_glStencilFuncSeparate (void);
#define      gdk_gl_glStencilFuncSeparate(proc, frontfunc, backfunc, ref, mask) \
  ( ((GdkGLProc_glStencilFuncSeparate) (proc)) (frontfunc, backfunc, ref, mask) )

/* glStencilMaskSeparate */
typedef void (APIENTRYP GdkGLProc_glStencilMaskSeparate) (GLenum face, GLuint mask);
GdkGLProc    gdk_gl_get_glStencilMaskSeparate (void);
#define      gdk_gl_glStencilMaskSeparate(proc, face, mask) \
  ( ((GdkGLProc_glStencilMaskSeparate) (proc)) (face, mask) )

/* glAttachShader */
typedef void (APIENTRYP GdkGLProc_glAttachShader) (GLuint program, GLuint shader);
GdkGLProc    gdk_gl_get_glAttachShader (void);
#define      gdk_gl_glAttachShader(proc, program, shader) \
  ( ((GdkGLProc_glAttachShader) (proc)) (program, shader) )

/* glBindAttribLocation */
typedef void (APIENTRYP GdkGLProc_glBindAttribLocation) (GLuint program, GLuint index, const GLchar *name);
GdkGLProc    gdk_gl_get_glBindAttribLocation (void);
#define      gdk_gl_glBindAttribLocation(proc, program, index, name) \
  ( ((GdkGLProc_glBindAttribLocation) (proc)) (program, index, name) )

/* glCompileShader */
typedef void (APIENTRYP GdkGLProc_glCompileShader) (GLuint shader);
GdkGLProc    gdk_gl_get_glCompileShader (void);
#define      gdk_gl_glCompileShader(proc, shader) \
  ( ((GdkGLProc_glCompileShader) (proc)) (shader) )

/* glCreateProgram */
typedef GLuint (APIENTRYP GdkGLProc_glCreateProgram) (void);
GdkGLProc    gdk_gl_get_glCreateProgram (void);
#define      gdk_gl_glCreateProgram(proc) \
  ( ((GdkGLProc_glCreateProgram) (proc)) () )

/* glCreateShader */
typedef GLuint (APIENTRYP GdkGLProc_glCreateShader) (GLenum type);
GdkGLProc    gdk_gl_get_glCreateShader (void);
#define      gdk_gl_glCreateShader(proc, type) \
  ( ((GdkGLProc_glCreateShader) (proc)) (type) )

/* glDeleteProgram */
typedef void (APIENTRYP GdkGLProc_glDeleteProgram) (GLuint program);
GdkGLProc    gdk_gl_get_glDeleteProgram (void);
#define      gdk_gl_glDeleteProgram(proc, program) \
  ( ((GdkGLProc_glDeleteProgram) (proc)) (program) )

/* glDeleteShader */
typedef void (APIENTRYP GdkGLProc_glDeleteShader) (GLuint shader);
GdkGLProc    gdk_gl_get_glDeleteShader (void);
#define      gdk_gl_glDeleteShader(proc, shader) \
  ( ((GdkGLProc_glDeleteShader) (proc)) (shader) )

/* glDetachShader */
typedef void (APIENTRYP GdkGLProc_glDetachShader) (GLuint program, GLuint shader);
GdkGLProc    gdk_gl_get_glDetachShader (void);
#define      gdk_gl_glDetachShader(proc, program, shader) \
  ( ((GdkGLProc_glDetachShader) (proc)) (program, shader) )

/* glDisableVertexAttribArray */
typedef void (APIENTRYP GdkGLProc_glDisableVertexAttribArray) (GLuint index);
GdkGLProc    gdk_gl_get_glDisableVertexAttribArray (void);
#define      gdk_gl_glDisableVertexAttribArray(proc, index) \
  ( ((GdkGLProc_glDisableVertexAttribArray) (proc)) (index) )

/* glEnableVertexAttribArray */
typedef void (APIENTRYP GdkGLProc_glEnableVertexAttribArray) (GLuint index);
GdkGLProc    gdk_gl_get_glEnableVertexAttribArray (void);
#define      gdk_gl_glEnableVertexAttribArray(proc, index) \
  ( ((GdkGLProc_glEnableVertexAttribArray) (proc)) (index) )

/* glGetActiveAttrib */
typedef void (APIENTRYP GdkGLProc_glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GdkGLProc    gdk_gl_get_glGetActiveAttrib (void);
#define      gdk_gl_glGetActiveAttrib(proc, program, index, bufSize, length, size, type, name) \
  ( ((GdkGLProc_glGetActiveAttrib) (proc)) (program, index, bufSize, length, size, type, name) )

/* glGetActiveUniform */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
GdkGLProc    gdk_gl_get_glGetActiveUniform (void);
#define      gdk_gl_glGetActiveUniform(proc, program, index, bufSize, length, size, type, name) \
  ( ((GdkGLProc_glGetActiveUniform) (proc)) (program, index, bufSize, length, size, type, name) )

/* glGetAttachedShaders */
typedef void (APIENTRYP GdkGLProc_glGetAttachedShaders) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
GdkGLProc    gdk_gl_get_glGetAttachedShaders (void);
#define      gdk_gl_glGetAttachedShaders(proc, program, maxCount, count, obj) \
  ( ((GdkGLProc_glGetAttachedShaders) (proc)) (program, maxCount, count, obj) )

/* glGetAttribLocation */
typedef GLint (APIENTRYP GdkGLProc_glGetAttribLocation) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glGetAttribLocation (void);
#define      gdk_gl_glGetAttribLocation(proc, program, name) \
  ( ((GdkGLProc_glGetAttribLocation) (proc)) (program, name) )

/* glGetProgramiv */
typedef void (APIENTRYP GdkGLProc_glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetProgramiv (void);
#define      gdk_gl_glGetProgramiv(proc, program, pname, params) \
  ( ((GdkGLProc_glGetProgramiv) (proc)) (program, pname, params) )

/* glGetProgramInfoLog */
typedef void (APIENTRYP GdkGLProc_glGetProgramInfoLog) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GdkGLProc    gdk_gl_get_glGetProgramInfoLog (void);
#define      gdk_gl_glGetProgramInfoLog(proc, program, bufSize, length, infoLog) \
  ( ((GdkGLProc_glGetProgramInfoLog) (proc)) (program, bufSize, length, infoLog) )

/* glGetShaderiv */
typedef void (APIENTRYP GdkGLProc_glGetShaderiv) (GLuint shader, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetShaderiv (void);
#define      gdk_gl_glGetShaderiv(proc, shader, pname, params) \
  ( ((GdkGLProc_glGetShaderiv) (proc)) (shader, pname, params) )

/* glGetShaderInfoLog */
typedef void (APIENTRYP GdkGLProc_glGetShaderInfoLog) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
GdkGLProc    gdk_gl_get_glGetShaderInfoLog (void);
#define      gdk_gl_glGetShaderInfoLog(proc, shader, bufSize, length, infoLog) \
  ( ((GdkGLProc_glGetShaderInfoLog) (proc)) (shader, bufSize, length, infoLog) )

/* glGetShaderSource */
typedef void (APIENTRYP GdkGLProc_glGetShaderSource) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
GdkGLProc    gdk_gl_get_glGetShaderSource (void);
#define      gdk_gl_glGetShaderSource(proc, shader, bufSize, length, source) \
  ( ((GdkGLProc_glGetShaderSource) (proc)) (shader, bufSize, length, source) )

/* glGetUniformLocation */
typedef GLint (APIENTRYP GdkGLProc_glGetUniformLocation) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glGetUniformLocation (void);
#define      gdk_gl_glGetUniformLocation(proc, program, name) \
  ( ((GdkGLProc_glGetUniformLocation) (proc)) (program, name) )

/* glGetUniformfv */
typedef void (APIENTRYP GdkGLProc_glGetUniformfv) (GLuint program, GLint location, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetUniformfv (void);
#define      gdk_gl_glGetUniformfv(proc, program, location, params) \
  ( ((GdkGLProc_glGetUniformfv) (proc)) (program, location, params) )

/* glGetUniformiv */
typedef void (APIENTRYP GdkGLProc_glGetUniformiv) (GLuint program, GLint location, GLint *params);
GdkGLProc    gdk_gl_get_glGetUniformiv (void);
#define      gdk_gl_glGetUniformiv(proc, program, location, params) \
  ( ((GdkGLProc_glGetUniformiv) (proc)) (program, location, params) )

/* glGetVertexAttribdv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribdv) (GLuint index, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribdv (void);
#define      gdk_gl_glGetVertexAttribdv(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribdv) (proc)) (index, pname, params) )

/* glGetVertexAttribfv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribfv (void);
#define      gdk_gl_glGetVertexAttribfv(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribfv) (proc)) (index, pname, params) )

/* glGetVertexAttribiv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribiv) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribiv (void);
#define      gdk_gl_glGetVertexAttribiv(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribiv) (proc)) (index, pname, params) )

/* glGetVertexAttribPointerv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribPointerv) (GLuint index, GLenum pname, GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glGetVertexAttribPointerv (void);
#define      gdk_gl_glGetVertexAttribPointerv(proc, index, pname, pointer) \
  ( ((GdkGLProc_glGetVertexAttribPointerv) (proc)) (index, pname, pointer) )

/* glIsProgram */
typedef GLboolean (APIENTRYP GdkGLProc_glIsProgram) (GLuint program);
GdkGLProc    gdk_gl_get_glIsProgram (void);
#define      gdk_gl_glIsProgram(proc, program) \
  ( ((GdkGLProc_glIsProgram) (proc)) (program) )

/* glIsShader */
typedef GLboolean (APIENTRYP GdkGLProc_glIsShader) (GLuint shader);
GdkGLProc    gdk_gl_get_glIsShader (void);
#define      gdk_gl_glIsShader(proc, shader) \
  ( ((GdkGLProc_glIsShader) (proc)) (shader) )

/* glLinkProgram */
typedef void (APIENTRYP GdkGLProc_glLinkProgram) (GLuint program);
GdkGLProc    gdk_gl_get_glLinkProgram (void);
#define      gdk_gl_glLinkProgram(proc, program) \
  ( ((GdkGLProc_glLinkProgram) (proc)) (program) )

/* glShaderSource */
typedef void (APIENTRYP GdkGLProc_glShaderSource) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
GdkGLProc    gdk_gl_get_glShaderSource (void);
#define      gdk_gl_glShaderSource(proc, shader, count, string, length) \
  ( ((GdkGLProc_glShaderSource) (proc)) (shader, count, string, length) )

/* glUseProgram */
typedef void (APIENTRYP GdkGLProc_glUseProgram) (GLuint program);
GdkGLProc    gdk_gl_get_glUseProgram (void);
#define      gdk_gl_glUseProgram(proc, program) \
  ( ((GdkGLProc_glUseProgram) (proc)) (program) )

/* glUniform1f */
typedef void (APIENTRYP GdkGLProc_glUniform1f) (GLint location, GLfloat v0);
GdkGLProc    gdk_gl_get_glUniform1f (void);
#define      gdk_gl_glUniform1f(proc, location, v0) \
  ( ((GdkGLProc_glUniform1f) (proc)) (location, v0) )

/* glUniform2f */
typedef void (APIENTRYP GdkGLProc_glUniform2f) (GLint location, GLfloat v0, GLfloat v1);
GdkGLProc    gdk_gl_get_glUniform2f (void);
#define      gdk_gl_glUniform2f(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2f) (proc)) (location, v0, v1) )

/* glUniform3f */
typedef void (APIENTRYP GdkGLProc_glUniform3f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GdkGLProc    gdk_gl_get_glUniform3f (void);
#define      gdk_gl_glUniform3f(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3f) (proc)) (location, v0, v1, v2) )

/* glUniform4f */
typedef void (APIENTRYP GdkGLProc_glUniform4f) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GdkGLProc    gdk_gl_get_glUniform4f (void);
#define      gdk_gl_glUniform4f(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4f) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1i */
typedef void (APIENTRYP GdkGLProc_glUniform1i) (GLint location, GLint v0);
GdkGLProc    gdk_gl_get_glUniform1i (void);
#define      gdk_gl_glUniform1i(proc, location, v0) \
  ( ((GdkGLProc_glUniform1i) (proc)) (location, v0) )

/* glUniform2i */
typedef void (APIENTRYP GdkGLProc_glUniform2i) (GLint location, GLint v0, GLint v1);
GdkGLProc    gdk_gl_get_glUniform2i (void);
#define      gdk_gl_glUniform2i(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2i) (proc)) (location, v0, v1) )

/* glUniform3i */
typedef void (APIENTRYP GdkGLProc_glUniform3i) (GLint location, GLint v0, GLint v1, GLint v2);
GdkGLProc    gdk_gl_get_glUniform3i (void);
#define      gdk_gl_glUniform3i(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3i) (proc)) (location, v0, v1, v2) )

/* glUniform4i */
typedef void (APIENTRYP GdkGLProc_glUniform4i) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GdkGLProc    gdk_gl_get_glUniform4i (void);
#define      gdk_gl_glUniform4i(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4i) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1fv */
typedef void (APIENTRYP GdkGLProc_glUniform1fv) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform1fv (void);
#define      gdk_gl_glUniform1fv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1fv) (proc)) (location, count, value) )

/* glUniform2fv */
typedef void (APIENTRYP GdkGLProc_glUniform2fv) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform2fv (void);
#define      gdk_gl_glUniform2fv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2fv) (proc)) (location, count, value) )

/* glUniform3fv */
typedef void (APIENTRYP GdkGLProc_glUniform3fv) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform3fv (void);
#define      gdk_gl_glUniform3fv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3fv) (proc)) (location, count, value) )

/* glUniform4fv */
typedef void (APIENTRYP GdkGLProc_glUniform4fv) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform4fv (void);
#define      gdk_gl_glUniform4fv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4fv) (proc)) (location, count, value) )

/* glUniform1iv */
typedef void (APIENTRYP GdkGLProc_glUniform1iv) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform1iv (void);
#define      gdk_gl_glUniform1iv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1iv) (proc)) (location, count, value) )

/* glUniform2iv */
typedef void (APIENTRYP GdkGLProc_glUniform2iv) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform2iv (void);
#define      gdk_gl_glUniform2iv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2iv) (proc)) (location, count, value) )

/* glUniform3iv */
typedef void (APIENTRYP GdkGLProc_glUniform3iv) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform3iv (void);
#define      gdk_gl_glUniform3iv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3iv) (proc)) (location, count, value) )

/* glUniform4iv */
typedef void (APIENTRYP GdkGLProc_glUniform4iv) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform4iv (void);
#define      gdk_gl_glUniform4iv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4iv) (proc)) (location, count, value) )

/* glUniformMatrix2fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix2fv (void);
#define      gdk_gl_glUniformMatrix2fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix2fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix3fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix3fv (void);
#define      gdk_gl_glUniformMatrix3fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix3fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix4fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix4fv (void);
#define      gdk_gl_glUniformMatrix4fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix4fv) (proc)) (location, count, transpose, value) )

/* glValidateProgram */
typedef void (APIENTRYP GdkGLProc_glValidateProgram) (GLuint program);
GdkGLProc    gdk_gl_get_glValidateProgram (void);
#define      gdk_gl_glValidateProgram(proc, program) \
  ( ((GdkGLProc_glValidateProgram) (proc)) (program) )

/* glVertexAttrib1d */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1d) (GLuint index, GLdouble x);
GdkGLProc    gdk_gl_get_glVertexAttrib1d (void);
#define      gdk_gl_glVertexAttrib1d(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1d) (proc)) (index, x) )

/* glVertexAttrib1dv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1dv) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1dv (void);
#define      gdk_gl_glVertexAttrib1dv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1dv) (proc)) (index, v) )

/* glVertexAttrib1f */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1f) (GLuint index, GLfloat x);
GdkGLProc    gdk_gl_get_glVertexAttrib1f (void);
#define      gdk_gl_glVertexAttrib1f(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1f) (proc)) (index, x) )

/* glVertexAttrib1fv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1fv) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1fv (void);
#define      gdk_gl_glVertexAttrib1fv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1fv) (proc)) (index, v) )

/* glVertexAttrib1s */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1s) (GLuint index, GLshort x);
GdkGLProc    gdk_gl_get_glVertexAttrib1s (void);
#define      gdk_gl_glVertexAttrib1s(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1s) (proc)) (index, x) )

/* glVertexAttrib1sv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1sv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1sv (void);
#define      gdk_gl_glVertexAttrib1sv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1sv) (proc)) (index, v) )

/* glVertexAttrib2d */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2d) (GLuint index, GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glVertexAttrib2d (void);
#define      gdk_gl_glVertexAttrib2d(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2d) (proc)) (index, x, y) )

/* glVertexAttrib2dv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2dv) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2dv (void);
#define      gdk_gl_glVertexAttrib2dv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2dv) (proc)) (index, v) )

/* glVertexAttrib2f */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2f) (GLuint index, GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glVertexAttrib2f (void);
#define      gdk_gl_glVertexAttrib2f(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2f) (proc)) (index, x, y) )

/* glVertexAttrib2fv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2fv) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2fv (void);
#define      gdk_gl_glVertexAttrib2fv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2fv) (proc)) (index, v) )

/* glVertexAttrib2s */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2s) (GLuint index, GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glVertexAttrib2s (void);
#define      gdk_gl_glVertexAttrib2s(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2s) (proc)) (index, x, y) )

/* glVertexAttrib2sv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2sv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2sv (void);
#define      gdk_gl_glVertexAttrib2sv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2sv) (proc)) (index, v) )

/* glVertexAttrib3d */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3d) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glVertexAttrib3d (void);
#define      gdk_gl_glVertexAttrib3d(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3d) (proc)) (index, x, y, z) )

/* glVertexAttrib3dv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3dv) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3dv (void);
#define      gdk_gl_glVertexAttrib3dv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3dv) (proc)) (index, v) )

/* glVertexAttrib3f */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3f) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glVertexAttrib3f (void);
#define      gdk_gl_glVertexAttrib3f(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3f) (proc)) (index, x, y, z) )

/* glVertexAttrib3fv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3fv) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3fv (void);
#define      gdk_gl_glVertexAttrib3fv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3fv) (proc)) (index, v) )

/* glVertexAttrib3s */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3s) (GLuint index, GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glVertexAttrib3s (void);
#define      gdk_gl_glVertexAttrib3s(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3s) (proc)) (index, x, y, z) )

/* glVertexAttrib3sv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3sv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3sv (void);
#define      gdk_gl_glVertexAttrib3sv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3sv) (proc)) (index, v) )

/* glVertexAttrib4Nbv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nbv) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nbv (void);
#define      gdk_gl_glVertexAttrib4Nbv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Nbv) (proc)) (index, v) )

/* glVertexAttrib4Niv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Niv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Niv (void);
#define      gdk_gl_glVertexAttrib4Niv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Niv) (proc)) (index, v) )

/* glVertexAttrib4Nsv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nsv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nsv (void);
#define      gdk_gl_glVertexAttrib4Nsv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Nsv) (proc)) (index, v) )

/* glVertexAttrib4Nub */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nub) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nub (void);
#define      gdk_gl_glVertexAttrib4Nub(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4Nub) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4Nubv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nubv) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nubv (void);
#define      gdk_gl_glVertexAttrib4Nubv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Nubv) (proc)) (index, v) )

/* glVertexAttrib4Nuiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nuiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nuiv (void);
#define      gdk_gl_glVertexAttrib4Nuiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Nuiv) (proc)) (index, v) )

/* glVertexAttrib4Nusv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4Nusv) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4Nusv (void);
#define      gdk_gl_glVertexAttrib4Nusv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4Nusv) (proc)) (index, v) )

/* glVertexAttrib4bv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4bv) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4bv (void);
#define      gdk_gl_glVertexAttrib4bv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4bv) (proc)) (index, v) )

/* glVertexAttrib4d */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4d) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glVertexAttrib4d (void);
#define      gdk_gl_glVertexAttrib4d(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4d) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4dv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4dv) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4dv (void);
#define      gdk_gl_glVertexAttrib4dv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4dv) (proc)) (index, v) )

/* glVertexAttrib4f */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4f) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glVertexAttrib4f (void);
#define      gdk_gl_glVertexAttrib4f(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4f) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4fv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4fv) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4fv (void);
#define      gdk_gl_glVertexAttrib4fv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4fv) (proc)) (index, v) )

/* glVertexAttrib4iv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4iv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4iv (void);
#define      gdk_gl_glVertexAttrib4iv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4iv) (proc)) (index, v) )

/* glVertexAttrib4s */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4s) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
GdkGLProc    gdk_gl_get_glVertexAttrib4s (void);
#define      gdk_gl_glVertexAttrib4s(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4s) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4sv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4sv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4sv (void);
#define      gdk_gl_glVertexAttrib4sv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4sv) (proc)) (index, v) )

/* glVertexAttrib4ubv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4ubv) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4ubv (void);
#define      gdk_gl_glVertexAttrib4ubv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4ubv) (proc)) (index, v) )

/* glVertexAttrib4uiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4uiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4uiv (void);
#define      gdk_gl_glVertexAttrib4uiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4uiv) (proc)) (index, v) )

/* glVertexAttrib4usv */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4usv) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4usv (void);
#define      gdk_gl_glVertexAttrib4usv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4usv) (proc)) (index, v) )

/* glVertexAttribPointer */
typedef void (APIENTRYP GdkGLProc_glVertexAttribPointer) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexAttribPointer (void);
#define      gdk_gl_glVertexAttribPointer(proc, index, size, type, normalized, stride, pointer) \
  ( ((GdkGLProc_glVertexAttribPointer) (proc)) (index, size, type, normalized, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_2_0 GdkGL_GL_VERSION_2_0;

struct _GdkGL_GL_VERSION_2_0
{
  GdkGLProc_glBlendEquationSeparate glBlendEquationSeparate;
  GdkGLProc_glDrawBuffers glDrawBuffers;
  GdkGLProc_glStencilOpSeparate glStencilOpSeparate;
  GdkGLProc_glStencilFuncSeparate glStencilFuncSeparate;
  GdkGLProc_glStencilMaskSeparate glStencilMaskSeparate;
  GdkGLProc_glAttachShader glAttachShader;
  GdkGLProc_glBindAttribLocation glBindAttribLocation;
  GdkGLProc_glCompileShader glCompileShader;
  GdkGLProc_glCreateProgram glCreateProgram;
  GdkGLProc_glCreateShader glCreateShader;
  GdkGLProc_glDeleteProgram glDeleteProgram;
  GdkGLProc_glDeleteShader glDeleteShader;
  GdkGLProc_glDetachShader glDetachShader;
  GdkGLProc_glDisableVertexAttribArray glDisableVertexAttribArray;
  GdkGLProc_glEnableVertexAttribArray glEnableVertexAttribArray;
  GdkGLProc_glGetActiveAttrib glGetActiveAttrib;
  GdkGLProc_glGetActiveUniform glGetActiveUniform;
  GdkGLProc_glGetAttachedShaders glGetAttachedShaders;
  GdkGLProc_glGetAttribLocation glGetAttribLocation;
  GdkGLProc_glGetProgramiv glGetProgramiv;
  GdkGLProc_glGetProgramInfoLog glGetProgramInfoLog;
  GdkGLProc_glGetShaderiv glGetShaderiv;
  GdkGLProc_glGetShaderInfoLog glGetShaderInfoLog;
  GdkGLProc_glGetShaderSource glGetShaderSource;
  GdkGLProc_glGetUniformLocation glGetUniformLocation;
  GdkGLProc_glGetUniformfv glGetUniformfv;
  GdkGLProc_glGetUniformiv glGetUniformiv;
  GdkGLProc_glGetVertexAttribdv glGetVertexAttribdv;
  GdkGLProc_glGetVertexAttribfv glGetVertexAttribfv;
  GdkGLProc_glGetVertexAttribiv glGetVertexAttribiv;
  GdkGLProc_glGetVertexAttribPointerv glGetVertexAttribPointerv;
  GdkGLProc_glIsProgram glIsProgram;
  GdkGLProc_glIsShader glIsShader;
  GdkGLProc_glLinkProgram glLinkProgram;
  GdkGLProc_glShaderSource glShaderSource;
  GdkGLProc_glUseProgram glUseProgram;
  GdkGLProc_glUniform1f glUniform1f;
  GdkGLProc_glUniform2f glUniform2f;
  GdkGLProc_glUniform3f glUniform3f;
  GdkGLProc_glUniform4f glUniform4f;
  GdkGLProc_glUniform1i glUniform1i;
  GdkGLProc_glUniform2i glUniform2i;
  GdkGLProc_glUniform3i glUniform3i;
  GdkGLProc_glUniform4i glUniform4i;
  GdkGLProc_glUniform1fv glUniform1fv;
  GdkGLProc_glUniform2fv glUniform2fv;
  GdkGLProc_glUniform3fv glUniform3fv;
  GdkGLProc_glUniform4fv glUniform4fv;
  GdkGLProc_glUniform1iv glUniform1iv;
  GdkGLProc_glUniform2iv glUniform2iv;
  GdkGLProc_glUniform3iv glUniform3iv;
  GdkGLProc_glUniform4iv glUniform4iv;
  GdkGLProc_glUniformMatrix2fv glUniformMatrix2fv;
  GdkGLProc_glUniformMatrix3fv glUniformMatrix3fv;
  GdkGLProc_glUniformMatrix4fv glUniformMatrix4fv;
  GdkGLProc_glValidateProgram glValidateProgram;
  GdkGLProc_glVertexAttrib1d glVertexAttrib1d;
  GdkGLProc_glVertexAttrib1dv glVertexAttrib1dv;
  GdkGLProc_glVertexAttrib1f glVertexAttrib1f;
  GdkGLProc_glVertexAttrib1fv glVertexAttrib1fv;
  GdkGLProc_glVertexAttrib1s glVertexAttrib1s;
  GdkGLProc_glVertexAttrib1sv glVertexAttrib1sv;
  GdkGLProc_glVertexAttrib2d glVertexAttrib2d;
  GdkGLProc_glVertexAttrib2dv glVertexAttrib2dv;
  GdkGLProc_glVertexAttrib2f glVertexAttrib2f;
  GdkGLProc_glVertexAttrib2fv glVertexAttrib2fv;
  GdkGLProc_glVertexAttrib2s glVertexAttrib2s;
  GdkGLProc_glVertexAttrib2sv glVertexAttrib2sv;
  GdkGLProc_glVertexAttrib3d glVertexAttrib3d;
  GdkGLProc_glVertexAttrib3dv glVertexAttrib3dv;
  GdkGLProc_glVertexAttrib3f glVertexAttrib3f;
  GdkGLProc_glVertexAttrib3fv glVertexAttrib3fv;
  GdkGLProc_glVertexAttrib3s glVertexAttrib3s;
  GdkGLProc_glVertexAttrib3sv glVertexAttrib3sv;
  GdkGLProc_glVertexAttrib4Nbv glVertexAttrib4Nbv;
  GdkGLProc_glVertexAttrib4Niv glVertexAttrib4Niv;
  GdkGLProc_glVertexAttrib4Nsv glVertexAttrib4Nsv;
  GdkGLProc_glVertexAttrib4Nub glVertexAttrib4Nub;
  GdkGLProc_glVertexAttrib4Nubv glVertexAttrib4Nubv;
  GdkGLProc_glVertexAttrib4Nuiv glVertexAttrib4Nuiv;
  GdkGLProc_glVertexAttrib4Nusv glVertexAttrib4Nusv;
  GdkGLProc_glVertexAttrib4bv glVertexAttrib4bv;
  GdkGLProc_glVertexAttrib4d glVertexAttrib4d;
  GdkGLProc_glVertexAttrib4dv glVertexAttrib4dv;
  GdkGLProc_glVertexAttrib4f glVertexAttrib4f;
  GdkGLProc_glVertexAttrib4fv glVertexAttrib4fv;
  GdkGLProc_glVertexAttrib4iv glVertexAttrib4iv;
  GdkGLProc_glVertexAttrib4s glVertexAttrib4s;
  GdkGLProc_glVertexAttrib4sv glVertexAttrib4sv;
  GdkGLProc_glVertexAttrib4ubv glVertexAttrib4ubv;
  GdkGLProc_glVertexAttrib4uiv glVertexAttrib4uiv;
  GdkGLProc_glVertexAttrib4usv glVertexAttrib4usv;
  GdkGLProc_glVertexAttribPointer glVertexAttribPointer;
};

GdkGL_GL_VERSION_2_0 *gdk_gl_get_GL_VERSION_2_0 (void);

/*
 * GL_VERSION_2_1
 */

/* glUniformMatrix2x3fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix2x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix2x3fv (void);
#define      gdk_gl_glUniformMatrix2x3fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix2x3fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix3x2fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix3x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix3x2fv (void);
#define      gdk_gl_glUniformMatrix3x2fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix3x2fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix2x4fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix2x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix2x4fv (void);
#define      gdk_gl_glUniformMatrix2x4fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix2x4fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix4x2fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix4x2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix4x2fv (void);
#define      gdk_gl_glUniformMatrix4x2fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix4x2fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix3x4fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix3x4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix3x4fv (void);
#define      gdk_gl_glUniformMatrix3x4fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix3x4fv) (proc)) (location, count, transpose, value) )

/* glUniformMatrix4x3fv */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix4x3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix4x3fv (void);
#define      gdk_gl_glUniformMatrix4x3fv(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix4x3fv) (proc)) (location, count, transpose, value) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_2_1 GdkGL_GL_VERSION_2_1;

struct _GdkGL_GL_VERSION_2_1
{
  GdkGLProc_glUniformMatrix2x3fv glUniformMatrix2x3fv;
  GdkGLProc_glUniformMatrix3x2fv glUniformMatrix3x2fv;
  GdkGLProc_glUniformMatrix2x4fv glUniformMatrix2x4fv;
  GdkGLProc_glUniformMatrix4x2fv glUniformMatrix4x2fv;
  GdkGLProc_glUniformMatrix3x4fv glUniformMatrix3x4fv;
  GdkGLProc_glUniformMatrix4x3fv glUniformMatrix4x3fv;
};

GdkGL_GL_VERSION_2_1 *gdk_gl_get_GL_VERSION_2_1 (void);

/*
 * GL_VERSION_3_0
 */

/* glColorMaski */
typedef void (APIENTRYP GdkGLProc_glColorMaski) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
GdkGLProc    gdk_gl_get_glColorMaski (void);
#define      gdk_gl_glColorMaski(proc, index, r, g, b, a) \
  ( ((GdkGLProc_glColorMaski) (proc)) (index, r, g, b, a) )

/* glGetBooleani_v */
typedef void (APIENTRYP GdkGLProc_glGetBooleani_v) (GLenum target, GLuint index, GLboolean *data);
GdkGLProc    gdk_gl_get_glGetBooleani_v (void);
#define      gdk_gl_glGetBooleani_v(proc, target, index, data) \
  ( ((GdkGLProc_glGetBooleani_v) (proc)) (target, index, data) )

/* glGetIntegeri_v */
typedef void (APIENTRYP GdkGLProc_glGetIntegeri_v) (GLenum target, GLuint index, GLint *data);
GdkGLProc    gdk_gl_get_glGetIntegeri_v (void);
#define      gdk_gl_glGetIntegeri_v(proc, target, index, data) \
  ( ((GdkGLProc_glGetIntegeri_v) (proc)) (target, index, data) )

/* glEnablei */
typedef void (APIENTRYP GdkGLProc_glEnablei) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glEnablei (void);
#define      gdk_gl_glEnablei(proc, target, index) \
  ( ((GdkGLProc_glEnablei) (proc)) (target, index) )

/* glDisablei */
typedef void (APIENTRYP GdkGLProc_glDisablei) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glDisablei (void);
#define      gdk_gl_glDisablei(proc, target, index) \
  ( ((GdkGLProc_glDisablei) (proc)) (target, index) )

/* glIsEnabledi */
typedef GLboolean (APIENTRYP GdkGLProc_glIsEnabledi) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glIsEnabledi (void);
#define      gdk_gl_glIsEnabledi(proc, target, index) \
  ( ((GdkGLProc_glIsEnabledi) (proc)) (target, index) )

/* glBeginTransformFeedback */
typedef void (APIENTRYP GdkGLProc_glBeginTransformFeedback) (GLenum primitiveMode);
GdkGLProc    gdk_gl_get_glBeginTransformFeedback (void);
#define      gdk_gl_glBeginTransformFeedback(proc, primitiveMode) \
  ( ((GdkGLProc_glBeginTransformFeedback) (proc)) (primitiveMode) )

/* glEndTransformFeedback */
typedef void (APIENTRYP GdkGLProc_glEndTransformFeedback) (void);
GdkGLProc    gdk_gl_get_glEndTransformFeedback (void);
#define      gdk_gl_glEndTransformFeedback(proc) \
  ( ((GdkGLProc_glEndTransformFeedback) (proc)) () )

/* glBindBufferRange */
typedef void (APIENTRYP GdkGLProc_glBindBufferRange) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GdkGLProc    gdk_gl_get_glBindBufferRange (void);
#define      gdk_gl_glBindBufferRange(proc, target, index, buffer, offset, size) \
  ( ((GdkGLProc_glBindBufferRange) (proc)) (target, index, buffer, offset, size) )

/* glBindBufferBase */
typedef void (APIENTRYP GdkGLProc_glBindBufferBase) (GLenum target, GLuint index, GLuint buffer);
GdkGLProc    gdk_gl_get_glBindBufferBase (void);
#define      gdk_gl_glBindBufferBase(proc, target, index, buffer) \
  ( ((GdkGLProc_glBindBufferBase) (proc)) (target, index, buffer) )

/* glTransformFeedbackVaryings */
typedef void (APIENTRYP GdkGLProc_glTransformFeedbackVaryings) (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode);
GdkGLProc    gdk_gl_get_glTransformFeedbackVaryings (void);
#define      gdk_gl_glTransformFeedbackVaryings(proc, program, count, varyings, bufferMode) \
  ( ((GdkGLProc_glTransformFeedbackVaryings) (proc)) (program, count, varyings, bufferMode) )

/* glGetTransformFeedbackVarying */
typedef void (APIENTRYP GdkGLProc_glGetTransformFeedbackVarying) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GdkGLProc    gdk_gl_get_glGetTransformFeedbackVarying (void);
#define      gdk_gl_glGetTransformFeedbackVarying(proc, program, index, bufSize, length, size, type, name) \
  ( ((GdkGLProc_glGetTransformFeedbackVarying) (proc)) (program, index, bufSize, length, size, type, name) )

/* glClampColor */
typedef void (APIENTRYP GdkGLProc_glClampColor) (GLenum target, GLenum clamp);
GdkGLProc    gdk_gl_get_glClampColor (void);
#define      gdk_gl_glClampColor(proc, target, clamp) \
  ( ((GdkGLProc_glClampColor) (proc)) (target, clamp) )

/* glBeginConditionalRender */
typedef void (APIENTRYP GdkGLProc_glBeginConditionalRender) (GLuint id, GLenum mode);
GdkGLProc    gdk_gl_get_glBeginConditionalRender (void);
#define      gdk_gl_glBeginConditionalRender(proc, id, mode) \
  ( ((GdkGLProc_glBeginConditionalRender) (proc)) (id, mode) )

/* glEndConditionalRender */
typedef void (APIENTRYP GdkGLProc_glEndConditionalRender) (void);
GdkGLProc    gdk_gl_get_glEndConditionalRender (void);
#define      gdk_gl_glEndConditionalRender(proc) \
  ( ((GdkGLProc_glEndConditionalRender) (proc)) () )

/* glVertexAttribIPointer */
typedef void (APIENTRYP GdkGLProc_glVertexAttribIPointer) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexAttribIPointer (void);
#define      gdk_gl_glVertexAttribIPointer(proc, index, size, type, stride, pointer) \
  ( ((GdkGLProc_glVertexAttribIPointer) (proc)) (index, size, type, stride, pointer) )

/* glGetVertexAttribIiv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribIiv) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribIiv (void);
#define      gdk_gl_glGetVertexAttribIiv(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribIiv) (proc)) (index, pname, params) )

/* glGetVertexAttribIuiv */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribIuiv) (GLuint index, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribIuiv (void);
#define      gdk_gl_glGetVertexAttribIuiv(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribIuiv) (proc)) (index, pname, params) )

/* glVertexAttribI1i */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1i) (GLuint index, GLint x);
GdkGLProc    gdk_gl_get_glVertexAttribI1i (void);
#define      gdk_gl_glVertexAttribI1i(proc, index, x) \
  ( ((GdkGLProc_glVertexAttribI1i) (proc)) (index, x) )

/* glVertexAttribI2i */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2i) (GLuint index, GLint x, GLint y);
GdkGLProc    gdk_gl_get_glVertexAttribI2i (void);
#define      gdk_gl_glVertexAttribI2i(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttribI2i) (proc)) (index, x, y) )

/* glVertexAttribI3i */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3i) (GLuint index, GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glVertexAttribI3i (void);
#define      gdk_gl_glVertexAttribI3i(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttribI3i) (proc)) (index, x, y, z) )

/* glVertexAttribI4i */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4i) (GLuint index, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glVertexAttribI4i (void);
#define      gdk_gl_glVertexAttribI4i(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttribI4i) (proc)) (index, x, y, z, w) )

/* glVertexAttribI1ui */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1ui) (GLuint index, GLuint x);
GdkGLProc    gdk_gl_get_glVertexAttribI1ui (void);
#define      gdk_gl_glVertexAttribI1ui(proc, index, x) \
  ( ((GdkGLProc_glVertexAttribI1ui) (proc)) (index, x) )

/* glVertexAttribI2ui */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2ui) (GLuint index, GLuint x, GLuint y);
GdkGLProc    gdk_gl_get_glVertexAttribI2ui (void);
#define      gdk_gl_glVertexAttribI2ui(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttribI2ui) (proc)) (index, x, y) )

/* glVertexAttribI3ui */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3ui) (GLuint index, GLuint x, GLuint y, GLuint z);
GdkGLProc    gdk_gl_get_glVertexAttribI3ui (void);
#define      gdk_gl_glVertexAttribI3ui(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttribI3ui) (proc)) (index, x, y, z) )

/* glVertexAttribI4ui */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4ui) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GdkGLProc    gdk_gl_get_glVertexAttribI4ui (void);
#define      gdk_gl_glVertexAttribI4ui(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttribI4ui) (proc)) (index, x, y, z, w) )

/* glVertexAttribI1iv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1iv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI1iv (void);
#define      gdk_gl_glVertexAttribI1iv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI1iv) (proc)) (index, v) )

/* glVertexAttribI2iv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2iv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI2iv (void);
#define      gdk_gl_glVertexAttribI2iv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI2iv) (proc)) (index, v) )

/* glVertexAttribI3iv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3iv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI3iv (void);
#define      gdk_gl_glVertexAttribI3iv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI3iv) (proc)) (index, v) )

/* glVertexAttribI4iv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4iv) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4iv (void);
#define      gdk_gl_glVertexAttribI4iv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4iv) (proc)) (index, v) )

/* glVertexAttribI1uiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1uiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI1uiv (void);
#define      gdk_gl_glVertexAttribI1uiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI1uiv) (proc)) (index, v) )

/* glVertexAttribI2uiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2uiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI2uiv (void);
#define      gdk_gl_glVertexAttribI2uiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI2uiv) (proc)) (index, v) )

/* glVertexAttribI3uiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3uiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI3uiv (void);
#define      gdk_gl_glVertexAttribI3uiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI3uiv) (proc)) (index, v) )

/* glVertexAttribI4uiv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4uiv) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4uiv (void);
#define      gdk_gl_glVertexAttribI4uiv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4uiv) (proc)) (index, v) )

/* glVertexAttribI4bv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4bv) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4bv (void);
#define      gdk_gl_glVertexAttribI4bv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4bv) (proc)) (index, v) )

/* glVertexAttribI4sv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4sv) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4sv (void);
#define      gdk_gl_glVertexAttribI4sv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4sv) (proc)) (index, v) )

/* glVertexAttribI4ubv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4ubv) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4ubv (void);
#define      gdk_gl_glVertexAttribI4ubv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4ubv) (proc)) (index, v) )

/* glVertexAttribI4usv */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4usv) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4usv (void);
#define      gdk_gl_glVertexAttribI4usv(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4usv) (proc)) (index, v) )

/* glGetUniformuiv */
typedef void (APIENTRYP GdkGLProc_glGetUniformuiv) (GLuint program, GLint location, GLuint *params);
GdkGLProc    gdk_gl_get_glGetUniformuiv (void);
#define      gdk_gl_glGetUniformuiv(proc, program, location, params) \
  ( ((GdkGLProc_glGetUniformuiv) (proc)) (program, location, params) )

/* glBindFragDataLocation */
typedef void (APIENTRYP GdkGLProc_glBindFragDataLocation) (GLuint program, GLuint color, const GLchar *name);
GdkGLProc    gdk_gl_get_glBindFragDataLocation (void);
#define      gdk_gl_glBindFragDataLocation(proc, program, color, name) \
  ( ((GdkGLProc_glBindFragDataLocation) (proc)) (program, color, name) )

/* glGetFragDataLocation */
typedef GLint (APIENTRYP GdkGLProc_glGetFragDataLocation) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glGetFragDataLocation (void);
#define      gdk_gl_glGetFragDataLocation(proc, program, name) \
  ( ((GdkGLProc_glGetFragDataLocation) (proc)) (program, name) )

/* glUniform1ui */
typedef void (APIENTRYP GdkGLProc_glUniform1ui) (GLint location, GLuint v0);
GdkGLProc    gdk_gl_get_glUniform1ui (void);
#define      gdk_gl_glUniform1ui(proc, location, v0) \
  ( ((GdkGLProc_glUniform1ui) (proc)) (location, v0) )

/* glUniform2ui */
typedef void (APIENTRYP GdkGLProc_glUniform2ui) (GLint location, GLuint v0, GLuint v1);
GdkGLProc    gdk_gl_get_glUniform2ui (void);
#define      gdk_gl_glUniform2ui(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2ui) (proc)) (location, v0, v1) )

/* glUniform3ui */
typedef void (APIENTRYP GdkGLProc_glUniform3ui) (GLint location, GLuint v0, GLuint v1, GLuint v2);
GdkGLProc    gdk_gl_get_glUniform3ui (void);
#define      gdk_gl_glUniform3ui(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3ui) (proc)) (location, v0, v1, v2) )

/* glUniform4ui */
typedef void (APIENTRYP GdkGLProc_glUniform4ui) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GdkGLProc    gdk_gl_get_glUniform4ui (void);
#define      gdk_gl_glUniform4ui(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4ui) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1uiv */
typedef void (APIENTRYP GdkGLProc_glUniform1uiv) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform1uiv (void);
#define      gdk_gl_glUniform1uiv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1uiv) (proc)) (location, count, value) )

/* glUniform2uiv */
typedef void (APIENTRYP GdkGLProc_glUniform2uiv) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform2uiv (void);
#define      gdk_gl_glUniform2uiv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2uiv) (proc)) (location, count, value) )

/* glUniform3uiv */
typedef void (APIENTRYP GdkGLProc_glUniform3uiv) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform3uiv (void);
#define      gdk_gl_glUniform3uiv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3uiv) (proc)) (location, count, value) )

/* glUniform4uiv */
typedef void (APIENTRYP GdkGLProc_glUniform4uiv) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform4uiv (void);
#define      gdk_gl_glUniform4uiv(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4uiv) (proc)) (location, count, value) )

/* glTexParameterIiv */
typedef void (APIENTRYP GdkGLProc_glTexParameterIiv) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glTexParameterIiv (void);
#define      gdk_gl_glTexParameterIiv(proc, target, pname, params) \
  ( ((GdkGLProc_glTexParameterIiv) (proc)) (target, pname, params) )

/* glTexParameterIuiv */
typedef void (APIENTRYP GdkGLProc_glTexParameterIuiv) (GLenum target, GLenum pname, const GLuint *params);
GdkGLProc    gdk_gl_get_glTexParameterIuiv (void);
#define      gdk_gl_glTexParameterIuiv(proc, target, pname, params) \
  ( ((GdkGLProc_glTexParameterIuiv) (proc)) (target, pname, params) )

/* glGetTexParameterIiv */
typedef void (APIENTRYP GdkGLProc_glGetTexParameterIiv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTexParameterIiv (void);
#define      gdk_gl_glGetTexParameterIiv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetTexParameterIiv) (proc)) (target, pname, params) )

/* glGetTexParameterIuiv */
typedef void (APIENTRYP GdkGLProc_glGetTexParameterIuiv) (GLenum target, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetTexParameterIuiv (void);
#define      gdk_gl_glGetTexParameterIuiv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetTexParameterIuiv) (proc)) (target, pname, params) )

/* glClearBufferiv */
typedef void (APIENTRYP GdkGLProc_glClearBufferiv) (GLenum buffer, GLint drawbuffer, const GLint *value);
GdkGLProc    gdk_gl_get_glClearBufferiv (void);
#define      gdk_gl_glClearBufferiv(proc, buffer, drawbuffer, value) \
  ( ((GdkGLProc_glClearBufferiv) (proc)) (buffer, drawbuffer, value) )

/* glClearBufferuiv */
typedef void (APIENTRYP GdkGLProc_glClearBufferuiv) (GLenum buffer, GLint drawbuffer, const GLuint *value);
GdkGLProc    gdk_gl_get_glClearBufferuiv (void);
#define      gdk_gl_glClearBufferuiv(proc, buffer, drawbuffer, value) \
  ( ((GdkGLProc_glClearBufferuiv) (proc)) (buffer, drawbuffer, value) )

/* glClearBufferfv */
typedef void (APIENTRYP GdkGLProc_glClearBufferfv) (GLenum buffer, GLint drawbuffer, const GLfloat *value);
GdkGLProc    gdk_gl_get_glClearBufferfv (void);
#define      gdk_gl_glClearBufferfv(proc, buffer, drawbuffer, value) \
  ( ((GdkGLProc_glClearBufferfv) (proc)) (buffer, drawbuffer, value) )

/* glClearBufferfi */
typedef void (APIENTRYP GdkGLProc_glClearBufferfi) (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
GdkGLProc    gdk_gl_get_glClearBufferfi (void);
#define      gdk_gl_glClearBufferfi(proc, buffer, drawbuffer, depth, stencil) \
  ( ((GdkGLProc_glClearBufferfi) (proc)) (buffer, drawbuffer, depth, stencil) )

/* glGetStringi */
typedef const GLubyte * (APIENTRYP GdkGLProc_glGetStringi) (GLenum name, GLuint index);
GdkGLProc    gdk_gl_get_glGetStringi (void);
#define      gdk_gl_glGetStringi(proc, name, index) \
  ( ((GdkGLProc_glGetStringi) (proc)) (name, index) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_3_0 GdkGL_GL_VERSION_3_0;

struct _GdkGL_GL_VERSION_3_0
{
  GdkGLProc_glColorMaski glColorMaski;
  GdkGLProc_glGetBooleani_v glGetBooleani_v;
  GdkGLProc_glGetIntegeri_v glGetIntegeri_v;
  GdkGLProc_glEnablei glEnablei;
  GdkGLProc_glDisablei glDisablei;
  GdkGLProc_glIsEnabledi glIsEnabledi;
  GdkGLProc_glBeginTransformFeedback glBeginTransformFeedback;
  GdkGLProc_glEndTransformFeedback glEndTransformFeedback;
  GdkGLProc_glBindBufferRange glBindBufferRange;
  GdkGLProc_glBindBufferBase glBindBufferBase;
  GdkGLProc_glTransformFeedbackVaryings glTransformFeedbackVaryings;
  GdkGLProc_glGetTransformFeedbackVarying glGetTransformFeedbackVarying;
  GdkGLProc_glClampColor glClampColor;
  GdkGLProc_glBeginConditionalRender glBeginConditionalRender;
  GdkGLProc_glEndConditionalRender glEndConditionalRender;
  GdkGLProc_glVertexAttribIPointer glVertexAttribIPointer;
  GdkGLProc_glGetVertexAttribIiv glGetVertexAttribIiv;
  GdkGLProc_glGetVertexAttribIuiv glGetVertexAttribIuiv;
  GdkGLProc_glVertexAttribI1i glVertexAttribI1i;
  GdkGLProc_glVertexAttribI2i glVertexAttribI2i;
  GdkGLProc_glVertexAttribI3i glVertexAttribI3i;
  GdkGLProc_glVertexAttribI4i glVertexAttribI4i;
  GdkGLProc_glVertexAttribI1ui glVertexAttribI1ui;
  GdkGLProc_glVertexAttribI2ui glVertexAttribI2ui;
  GdkGLProc_glVertexAttribI3ui glVertexAttribI3ui;
  GdkGLProc_glVertexAttribI4ui glVertexAttribI4ui;
  GdkGLProc_glVertexAttribI1iv glVertexAttribI1iv;
  GdkGLProc_glVertexAttribI2iv glVertexAttribI2iv;
  GdkGLProc_glVertexAttribI3iv glVertexAttribI3iv;
  GdkGLProc_glVertexAttribI4iv glVertexAttribI4iv;
  GdkGLProc_glVertexAttribI1uiv glVertexAttribI1uiv;
  GdkGLProc_glVertexAttribI2uiv glVertexAttribI2uiv;
  GdkGLProc_glVertexAttribI3uiv glVertexAttribI3uiv;
  GdkGLProc_glVertexAttribI4uiv glVertexAttribI4uiv;
  GdkGLProc_glVertexAttribI4bv glVertexAttribI4bv;
  GdkGLProc_glVertexAttribI4sv glVertexAttribI4sv;
  GdkGLProc_glVertexAttribI4ubv glVertexAttribI4ubv;
  GdkGLProc_glVertexAttribI4usv glVertexAttribI4usv;
  GdkGLProc_glGetUniformuiv glGetUniformuiv;
  GdkGLProc_glBindFragDataLocation glBindFragDataLocation;
  GdkGLProc_glGetFragDataLocation glGetFragDataLocation;
  GdkGLProc_glUniform1ui glUniform1ui;
  GdkGLProc_glUniform2ui glUniform2ui;
  GdkGLProc_glUniform3ui glUniform3ui;
  GdkGLProc_glUniform4ui glUniform4ui;
  GdkGLProc_glUniform1uiv glUniform1uiv;
  GdkGLProc_glUniform2uiv glUniform2uiv;
  GdkGLProc_glUniform3uiv glUniform3uiv;
  GdkGLProc_glUniform4uiv glUniform4uiv;
  GdkGLProc_glTexParameterIiv glTexParameterIiv;
  GdkGLProc_glTexParameterIuiv glTexParameterIuiv;
  GdkGLProc_glGetTexParameterIiv glGetTexParameterIiv;
  GdkGLProc_glGetTexParameterIuiv glGetTexParameterIuiv;
  GdkGLProc_glClearBufferiv glClearBufferiv;
  GdkGLProc_glClearBufferuiv glClearBufferuiv;
  GdkGLProc_glClearBufferfv glClearBufferfv;
  GdkGLProc_glClearBufferfi glClearBufferfi;
  GdkGLProc_glGetStringi glGetStringi;
};

GdkGL_GL_VERSION_3_0 *gdk_gl_get_GL_VERSION_3_0 (void);

/*
 * GL_VERSION_3_1
 */

/* glDrawArraysInstanced */
typedef void (APIENTRYP GdkGLProc_glDrawArraysInstanced) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawArraysInstanced (void);
#define      gdk_gl_glDrawArraysInstanced(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glDrawArraysInstanced) (proc)) (mode, first, count, primcount) )

/* glDrawElementsInstanced */
typedef void (APIENTRYP GdkGLProc_glDrawElementsInstanced) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawElementsInstanced (void);
#define      gdk_gl_glDrawElementsInstanced(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glDrawElementsInstanced) (proc)) (mode, count, type, indices, primcount) )

/* glTexBuffer */
typedef void (APIENTRYP GdkGLProc_glTexBuffer) (GLenum target, GLenum internalformat, GLuint buffer);
GdkGLProc    gdk_gl_get_glTexBuffer (void);
#define      gdk_gl_glTexBuffer(proc, target, internalformat, buffer) \
  ( ((GdkGLProc_glTexBuffer) (proc)) (target, internalformat, buffer) )

/* glPrimitiveRestartIndex */
typedef void (APIENTRYP GdkGLProc_glPrimitiveRestartIndex) (GLuint index);
GdkGLProc    gdk_gl_get_glPrimitiveRestartIndex (void);
#define      gdk_gl_glPrimitiveRestartIndex(proc, index) \
  ( ((GdkGLProc_glPrimitiveRestartIndex) (proc)) (index) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_3_1 GdkGL_GL_VERSION_3_1;

struct _GdkGL_GL_VERSION_3_1
{
  GdkGLProc_glDrawArraysInstanced glDrawArraysInstanced;
  GdkGLProc_glDrawElementsInstanced glDrawElementsInstanced;
  GdkGLProc_glTexBuffer glTexBuffer;
  GdkGLProc_glPrimitiveRestartIndex glPrimitiveRestartIndex;
};

GdkGL_GL_VERSION_3_1 *gdk_gl_get_GL_VERSION_3_1 (void);

/*
 * GL_VERSION_3_2
 */

/* glGetInteger64i_v */
typedef void (APIENTRYP GdkGLProc_glGetInteger64i_v) (GLenum target, GLuint index, GLint64 *data);
GdkGLProc    gdk_gl_get_glGetInteger64i_v (void);
#define      gdk_gl_glGetInteger64i_v(proc, target, index, data) \
  ( ((GdkGLProc_glGetInteger64i_v) (proc)) (target, index, data) )

/* glGetBufferParameteri64v */
typedef void (APIENTRYP GdkGLProc_glGetBufferParameteri64v) (GLenum target, GLenum pname, GLint64 *params);
GdkGLProc    gdk_gl_get_glGetBufferParameteri64v (void);
#define      gdk_gl_glGetBufferParameteri64v(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferParameteri64v) (proc)) (target, pname, params) )

/* glProgramParameteri */
typedef void (APIENTRYP GdkGLProc_glProgramParameteri) (GLuint program, GLenum pname, GLint value);
GdkGLProc    gdk_gl_get_glProgramParameteri (void);
#define      gdk_gl_glProgramParameteri(proc, program, pname, value) \
  ( ((GdkGLProc_glProgramParameteri) (proc)) (program, pname, value) )

/* glFramebufferTexture */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture) (GLenum target, GLenum attachment, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTexture (void);
#define      gdk_gl_glFramebufferTexture(proc, target, attachment, texture, level) \
  ( ((GdkGLProc_glFramebufferTexture) (proc)) (target, attachment, texture, level) )

/* glFramebufferTextureFace */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureFace) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
GdkGLProc    gdk_gl_get_glFramebufferTextureFace (void);
#define      gdk_gl_glFramebufferTextureFace(proc, target, attachment, texture, level, face) \
  ( ((GdkGLProc_glFramebufferTextureFace) (proc)) (target, attachment, texture, level, face) )

/* proc struct */

typedef struct _GdkGL_GL_VERSION_3_2 GdkGL_GL_VERSION_3_2;

struct _GdkGL_GL_VERSION_3_2
{
  GdkGLProc_glGetInteger64i_v glGetInteger64i_v;
  GdkGLProc_glGetBufferParameteri64v glGetBufferParameteri64v;
  GdkGLProc_glProgramParameteri glProgramParameteri;
  GdkGLProc_glFramebufferTexture glFramebufferTexture;
  GdkGLProc_glFramebufferTextureFace glFramebufferTextureFace;
};

GdkGL_GL_VERSION_3_2 *gdk_gl_get_GL_VERSION_3_2 (void);

/*
 * GL_ARB_multitexture
 */

/* glActiveTextureARB */
typedef void (APIENTRYP GdkGLProc_glActiveTextureARB) (GLenum texture);
GdkGLProc    gdk_gl_get_glActiveTextureARB (void);
#define      gdk_gl_glActiveTextureARB(proc, texture) \
  ( ((GdkGLProc_glActiveTextureARB) (proc)) (texture) )

/* glClientActiveTextureARB */
typedef void (APIENTRYP GdkGLProc_glClientActiveTextureARB) (GLenum texture);
GdkGLProc    gdk_gl_get_glClientActiveTextureARB (void);
#define      gdk_gl_glClientActiveTextureARB(proc, texture) \
  ( ((GdkGLProc_glClientActiveTextureARB) (proc)) (texture) )

/* glMultiTexCoord1dARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1dARB) (GLenum target, GLdouble s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dARB (void);
#define      gdk_gl_glMultiTexCoord1dARB(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1dARB) (proc)) (target, s) )

/* glMultiTexCoord1dvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1dvARB) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dvARB (void);
#define      gdk_gl_glMultiTexCoord1dvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1dvARB) (proc)) (target, v) )

/* glMultiTexCoord1fARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1fARB) (GLenum target, GLfloat s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fARB (void);
#define      gdk_gl_glMultiTexCoord1fARB(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1fARB) (proc)) (target, s) )

/* glMultiTexCoord1fvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1fvARB) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fvARB (void);
#define      gdk_gl_glMultiTexCoord1fvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1fvARB) (proc)) (target, v) )

/* glMultiTexCoord1iARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1iARB) (GLenum target, GLint s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1iARB (void);
#define      gdk_gl_glMultiTexCoord1iARB(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1iARB) (proc)) (target, s) )

/* glMultiTexCoord1ivARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1ivARB) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1ivARB (void);
#define      gdk_gl_glMultiTexCoord1ivARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1ivARB) (proc)) (target, v) )

/* glMultiTexCoord1sARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1sARB) (GLenum target, GLshort s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1sARB (void);
#define      gdk_gl_glMultiTexCoord1sARB(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1sARB) (proc)) (target, s) )

/* glMultiTexCoord1svARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1svARB) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1svARB (void);
#define      gdk_gl_glMultiTexCoord1svARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1svARB) (proc)) (target, v) )

/* glMultiTexCoord2dARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2dARB) (GLenum target, GLdouble s, GLdouble t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dARB (void);
#define      gdk_gl_glMultiTexCoord2dARB(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2dARB) (proc)) (target, s, t) )

/* glMultiTexCoord2dvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2dvARB) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dvARB (void);
#define      gdk_gl_glMultiTexCoord2dvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2dvARB) (proc)) (target, v) )

/* glMultiTexCoord2fARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fARB (void);
#define      gdk_gl_glMultiTexCoord2fARB(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2fARB) (proc)) (target, s, t) )

/* glMultiTexCoord2fvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2fvARB) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fvARB (void);
#define      gdk_gl_glMultiTexCoord2fvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2fvARB) (proc)) (target, v) )

/* glMultiTexCoord2iARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2iARB) (GLenum target, GLint s, GLint t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2iARB (void);
#define      gdk_gl_glMultiTexCoord2iARB(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2iARB) (proc)) (target, s, t) )

/* glMultiTexCoord2ivARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2ivARB) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2ivARB (void);
#define      gdk_gl_glMultiTexCoord2ivARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2ivARB) (proc)) (target, v) )

/* glMultiTexCoord2sARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2sARB) (GLenum target, GLshort s, GLshort t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2sARB (void);
#define      gdk_gl_glMultiTexCoord2sARB(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2sARB) (proc)) (target, s, t) )

/* glMultiTexCoord2svARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2svARB) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2svARB (void);
#define      gdk_gl_glMultiTexCoord2svARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2svARB) (proc)) (target, v) )

/* glMultiTexCoord3dARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3dARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dARB (void);
#define      gdk_gl_glMultiTexCoord3dARB(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3dARB) (proc)) (target, s, t, r) )

/* glMultiTexCoord3dvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3dvARB) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dvARB (void);
#define      gdk_gl_glMultiTexCoord3dvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3dvARB) (proc)) (target, v) )

/* glMultiTexCoord3fARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3fARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fARB (void);
#define      gdk_gl_glMultiTexCoord3fARB(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3fARB) (proc)) (target, s, t, r) )

/* glMultiTexCoord3fvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3fvARB) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fvARB (void);
#define      gdk_gl_glMultiTexCoord3fvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3fvARB) (proc)) (target, v) )

/* glMultiTexCoord3iARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3iARB) (GLenum target, GLint s, GLint t, GLint r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3iARB (void);
#define      gdk_gl_glMultiTexCoord3iARB(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3iARB) (proc)) (target, s, t, r) )

/* glMultiTexCoord3ivARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3ivARB) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3ivARB (void);
#define      gdk_gl_glMultiTexCoord3ivARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3ivARB) (proc)) (target, v) )

/* glMultiTexCoord3sARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3sARB) (GLenum target, GLshort s, GLshort t, GLshort r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3sARB (void);
#define      gdk_gl_glMultiTexCoord3sARB(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3sARB) (proc)) (target, s, t, r) )

/* glMultiTexCoord3svARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3svARB) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3svARB (void);
#define      gdk_gl_glMultiTexCoord3svARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3svARB) (proc)) (target, v) )

/* glMultiTexCoord4dARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4dARB) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dARB (void);
#define      gdk_gl_glMultiTexCoord4dARB(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4dARB) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4dvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4dvARB) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dvARB (void);
#define      gdk_gl_glMultiTexCoord4dvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4dvARB) (proc)) (target, v) )

/* glMultiTexCoord4fARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4fARB) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fARB (void);
#define      gdk_gl_glMultiTexCoord4fARB(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4fARB) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4fvARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4fvARB) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fvARB (void);
#define      gdk_gl_glMultiTexCoord4fvARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4fvARB) (proc)) (target, v) )

/* glMultiTexCoord4iARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4iARB) (GLenum target, GLint s, GLint t, GLint r, GLint q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4iARB (void);
#define      gdk_gl_glMultiTexCoord4iARB(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4iARB) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4ivARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4ivARB) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4ivARB (void);
#define      gdk_gl_glMultiTexCoord4ivARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4ivARB) (proc)) (target, v) )

/* glMultiTexCoord4sARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4sARB) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4sARB (void);
#define      gdk_gl_glMultiTexCoord4sARB(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4sARB) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4svARB */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4svARB) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4svARB (void);
#define      gdk_gl_glMultiTexCoord4svARB(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4svARB) (proc)) (target, v) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_multitexture GdkGL_GL_ARB_multitexture;

struct _GdkGL_GL_ARB_multitexture
{
  GdkGLProc_glActiveTextureARB glActiveTextureARB;
  GdkGLProc_glClientActiveTextureARB glClientActiveTextureARB;
  GdkGLProc_glMultiTexCoord1dARB glMultiTexCoord1dARB;
  GdkGLProc_glMultiTexCoord1dvARB glMultiTexCoord1dvARB;
  GdkGLProc_glMultiTexCoord1fARB glMultiTexCoord1fARB;
  GdkGLProc_glMultiTexCoord1fvARB glMultiTexCoord1fvARB;
  GdkGLProc_glMultiTexCoord1iARB glMultiTexCoord1iARB;
  GdkGLProc_glMultiTexCoord1ivARB glMultiTexCoord1ivARB;
  GdkGLProc_glMultiTexCoord1sARB glMultiTexCoord1sARB;
  GdkGLProc_glMultiTexCoord1svARB glMultiTexCoord1svARB;
  GdkGLProc_glMultiTexCoord2dARB glMultiTexCoord2dARB;
  GdkGLProc_glMultiTexCoord2dvARB glMultiTexCoord2dvARB;
  GdkGLProc_glMultiTexCoord2fARB glMultiTexCoord2fARB;
  GdkGLProc_glMultiTexCoord2fvARB glMultiTexCoord2fvARB;
  GdkGLProc_glMultiTexCoord2iARB glMultiTexCoord2iARB;
  GdkGLProc_glMultiTexCoord2ivARB glMultiTexCoord2ivARB;
  GdkGLProc_glMultiTexCoord2sARB glMultiTexCoord2sARB;
  GdkGLProc_glMultiTexCoord2svARB glMultiTexCoord2svARB;
  GdkGLProc_glMultiTexCoord3dARB glMultiTexCoord3dARB;
  GdkGLProc_glMultiTexCoord3dvARB glMultiTexCoord3dvARB;
  GdkGLProc_glMultiTexCoord3fARB glMultiTexCoord3fARB;
  GdkGLProc_glMultiTexCoord3fvARB glMultiTexCoord3fvARB;
  GdkGLProc_glMultiTexCoord3iARB glMultiTexCoord3iARB;
  GdkGLProc_glMultiTexCoord3ivARB glMultiTexCoord3ivARB;
  GdkGLProc_glMultiTexCoord3sARB glMultiTexCoord3sARB;
  GdkGLProc_glMultiTexCoord3svARB glMultiTexCoord3svARB;
  GdkGLProc_glMultiTexCoord4dARB glMultiTexCoord4dARB;
  GdkGLProc_glMultiTexCoord4dvARB glMultiTexCoord4dvARB;
  GdkGLProc_glMultiTexCoord4fARB glMultiTexCoord4fARB;
  GdkGLProc_glMultiTexCoord4fvARB glMultiTexCoord4fvARB;
  GdkGLProc_glMultiTexCoord4iARB glMultiTexCoord4iARB;
  GdkGLProc_glMultiTexCoord4ivARB glMultiTexCoord4ivARB;
  GdkGLProc_glMultiTexCoord4sARB glMultiTexCoord4sARB;
  GdkGLProc_glMultiTexCoord4svARB glMultiTexCoord4svARB;
};

GdkGL_GL_ARB_multitexture *gdk_gl_get_GL_ARB_multitexture (void);

/*
 * GL_ARB_transpose_matrix
 */

/* glLoadTransposeMatrixfARB */
typedef void (APIENTRYP GdkGLProc_glLoadTransposeMatrixfARB) (const GLfloat *m);
GdkGLProc    gdk_gl_get_glLoadTransposeMatrixfARB (void);
#define      gdk_gl_glLoadTransposeMatrixfARB(proc, m) \
  ( ((GdkGLProc_glLoadTransposeMatrixfARB) (proc)) (m) )

/* glLoadTransposeMatrixdARB */
typedef void (APIENTRYP GdkGLProc_glLoadTransposeMatrixdARB) (const GLdouble *m);
GdkGLProc    gdk_gl_get_glLoadTransposeMatrixdARB (void);
#define      gdk_gl_glLoadTransposeMatrixdARB(proc, m) \
  ( ((GdkGLProc_glLoadTransposeMatrixdARB) (proc)) (m) )

/* glMultTransposeMatrixfARB */
typedef void (APIENTRYP GdkGLProc_glMultTransposeMatrixfARB) (const GLfloat *m);
GdkGLProc    gdk_gl_get_glMultTransposeMatrixfARB (void);
#define      gdk_gl_glMultTransposeMatrixfARB(proc, m) \
  ( ((GdkGLProc_glMultTransposeMatrixfARB) (proc)) (m) )

/* glMultTransposeMatrixdARB */
typedef void (APIENTRYP GdkGLProc_glMultTransposeMatrixdARB) (const GLdouble *m);
GdkGLProc    gdk_gl_get_glMultTransposeMatrixdARB (void);
#define      gdk_gl_glMultTransposeMatrixdARB(proc, m) \
  ( ((GdkGLProc_glMultTransposeMatrixdARB) (proc)) (m) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_transpose_matrix GdkGL_GL_ARB_transpose_matrix;

struct _GdkGL_GL_ARB_transpose_matrix
{
  GdkGLProc_glLoadTransposeMatrixfARB glLoadTransposeMatrixfARB;
  GdkGLProc_glLoadTransposeMatrixdARB glLoadTransposeMatrixdARB;
  GdkGLProc_glMultTransposeMatrixfARB glMultTransposeMatrixfARB;
  GdkGLProc_glMultTransposeMatrixdARB glMultTransposeMatrixdARB;
};

GdkGL_GL_ARB_transpose_matrix *gdk_gl_get_GL_ARB_transpose_matrix (void);

/*
 * GL_ARB_multisample
 */

/* glSampleCoverageARB */
typedef void (APIENTRYP GdkGLProc_glSampleCoverageARB) (GLclampf value, GLboolean invert);
GdkGLProc    gdk_gl_get_glSampleCoverageARB (void);
#define      gdk_gl_glSampleCoverageARB(proc, value, invert) \
  ( ((GdkGLProc_glSampleCoverageARB) (proc)) (value, invert) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_multisample GdkGL_GL_ARB_multisample;

struct _GdkGL_GL_ARB_multisample
{
  GdkGLProc_glSampleCoverageARB glSampleCoverageARB;
};

GdkGL_GL_ARB_multisample *gdk_gl_get_GL_ARB_multisample (void);

/*
 * GL_ARB_texture_compression
 */

/* glCompressedTexImage3DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage3DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage3DARB (void);
#define      gdk_gl_glCompressedTexImage3DARB(proc, target, level, internalformat, width, height, depth, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage3DARB) (proc)) (target, level, internalformat, width, height, depth, border, imageSize, data) )

/* glCompressedTexImage2DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage2DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage2DARB (void);
#define      gdk_gl_glCompressedTexImage2DARB(proc, target, level, internalformat, width, height, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage2DARB) (proc)) (target, level, internalformat, width, height, border, imageSize, data) )

/* glCompressedTexImage1DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexImage1DARB) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexImage1DARB (void);
#define      gdk_gl_glCompressedTexImage1DARB(proc, target, level, internalformat, width, border, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexImage1DARB) (proc)) (target, level, internalformat, width, border, imageSize, data) )

/* glCompressedTexSubImage3DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage3DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage3DARB (void);
#define      gdk_gl_glCompressedTexSubImage3DARB(proc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage3DARB) (proc)) (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data) )

/* glCompressedTexSubImage2DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage2DARB) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage2DARB (void);
#define      gdk_gl_glCompressedTexSubImage2DARB(proc, target, level, xoffset, yoffset, width, height, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage2DARB) (proc)) (target, level, xoffset, yoffset, width, height, format, imageSize, data) )

/* glCompressedTexSubImage1DARB */
typedef void (APIENTRYP GdkGLProc_glCompressedTexSubImage1DARB) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
GdkGLProc    gdk_gl_get_glCompressedTexSubImage1DARB (void);
#define      gdk_gl_glCompressedTexSubImage1DARB(proc, target, level, xoffset, width, format, imageSize, data) \
  ( ((GdkGLProc_glCompressedTexSubImage1DARB) (proc)) (target, level, xoffset, width, format, imageSize, data) )

/* glGetCompressedTexImageARB */
typedef void (APIENTRYP GdkGLProc_glGetCompressedTexImageARB) (GLenum target, GLint level, GLvoid *img);
GdkGLProc    gdk_gl_get_glGetCompressedTexImageARB (void);
#define      gdk_gl_glGetCompressedTexImageARB(proc, target, level, img) \
  ( ((GdkGLProc_glGetCompressedTexImageARB) (proc)) (target, level, img) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_texture_compression GdkGL_GL_ARB_texture_compression;

struct _GdkGL_GL_ARB_texture_compression
{
  GdkGLProc_glCompressedTexImage3DARB glCompressedTexImage3DARB;
  GdkGLProc_glCompressedTexImage2DARB glCompressedTexImage2DARB;
  GdkGLProc_glCompressedTexImage1DARB glCompressedTexImage1DARB;
  GdkGLProc_glCompressedTexSubImage3DARB glCompressedTexSubImage3DARB;
  GdkGLProc_glCompressedTexSubImage2DARB glCompressedTexSubImage2DARB;
  GdkGLProc_glCompressedTexSubImage1DARB glCompressedTexSubImage1DARB;
  GdkGLProc_glGetCompressedTexImageARB glGetCompressedTexImageARB;
};

GdkGL_GL_ARB_texture_compression *gdk_gl_get_GL_ARB_texture_compression (void);

/*
 * GL_ARB_point_parameters
 */

/* glPointParameterfARB */
typedef void (APIENTRYP GdkGLProc_glPointParameterfARB) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPointParameterfARB (void);
#define      gdk_gl_glPointParameterfARB(proc, pname, param) \
  ( ((GdkGLProc_glPointParameterfARB) (proc)) (pname, param) )

/* glPointParameterfvARB */
typedef void (APIENTRYP GdkGLProc_glPointParameterfvARB) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPointParameterfvARB (void);
#define      gdk_gl_glPointParameterfvARB(proc, pname, params) \
  ( ((GdkGLProc_glPointParameterfvARB) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_point_parameters GdkGL_GL_ARB_point_parameters;

struct _GdkGL_GL_ARB_point_parameters
{
  GdkGLProc_glPointParameterfARB glPointParameterfARB;
  GdkGLProc_glPointParameterfvARB glPointParameterfvARB;
};

GdkGL_GL_ARB_point_parameters *gdk_gl_get_GL_ARB_point_parameters (void);

/*
 * GL_ARB_vertex_blend
 */

/* glWeightbvARB */
typedef void (APIENTRYP GdkGLProc_glWeightbvARB) (GLint size, const GLbyte *weights);
GdkGLProc    gdk_gl_get_glWeightbvARB (void);
#define      gdk_gl_glWeightbvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightbvARB) (proc)) (size, weights) )

/* glWeightsvARB */
typedef void (APIENTRYP GdkGLProc_glWeightsvARB) (GLint size, const GLshort *weights);
GdkGLProc    gdk_gl_get_glWeightsvARB (void);
#define      gdk_gl_glWeightsvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightsvARB) (proc)) (size, weights) )

/* glWeightivARB */
typedef void (APIENTRYP GdkGLProc_glWeightivARB) (GLint size, const GLint *weights);
GdkGLProc    gdk_gl_get_glWeightivARB (void);
#define      gdk_gl_glWeightivARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightivARB) (proc)) (size, weights) )

/* glWeightfvARB */
typedef void (APIENTRYP GdkGLProc_glWeightfvARB) (GLint size, const GLfloat *weights);
GdkGLProc    gdk_gl_get_glWeightfvARB (void);
#define      gdk_gl_glWeightfvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightfvARB) (proc)) (size, weights) )

/* glWeightdvARB */
typedef void (APIENTRYP GdkGLProc_glWeightdvARB) (GLint size, const GLdouble *weights);
GdkGLProc    gdk_gl_get_glWeightdvARB (void);
#define      gdk_gl_glWeightdvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightdvARB) (proc)) (size, weights) )

/* glWeightubvARB */
typedef void (APIENTRYP GdkGLProc_glWeightubvARB) (GLint size, const GLubyte *weights);
GdkGLProc    gdk_gl_get_glWeightubvARB (void);
#define      gdk_gl_glWeightubvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightubvARB) (proc)) (size, weights) )

/* glWeightusvARB */
typedef void (APIENTRYP GdkGLProc_glWeightusvARB) (GLint size, const GLushort *weights);
GdkGLProc    gdk_gl_get_glWeightusvARB (void);
#define      gdk_gl_glWeightusvARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightusvARB) (proc)) (size, weights) )

/* glWeightuivARB */
typedef void (APIENTRYP GdkGLProc_glWeightuivARB) (GLint size, const GLuint *weights);
GdkGLProc    gdk_gl_get_glWeightuivARB (void);
#define      gdk_gl_glWeightuivARB(proc, size, weights) \
  ( ((GdkGLProc_glWeightuivARB) (proc)) (size, weights) )

/* glWeightPointerARB */
typedef void (APIENTRYP GdkGLProc_glWeightPointerARB) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glWeightPointerARB (void);
#define      gdk_gl_glWeightPointerARB(proc, size, type, stride, pointer) \
  ( ((GdkGLProc_glWeightPointerARB) (proc)) (size, type, stride, pointer) )

/* glVertexBlendARB */
typedef void (APIENTRYP GdkGLProc_glVertexBlendARB) (GLint count);
GdkGLProc    gdk_gl_get_glVertexBlendARB (void);
#define      gdk_gl_glVertexBlendARB(proc, count) \
  ( ((GdkGLProc_glVertexBlendARB) (proc)) (count) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_vertex_blend GdkGL_GL_ARB_vertex_blend;

struct _GdkGL_GL_ARB_vertex_blend
{
  GdkGLProc_glWeightbvARB glWeightbvARB;
  GdkGLProc_glWeightsvARB glWeightsvARB;
  GdkGLProc_glWeightivARB glWeightivARB;
  GdkGLProc_glWeightfvARB glWeightfvARB;
  GdkGLProc_glWeightdvARB glWeightdvARB;
  GdkGLProc_glWeightubvARB glWeightubvARB;
  GdkGLProc_glWeightusvARB glWeightusvARB;
  GdkGLProc_glWeightuivARB glWeightuivARB;
  GdkGLProc_glWeightPointerARB glWeightPointerARB;
  GdkGLProc_glVertexBlendARB glVertexBlendARB;
};

GdkGL_GL_ARB_vertex_blend *gdk_gl_get_GL_ARB_vertex_blend (void);

/*
 * GL_ARB_matrix_palette
 */

/* glCurrentPaletteMatrixARB */
typedef void (APIENTRYP GdkGLProc_glCurrentPaletteMatrixARB) (GLint index);
GdkGLProc    gdk_gl_get_glCurrentPaletteMatrixARB (void);
#define      gdk_gl_glCurrentPaletteMatrixARB(proc, index) \
  ( ((GdkGLProc_glCurrentPaletteMatrixARB) (proc)) (index) )

/* glMatrixIndexubvARB */
typedef void (APIENTRYP GdkGLProc_glMatrixIndexubvARB) (GLint size, const GLubyte *indices);
GdkGLProc    gdk_gl_get_glMatrixIndexubvARB (void);
#define      gdk_gl_glMatrixIndexubvARB(proc, size, indices) \
  ( ((GdkGLProc_glMatrixIndexubvARB) (proc)) (size, indices) )

/* glMatrixIndexusvARB */
typedef void (APIENTRYP GdkGLProc_glMatrixIndexusvARB) (GLint size, const GLushort *indices);
GdkGLProc    gdk_gl_get_glMatrixIndexusvARB (void);
#define      gdk_gl_glMatrixIndexusvARB(proc, size, indices) \
  ( ((GdkGLProc_glMatrixIndexusvARB) (proc)) (size, indices) )

/* glMatrixIndexuivARB */
typedef void (APIENTRYP GdkGLProc_glMatrixIndexuivARB) (GLint size, const GLuint *indices);
GdkGLProc    gdk_gl_get_glMatrixIndexuivARB (void);
#define      gdk_gl_glMatrixIndexuivARB(proc, size, indices) \
  ( ((GdkGLProc_glMatrixIndexuivARB) (proc)) (size, indices) )

/* glMatrixIndexPointerARB */
typedef void (APIENTRYP GdkGLProc_glMatrixIndexPointerARB) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glMatrixIndexPointerARB (void);
#define      gdk_gl_glMatrixIndexPointerARB(proc, size, type, stride, pointer) \
  ( ((GdkGLProc_glMatrixIndexPointerARB) (proc)) (size, type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_matrix_palette GdkGL_GL_ARB_matrix_palette;

struct _GdkGL_GL_ARB_matrix_palette
{
  GdkGLProc_glCurrentPaletteMatrixARB glCurrentPaletteMatrixARB;
  GdkGLProc_glMatrixIndexubvARB glMatrixIndexubvARB;
  GdkGLProc_glMatrixIndexusvARB glMatrixIndexusvARB;
  GdkGLProc_glMatrixIndexuivARB glMatrixIndexuivARB;
  GdkGLProc_glMatrixIndexPointerARB glMatrixIndexPointerARB;
};

GdkGL_GL_ARB_matrix_palette *gdk_gl_get_GL_ARB_matrix_palette (void);

/*
 * GL_ARB_window_pos
 */

/* glWindowPos2dARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2dARB) (GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glWindowPos2dARB (void);
#define      gdk_gl_glWindowPos2dARB(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2dARB) (proc)) (x, y) )

/* glWindowPos2dvARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2dvARB) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos2dvARB (void);
#define      gdk_gl_glWindowPos2dvARB(proc, v) \
  ( ((GdkGLProc_glWindowPos2dvARB) (proc)) (v) )

/* glWindowPos2fARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2fARB) (GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glWindowPos2fARB (void);
#define      gdk_gl_glWindowPos2fARB(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2fARB) (proc)) (x, y) )

/* glWindowPos2fvARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2fvARB) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos2fvARB (void);
#define      gdk_gl_glWindowPos2fvARB(proc, v) \
  ( ((GdkGLProc_glWindowPos2fvARB) (proc)) (v) )

/* glWindowPos2iARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2iARB) (GLint x, GLint y);
GdkGLProc    gdk_gl_get_glWindowPos2iARB (void);
#define      gdk_gl_glWindowPos2iARB(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2iARB) (proc)) (x, y) )

/* glWindowPos2ivARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2ivARB) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos2ivARB (void);
#define      gdk_gl_glWindowPos2ivARB(proc, v) \
  ( ((GdkGLProc_glWindowPos2ivARB) (proc)) (v) )

/* glWindowPos2sARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2sARB) (GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glWindowPos2sARB (void);
#define      gdk_gl_glWindowPos2sARB(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2sARB) (proc)) (x, y) )

/* glWindowPos2svARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos2svARB) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos2svARB (void);
#define      gdk_gl_glWindowPos2svARB(proc, v) \
  ( ((GdkGLProc_glWindowPos2svARB) (proc)) (v) )

/* glWindowPos3dARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3dARB) (GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glWindowPos3dARB (void);
#define      gdk_gl_glWindowPos3dARB(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3dARB) (proc)) (x, y, z) )

/* glWindowPos3dvARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3dvARB) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos3dvARB (void);
#define      gdk_gl_glWindowPos3dvARB(proc, v) \
  ( ((GdkGLProc_glWindowPos3dvARB) (proc)) (v) )

/* glWindowPos3fARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3fARB) (GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glWindowPos3fARB (void);
#define      gdk_gl_glWindowPos3fARB(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3fARB) (proc)) (x, y, z) )

/* glWindowPos3fvARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3fvARB) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos3fvARB (void);
#define      gdk_gl_glWindowPos3fvARB(proc, v) \
  ( ((GdkGLProc_glWindowPos3fvARB) (proc)) (v) )

/* glWindowPos3iARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3iARB) (GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glWindowPos3iARB (void);
#define      gdk_gl_glWindowPos3iARB(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3iARB) (proc)) (x, y, z) )

/* glWindowPos3ivARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3ivARB) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos3ivARB (void);
#define      gdk_gl_glWindowPos3ivARB(proc, v) \
  ( ((GdkGLProc_glWindowPos3ivARB) (proc)) (v) )

/* glWindowPos3sARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3sARB) (GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glWindowPos3sARB (void);
#define      gdk_gl_glWindowPos3sARB(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3sARB) (proc)) (x, y, z) )

/* glWindowPos3svARB */
typedef void (APIENTRYP GdkGLProc_glWindowPos3svARB) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos3svARB (void);
#define      gdk_gl_glWindowPos3svARB(proc, v) \
  ( ((GdkGLProc_glWindowPos3svARB) (proc)) (v) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_window_pos GdkGL_GL_ARB_window_pos;

struct _GdkGL_GL_ARB_window_pos
{
  GdkGLProc_glWindowPos2dARB glWindowPos2dARB;
  GdkGLProc_glWindowPos2dvARB glWindowPos2dvARB;
  GdkGLProc_glWindowPos2fARB glWindowPos2fARB;
  GdkGLProc_glWindowPos2fvARB glWindowPos2fvARB;
  GdkGLProc_glWindowPos2iARB glWindowPos2iARB;
  GdkGLProc_glWindowPos2ivARB glWindowPos2ivARB;
  GdkGLProc_glWindowPos2sARB glWindowPos2sARB;
  GdkGLProc_glWindowPos2svARB glWindowPos2svARB;
  GdkGLProc_glWindowPos3dARB glWindowPos3dARB;
  GdkGLProc_glWindowPos3dvARB glWindowPos3dvARB;
  GdkGLProc_glWindowPos3fARB glWindowPos3fARB;
  GdkGLProc_glWindowPos3fvARB glWindowPos3fvARB;
  GdkGLProc_glWindowPos3iARB glWindowPos3iARB;
  GdkGLProc_glWindowPos3ivARB glWindowPos3ivARB;
  GdkGLProc_glWindowPos3sARB glWindowPos3sARB;
  GdkGLProc_glWindowPos3svARB glWindowPos3svARB;
};

GdkGL_GL_ARB_window_pos *gdk_gl_get_GL_ARB_window_pos (void);

/*
 * GL_ARB_vertex_program
 */

/* glVertexAttrib1dARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1dARB) (GLuint index, GLdouble x);
GdkGLProc    gdk_gl_get_glVertexAttrib1dARB (void);
#define      gdk_gl_glVertexAttrib1dARB(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1dARB) (proc)) (index, x) )

/* glVertexAttrib1dvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1dvARB) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1dvARB (void);
#define      gdk_gl_glVertexAttrib1dvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1dvARB) (proc)) (index, v) )

/* glVertexAttrib1fARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1fARB) (GLuint index, GLfloat x);
GdkGLProc    gdk_gl_get_glVertexAttrib1fARB (void);
#define      gdk_gl_glVertexAttrib1fARB(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1fARB) (proc)) (index, x) )

/* glVertexAttrib1fvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1fvARB) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1fvARB (void);
#define      gdk_gl_glVertexAttrib1fvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1fvARB) (proc)) (index, v) )

/* glVertexAttrib1sARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1sARB) (GLuint index, GLshort x);
GdkGLProc    gdk_gl_get_glVertexAttrib1sARB (void);
#define      gdk_gl_glVertexAttrib1sARB(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1sARB) (proc)) (index, x) )

/* glVertexAttrib1svARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1svARB) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1svARB (void);
#define      gdk_gl_glVertexAttrib1svARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1svARB) (proc)) (index, v) )

/* glVertexAttrib2dARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2dARB) (GLuint index, GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glVertexAttrib2dARB (void);
#define      gdk_gl_glVertexAttrib2dARB(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2dARB) (proc)) (index, x, y) )

/* glVertexAttrib2dvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2dvARB) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2dvARB (void);
#define      gdk_gl_glVertexAttrib2dvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2dvARB) (proc)) (index, v) )

/* glVertexAttrib2fARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2fARB) (GLuint index, GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glVertexAttrib2fARB (void);
#define      gdk_gl_glVertexAttrib2fARB(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2fARB) (proc)) (index, x, y) )

/* glVertexAttrib2fvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2fvARB) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2fvARB (void);
#define      gdk_gl_glVertexAttrib2fvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2fvARB) (proc)) (index, v) )

/* glVertexAttrib2sARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2sARB) (GLuint index, GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glVertexAttrib2sARB (void);
#define      gdk_gl_glVertexAttrib2sARB(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2sARB) (proc)) (index, x, y) )

/* glVertexAttrib2svARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2svARB) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2svARB (void);
#define      gdk_gl_glVertexAttrib2svARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2svARB) (proc)) (index, v) )

/* glVertexAttrib3dARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3dARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glVertexAttrib3dARB (void);
#define      gdk_gl_glVertexAttrib3dARB(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3dARB) (proc)) (index, x, y, z) )

/* glVertexAttrib3dvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3dvARB) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3dvARB (void);
#define      gdk_gl_glVertexAttrib3dvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3dvARB) (proc)) (index, v) )

/* glVertexAttrib3fARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3fARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glVertexAttrib3fARB (void);
#define      gdk_gl_glVertexAttrib3fARB(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3fARB) (proc)) (index, x, y, z) )

/* glVertexAttrib3fvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3fvARB) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3fvARB (void);
#define      gdk_gl_glVertexAttrib3fvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3fvARB) (proc)) (index, v) )

/* glVertexAttrib3sARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3sARB) (GLuint index, GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glVertexAttrib3sARB (void);
#define      gdk_gl_glVertexAttrib3sARB(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3sARB) (proc)) (index, x, y, z) )

/* glVertexAttrib3svARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3svARB) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3svARB (void);
#define      gdk_gl_glVertexAttrib3svARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3svARB) (proc)) (index, v) )

/* glVertexAttrib4NbvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NbvARB) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NbvARB (void);
#define      gdk_gl_glVertexAttrib4NbvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NbvARB) (proc)) (index, v) )

/* glVertexAttrib4NivARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NivARB) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NivARB (void);
#define      gdk_gl_glVertexAttrib4NivARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NivARB) (proc)) (index, v) )

/* glVertexAttrib4NsvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NsvARB) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NsvARB (void);
#define      gdk_gl_glVertexAttrib4NsvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NsvARB) (proc)) (index, v) )

/* glVertexAttrib4NubARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NubARB) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GdkGLProc    gdk_gl_get_glVertexAttrib4NubARB (void);
#define      gdk_gl_glVertexAttrib4NubARB(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4NubARB) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4NubvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NubvARB) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NubvARB (void);
#define      gdk_gl_glVertexAttrib4NubvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NubvARB) (proc)) (index, v) )

/* glVertexAttrib4NuivARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NuivARB) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NuivARB (void);
#define      gdk_gl_glVertexAttrib4NuivARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NuivARB) (proc)) (index, v) )

/* glVertexAttrib4NusvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4NusvARB) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4NusvARB (void);
#define      gdk_gl_glVertexAttrib4NusvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4NusvARB) (proc)) (index, v) )

/* glVertexAttrib4bvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4bvARB) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4bvARB (void);
#define      gdk_gl_glVertexAttrib4bvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4bvARB) (proc)) (index, v) )

/* glVertexAttrib4dARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4dARB) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glVertexAttrib4dARB (void);
#define      gdk_gl_glVertexAttrib4dARB(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4dARB) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4dvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4dvARB) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4dvARB (void);
#define      gdk_gl_glVertexAttrib4dvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4dvARB) (proc)) (index, v) )

/* glVertexAttrib4fARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4fARB) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glVertexAttrib4fARB (void);
#define      gdk_gl_glVertexAttrib4fARB(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4fARB) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4fvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4fvARB) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4fvARB (void);
#define      gdk_gl_glVertexAttrib4fvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4fvARB) (proc)) (index, v) )

/* glVertexAttrib4ivARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4ivARB) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4ivARB (void);
#define      gdk_gl_glVertexAttrib4ivARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4ivARB) (proc)) (index, v) )

/* glVertexAttrib4sARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4sARB) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
GdkGLProc    gdk_gl_get_glVertexAttrib4sARB (void);
#define      gdk_gl_glVertexAttrib4sARB(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4sARB) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4svARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4svARB) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4svARB (void);
#define      gdk_gl_glVertexAttrib4svARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4svARB) (proc)) (index, v) )

/* glVertexAttrib4ubvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4ubvARB) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4ubvARB (void);
#define      gdk_gl_glVertexAttrib4ubvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4ubvARB) (proc)) (index, v) )

/* glVertexAttrib4uivARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4uivARB) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4uivARB (void);
#define      gdk_gl_glVertexAttrib4uivARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4uivARB) (proc)) (index, v) )

/* glVertexAttrib4usvARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4usvARB) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4usvARB (void);
#define      gdk_gl_glVertexAttrib4usvARB(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4usvARB) (proc)) (index, v) )

/* glVertexAttribPointerARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttribPointerARB) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexAttribPointerARB (void);
#define      gdk_gl_glVertexAttribPointerARB(proc, index, size, type, normalized, stride, pointer) \
  ( ((GdkGLProc_glVertexAttribPointerARB) (proc)) (index, size, type, normalized, stride, pointer) )

/* glEnableVertexAttribArrayARB */
typedef void (APIENTRYP GdkGLProc_glEnableVertexAttribArrayARB) (GLuint index);
GdkGLProc    gdk_gl_get_glEnableVertexAttribArrayARB (void);
#define      gdk_gl_glEnableVertexAttribArrayARB(proc, index) \
  ( ((GdkGLProc_glEnableVertexAttribArrayARB) (proc)) (index) )

/* glDisableVertexAttribArrayARB */
typedef void (APIENTRYP GdkGLProc_glDisableVertexAttribArrayARB) (GLuint index);
GdkGLProc    gdk_gl_get_glDisableVertexAttribArrayARB (void);
#define      gdk_gl_glDisableVertexAttribArrayARB(proc, index) \
  ( ((GdkGLProc_glDisableVertexAttribArrayARB) (proc)) (index) )

/* glProgramStringARB */
typedef void (APIENTRYP GdkGLProc_glProgramStringARB) (GLenum target, GLenum format, GLsizei len, const GLvoid *string);
GdkGLProc    gdk_gl_get_glProgramStringARB (void);
#define      gdk_gl_glProgramStringARB(proc, target, format, len, string) \
  ( ((GdkGLProc_glProgramStringARB) (proc)) (target, format, len, string) )

/* glBindProgramARB */
typedef void (APIENTRYP GdkGLProc_glBindProgramARB) (GLenum target, GLuint program);
GdkGLProc    gdk_gl_get_glBindProgramARB (void);
#define      gdk_gl_glBindProgramARB(proc, target, program) \
  ( ((GdkGLProc_glBindProgramARB) (proc)) (target, program) )

/* glDeleteProgramsARB */
typedef void (APIENTRYP GdkGLProc_glDeleteProgramsARB) (GLsizei n, const GLuint *programs);
GdkGLProc    gdk_gl_get_glDeleteProgramsARB (void);
#define      gdk_gl_glDeleteProgramsARB(proc, n, programs) \
  ( ((GdkGLProc_glDeleteProgramsARB) (proc)) (n, programs) )

/* glGenProgramsARB */
typedef void (APIENTRYP GdkGLProc_glGenProgramsARB) (GLsizei n, GLuint *programs);
GdkGLProc    gdk_gl_get_glGenProgramsARB (void);
#define      gdk_gl_glGenProgramsARB(proc, n, programs) \
  ( ((GdkGLProc_glGenProgramsARB) (proc)) (n, programs) )

/* glProgramEnvParameter4dARB */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glProgramEnvParameter4dARB (void);
#define      gdk_gl_glProgramEnvParameter4dARB(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramEnvParameter4dARB) (proc)) (target, index, x, y, z, w) )

/* glProgramEnvParameter4dvARB */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
GdkGLProc    gdk_gl_get_glProgramEnvParameter4dvARB (void);
#define      gdk_gl_glProgramEnvParameter4dvARB(proc, target, index, params) \
  ( ((GdkGLProc_glProgramEnvParameter4dvARB) (proc)) (target, index, params) )

/* glProgramEnvParameter4fARB */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glProgramEnvParameter4fARB (void);
#define      gdk_gl_glProgramEnvParameter4fARB(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramEnvParameter4fARB) (proc)) (target, index, x, y, z, w) )

/* glProgramEnvParameter4fvARB */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
GdkGLProc    gdk_gl_get_glProgramEnvParameter4fvARB (void);
#define      gdk_gl_glProgramEnvParameter4fvARB(proc, target, index, params) \
  ( ((GdkGLProc_glProgramEnvParameter4fvARB) (proc)) (target, index, params) )

/* glProgramLocalParameter4dARB */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameter4dARB) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glProgramLocalParameter4dARB (void);
#define      gdk_gl_glProgramLocalParameter4dARB(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramLocalParameter4dARB) (proc)) (target, index, x, y, z, w) )

/* glProgramLocalParameter4dvARB */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameter4dvARB) (GLenum target, GLuint index, const GLdouble *params);
GdkGLProc    gdk_gl_get_glProgramLocalParameter4dvARB (void);
#define      gdk_gl_glProgramLocalParameter4dvARB(proc, target, index, params) \
  ( ((GdkGLProc_glProgramLocalParameter4dvARB) (proc)) (target, index, params) )

/* glProgramLocalParameter4fARB */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameter4fARB) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glProgramLocalParameter4fARB (void);
#define      gdk_gl_glProgramLocalParameter4fARB(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramLocalParameter4fARB) (proc)) (target, index, x, y, z, w) )

/* glProgramLocalParameter4fvARB */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameter4fvARB) (GLenum target, GLuint index, const GLfloat *params);
GdkGLProc    gdk_gl_get_glProgramLocalParameter4fvARB (void);
#define      gdk_gl_glProgramLocalParameter4fvARB(proc, target, index, params) \
  ( ((GdkGLProc_glProgramLocalParameter4fvARB) (proc)) (target, index, params) )

/* glGetProgramEnvParameterdvARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramEnvParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetProgramEnvParameterdvARB (void);
#define      gdk_gl_glGetProgramEnvParameterdvARB(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramEnvParameterdvARB) (proc)) (target, index, params) )

/* glGetProgramEnvParameterfvARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramEnvParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetProgramEnvParameterfvARB (void);
#define      gdk_gl_glGetProgramEnvParameterfvARB(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramEnvParameterfvARB) (proc)) (target, index, params) )

/* glGetProgramLocalParameterdvARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramLocalParameterdvARB) (GLenum target, GLuint index, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetProgramLocalParameterdvARB (void);
#define      gdk_gl_glGetProgramLocalParameterdvARB(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramLocalParameterdvARB) (proc)) (target, index, params) )

/* glGetProgramLocalParameterfvARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramLocalParameterfvARB) (GLenum target, GLuint index, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetProgramLocalParameterfvARB (void);
#define      gdk_gl_glGetProgramLocalParameterfvARB(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramLocalParameterfvARB) (proc)) (target, index, params) )

/* glGetProgramivARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramivARB) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetProgramivARB (void);
#define      gdk_gl_glGetProgramivARB(proc, target, pname, params) \
  ( ((GdkGLProc_glGetProgramivARB) (proc)) (target, pname, params) )

/* glGetProgramStringARB */
typedef void (APIENTRYP GdkGLProc_glGetProgramStringARB) (GLenum target, GLenum pname, GLvoid *string);
GdkGLProc    gdk_gl_get_glGetProgramStringARB (void);
#define      gdk_gl_glGetProgramStringARB(proc, target, pname, string) \
  ( ((GdkGLProc_glGetProgramStringARB) (proc)) (target, pname, string) )

/* glGetVertexAttribdvARB */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribdvARB) (GLuint index, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribdvARB (void);
#define      gdk_gl_glGetVertexAttribdvARB(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribdvARB) (proc)) (index, pname, params) )

/* glGetVertexAttribfvARB */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribfvARB) (GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribfvARB (void);
#define      gdk_gl_glGetVertexAttribfvARB(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribfvARB) (proc)) (index, pname, params) )

/* glGetVertexAttribivARB */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribivARB) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribivARB (void);
#define      gdk_gl_glGetVertexAttribivARB(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribivARB) (proc)) (index, pname, params) )

/* glGetVertexAttribPointervARB */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribPointervARB) (GLuint index, GLenum pname, GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glGetVertexAttribPointervARB (void);
#define      gdk_gl_glGetVertexAttribPointervARB(proc, index, pname, pointer) \
  ( ((GdkGLProc_glGetVertexAttribPointervARB) (proc)) (index, pname, pointer) )

/* glIsProgramARB */
typedef GLboolean (APIENTRYP GdkGLProc_glIsProgramARB) (GLuint program);
GdkGLProc    gdk_gl_get_glIsProgramARB (void);
#define      gdk_gl_glIsProgramARB(proc, program) \
  ( ((GdkGLProc_glIsProgramARB) (proc)) (program) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_vertex_program GdkGL_GL_ARB_vertex_program;

struct _GdkGL_GL_ARB_vertex_program
{
  GdkGLProc_glVertexAttrib1dARB glVertexAttrib1dARB;
  GdkGLProc_glVertexAttrib1dvARB glVertexAttrib1dvARB;
  GdkGLProc_glVertexAttrib1fARB glVertexAttrib1fARB;
  GdkGLProc_glVertexAttrib1fvARB glVertexAttrib1fvARB;
  GdkGLProc_glVertexAttrib1sARB glVertexAttrib1sARB;
  GdkGLProc_glVertexAttrib1svARB glVertexAttrib1svARB;
  GdkGLProc_glVertexAttrib2dARB glVertexAttrib2dARB;
  GdkGLProc_glVertexAttrib2dvARB glVertexAttrib2dvARB;
  GdkGLProc_glVertexAttrib2fARB glVertexAttrib2fARB;
  GdkGLProc_glVertexAttrib2fvARB glVertexAttrib2fvARB;
  GdkGLProc_glVertexAttrib2sARB glVertexAttrib2sARB;
  GdkGLProc_glVertexAttrib2svARB glVertexAttrib2svARB;
  GdkGLProc_glVertexAttrib3dARB glVertexAttrib3dARB;
  GdkGLProc_glVertexAttrib3dvARB glVertexAttrib3dvARB;
  GdkGLProc_glVertexAttrib3fARB glVertexAttrib3fARB;
  GdkGLProc_glVertexAttrib3fvARB glVertexAttrib3fvARB;
  GdkGLProc_glVertexAttrib3sARB glVertexAttrib3sARB;
  GdkGLProc_glVertexAttrib3svARB glVertexAttrib3svARB;
  GdkGLProc_glVertexAttrib4NbvARB glVertexAttrib4NbvARB;
  GdkGLProc_glVertexAttrib4NivARB glVertexAttrib4NivARB;
  GdkGLProc_glVertexAttrib4NsvARB glVertexAttrib4NsvARB;
  GdkGLProc_glVertexAttrib4NubARB glVertexAttrib4NubARB;
  GdkGLProc_glVertexAttrib4NubvARB glVertexAttrib4NubvARB;
  GdkGLProc_glVertexAttrib4NuivARB glVertexAttrib4NuivARB;
  GdkGLProc_glVertexAttrib4NusvARB glVertexAttrib4NusvARB;
  GdkGLProc_glVertexAttrib4bvARB glVertexAttrib4bvARB;
  GdkGLProc_glVertexAttrib4dARB glVertexAttrib4dARB;
  GdkGLProc_glVertexAttrib4dvARB glVertexAttrib4dvARB;
  GdkGLProc_glVertexAttrib4fARB glVertexAttrib4fARB;
  GdkGLProc_glVertexAttrib4fvARB glVertexAttrib4fvARB;
  GdkGLProc_glVertexAttrib4ivARB glVertexAttrib4ivARB;
  GdkGLProc_glVertexAttrib4sARB glVertexAttrib4sARB;
  GdkGLProc_glVertexAttrib4svARB glVertexAttrib4svARB;
  GdkGLProc_glVertexAttrib4ubvARB glVertexAttrib4ubvARB;
  GdkGLProc_glVertexAttrib4uivARB glVertexAttrib4uivARB;
  GdkGLProc_glVertexAttrib4usvARB glVertexAttrib4usvARB;
  GdkGLProc_glVertexAttribPointerARB glVertexAttribPointerARB;
  GdkGLProc_glEnableVertexAttribArrayARB glEnableVertexAttribArrayARB;
  GdkGLProc_glDisableVertexAttribArrayARB glDisableVertexAttribArrayARB;
  GdkGLProc_glProgramStringARB glProgramStringARB;
  GdkGLProc_glBindProgramARB glBindProgramARB;
  GdkGLProc_glDeleteProgramsARB glDeleteProgramsARB;
  GdkGLProc_glGenProgramsARB glGenProgramsARB;
  GdkGLProc_glProgramEnvParameter4dARB glProgramEnvParameter4dARB;
  GdkGLProc_glProgramEnvParameter4dvARB glProgramEnvParameter4dvARB;
  GdkGLProc_glProgramEnvParameter4fARB glProgramEnvParameter4fARB;
  GdkGLProc_glProgramEnvParameter4fvARB glProgramEnvParameter4fvARB;
  GdkGLProc_glProgramLocalParameter4dARB glProgramLocalParameter4dARB;
  GdkGLProc_glProgramLocalParameter4dvARB glProgramLocalParameter4dvARB;
  GdkGLProc_glProgramLocalParameter4fARB glProgramLocalParameter4fARB;
  GdkGLProc_glProgramLocalParameter4fvARB glProgramLocalParameter4fvARB;
  GdkGLProc_glGetProgramEnvParameterdvARB glGetProgramEnvParameterdvARB;
  GdkGLProc_glGetProgramEnvParameterfvARB glGetProgramEnvParameterfvARB;
  GdkGLProc_glGetProgramLocalParameterdvARB glGetProgramLocalParameterdvARB;
  GdkGLProc_glGetProgramLocalParameterfvARB glGetProgramLocalParameterfvARB;
  GdkGLProc_glGetProgramivARB glGetProgramivARB;
  GdkGLProc_glGetProgramStringARB glGetProgramStringARB;
  GdkGLProc_glGetVertexAttribdvARB glGetVertexAttribdvARB;
  GdkGLProc_glGetVertexAttribfvARB glGetVertexAttribfvARB;
  GdkGLProc_glGetVertexAttribivARB glGetVertexAttribivARB;
  GdkGLProc_glGetVertexAttribPointervARB glGetVertexAttribPointervARB;
  GdkGLProc_glIsProgramARB glIsProgramARB;
};

GdkGL_GL_ARB_vertex_program *gdk_gl_get_GL_ARB_vertex_program (void);

/*
 * GL_ARB_vertex_buffer_object
 */

/* glBindBufferARB */
typedef void (APIENTRYP GdkGLProc_glBindBufferARB) (GLenum target, GLuint buffer);
GdkGLProc    gdk_gl_get_glBindBufferARB (void);
#define      gdk_gl_glBindBufferARB(proc, target, buffer) \
  ( ((GdkGLProc_glBindBufferARB) (proc)) (target, buffer) )

/* glDeleteBuffersARB */
typedef void (APIENTRYP GdkGLProc_glDeleteBuffersARB) (GLsizei n, const GLuint *buffers);
GdkGLProc    gdk_gl_get_glDeleteBuffersARB (void);
#define      gdk_gl_glDeleteBuffersARB(proc, n, buffers) \
  ( ((GdkGLProc_glDeleteBuffersARB) (proc)) (n, buffers) )

/* glGenBuffersARB */
typedef void (APIENTRYP GdkGLProc_glGenBuffersARB) (GLsizei n, GLuint *buffers);
GdkGLProc    gdk_gl_get_glGenBuffersARB (void);
#define      gdk_gl_glGenBuffersARB(proc, n, buffers) \
  ( ((GdkGLProc_glGenBuffersARB) (proc)) (n, buffers) )

/* glIsBufferARB */
typedef GLboolean (APIENTRYP GdkGLProc_glIsBufferARB) (GLuint buffer);
GdkGLProc    gdk_gl_get_glIsBufferARB (void);
#define      gdk_gl_glIsBufferARB(proc, buffer) \
  ( ((GdkGLProc_glIsBufferARB) (proc)) (buffer) )

/* glBufferDataARB */
typedef void (APIENTRYP GdkGLProc_glBufferDataARB) (GLenum target, GLsizeiptrARB size, const GLvoid *data, GLenum usage);
GdkGLProc    gdk_gl_get_glBufferDataARB (void);
#define      gdk_gl_glBufferDataARB(proc, target, size, data, usage) \
  ( ((GdkGLProc_glBufferDataARB) (proc)) (target, size, data, usage) )

/* glBufferSubDataARB */
typedef void (APIENTRYP GdkGLProc_glBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid *data);
GdkGLProc    gdk_gl_get_glBufferSubDataARB (void);
#define      gdk_gl_glBufferSubDataARB(proc, target, offset, size, data) \
  ( ((GdkGLProc_glBufferSubDataARB) (proc)) (target, offset, size, data) )

/* glGetBufferSubDataARB */
typedef void (APIENTRYP GdkGLProc_glGetBufferSubDataARB) (GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid *data);
GdkGLProc    gdk_gl_get_glGetBufferSubDataARB (void);
#define      gdk_gl_glGetBufferSubDataARB(proc, target, offset, size, data) \
  ( ((GdkGLProc_glGetBufferSubDataARB) (proc)) (target, offset, size, data) )

/* glMapBufferARB */
typedef GLvoid* (APIENTRYP GdkGLProc_glMapBufferARB) (GLenum target, GLenum access);
GdkGLProc    gdk_gl_get_glMapBufferARB (void);
#define      gdk_gl_glMapBufferARB(proc, target, access) \
  ( ((GdkGLProc_glMapBufferARB) (proc)) (target, access) )

/* glUnmapBufferARB */
typedef GLboolean (APIENTRYP GdkGLProc_glUnmapBufferARB) (GLenum target);
GdkGLProc    gdk_gl_get_glUnmapBufferARB (void);
#define      gdk_gl_glUnmapBufferARB(proc, target) \
  ( ((GdkGLProc_glUnmapBufferARB) (proc)) (target) )

/* glGetBufferParameterivARB */
typedef void (APIENTRYP GdkGLProc_glGetBufferParameterivARB) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetBufferParameterivARB (void);
#define      gdk_gl_glGetBufferParameterivARB(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferParameterivARB) (proc)) (target, pname, params) )

/* glGetBufferPointervARB */
typedef void (APIENTRYP GdkGLProc_glGetBufferPointervARB) (GLenum target, GLenum pname, GLvoid* *params);
GdkGLProc    gdk_gl_get_glGetBufferPointervARB (void);
#define      gdk_gl_glGetBufferPointervARB(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferPointervARB) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_vertex_buffer_object GdkGL_GL_ARB_vertex_buffer_object;

struct _GdkGL_GL_ARB_vertex_buffer_object
{
  GdkGLProc_glBindBufferARB glBindBufferARB;
  GdkGLProc_glDeleteBuffersARB glDeleteBuffersARB;
  GdkGLProc_glGenBuffersARB glGenBuffersARB;
  GdkGLProc_glIsBufferARB glIsBufferARB;
  GdkGLProc_glBufferDataARB glBufferDataARB;
  GdkGLProc_glBufferSubDataARB glBufferSubDataARB;
  GdkGLProc_glGetBufferSubDataARB glGetBufferSubDataARB;
  GdkGLProc_glMapBufferARB glMapBufferARB;
  GdkGLProc_glUnmapBufferARB glUnmapBufferARB;
  GdkGLProc_glGetBufferParameterivARB glGetBufferParameterivARB;
  GdkGLProc_glGetBufferPointervARB glGetBufferPointervARB;
};

GdkGL_GL_ARB_vertex_buffer_object *gdk_gl_get_GL_ARB_vertex_buffer_object (void);

/*
 * GL_ARB_occlusion_query
 */

/* glGenQueriesARB */
typedef void (APIENTRYP GdkGLProc_glGenQueriesARB) (GLsizei n, GLuint *ids);
GdkGLProc    gdk_gl_get_glGenQueriesARB (void);
#define      gdk_gl_glGenQueriesARB(proc, n, ids) \
  ( ((GdkGLProc_glGenQueriesARB) (proc)) (n, ids) )

/* glDeleteQueriesARB */
typedef void (APIENTRYP GdkGLProc_glDeleteQueriesARB) (GLsizei n, const GLuint *ids);
GdkGLProc    gdk_gl_get_glDeleteQueriesARB (void);
#define      gdk_gl_glDeleteQueriesARB(proc, n, ids) \
  ( ((GdkGLProc_glDeleteQueriesARB) (proc)) (n, ids) )

/* glIsQueryARB */
typedef GLboolean (APIENTRYP GdkGLProc_glIsQueryARB) (GLuint id);
GdkGLProc    gdk_gl_get_glIsQueryARB (void);
#define      gdk_gl_glIsQueryARB(proc, id) \
  ( ((GdkGLProc_glIsQueryARB) (proc)) (id) )

/* glBeginQueryARB */
typedef void (APIENTRYP GdkGLProc_glBeginQueryARB) (GLenum target, GLuint id);
GdkGLProc    gdk_gl_get_glBeginQueryARB (void);
#define      gdk_gl_glBeginQueryARB(proc, target, id) \
  ( ((GdkGLProc_glBeginQueryARB) (proc)) (target, id) )

/* glEndQueryARB */
typedef void (APIENTRYP GdkGLProc_glEndQueryARB) (GLenum target);
GdkGLProc    gdk_gl_get_glEndQueryARB (void);
#define      gdk_gl_glEndQueryARB(proc, target) \
  ( ((GdkGLProc_glEndQueryARB) (proc)) (target) )

/* glGetQueryivARB */
typedef void (APIENTRYP GdkGLProc_glGetQueryivARB) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetQueryivARB (void);
#define      gdk_gl_glGetQueryivARB(proc, target, pname, params) \
  ( ((GdkGLProc_glGetQueryivARB) (proc)) (target, pname, params) )

/* glGetQueryObjectivARB */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjectivARB) (GLuint id, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetQueryObjectivARB (void);
#define      gdk_gl_glGetQueryObjectivARB(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjectivARB) (proc)) (id, pname, params) )

/* glGetQueryObjectuivARB */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjectuivARB) (GLuint id, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetQueryObjectuivARB (void);
#define      gdk_gl_glGetQueryObjectuivARB(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjectuivARB) (proc)) (id, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_occlusion_query GdkGL_GL_ARB_occlusion_query;

struct _GdkGL_GL_ARB_occlusion_query
{
  GdkGLProc_glGenQueriesARB glGenQueriesARB;
  GdkGLProc_glDeleteQueriesARB glDeleteQueriesARB;
  GdkGLProc_glIsQueryARB glIsQueryARB;
  GdkGLProc_glBeginQueryARB glBeginQueryARB;
  GdkGLProc_glEndQueryARB glEndQueryARB;
  GdkGLProc_glGetQueryivARB glGetQueryivARB;
  GdkGLProc_glGetQueryObjectivARB glGetQueryObjectivARB;
  GdkGLProc_glGetQueryObjectuivARB glGetQueryObjectuivARB;
};

GdkGL_GL_ARB_occlusion_query *gdk_gl_get_GL_ARB_occlusion_query (void);

/*
 * GL_ARB_shader_objects
 */

/* glDeleteObjectARB */
typedef void (APIENTRYP GdkGLProc_glDeleteObjectARB) (GLhandleARB obj);
GdkGLProc    gdk_gl_get_glDeleteObjectARB (void);
#define      gdk_gl_glDeleteObjectARB(proc, obj) \
  ( ((GdkGLProc_glDeleteObjectARB) (proc)) (obj) )

/* glGetHandleARB */
typedef GLhandleARB (APIENTRYP GdkGLProc_glGetHandleARB) (GLenum pname);
GdkGLProc    gdk_gl_get_glGetHandleARB (void);
#define      gdk_gl_glGetHandleARB(proc, pname) \
  ( ((GdkGLProc_glGetHandleARB) (proc)) (pname) )

/* glDetachObjectARB */
typedef void (APIENTRYP GdkGLProc_glDetachObjectARB) (GLhandleARB containerObj, GLhandleARB attachedObj);
GdkGLProc    gdk_gl_get_glDetachObjectARB (void);
#define      gdk_gl_glDetachObjectARB(proc, containerObj, attachedObj) \
  ( ((GdkGLProc_glDetachObjectARB) (proc)) (containerObj, attachedObj) )

/* glCreateShaderObjectARB */
typedef GLhandleARB (APIENTRYP GdkGLProc_glCreateShaderObjectARB) (GLenum shaderType);
GdkGLProc    gdk_gl_get_glCreateShaderObjectARB (void);
#define      gdk_gl_glCreateShaderObjectARB(proc, shaderType) \
  ( ((GdkGLProc_glCreateShaderObjectARB) (proc)) (shaderType) )

/* glShaderSourceARB */
typedef void (APIENTRYP GdkGLProc_glShaderSourceARB) (GLhandleARB shaderObj, GLsizei count, const GLcharARB* *string, const GLint *length);
GdkGLProc    gdk_gl_get_glShaderSourceARB (void);
#define      gdk_gl_glShaderSourceARB(proc, shaderObj, count, string, length) \
  ( ((GdkGLProc_glShaderSourceARB) (proc)) (shaderObj, count, string, length) )

/* glCompileShaderARB */
typedef void (APIENTRYP GdkGLProc_glCompileShaderARB) (GLhandleARB shaderObj);
GdkGLProc    gdk_gl_get_glCompileShaderARB (void);
#define      gdk_gl_glCompileShaderARB(proc, shaderObj) \
  ( ((GdkGLProc_glCompileShaderARB) (proc)) (shaderObj) )

/* glCreateProgramObjectARB */
typedef GLhandleARB (APIENTRYP GdkGLProc_glCreateProgramObjectARB) (void);
GdkGLProc    gdk_gl_get_glCreateProgramObjectARB (void);
#define      gdk_gl_glCreateProgramObjectARB(proc) \
  ( ((GdkGLProc_glCreateProgramObjectARB) (proc)) () )

/* glAttachObjectARB */
typedef void (APIENTRYP GdkGLProc_glAttachObjectARB) (GLhandleARB containerObj, GLhandleARB obj);
GdkGLProc    gdk_gl_get_glAttachObjectARB (void);
#define      gdk_gl_glAttachObjectARB(proc, containerObj, obj) \
  ( ((GdkGLProc_glAttachObjectARB) (proc)) (containerObj, obj) )

/* glLinkProgramARB */
typedef void (APIENTRYP GdkGLProc_glLinkProgramARB) (GLhandleARB programObj);
GdkGLProc    gdk_gl_get_glLinkProgramARB (void);
#define      gdk_gl_glLinkProgramARB(proc, programObj) \
  ( ((GdkGLProc_glLinkProgramARB) (proc)) (programObj) )

/* glUseProgramObjectARB */
typedef void (APIENTRYP GdkGLProc_glUseProgramObjectARB) (GLhandleARB programObj);
GdkGLProc    gdk_gl_get_glUseProgramObjectARB (void);
#define      gdk_gl_glUseProgramObjectARB(proc, programObj) \
  ( ((GdkGLProc_glUseProgramObjectARB) (proc)) (programObj) )

/* glValidateProgramARB */
typedef void (APIENTRYP GdkGLProc_glValidateProgramARB) (GLhandleARB programObj);
GdkGLProc    gdk_gl_get_glValidateProgramARB (void);
#define      gdk_gl_glValidateProgramARB(proc, programObj) \
  ( ((GdkGLProc_glValidateProgramARB) (proc)) (programObj) )

/* glUniform1fARB */
typedef void (APIENTRYP GdkGLProc_glUniform1fARB) (GLint location, GLfloat v0);
GdkGLProc    gdk_gl_get_glUniform1fARB (void);
#define      gdk_gl_glUniform1fARB(proc, location, v0) \
  ( ((GdkGLProc_glUniform1fARB) (proc)) (location, v0) )

/* glUniform2fARB */
typedef void (APIENTRYP GdkGLProc_glUniform2fARB) (GLint location, GLfloat v0, GLfloat v1);
GdkGLProc    gdk_gl_get_glUniform2fARB (void);
#define      gdk_gl_glUniform2fARB(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2fARB) (proc)) (location, v0, v1) )

/* glUniform3fARB */
typedef void (APIENTRYP GdkGLProc_glUniform3fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GdkGLProc    gdk_gl_get_glUniform3fARB (void);
#define      gdk_gl_glUniform3fARB(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3fARB) (proc)) (location, v0, v1, v2) )

/* glUniform4fARB */
typedef void (APIENTRYP GdkGLProc_glUniform4fARB) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GdkGLProc    gdk_gl_get_glUniform4fARB (void);
#define      gdk_gl_glUniform4fARB(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4fARB) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1iARB */
typedef void (APIENTRYP GdkGLProc_glUniform1iARB) (GLint location, GLint v0);
GdkGLProc    gdk_gl_get_glUniform1iARB (void);
#define      gdk_gl_glUniform1iARB(proc, location, v0) \
  ( ((GdkGLProc_glUniform1iARB) (proc)) (location, v0) )

/* glUniform2iARB */
typedef void (APIENTRYP GdkGLProc_glUniform2iARB) (GLint location, GLint v0, GLint v1);
GdkGLProc    gdk_gl_get_glUniform2iARB (void);
#define      gdk_gl_glUniform2iARB(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2iARB) (proc)) (location, v0, v1) )

/* glUniform3iARB */
typedef void (APIENTRYP GdkGLProc_glUniform3iARB) (GLint location, GLint v0, GLint v1, GLint v2);
GdkGLProc    gdk_gl_get_glUniform3iARB (void);
#define      gdk_gl_glUniform3iARB(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3iARB) (proc)) (location, v0, v1, v2) )

/* glUniform4iARB */
typedef void (APIENTRYP GdkGLProc_glUniform4iARB) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GdkGLProc    gdk_gl_get_glUniform4iARB (void);
#define      gdk_gl_glUniform4iARB(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4iARB) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1fvARB */
typedef void (APIENTRYP GdkGLProc_glUniform1fvARB) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform1fvARB (void);
#define      gdk_gl_glUniform1fvARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1fvARB) (proc)) (location, count, value) )

/* glUniform2fvARB */
typedef void (APIENTRYP GdkGLProc_glUniform2fvARB) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform2fvARB (void);
#define      gdk_gl_glUniform2fvARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2fvARB) (proc)) (location, count, value) )

/* glUniform3fvARB */
typedef void (APIENTRYP GdkGLProc_glUniform3fvARB) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform3fvARB (void);
#define      gdk_gl_glUniform3fvARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3fvARB) (proc)) (location, count, value) )

/* glUniform4fvARB */
typedef void (APIENTRYP GdkGLProc_glUniform4fvARB) (GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniform4fvARB (void);
#define      gdk_gl_glUniform4fvARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4fvARB) (proc)) (location, count, value) )

/* glUniform1ivARB */
typedef void (APIENTRYP GdkGLProc_glUniform1ivARB) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform1ivARB (void);
#define      gdk_gl_glUniform1ivARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1ivARB) (proc)) (location, count, value) )

/* glUniform2ivARB */
typedef void (APIENTRYP GdkGLProc_glUniform2ivARB) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform2ivARB (void);
#define      gdk_gl_glUniform2ivARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2ivARB) (proc)) (location, count, value) )

/* glUniform3ivARB */
typedef void (APIENTRYP GdkGLProc_glUniform3ivARB) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform3ivARB (void);
#define      gdk_gl_glUniform3ivARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3ivARB) (proc)) (location, count, value) )

/* glUniform4ivARB */
typedef void (APIENTRYP GdkGLProc_glUniform4ivARB) (GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glUniform4ivARB (void);
#define      gdk_gl_glUniform4ivARB(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4ivARB) (proc)) (location, count, value) )

/* glUniformMatrix2fvARB */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix2fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix2fvARB (void);
#define      gdk_gl_glUniformMatrix2fvARB(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix2fvARB) (proc)) (location, count, transpose, value) )

/* glUniformMatrix3fvARB */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix3fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix3fvARB (void);
#define      gdk_gl_glUniformMatrix3fvARB(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix3fvARB) (proc)) (location, count, transpose, value) )

/* glUniformMatrix4fvARB */
typedef void (APIENTRYP GdkGLProc_glUniformMatrix4fvARB) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glUniformMatrix4fvARB (void);
#define      gdk_gl_glUniformMatrix4fvARB(proc, location, count, transpose, value) \
  ( ((GdkGLProc_glUniformMatrix4fvARB) (proc)) (location, count, transpose, value) )

/* glGetObjectParameterfvARB */
typedef void (APIENTRYP GdkGLProc_glGetObjectParameterfvARB) (GLhandleARB obj, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetObjectParameterfvARB (void);
#define      gdk_gl_glGetObjectParameterfvARB(proc, obj, pname, params) \
  ( ((GdkGLProc_glGetObjectParameterfvARB) (proc)) (obj, pname, params) )

/* glGetObjectParameterivARB */
typedef void (APIENTRYP GdkGLProc_glGetObjectParameterivARB) (GLhandleARB obj, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetObjectParameterivARB (void);
#define      gdk_gl_glGetObjectParameterivARB(proc, obj, pname, params) \
  ( ((GdkGLProc_glGetObjectParameterivARB) (proc)) (obj, pname, params) )

/* glGetInfoLogARB */
typedef void (APIENTRYP GdkGLProc_glGetInfoLogARB) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog);
GdkGLProc    gdk_gl_get_glGetInfoLogARB (void);
#define      gdk_gl_glGetInfoLogARB(proc, obj, maxLength, length, infoLog) \
  ( ((GdkGLProc_glGetInfoLogARB) (proc)) (obj, maxLength, length, infoLog) )

/* glGetAttachedObjectsARB */
typedef void (APIENTRYP GdkGLProc_glGetAttachedObjectsARB) (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj);
GdkGLProc    gdk_gl_get_glGetAttachedObjectsARB (void);
#define      gdk_gl_glGetAttachedObjectsARB(proc, containerObj, maxCount, count, obj) \
  ( ((GdkGLProc_glGetAttachedObjectsARB) (proc)) (containerObj, maxCount, count, obj) )

/* glGetUniformLocationARB */
typedef GLint (APIENTRYP GdkGLProc_glGetUniformLocationARB) (GLhandleARB programObj, const GLcharARB *name);
GdkGLProc    gdk_gl_get_glGetUniformLocationARB (void);
#define      gdk_gl_glGetUniformLocationARB(proc, programObj, name) \
  ( ((GdkGLProc_glGetUniformLocationARB) (proc)) (programObj, name) )

/* glGetActiveUniformARB */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniformARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
GdkGLProc    gdk_gl_get_glGetActiveUniformARB (void);
#define      gdk_gl_glGetActiveUniformARB(proc, programObj, index, maxLength, length, size, type, name) \
  ( ((GdkGLProc_glGetActiveUniformARB) (proc)) (programObj, index, maxLength, length, size, type, name) )

/* glGetUniformfvARB */
typedef void (APIENTRYP GdkGLProc_glGetUniformfvARB) (GLhandleARB programObj, GLint location, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetUniformfvARB (void);
#define      gdk_gl_glGetUniformfvARB(proc, programObj, location, params) \
  ( ((GdkGLProc_glGetUniformfvARB) (proc)) (programObj, location, params) )

/* glGetUniformivARB */
typedef void (APIENTRYP GdkGLProc_glGetUniformivARB) (GLhandleARB programObj, GLint location, GLint *params);
GdkGLProc    gdk_gl_get_glGetUniformivARB (void);
#define      gdk_gl_glGetUniformivARB(proc, programObj, location, params) \
  ( ((GdkGLProc_glGetUniformivARB) (proc)) (programObj, location, params) )

/* glGetShaderSourceARB */
typedef void (APIENTRYP GdkGLProc_glGetShaderSourceARB) (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source);
GdkGLProc    gdk_gl_get_glGetShaderSourceARB (void);
#define      gdk_gl_glGetShaderSourceARB(proc, obj, maxLength, length, source) \
  ( ((GdkGLProc_glGetShaderSourceARB) (proc)) (obj, maxLength, length, source) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_shader_objects GdkGL_GL_ARB_shader_objects;

struct _GdkGL_GL_ARB_shader_objects
{
  GdkGLProc_glDeleteObjectARB glDeleteObjectARB;
  GdkGLProc_glGetHandleARB glGetHandleARB;
  GdkGLProc_glDetachObjectARB glDetachObjectARB;
  GdkGLProc_glCreateShaderObjectARB glCreateShaderObjectARB;
  GdkGLProc_glShaderSourceARB glShaderSourceARB;
  GdkGLProc_glCompileShaderARB glCompileShaderARB;
  GdkGLProc_glCreateProgramObjectARB glCreateProgramObjectARB;
  GdkGLProc_glAttachObjectARB glAttachObjectARB;
  GdkGLProc_glLinkProgramARB glLinkProgramARB;
  GdkGLProc_glUseProgramObjectARB glUseProgramObjectARB;
  GdkGLProc_glValidateProgramARB glValidateProgramARB;
  GdkGLProc_glUniform1fARB glUniform1fARB;
  GdkGLProc_glUniform2fARB glUniform2fARB;
  GdkGLProc_glUniform3fARB glUniform3fARB;
  GdkGLProc_glUniform4fARB glUniform4fARB;
  GdkGLProc_glUniform1iARB glUniform1iARB;
  GdkGLProc_glUniform2iARB glUniform2iARB;
  GdkGLProc_glUniform3iARB glUniform3iARB;
  GdkGLProc_glUniform4iARB glUniform4iARB;
  GdkGLProc_glUniform1fvARB glUniform1fvARB;
  GdkGLProc_glUniform2fvARB glUniform2fvARB;
  GdkGLProc_glUniform3fvARB glUniform3fvARB;
  GdkGLProc_glUniform4fvARB glUniform4fvARB;
  GdkGLProc_glUniform1ivARB glUniform1ivARB;
  GdkGLProc_glUniform2ivARB glUniform2ivARB;
  GdkGLProc_glUniform3ivARB glUniform3ivARB;
  GdkGLProc_glUniform4ivARB glUniform4ivARB;
  GdkGLProc_glUniformMatrix2fvARB glUniformMatrix2fvARB;
  GdkGLProc_glUniformMatrix3fvARB glUniformMatrix3fvARB;
  GdkGLProc_glUniformMatrix4fvARB glUniformMatrix4fvARB;
  GdkGLProc_glGetObjectParameterfvARB glGetObjectParameterfvARB;
  GdkGLProc_glGetObjectParameterivARB glGetObjectParameterivARB;
  GdkGLProc_glGetInfoLogARB glGetInfoLogARB;
  GdkGLProc_glGetAttachedObjectsARB glGetAttachedObjectsARB;
  GdkGLProc_glGetUniformLocationARB glGetUniformLocationARB;
  GdkGLProc_glGetActiveUniformARB glGetActiveUniformARB;
  GdkGLProc_glGetUniformfvARB glGetUniformfvARB;
  GdkGLProc_glGetUniformivARB glGetUniformivARB;
  GdkGLProc_glGetShaderSourceARB glGetShaderSourceARB;
};

GdkGL_GL_ARB_shader_objects *gdk_gl_get_GL_ARB_shader_objects (void);

/*
 * GL_ARB_vertex_shader
 */

/* glBindAttribLocationARB */
typedef void (APIENTRYP GdkGLProc_glBindAttribLocationARB) (GLhandleARB programObj, GLuint index, const GLcharARB *name);
GdkGLProc    gdk_gl_get_glBindAttribLocationARB (void);
#define      gdk_gl_glBindAttribLocationARB(proc, programObj, index, name) \
  ( ((GdkGLProc_glBindAttribLocationARB) (proc)) (programObj, index, name) )

/* glGetActiveAttribARB */
typedef void (APIENTRYP GdkGLProc_glGetActiveAttribARB) (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name);
GdkGLProc    gdk_gl_get_glGetActiveAttribARB (void);
#define      gdk_gl_glGetActiveAttribARB(proc, programObj, index, maxLength, length, size, type, name) \
  ( ((GdkGLProc_glGetActiveAttribARB) (proc)) (programObj, index, maxLength, length, size, type, name) )

/* glGetAttribLocationARB */
typedef GLint (APIENTRYP GdkGLProc_glGetAttribLocationARB) (GLhandleARB programObj, const GLcharARB *name);
GdkGLProc    gdk_gl_get_glGetAttribLocationARB (void);
#define      gdk_gl_glGetAttribLocationARB(proc, programObj, name) \
  ( ((GdkGLProc_glGetAttribLocationARB) (proc)) (programObj, name) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_vertex_shader GdkGL_GL_ARB_vertex_shader;

struct _GdkGL_GL_ARB_vertex_shader
{
  GdkGLProc_glBindAttribLocationARB glBindAttribLocationARB;
  GdkGLProc_glGetActiveAttribARB glGetActiveAttribARB;
  GdkGLProc_glGetAttribLocationARB glGetAttribLocationARB;
};

GdkGL_GL_ARB_vertex_shader *gdk_gl_get_GL_ARB_vertex_shader (void);

/*
 * GL_ARB_draw_buffers
 */

/* glDrawBuffersARB */
typedef void (APIENTRYP GdkGLProc_glDrawBuffersARB) (GLsizei n, const GLenum *bufs);
GdkGLProc    gdk_gl_get_glDrawBuffersARB (void);
#define      gdk_gl_glDrawBuffersARB(proc, n, bufs) \
  ( ((GdkGLProc_glDrawBuffersARB) (proc)) (n, bufs) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_draw_buffers GdkGL_GL_ARB_draw_buffers;

struct _GdkGL_GL_ARB_draw_buffers
{
  GdkGLProc_glDrawBuffersARB glDrawBuffersARB;
};

GdkGL_GL_ARB_draw_buffers *gdk_gl_get_GL_ARB_draw_buffers (void);

/*
 * GL_ARB_color_buffer_float
 */

/* glClampColorARB */
typedef void (APIENTRYP GdkGLProc_glClampColorARB) (GLenum target, GLenum clamp);
GdkGLProc    gdk_gl_get_glClampColorARB (void);
#define      gdk_gl_glClampColorARB(proc, target, clamp) \
  ( ((GdkGLProc_glClampColorARB) (proc)) (target, clamp) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_color_buffer_float GdkGL_GL_ARB_color_buffer_float;

struct _GdkGL_GL_ARB_color_buffer_float
{
  GdkGLProc_glClampColorARB glClampColorARB;
};

GdkGL_GL_ARB_color_buffer_float *gdk_gl_get_GL_ARB_color_buffer_float (void);

/*
 * GL_ARB_draw_instanced
 */

/* glDrawArraysInstancedARB */
typedef void (APIENTRYP GdkGLProc_glDrawArraysInstancedARB) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawArraysInstancedARB (void);
#define      gdk_gl_glDrawArraysInstancedARB(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glDrawArraysInstancedARB) (proc)) (mode, first, count, primcount) )

/* glDrawElementsInstancedARB */
typedef void (APIENTRYP GdkGLProc_glDrawElementsInstancedARB) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawElementsInstancedARB (void);
#define      gdk_gl_glDrawElementsInstancedARB(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glDrawElementsInstancedARB) (proc)) (mode, count, type, indices, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_draw_instanced GdkGL_GL_ARB_draw_instanced;

struct _GdkGL_GL_ARB_draw_instanced
{
  GdkGLProc_glDrawArraysInstancedARB glDrawArraysInstancedARB;
  GdkGLProc_glDrawElementsInstancedARB glDrawElementsInstancedARB;
};

GdkGL_GL_ARB_draw_instanced *gdk_gl_get_GL_ARB_draw_instanced (void);

/*
 * GL_ARB_framebuffer_object
 */

/* glIsRenderbuffer */
typedef GLboolean (APIENTRYP GdkGLProc_glIsRenderbuffer) (GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glIsRenderbuffer (void);
#define      gdk_gl_glIsRenderbuffer(proc, renderbuffer) \
  ( ((GdkGLProc_glIsRenderbuffer) (proc)) (renderbuffer) )

/* glBindRenderbuffer */
typedef void (APIENTRYP GdkGLProc_glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glBindRenderbuffer (void);
#define      gdk_gl_glBindRenderbuffer(proc, target, renderbuffer) \
  ( ((GdkGLProc_glBindRenderbuffer) (proc)) (target, renderbuffer) )

/* glDeleteRenderbuffers */
typedef void (APIENTRYP GdkGLProc_glDeleteRenderbuffers) (GLsizei n, const GLuint *renderbuffers);
GdkGLProc    gdk_gl_get_glDeleteRenderbuffers (void);
#define      gdk_gl_glDeleteRenderbuffers(proc, n, renderbuffers) \
  ( ((GdkGLProc_glDeleteRenderbuffers) (proc)) (n, renderbuffers) )

/* glGenRenderbuffers */
typedef void (APIENTRYP GdkGLProc_glGenRenderbuffers) (GLsizei n, GLuint *renderbuffers);
GdkGLProc    gdk_gl_get_glGenRenderbuffers (void);
#define      gdk_gl_glGenRenderbuffers(proc, n, renderbuffers) \
  ( ((GdkGLProc_glGenRenderbuffers) (proc)) (n, renderbuffers) )

/* glRenderbufferStorage */
typedef void (APIENTRYP GdkGLProc_glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glRenderbufferStorage (void);
#define      gdk_gl_glRenderbufferStorage(proc, target, internalformat, width, height) \
  ( ((GdkGLProc_glRenderbufferStorage) (proc)) (target, internalformat, width, height) )

/* glGetRenderbufferParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetRenderbufferParameteriv (void);
#define      gdk_gl_glGetRenderbufferParameteriv(proc, target, pname, params) \
  ( ((GdkGLProc_glGetRenderbufferParameteriv) (proc)) (target, pname, params) )

/* glIsFramebuffer */
typedef GLboolean (APIENTRYP GdkGLProc_glIsFramebuffer) (GLuint framebuffer);
GdkGLProc    gdk_gl_get_glIsFramebuffer (void);
#define      gdk_gl_glIsFramebuffer(proc, framebuffer) \
  ( ((GdkGLProc_glIsFramebuffer) (proc)) (framebuffer) )

/* glBindFramebuffer */
typedef void (APIENTRYP GdkGLProc_glBindFramebuffer) (GLenum target, GLuint framebuffer);
GdkGLProc    gdk_gl_get_glBindFramebuffer (void);
#define      gdk_gl_glBindFramebuffer(proc, target, framebuffer) \
  ( ((GdkGLProc_glBindFramebuffer) (proc)) (target, framebuffer) )

/* glDeleteFramebuffers */
typedef void (APIENTRYP GdkGLProc_glDeleteFramebuffers) (GLsizei n, const GLuint *framebuffers);
GdkGLProc    gdk_gl_get_glDeleteFramebuffers (void);
#define      gdk_gl_glDeleteFramebuffers(proc, n, framebuffers) \
  ( ((GdkGLProc_glDeleteFramebuffers) (proc)) (n, framebuffers) )

/* glGenFramebuffers */
typedef void (APIENTRYP GdkGLProc_glGenFramebuffers) (GLsizei n, GLuint *framebuffers);
GdkGLProc    gdk_gl_get_glGenFramebuffers (void);
#define      gdk_gl_glGenFramebuffers(proc, n, framebuffers) \
  ( ((GdkGLProc_glGenFramebuffers) (proc)) (n, framebuffers) )

/* glCheckFramebufferStatus */
typedef GLenum (APIENTRYP GdkGLProc_glCheckFramebufferStatus) (GLenum target);
GdkGLProc    gdk_gl_get_glCheckFramebufferStatus (void);
#define      gdk_gl_glCheckFramebufferStatus(proc, target) \
  ( ((GdkGLProc_glCheckFramebufferStatus) (proc)) (target) )

/* glFramebufferTexture1D */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture1D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTexture1D (void);
#define      gdk_gl_glFramebufferTexture1D(proc, target, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glFramebufferTexture1D) (proc)) (target, attachment, textarget, texture, level) )

/* glFramebufferTexture2D */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTexture2D (void);
#define      gdk_gl_glFramebufferTexture2D(proc, target, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glFramebufferTexture2D) (proc)) (target, attachment, textarget, texture, level) )

/* glFramebufferTexture3D */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture3D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GdkGLProc    gdk_gl_get_glFramebufferTexture3D (void);
#define      gdk_gl_glFramebufferTexture3D(proc, target, attachment, textarget, texture, level, zoffset) \
  ( ((GdkGLProc_glFramebufferTexture3D) (proc)) (target, attachment, textarget, texture, level, zoffset) )

/* glFramebufferRenderbuffer */
typedef void (APIENTRYP GdkGLProc_glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glFramebufferRenderbuffer (void);
#define      gdk_gl_glFramebufferRenderbuffer(proc, target, attachment, renderbuffertarget, renderbuffer) \
  ( ((GdkGLProc_glFramebufferRenderbuffer) (proc)) (target, attachment, renderbuffertarget, renderbuffer) )

/* glGetFramebufferAttachmentParameteriv */
typedef void (APIENTRYP GdkGLProc_glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFramebufferAttachmentParameteriv (void);
#define      gdk_gl_glGetFramebufferAttachmentParameteriv(proc, target, attachment, pname, params) \
  ( ((GdkGLProc_glGetFramebufferAttachmentParameteriv) (proc)) (target, attachment, pname, params) )

/* glGenerateMipmap */
typedef void (APIENTRYP GdkGLProc_glGenerateMipmap) (GLenum target);
GdkGLProc    gdk_gl_get_glGenerateMipmap (void);
#define      gdk_gl_glGenerateMipmap(proc, target) \
  ( ((GdkGLProc_glGenerateMipmap) (proc)) (target) )

/* glBlitFramebuffer */
typedef void (APIENTRYP GdkGLProc_glBlitFramebuffer) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GdkGLProc    gdk_gl_get_glBlitFramebuffer (void);
#define      gdk_gl_glBlitFramebuffer(proc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) \
  ( ((GdkGLProc_glBlitFramebuffer) (proc)) (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) )

/* glRenderbufferStorageMultisample */
typedef void (APIENTRYP GdkGLProc_glRenderbufferStorageMultisample) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glRenderbufferStorageMultisample (void);
#define      gdk_gl_glRenderbufferStorageMultisample(proc, target, samples, internalformat, width, height) \
  ( ((GdkGLProc_glRenderbufferStorageMultisample) (proc)) (target, samples, internalformat, width, height) )

/* glFramebufferTextureLayer */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureLayer) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
GdkGLProc    gdk_gl_get_glFramebufferTextureLayer (void);
#define      gdk_gl_glFramebufferTextureLayer(proc, target, attachment, texture, level, layer) \
  ( ((GdkGLProc_glFramebufferTextureLayer) (proc)) (target, attachment, texture, level, layer) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_framebuffer_object GdkGL_GL_ARB_framebuffer_object;

struct _GdkGL_GL_ARB_framebuffer_object
{
  GdkGLProc_glIsRenderbuffer glIsRenderbuffer;
  GdkGLProc_glBindRenderbuffer glBindRenderbuffer;
  GdkGLProc_glDeleteRenderbuffers glDeleteRenderbuffers;
  GdkGLProc_glGenRenderbuffers glGenRenderbuffers;
  GdkGLProc_glRenderbufferStorage glRenderbufferStorage;
  GdkGLProc_glGetRenderbufferParameteriv glGetRenderbufferParameteriv;
  GdkGLProc_glIsFramebuffer glIsFramebuffer;
  GdkGLProc_glBindFramebuffer glBindFramebuffer;
  GdkGLProc_glDeleteFramebuffers glDeleteFramebuffers;
  GdkGLProc_glGenFramebuffers glGenFramebuffers;
  GdkGLProc_glCheckFramebufferStatus glCheckFramebufferStatus;
  GdkGLProc_glFramebufferTexture1D glFramebufferTexture1D;
  GdkGLProc_glFramebufferTexture2D glFramebufferTexture2D;
  GdkGLProc_glFramebufferTexture3D glFramebufferTexture3D;
  GdkGLProc_glFramebufferRenderbuffer glFramebufferRenderbuffer;
  GdkGLProc_glGetFramebufferAttachmentParameteriv glGetFramebufferAttachmentParameteriv;
  GdkGLProc_glGenerateMipmap glGenerateMipmap;
  GdkGLProc_glBlitFramebuffer glBlitFramebuffer;
  GdkGLProc_glRenderbufferStorageMultisample glRenderbufferStorageMultisample;
  GdkGLProc_glFramebufferTextureLayer glFramebufferTextureLayer;
};

GdkGL_GL_ARB_framebuffer_object *gdk_gl_get_GL_ARB_framebuffer_object (void);

/*
 * GL_ARB_geometry_shader4
 */

/* glProgramParameteriARB */
typedef void (APIENTRYP GdkGLProc_glProgramParameteriARB) (GLuint program, GLenum pname, GLint value);
GdkGLProc    gdk_gl_get_glProgramParameteriARB (void);
#define      gdk_gl_glProgramParameteriARB(proc, program, pname, value) \
  ( ((GdkGLProc_glProgramParameteriARB) (proc)) (program, pname, value) )

/* glFramebufferTextureARB */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureARB) (GLenum target, GLenum attachment, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTextureARB (void);
#define      gdk_gl_glFramebufferTextureARB(proc, target, attachment, texture, level) \
  ( ((GdkGLProc_glFramebufferTextureARB) (proc)) (target, attachment, texture, level) )

/* glFramebufferTextureLayerARB */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureLayerARB) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
GdkGLProc    gdk_gl_get_glFramebufferTextureLayerARB (void);
#define      gdk_gl_glFramebufferTextureLayerARB(proc, target, attachment, texture, level, layer) \
  ( ((GdkGLProc_glFramebufferTextureLayerARB) (proc)) (target, attachment, texture, level, layer) )

/* glFramebufferTextureFaceARB */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureFaceARB) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
GdkGLProc    gdk_gl_get_glFramebufferTextureFaceARB (void);
#define      gdk_gl_glFramebufferTextureFaceARB(proc, target, attachment, texture, level, face) \
  ( ((GdkGLProc_glFramebufferTextureFaceARB) (proc)) (target, attachment, texture, level, face) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_geometry_shader4 GdkGL_GL_ARB_geometry_shader4;

struct _GdkGL_GL_ARB_geometry_shader4
{
  GdkGLProc_glProgramParameteriARB glProgramParameteriARB;
  GdkGLProc_glFramebufferTextureARB glFramebufferTextureARB;
  GdkGLProc_glFramebufferTextureLayerARB glFramebufferTextureLayerARB;
  GdkGLProc_glFramebufferTextureFaceARB glFramebufferTextureFaceARB;
};

GdkGL_GL_ARB_geometry_shader4 *gdk_gl_get_GL_ARB_geometry_shader4 (void);

/*
 * GL_ARB_instanced_arrays
 */

/* glVertexAttribDivisorARB */
typedef void (APIENTRYP GdkGLProc_glVertexAttribDivisorARB) (GLuint index, GLuint divisor);
GdkGLProc    gdk_gl_get_glVertexAttribDivisorARB (void);
#define      gdk_gl_glVertexAttribDivisorARB(proc, index, divisor) \
  ( ((GdkGLProc_glVertexAttribDivisorARB) (proc)) (index, divisor) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_instanced_arrays GdkGL_GL_ARB_instanced_arrays;

struct _GdkGL_GL_ARB_instanced_arrays
{
  GdkGLProc_glVertexAttribDivisorARB glVertexAttribDivisorARB;
};

GdkGL_GL_ARB_instanced_arrays *gdk_gl_get_GL_ARB_instanced_arrays (void);

/*
 * GL_ARB_map_buffer_range
 */

/* glMapBufferRange */
typedef GLvoid* (APIENTRYP GdkGLProc_glMapBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
GdkGLProc    gdk_gl_get_glMapBufferRange (void);
#define      gdk_gl_glMapBufferRange(proc, target, offset, length, access) \
  ( ((GdkGLProc_glMapBufferRange) (proc)) (target, offset, length, access) )

/* glFlushMappedBufferRange */
typedef void (APIENTRYP GdkGLProc_glFlushMappedBufferRange) (GLenum target, GLintptr offset, GLsizeiptr length);
GdkGLProc    gdk_gl_get_glFlushMappedBufferRange (void);
#define      gdk_gl_glFlushMappedBufferRange(proc, target, offset, length) \
  ( ((GdkGLProc_glFlushMappedBufferRange) (proc)) (target, offset, length) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_map_buffer_range GdkGL_GL_ARB_map_buffer_range;

struct _GdkGL_GL_ARB_map_buffer_range
{
  GdkGLProc_glMapBufferRange glMapBufferRange;
  GdkGLProc_glFlushMappedBufferRange glFlushMappedBufferRange;
};

GdkGL_GL_ARB_map_buffer_range *gdk_gl_get_GL_ARB_map_buffer_range (void);

/*
 * GL_ARB_texture_buffer_object
 */

/* glTexBufferARB */
typedef void (APIENTRYP GdkGLProc_glTexBufferARB) (GLenum target, GLenum internalformat, GLuint buffer);
GdkGLProc    gdk_gl_get_glTexBufferARB (void);
#define      gdk_gl_glTexBufferARB(proc, target, internalformat, buffer) \
  ( ((GdkGLProc_glTexBufferARB) (proc)) (target, internalformat, buffer) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_texture_buffer_object GdkGL_GL_ARB_texture_buffer_object;

struct _GdkGL_GL_ARB_texture_buffer_object
{
  GdkGLProc_glTexBufferARB glTexBufferARB;
};

GdkGL_GL_ARB_texture_buffer_object *gdk_gl_get_GL_ARB_texture_buffer_object (void);

/*
 * GL_ARB_vertex_array_object
 */

/* glBindVertexArray */
typedef void (APIENTRYP GdkGLProc_glBindVertexArray) (GLuint array);
GdkGLProc    gdk_gl_get_glBindVertexArray (void);
#define      gdk_gl_glBindVertexArray(proc, array) \
  ( ((GdkGLProc_glBindVertexArray) (proc)) (array) )

/* glDeleteVertexArrays */
typedef void (APIENTRYP GdkGLProc_glDeleteVertexArrays) (GLsizei n, const GLuint *arrays);
GdkGLProc    gdk_gl_get_glDeleteVertexArrays (void);
#define      gdk_gl_glDeleteVertexArrays(proc, n, arrays) \
  ( ((GdkGLProc_glDeleteVertexArrays) (proc)) (n, arrays) )

/* glGenVertexArrays */
typedef void (APIENTRYP GdkGLProc_glGenVertexArrays) (GLsizei n, GLuint *arrays);
GdkGLProc    gdk_gl_get_glGenVertexArrays (void);
#define      gdk_gl_glGenVertexArrays(proc, n, arrays) \
  ( ((GdkGLProc_glGenVertexArrays) (proc)) (n, arrays) )

/* glIsVertexArray */
typedef GLboolean (APIENTRYP GdkGLProc_glIsVertexArray) (GLuint array);
GdkGLProc    gdk_gl_get_glIsVertexArray (void);
#define      gdk_gl_glIsVertexArray(proc, array) \
  ( ((GdkGLProc_glIsVertexArray) (proc)) (array) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_vertex_array_object GdkGL_GL_ARB_vertex_array_object;

struct _GdkGL_GL_ARB_vertex_array_object
{
  GdkGLProc_glBindVertexArray glBindVertexArray;
  GdkGLProc_glDeleteVertexArrays glDeleteVertexArrays;
  GdkGLProc_glGenVertexArrays glGenVertexArrays;
  GdkGLProc_glIsVertexArray glIsVertexArray;
};

GdkGL_GL_ARB_vertex_array_object *gdk_gl_get_GL_ARB_vertex_array_object (void);

/*
 * GL_ARB_uniform_buffer_object
 */

/* glGetUniformIndices */
typedef void (APIENTRYP GdkGLProc_glGetUniformIndices) (GLuint program, GLsizei uniformCount, const GLchar* *uniformNames, GLuint *uniformIndices);
GdkGLProc    gdk_gl_get_glGetUniformIndices (void);
#define      gdk_gl_glGetUniformIndices(proc, program, uniformCount, uniformNames, uniformIndices) \
  ( ((GdkGLProc_glGetUniformIndices) (proc)) (program, uniformCount, uniformNames, uniformIndices) )

/* glGetActiveUniformsiv */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniformsiv) (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetActiveUniformsiv (void);
#define      gdk_gl_glGetActiveUniformsiv(proc, program, uniformCount, uniformIndices, pname, params) \
  ( ((GdkGLProc_glGetActiveUniformsiv) (proc)) (program, uniformCount, uniformIndices, pname, params) )

/* glGetActiveUniformName */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniformName) (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
GdkGLProc    gdk_gl_get_glGetActiveUniformName (void);
#define      gdk_gl_glGetActiveUniformName(proc, program, uniformIndex, bufSize, length, uniformName) \
  ( ((GdkGLProc_glGetActiveUniformName) (proc)) (program, uniformIndex, bufSize, length, uniformName) )

/* glGetUniformBlockIndex */
typedef GLuint (APIENTRYP GdkGLProc_glGetUniformBlockIndex) (GLuint program, const GLchar *uniformBlockName);
GdkGLProc    gdk_gl_get_glGetUniformBlockIndex (void);
#define      gdk_gl_glGetUniformBlockIndex(proc, program, uniformBlockName) \
  ( ((GdkGLProc_glGetUniformBlockIndex) (proc)) (program, uniformBlockName) )

/* glGetActiveUniformBlockiv */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniformBlockiv) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetActiveUniformBlockiv (void);
#define      gdk_gl_glGetActiveUniformBlockiv(proc, program, uniformBlockIndex, pname, params) \
  ( ((GdkGLProc_glGetActiveUniformBlockiv) (proc)) (program, uniformBlockIndex, pname, params) )

/* glGetActiveUniformBlockName */
typedef void (APIENTRYP GdkGLProc_glGetActiveUniformBlockName) (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
GdkGLProc    gdk_gl_get_glGetActiveUniformBlockName (void);
#define      gdk_gl_glGetActiveUniformBlockName(proc, program, uniformBlockIndex, bufSize, length, uniformBlockName) \
  ( ((GdkGLProc_glGetActiveUniformBlockName) (proc)) (program, uniformBlockIndex, bufSize, length, uniformBlockName) )

/* glUniformBlockBinding */
typedef void (APIENTRYP GdkGLProc_glUniformBlockBinding) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
GdkGLProc    gdk_gl_get_glUniformBlockBinding (void);
#define      gdk_gl_glUniformBlockBinding(proc, program, uniformBlockIndex, uniformBlockBinding) \
  ( ((GdkGLProc_glUniformBlockBinding) (proc)) (program, uniformBlockIndex, uniformBlockBinding) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_uniform_buffer_object GdkGL_GL_ARB_uniform_buffer_object;

struct _GdkGL_GL_ARB_uniform_buffer_object
{
  GdkGLProc_glGetUniformIndices glGetUniformIndices;
  GdkGLProc_glGetActiveUniformsiv glGetActiveUniformsiv;
  GdkGLProc_glGetActiveUniformName glGetActiveUniformName;
  GdkGLProc_glGetUniformBlockIndex glGetUniformBlockIndex;
  GdkGLProc_glGetActiveUniformBlockiv glGetActiveUniformBlockiv;
  GdkGLProc_glGetActiveUniformBlockName glGetActiveUniformBlockName;
  GdkGLProc_glUniformBlockBinding glUniformBlockBinding;
};

GdkGL_GL_ARB_uniform_buffer_object *gdk_gl_get_GL_ARB_uniform_buffer_object (void);

/*
 * GL_ARB_copy_buffer
 */

/* glCopyBufferSubData */
typedef void (APIENTRYP GdkGLProc_glCopyBufferSubData) (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
GdkGLProc    gdk_gl_get_glCopyBufferSubData (void);
#define      gdk_gl_glCopyBufferSubData(proc, readTarget, writeTarget, readOffset, writeOffset, size) \
  ( ((GdkGLProc_glCopyBufferSubData) (proc)) (readTarget, writeTarget, readOffset, writeOffset, size) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_copy_buffer GdkGL_GL_ARB_copy_buffer;

struct _GdkGL_GL_ARB_copy_buffer
{
  GdkGLProc_glCopyBufferSubData glCopyBufferSubData;
};

GdkGL_GL_ARB_copy_buffer *gdk_gl_get_GL_ARB_copy_buffer (void);

/*
 * GL_ARB_draw_elements_base_vertex
 */

/* glDrawElementsBaseVertex */
typedef void (APIENTRYP GdkGLProc_glDrawElementsBaseVertex) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
GdkGLProc    gdk_gl_get_glDrawElementsBaseVertex (void);
#define      gdk_gl_glDrawElementsBaseVertex(proc, mode, count, type, indices, basevertex) \
  ( ((GdkGLProc_glDrawElementsBaseVertex) (proc)) (mode, count, type, indices, basevertex) )

/* glDrawRangeElementsBaseVertex */
typedef void (APIENTRYP GdkGLProc_glDrawRangeElementsBaseVertex) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
GdkGLProc    gdk_gl_get_glDrawRangeElementsBaseVertex (void);
#define      gdk_gl_glDrawRangeElementsBaseVertex(proc, mode, start, end, count, type, indices, basevertex) \
  ( ((GdkGLProc_glDrawRangeElementsBaseVertex) (proc)) (mode, start, end, count, type, indices, basevertex) )

/* glDrawElementsInstancedBaseVertex */
typedef void (APIENTRYP GdkGLProc_glDrawElementsInstancedBaseVertex) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount, GLint basevertex);
GdkGLProc    gdk_gl_get_glDrawElementsInstancedBaseVertex (void);
#define      gdk_gl_glDrawElementsInstancedBaseVertex(proc, mode, count, type, indices, primcount, basevertex) \
  ( ((GdkGLProc_glDrawElementsInstancedBaseVertex) (proc)) (mode, count, type, indices, primcount, basevertex) )

/* glMultiDrawElementsBaseVertex */
typedef void (APIENTRYP GdkGLProc_glMultiDrawElementsBaseVertex) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount, const GLint *basevertex);
GdkGLProc    gdk_gl_get_glMultiDrawElementsBaseVertex (void);
#define      gdk_gl_glMultiDrawElementsBaseVertex(proc, mode, count, type, indices, primcount, basevertex) \
  ( ((GdkGLProc_glMultiDrawElementsBaseVertex) (proc)) (mode, count, type, indices, primcount, basevertex) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_draw_elements_base_vertex GdkGL_GL_ARB_draw_elements_base_vertex;

struct _GdkGL_GL_ARB_draw_elements_base_vertex
{
  GdkGLProc_glDrawElementsBaseVertex glDrawElementsBaseVertex;
  GdkGLProc_glDrawRangeElementsBaseVertex glDrawRangeElementsBaseVertex;
  GdkGLProc_glDrawElementsInstancedBaseVertex glDrawElementsInstancedBaseVertex;
  GdkGLProc_glMultiDrawElementsBaseVertex glMultiDrawElementsBaseVertex;
};

GdkGL_GL_ARB_draw_elements_base_vertex *gdk_gl_get_GL_ARB_draw_elements_base_vertex (void);

/*
 * GL_ARB_provoking_vertex
 */

/* glProvokingVertex */
typedef void (APIENTRYP GdkGLProc_glProvokingVertex) (GLenum mode);
GdkGLProc    gdk_gl_get_glProvokingVertex (void);
#define      gdk_gl_glProvokingVertex(proc, mode) \
  ( ((GdkGLProc_glProvokingVertex) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_provoking_vertex GdkGL_GL_ARB_provoking_vertex;

struct _GdkGL_GL_ARB_provoking_vertex
{
  GdkGLProc_glProvokingVertex glProvokingVertex;
};

GdkGL_GL_ARB_provoking_vertex *gdk_gl_get_GL_ARB_provoking_vertex (void);

/*
 * GL_ARB_sync
 */

/* glFenceSync */
typedef GLsync (APIENTRYP GdkGLProc_glFenceSync) (GLenum condition, GLbitfield flags);
GdkGLProc    gdk_gl_get_glFenceSync (void);
#define      gdk_gl_glFenceSync(proc, condition, flags) \
  ( ((GdkGLProc_glFenceSync) (proc)) (condition, flags) )

/* glIsSync */
typedef GLboolean (APIENTRYP GdkGLProc_glIsSync) (GLsync sync);
GdkGLProc    gdk_gl_get_glIsSync (void);
#define      gdk_gl_glIsSync(proc, sync) \
  ( ((GdkGLProc_glIsSync) (proc)) (sync) )

/* glDeleteSync */
typedef void (APIENTRYP GdkGLProc_glDeleteSync) (GLsync sync);
GdkGLProc    gdk_gl_get_glDeleteSync (void);
#define      gdk_gl_glDeleteSync(proc, sync) \
  ( ((GdkGLProc_glDeleteSync) (proc)) (sync) )

/* glClientWaitSync */
typedef GLenum (APIENTRYP GdkGLProc_glClientWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
GdkGLProc    gdk_gl_get_glClientWaitSync (void);
#define      gdk_gl_glClientWaitSync(proc, sync, flags, timeout) \
  ( ((GdkGLProc_glClientWaitSync) (proc)) (sync, flags, timeout) )

/* glWaitSync */
typedef void (APIENTRYP GdkGLProc_glWaitSync) (GLsync sync, GLbitfield flags, GLuint64 timeout);
GdkGLProc    gdk_gl_get_glWaitSync (void);
#define      gdk_gl_glWaitSync(proc, sync, flags, timeout) \
  ( ((GdkGLProc_glWaitSync) (proc)) (sync, flags, timeout) )

/* glGetInteger64v */
typedef void (APIENTRYP GdkGLProc_glGetInteger64v) (GLenum pname, GLint64 *params);
GdkGLProc    gdk_gl_get_glGetInteger64v (void);
#define      gdk_gl_glGetInteger64v(proc, pname, params) \
  ( ((GdkGLProc_glGetInteger64v) (proc)) (pname, params) )

/* glGetSynciv */
typedef void (APIENTRYP GdkGLProc_glGetSynciv) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
GdkGLProc    gdk_gl_get_glGetSynciv (void);
#define      gdk_gl_glGetSynciv(proc, sync, pname, bufSize, length, values) \
  ( ((GdkGLProc_glGetSynciv) (proc)) (sync, pname, bufSize, length, values) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_sync GdkGL_GL_ARB_sync;

struct _GdkGL_GL_ARB_sync
{
  GdkGLProc_glFenceSync glFenceSync;
  GdkGLProc_glIsSync glIsSync;
  GdkGLProc_glDeleteSync glDeleteSync;
  GdkGLProc_glClientWaitSync glClientWaitSync;
  GdkGLProc_glWaitSync glWaitSync;
  GdkGLProc_glGetInteger64v glGetInteger64v;
  GdkGLProc_glGetSynciv glGetSynciv;
};

GdkGL_GL_ARB_sync *gdk_gl_get_GL_ARB_sync (void);

/*
 * GL_ARB_texture_multisample
 */

/* glTexImage2DMultisample */
typedef void (APIENTRYP GdkGLProc_glTexImage2DMultisample) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
GdkGLProc    gdk_gl_get_glTexImage2DMultisample (void);
#define      gdk_gl_glTexImage2DMultisample(proc, target, samples, internalformat, width, height, fixedsamplelocations) \
  ( ((GdkGLProc_glTexImage2DMultisample) (proc)) (target, samples, internalformat, width, height, fixedsamplelocations) )

/* glTexImage3DMultisample */
typedef void (APIENTRYP GdkGLProc_glTexImage3DMultisample) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
GdkGLProc    gdk_gl_get_glTexImage3DMultisample (void);
#define      gdk_gl_glTexImage3DMultisample(proc, target, samples, internalformat, width, height, depth, fixedsamplelocations) \
  ( ((GdkGLProc_glTexImage3DMultisample) (proc)) (target, samples, internalformat, width, height, depth, fixedsamplelocations) )

/* glGetMultisamplefv */
typedef void (APIENTRYP GdkGLProc_glGetMultisamplefv) (GLenum pname, GLuint index, GLfloat *val);
GdkGLProc    gdk_gl_get_glGetMultisamplefv (void);
#define      gdk_gl_glGetMultisamplefv(proc, pname, index, val) \
  ( ((GdkGLProc_glGetMultisamplefv) (proc)) (pname, index, val) )

/* glSampleMaski */
typedef void (APIENTRYP GdkGLProc_glSampleMaski) (GLuint index, GLbitfield mask);
GdkGLProc    gdk_gl_get_glSampleMaski (void);
#define      gdk_gl_glSampleMaski(proc, index, mask) \
  ( ((GdkGLProc_glSampleMaski) (proc)) (index, mask) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_texture_multisample GdkGL_GL_ARB_texture_multisample;

struct _GdkGL_GL_ARB_texture_multisample
{
  GdkGLProc_glTexImage2DMultisample glTexImage2DMultisample;
  GdkGLProc_glTexImage3DMultisample glTexImage3DMultisample;
  GdkGLProc_glGetMultisamplefv glGetMultisamplefv;
  GdkGLProc_glSampleMaski glSampleMaski;
};

GdkGL_GL_ARB_texture_multisample *gdk_gl_get_GL_ARB_texture_multisample (void);

/*
 * GL_ARB_draw_buffers_blend
 */

/* glBlendEquationi */
typedef void (APIENTRYP GdkGLProc_glBlendEquationi) (GLuint buf, GLenum mode);
GdkGLProc    gdk_gl_get_glBlendEquationi (void);
#define      gdk_gl_glBlendEquationi(proc, buf, mode) \
  ( ((GdkGLProc_glBlendEquationi) (proc)) (buf, mode) )

/* glBlendEquationSeparatei */
typedef void (APIENTRYP GdkGLProc_glBlendEquationSeparatei) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
GdkGLProc    gdk_gl_get_glBlendEquationSeparatei (void);
#define      gdk_gl_glBlendEquationSeparatei(proc, buf, modeRGB, modeAlpha) \
  ( ((GdkGLProc_glBlendEquationSeparatei) (proc)) (buf, modeRGB, modeAlpha) )

/* glBlendFunci */
typedef void (APIENTRYP GdkGLProc_glBlendFunci) (GLuint buf, GLenum src, GLenum dst);
GdkGLProc    gdk_gl_get_glBlendFunci (void);
#define      gdk_gl_glBlendFunci(proc, buf, src, dst) \
  ( ((GdkGLProc_glBlendFunci) (proc)) (buf, src, dst) )

/* glBlendFuncSeparatei */
typedef void (APIENTRYP GdkGLProc_glBlendFuncSeparatei) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
GdkGLProc    gdk_gl_get_glBlendFuncSeparatei (void);
#define      gdk_gl_glBlendFuncSeparatei(proc, buf, srcRGB, dstRGB, srcAlpha, dstAlpha) \
  ( ((GdkGLProc_glBlendFuncSeparatei) (proc)) (buf, srcRGB, dstRGB, srcAlpha, dstAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_draw_buffers_blend GdkGL_GL_ARB_draw_buffers_blend;

struct _GdkGL_GL_ARB_draw_buffers_blend
{
  GdkGLProc_glBlendEquationi glBlendEquationi;
  GdkGLProc_glBlendEquationSeparatei glBlendEquationSeparatei;
  GdkGLProc_glBlendFunci glBlendFunci;
  GdkGLProc_glBlendFuncSeparatei glBlendFuncSeparatei;
};

GdkGL_GL_ARB_draw_buffers_blend *gdk_gl_get_GL_ARB_draw_buffers_blend (void);

/*
 * GL_ARB_sample_shading
 */

/* glMinSampleShading */
typedef void (APIENTRYP GdkGLProc_glMinSampleShading) (GLclampf value);
GdkGLProc    gdk_gl_get_glMinSampleShading (void);
#define      gdk_gl_glMinSampleShading(proc, value) \
  ( ((GdkGLProc_glMinSampleShading) (proc)) (value) )

/* proc struct */

typedef struct _GdkGL_GL_ARB_sample_shading GdkGL_GL_ARB_sample_shading;

struct _GdkGL_GL_ARB_sample_shading
{
  GdkGLProc_glMinSampleShading glMinSampleShading;
};

GdkGL_GL_ARB_sample_shading *gdk_gl_get_GL_ARB_sample_shading (void);

/*
 * GL_EXT_blend_color
 */

/* glBlendColorEXT */
typedef void (APIENTRYP GdkGLProc_glBlendColorEXT) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
GdkGLProc    gdk_gl_get_glBlendColorEXT (void);
#define      gdk_gl_glBlendColorEXT(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glBlendColorEXT) (proc)) (red, green, blue, alpha) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_blend_color GdkGL_GL_EXT_blend_color;

struct _GdkGL_GL_EXT_blend_color
{
  GdkGLProc_glBlendColorEXT glBlendColorEXT;
};

GdkGL_GL_EXT_blend_color *gdk_gl_get_GL_EXT_blend_color (void);

/*
 * GL_EXT_polygon_offset
 */

/* glPolygonOffsetEXT */
typedef void (APIENTRYP GdkGLProc_glPolygonOffsetEXT) (GLfloat factor, GLfloat bias);
GdkGLProc    gdk_gl_get_glPolygonOffsetEXT (void);
#define      gdk_gl_glPolygonOffsetEXT(proc, factor, bias) \
  ( ((GdkGLProc_glPolygonOffsetEXT) (proc)) (factor, bias) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_polygon_offset GdkGL_GL_EXT_polygon_offset;

struct _GdkGL_GL_EXT_polygon_offset
{
  GdkGLProc_glPolygonOffsetEXT glPolygonOffsetEXT;
};

GdkGL_GL_EXT_polygon_offset *gdk_gl_get_GL_EXT_polygon_offset (void);

/*
 * GL_EXT_texture3D
 */

/* glTexImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glTexImage3DEXT) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexImage3DEXT (void);
#define      gdk_gl_glTexImage3DEXT(proc, target, level, internalformat, width, height, depth, border, format, type, pixels) \
  ( ((GdkGLProc_glTexImage3DEXT) (proc)) (target, level, internalformat, width, height, depth, border, format, type, pixels) )

/* glTexSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glTexSubImage3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexSubImage3DEXT (void);
#define      gdk_gl_glTexSubImage3DEXT(proc, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) \
  ( ((GdkGLProc_glTexSubImage3DEXT) (proc)) (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_texture3D GdkGL_GL_EXT_texture3D;

struct _GdkGL_GL_EXT_texture3D
{
  GdkGLProc_glTexImage3DEXT glTexImage3DEXT;
  GdkGLProc_glTexSubImage3DEXT glTexSubImage3DEXT;
};

GdkGL_GL_EXT_texture3D *gdk_gl_get_GL_EXT_texture3D (void);

/*
 * GL_SGIS_texture_filter4
 */

/* glGetTexFilterFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glGetTexFilterFuncSGIS) (GLenum target, GLenum filter, GLfloat *weights);
GdkGLProc    gdk_gl_get_glGetTexFilterFuncSGIS (void);
#define      gdk_gl_glGetTexFilterFuncSGIS(proc, target, filter, weights) \
  ( ((GdkGLProc_glGetTexFilterFuncSGIS) (proc)) (target, filter, weights) )

/* glTexFilterFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glTexFilterFuncSGIS) (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights);
GdkGLProc    gdk_gl_get_glTexFilterFuncSGIS (void);
#define      gdk_gl_glTexFilterFuncSGIS(proc, target, filter, n, weights) \
  ( ((GdkGLProc_glTexFilterFuncSGIS) (proc)) (target, filter, n, weights) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_texture_filter4 GdkGL_GL_SGIS_texture_filter4;

struct _GdkGL_GL_SGIS_texture_filter4
{
  GdkGLProc_glGetTexFilterFuncSGIS glGetTexFilterFuncSGIS;
  GdkGLProc_glTexFilterFuncSGIS glTexFilterFuncSGIS;
};

GdkGL_GL_SGIS_texture_filter4 *gdk_gl_get_GL_SGIS_texture_filter4 (void);

/*
 * GL_EXT_subtexture
 */

/* glTexSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glTexSubImage1DEXT) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexSubImage1DEXT (void);
#define      gdk_gl_glTexSubImage1DEXT(proc, target, level, xoffset, width, format, type, pixels) \
  ( ((GdkGLProc_glTexSubImage1DEXT) (proc)) (target, level, xoffset, width, format, type, pixels) )

/* glTexSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glTexSubImage2DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexSubImage2DEXT (void);
#define      gdk_gl_glTexSubImage2DEXT(proc, target, level, xoffset, yoffset, width, height, format, type, pixels) \
  ( ((GdkGLProc_glTexSubImage2DEXT) (proc)) (target, level, xoffset, yoffset, width, height, format, type, pixels) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_subtexture GdkGL_GL_EXT_subtexture;

struct _GdkGL_GL_EXT_subtexture
{
  GdkGLProc_glTexSubImage1DEXT glTexSubImage1DEXT;
  GdkGLProc_glTexSubImage2DEXT glTexSubImage2DEXT;
};

GdkGL_GL_EXT_subtexture *gdk_gl_get_GL_EXT_subtexture (void);

/*
 * GL_EXT_copy_texture
 */

/* glCopyTexImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTexImage1DEXT) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GdkGLProc    gdk_gl_get_glCopyTexImage1DEXT (void);
#define      gdk_gl_glCopyTexImage1DEXT(proc, target, level, internalformat, x, y, width, border) \
  ( ((GdkGLProc_glCopyTexImage1DEXT) (proc)) (target, level, internalformat, x, y, width, border) )

/* glCopyTexImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTexImage2DEXT) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GdkGLProc    gdk_gl_get_glCopyTexImage2DEXT (void);
#define      gdk_gl_glCopyTexImage2DEXT(proc, target, level, internalformat, x, y, width, height, border) \
  ( ((GdkGLProc_glCopyTexImage2DEXT) (proc)) (target, level, internalformat, x, y, width, height, border) )

/* glCopyTexSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTexSubImage1DEXT) (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyTexSubImage1DEXT (void);
#define      gdk_gl_glCopyTexSubImage1DEXT(proc, target, level, xoffset, x, y, width) \
  ( ((GdkGLProc_glCopyTexSubImage1DEXT) (proc)) (target, level, xoffset, x, y, width) )

/* glCopyTexSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTexSubImage2DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyTexSubImage2DEXT (void);
#define      gdk_gl_glCopyTexSubImage2DEXT(proc, target, level, xoffset, yoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyTexSubImage2DEXT) (proc)) (target, level, xoffset, yoffset, x, y, width, height) )

/* glCopyTexSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTexSubImage3DEXT) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyTexSubImage3DEXT (void);
#define      gdk_gl_glCopyTexSubImage3DEXT(proc, target, level, xoffset, yoffset, zoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyTexSubImage3DEXT) (proc)) (target, level, xoffset, yoffset, zoffset, x, y, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_copy_texture GdkGL_GL_EXT_copy_texture;

struct _GdkGL_GL_EXT_copy_texture
{
  GdkGLProc_glCopyTexImage1DEXT glCopyTexImage1DEXT;
  GdkGLProc_glCopyTexImage2DEXT glCopyTexImage2DEXT;
  GdkGLProc_glCopyTexSubImage1DEXT glCopyTexSubImage1DEXT;
  GdkGLProc_glCopyTexSubImage2DEXT glCopyTexSubImage2DEXT;
  GdkGLProc_glCopyTexSubImage3DEXT glCopyTexSubImage3DEXT;
};

GdkGL_GL_EXT_copy_texture *gdk_gl_get_GL_EXT_copy_texture (void);

/*
 * GL_EXT_histogram
 */

/* glGetHistogramEXT */
typedef void (APIENTRYP GdkGLProc_glGetHistogramEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GdkGLProc    gdk_gl_get_glGetHistogramEXT (void);
#define      gdk_gl_glGetHistogramEXT(proc, target, reset, format, type, values) \
  ( ((GdkGLProc_glGetHistogramEXT) (proc)) (target, reset, format, type, values) )

/* glGetHistogramParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetHistogramParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetHistogramParameterfvEXT (void);
#define      gdk_gl_glGetHistogramParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetHistogramParameterfvEXT) (proc)) (target, pname, params) )

/* glGetHistogramParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetHistogramParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetHistogramParameterivEXT (void);
#define      gdk_gl_glGetHistogramParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetHistogramParameterivEXT) (proc)) (target, pname, params) )

/* glGetMinmaxEXT */
typedef void (APIENTRYP GdkGLProc_glGetMinmaxEXT) (GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
GdkGLProc    gdk_gl_get_glGetMinmaxEXT (void);
#define      gdk_gl_glGetMinmaxEXT(proc, target, reset, format, type, values) \
  ( ((GdkGLProc_glGetMinmaxEXT) (proc)) (target, reset, format, type, values) )

/* glGetMinmaxParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMinmaxParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMinmaxParameterfvEXT (void);
#define      gdk_gl_glGetMinmaxParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMinmaxParameterfvEXT) (proc)) (target, pname, params) )

/* glGetMinmaxParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMinmaxParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMinmaxParameterivEXT (void);
#define      gdk_gl_glGetMinmaxParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMinmaxParameterivEXT) (proc)) (target, pname, params) )

/* glHistogramEXT */
typedef void (APIENTRYP GdkGLProc_glHistogramEXT) (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
GdkGLProc    gdk_gl_get_glHistogramEXT (void);
#define      gdk_gl_glHistogramEXT(proc, target, width, internalformat, sink) \
  ( ((GdkGLProc_glHistogramEXT) (proc)) (target, width, internalformat, sink) )

/* glMinmaxEXT */
typedef void (APIENTRYP GdkGLProc_glMinmaxEXT) (GLenum target, GLenum internalformat, GLboolean sink);
GdkGLProc    gdk_gl_get_glMinmaxEXT (void);
#define      gdk_gl_glMinmaxEXT(proc, target, internalformat, sink) \
  ( ((GdkGLProc_glMinmaxEXT) (proc)) (target, internalformat, sink) )

/* glResetHistogramEXT */
typedef void (APIENTRYP GdkGLProc_glResetHistogramEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glResetHistogramEXT (void);
#define      gdk_gl_glResetHistogramEXT(proc, target) \
  ( ((GdkGLProc_glResetHistogramEXT) (proc)) (target) )

/* glResetMinmaxEXT */
typedef void (APIENTRYP GdkGLProc_glResetMinmaxEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glResetMinmaxEXT (void);
#define      gdk_gl_glResetMinmaxEXT(proc, target) \
  ( ((GdkGLProc_glResetMinmaxEXT) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_histogram GdkGL_GL_EXT_histogram;

struct _GdkGL_GL_EXT_histogram
{
  GdkGLProc_glGetHistogramEXT glGetHistogramEXT;
  GdkGLProc_glGetHistogramParameterfvEXT glGetHistogramParameterfvEXT;
  GdkGLProc_glGetHistogramParameterivEXT glGetHistogramParameterivEXT;
  GdkGLProc_glGetMinmaxEXT glGetMinmaxEXT;
  GdkGLProc_glGetMinmaxParameterfvEXT glGetMinmaxParameterfvEXT;
  GdkGLProc_glGetMinmaxParameterivEXT glGetMinmaxParameterivEXT;
  GdkGLProc_glHistogramEXT glHistogramEXT;
  GdkGLProc_glMinmaxEXT glMinmaxEXT;
  GdkGLProc_glResetHistogramEXT glResetHistogramEXT;
  GdkGLProc_glResetMinmaxEXT glResetMinmaxEXT;
};

GdkGL_GL_EXT_histogram *gdk_gl_get_GL_EXT_histogram (void);

/*
 * GL_EXT_convolution
 */

/* glConvolutionFilter1DEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionFilter1DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
GdkGLProc    gdk_gl_get_glConvolutionFilter1DEXT (void);
#define      gdk_gl_glConvolutionFilter1DEXT(proc, target, internalformat, width, format, type, image) \
  ( ((GdkGLProc_glConvolutionFilter1DEXT) (proc)) (target, internalformat, width, format, type, image) )

/* glConvolutionFilter2DEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionFilter2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
GdkGLProc    gdk_gl_get_glConvolutionFilter2DEXT (void);
#define      gdk_gl_glConvolutionFilter2DEXT(proc, target, internalformat, width, height, format, type, image) \
  ( ((GdkGLProc_glConvolutionFilter2DEXT) (proc)) (target, internalformat, width, height, format, type, image) )

/* glConvolutionParameterfEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameterfEXT) (GLenum target, GLenum pname, GLfloat params);
GdkGLProc    gdk_gl_get_glConvolutionParameterfEXT (void);
#define      gdk_gl_glConvolutionParameterfEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameterfEXT) (proc)) (target, pname, params) )

/* glConvolutionParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameterfvEXT) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glConvolutionParameterfvEXT (void);
#define      gdk_gl_glConvolutionParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameterfvEXT) (proc)) (target, pname, params) )

/* glConvolutionParameteriEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameteriEXT) (GLenum target, GLenum pname, GLint params);
GdkGLProc    gdk_gl_get_glConvolutionParameteriEXT (void);
#define      gdk_gl_glConvolutionParameteriEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameteriEXT) (proc)) (target, pname, params) )

/* glConvolutionParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glConvolutionParameterivEXT) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glConvolutionParameterivEXT (void);
#define      gdk_gl_glConvolutionParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glConvolutionParameterivEXT) (proc)) (target, pname, params) )

/* glCopyConvolutionFilter1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyConvolutionFilter1DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyConvolutionFilter1DEXT (void);
#define      gdk_gl_glCopyConvolutionFilter1DEXT(proc, target, internalformat, x, y, width) \
  ( ((GdkGLProc_glCopyConvolutionFilter1DEXT) (proc)) (target, internalformat, x, y, width) )

/* glCopyConvolutionFilter2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyConvolutionFilter2DEXT) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyConvolutionFilter2DEXT (void);
#define      gdk_gl_glCopyConvolutionFilter2DEXT(proc, target, internalformat, x, y, width, height) \
  ( ((GdkGLProc_glCopyConvolutionFilter2DEXT) (proc)) (target, internalformat, x, y, width, height) )

/* glGetConvolutionFilterEXT */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionFilterEXT) (GLenum target, GLenum format, GLenum type, GLvoid *image);
GdkGLProc    gdk_gl_get_glGetConvolutionFilterEXT (void);
#define      gdk_gl_glGetConvolutionFilterEXT(proc, target, format, type, image) \
  ( ((GdkGLProc_glGetConvolutionFilterEXT) (proc)) (target, format, type, image) )

/* glGetConvolutionParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetConvolutionParameterfvEXT (void);
#define      gdk_gl_glGetConvolutionParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetConvolutionParameterfvEXT) (proc)) (target, pname, params) )

/* glGetConvolutionParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetConvolutionParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetConvolutionParameterivEXT (void);
#define      gdk_gl_glGetConvolutionParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetConvolutionParameterivEXT) (proc)) (target, pname, params) )

/* glGetSeparableFilterEXT */
typedef void (APIENTRYP GdkGLProc_glGetSeparableFilterEXT) (GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
GdkGLProc    gdk_gl_get_glGetSeparableFilterEXT (void);
#define      gdk_gl_glGetSeparableFilterEXT(proc, target, format, type, row, column, span) \
  ( ((GdkGLProc_glGetSeparableFilterEXT) (proc)) (target, format, type, row, column, span) )

/* glSeparableFilter2DEXT */
typedef void (APIENTRYP GdkGLProc_glSeparableFilter2DEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
GdkGLProc    gdk_gl_get_glSeparableFilter2DEXT (void);
#define      gdk_gl_glSeparableFilter2DEXT(proc, target, internalformat, width, height, format, type, row, column) \
  ( ((GdkGLProc_glSeparableFilter2DEXT) (proc)) (target, internalformat, width, height, format, type, row, column) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_convolution GdkGL_GL_EXT_convolution;

struct _GdkGL_GL_EXT_convolution
{
  GdkGLProc_glConvolutionFilter1DEXT glConvolutionFilter1DEXT;
  GdkGLProc_glConvolutionFilter2DEXT glConvolutionFilter2DEXT;
  GdkGLProc_glConvolutionParameterfEXT glConvolutionParameterfEXT;
  GdkGLProc_glConvolutionParameterfvEXT glConvolutionParameterfvEXT;
  GdkGLProc_glConvolutionParameteriEXT glConvolutionParameteriEXT;
  GdkGLProc_glConvolutionParameterivEXT glConvolutionParameterivEXT;
  GdkGLProc_glCopyConvolutionFilter1DEXT glCopyConvolutionFilter1DEXT;
  GdkGLProc_glCopyConvolutionFilter2DEXT glCopyConvolutionFilter2DEXT;
  GdkGLProc_glGetConvolutionFilterEXT glGetConvolutionFilterEXT;
  GdkGLProc_glGetConvolutionParameterfvEXT glGetConvolutionParameterfvEXT;
  GdkGLProc_glGetConvolutionParameterivEXT glGetConvolutionParameterivEXT;
  GdkGLProc_glGetSeparableFilterEXT glGetSeparableFilterEXT;
  GdkGLProc_glSeparableFilter2DEXT glSeparableFilter2DEXT;
};

GdkGL_GL_EXT_convolution *gdk_gl_get_GL_EXT_convolution (void);

/*
 * GL_SGI_color_table
 */

/* glColorTableSGI */
typedef void (APIENTRYP GdkGLProc_glColorTableSGI) (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
GdkGLProc    gdk_gl_get_glColorTableSGI (void);
#define      gdk_gl_glColorTableSGI(proc, target, internalformat, width, format, type, table) \
  ( ((GdkGLProc_glColorTableSGI) (proc)) (target, internalformat, width, format, type, table) )

/* glColorTableParameterfvSGI */
typedef void (APIENTRYP GdkGLProc_glColorTableParameterfvSGI) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glColorTableParameterfvSGI (void);
#define      gdk_gl_glColorTableParameterfvSGI(proc, target, pname, params) \
  ( ((GdkGLProc_glColorTableParameterfvSGI) (proc)) (target, pname, params) )

/* glColorTableParameterivSGI */
typedef void (APIENTRYP GdkGLProc_glColorTableParameterivSGI) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glColorTableParameterivSGI (void);
#define      gdk_gl_glColorTableParameterivSGI(proc, target, pname, params) \
  ( ((GdkGLProc_glColorTableParameterivSGI) (proc)) (target, pname, params) )

/* glCopyColorTableSGI */
typedef void (APIENTRYP GdkGLProc_glCopyColorTableSGI) (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyColorTableSGI (void);
#define      gdk_gl_glCopyColorTableSGI(proc, target, internalformat, x, y, width) \
  ( ((GdkGLProc_glCopyColorTableSGI) (proc)) (target, internalformat, x, y, width) )

/* glGetColorTableSGI */
typedef void (APIENTRYP GdkGLProc_glGetColorTableSGI) (GLenum target, GLenum format, GLenum type, GLvoid *table);
GdkGLProc    gdk_gl_get_glGetColorTableSGI (void);
#define      gdk_gl_glGetColorTableSGI(proc, target, format, type, table) \
  ( ((GdkGLProc_glGetColorTableSGI) (proc)) (target, format, type, table) )

/* glGetColorTableParameterfvSGI */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameterfvSGI) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameterfvSGI (void);
#define      gdk_gl_glGetColorTableParameterfvSGI(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameterfvSGI) (proc)) (target, pname, params) )

/* glGetColorTableParameterivSGI */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameterivSGI) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameterivSGI (void);
#define      gdk_gl_glGetColorTableParameterivSGI(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameterivSGI) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGI_color_table GdkGL_GL_SGI_color_table;

struct _GdkGL_GL_SGI_color_table
{
  GdkGLProc_glColorTableSGI glColorTableSGI;
  GdkGLProc_glColorTableParameterfvSGI glColorTableParameterfvSGI;
  GdkGLProc_glColorTableParameterivSGI glColorTableParameterivSGI;
  GdkGLProc_glCopyColorTableSGI glCopyColorTableSGI;
  GdkGLProc_glGetColorTableSGI glGetColorTableSGI;
  GdkGLProc_glGetColorTableParameterfvSGI glGetColorTableParameterfvSGI;
  GdkGLProc_glGetColorTableParameterivSGI glGetColorTableParameterivSGI;
};

GdkGL_GL_SGI_color_table *gdk_gl_get_GL_SGI_color_table (void);

/*
 * GL_SGIX_pixel_texture
 */

/* glPixelTexGenSGIX */
typedef void (APIENTRYP GdkGLProc_glPixelTexGenSGIX) (GLenum mode);
GdkGLProc    gdk_gl_get_glPixelTexGenSGIX (void);
#define      gdk_gl_glPixelTexGenSGIX(proc, mode) \
  ( ((GdkGLProc_glPixelTexGenSGIX) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_pixel_texture GdkGL_GL_SGIX_pixel_texture;

struct _GdkGL_GL_SGIX_pixel_texture
{
  GdkGLProc_glPixelTexGenSGIX glPixelTexGenSGIX;
};

GdkGL_GL_SGIX_pixel_texture *gdk_gl_get_GL_SGIX_pixel_texture (void);

/*
 * GL_SGIS_pixel_texture
 */

/* glPixelTexGenParameteriSGIS */
typedef void (APIENTRYP GdkGLProc_glPixelTexGenParameteriSGIS) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPixelTexGenParameteriSGIS (void);
#define      gdk_gl_glPixelTexGenParameteriSGIS(proc, pname, param) \
  ( ((GdkGLProc_glPixelTexGenParameteriSGIS) (proc)) (pname, param) )

/* glPixelTexGenParameterivSGIS */
typedef void (APIENTRYP GdkGLProc_glPixelTexGenParameterivSGIS) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glPixelTexGenParameterivSGIS (void);
#define      gdk_gl_glPixelTexGenParameterivSGIS(proc, pname, params) \
  ( ((GdkGLProc_glPixelTexGenParameterivSGIS) (proc)) (pname, params) )

/* glPixelTexGenParameterfSGIS */
typedef void (APIENTRYP GdkGLProc_glPixelTexGenParameterfSGIS) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPixelTexGenParameterfSGIS (void);
#define      gdk_gl_glPixelTexGenParameterfSGIS(proc, pname, param) \
  ( ((GdkGLProc_glPixelTexGenParameterfSGIS) (proc)) (pname, param) )

/* glPixelTexGenParameterfvSGIS */
typedef void (APIENTRYP GdkGLProc_glPixelTexGenParameterfvSGIS) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPixelTexGenParameterfvSGIS (void);
#define      gdk_gl_glPixelTexGenParameterfvSGIS(proc, pname, params) \
  ( ((GdkGLProc_glPixelTexGenParameterfvSGIS) (proc)) (pname, params) )

/* glGetPixelTexGenParameterivSGIS */
typedef void (APIENTRYP GdkGLProc_glGetPixelTexGenParameterivSGIS) (GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetPixelTexGenParameterivSGIS (void);
#define      gdk_gl_glGetPixelTexGenParameterivSGIS(proc, pname, params) \
  ( ((GdkGLProc_glGetPixelTexGenParameterivSGIS) (proc)) (pname, params) )

/* glGetPixelTexGenParameterfvSGIS */
typedef void (APIENTRYP GdkGLProc_glGetPixelTexGenParameterfvSGIS) (GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetPixelTexGenParameterfvSGIS (void);
#define      gdk_gl_glGetPixelTexGenParameterfvSGIS(proc, pname, params) \
  ( ((GdkGLProc_glGetPixelTexGenParameterfvSGIS) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_pixel_texture GdkGL_GL_SGIS_pixel_texture;

struct _GdkGL_GL_SGIS_pixel_texture
{
  GdkGLProc_glPixelTexGenParameteriSGIS glPixelTexGenParameteriSGIS;
  GdkGLProc_glPixelTexGenParameterivSGIS glPixelTexGenParameterivSGIS;
  GdkGLProc_glPixelTexGenParameterfSGIS glPixelTexGenParameterfSGIS;
  GdkGLProc_glPixelTexGenParameterfvSGIS glPixelTexGenParameterfvSGIS;
  GdkGLProc_glGetPixelTexGenParameterivSGIS glGetPixelTexGenParameterivSGIS;
  GdkGLProc_glGetPixelTexGenParameterfvSGIS glGetPixelTexGenParameterfvSGIS;
};

GdkGL_GL_SGIS_pixel_texture *gdk_gl_get_GL_SGIS_pixel_texture (void);

/*
 * GL_SGIS_texture4D
 */

/* glTexImage4DSGIS */
typedef void (APIENTRYP GdkGLProc_glTexImage4DSGIS) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexImage4DSGIS (void);
#define      gdk_gl_glTexImage4DSGIS(proc, target, level, internalformat, width, height, depth, size4d, border, format, type, pixels) \
  ( ((GdkGLProc_glTexImage4DSGIS) (proc)) (target, level, internalformat, width, height, depth, size4d, border, format, type, pixels) )

/* glTexSubImage4DSGIS */
typedef void (APIENTRYP GdkGLProc_glTexSubImage4DSGIS) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTexSubImage4DSGIS (void);
#define      gdk_gl_glTexSubImage4DSGIS(proc, target, level, xoffset, yoffset, zoffset, woffset, width, height, depth, size4d, format, type, pixels) \
  ( ((GdkGLProc_glTexSubImage4DSGIS) (proc)) (target, level, xoffset, yoffset, zoffset, woffset, width, height, depth, size4d, format, type, pixels) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_texture4D GdkGL_GL_SGIS_texture4D;

struct _GdkGL_GL_SGIS_texture4D
{
  GdkGLProc_glTexImage4DSGIS glTexImage4DSGIS;
  GdkGLProc_glTexSubImage4DSGIS glTexSubImage4DSGIS;
};

GdkGL_GL_SGIS_texture4D *gdk_gl_get_GL_SGIS_texture4D (void);

/*
 * GL_EXT_texture_object
 */

/* glAreTexturesResidentEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glAreTexturesResidentEXT) (GLsizei n, const GLuint *textures, GLboolean *residences);
GdkGLProc    gdk_gl_get_glAreTexturesResidentEXT (void);
#define      gdk_gl_glAreTexturesResidentEXT(proc, n, textures, residences) \
  ( ((GdkGLProc_glAreTexturesResidentEXT) (proc)) (n, textures, residences) )

/* glBindTextureEXT */
typedef void (APIENTRYP GdkGLProc_glBindTextureEXT) (GLenum target, GLuint texture);
GdkGLProc    gdk_gl_get_glBindTextureEXT (void);
#define      gdk_gl_glBindTextureEXT(proc, target, texture) \
  ( ((GdkGLProc_glBindTextureEXT) (proc)) (target, texture) )

/* glDeleteTexturesEXT */
typedef void (APIENTRYP GdkGLProc_glDeleteTexturesEXT) (GLsizei n, const GLuint *textures);
GdkGLProc    gdk_gl_get_glDeleteTexturesEXT (void);
#define      gdk_gl_glDeleteTexturesEXT(proc, n, textures) \
  ( ((GdkGLProc_glDeleteTexturesEXT) (proc)) (n, textures) )

/* glGenTexturesEXT */
typedef void (APIENTRYP GdkGLProc_glGenTexturesEXT) (GLsizei n, GLuint *textures);
GdkGLProc    gdk_gl_get_glGenTexturesEXT (void);
#define      gdk_gl_glGenTexturesEXT(proc, n, textures) \
  ( ((GdkGLProc_glGenTexturesEXT) (proc)) (n, textures) )

/* glIsTextureEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glIsTextureEXT) (GLuint texture);
GdkGLProc    gdk_gl_get_glIsTextureEXT (void);
#define      gdk_gl_glIsTextureEXT(proc, texture) \
  ( ((GdkGLProc_glIsTextureEXT) (proc)) (texture) )

/* glPrioritizeTexturesEXT */
typedef void (APIENTRYP GdkGLProc_glPrioritizeTexturesEXT) (GLsizei n, const GLuint *textures, const GLclampf *priorities);
GdkGLProc    gdk_gl_get_glPrioritizeTexturesEXT (void);
#define      gdk_gl_glPrioritizeTexturesEXT(proc, n, textures, priorities) \
  ( ((GdkGLProc_glPrioritizeTexturesEXT) (proc)) (n, textures, priorities) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_texture_object GdkGL_GL_EXT_texture_object;

struct _GdkGL_GL_EXT_texture_object
{
  GdkGLProc_glAreTexturesResidentEXT glAreTexturesResidentEXT;
  GdkGLProc_glBindTextureEXT glBindTextureEXT;
  GdkGLProc_glDeleteTexturesEXT glDeleteTexturesEXT;
  GdkGLProc_glGenTexturesEXT glGenTexturesEXT;
  GdkGLProc_glIsTextureEXT glIsTextureEXT;
  GdkGLProc_glPrioritizeTexturesEXT glPrioritizeTexturesEXT;
};

GdkGL_GL_EXT_texture_object *gdk_gl_get_GL_EXT_texture_object (void);

/*
 * GL_SGIS_detail_texture
 */

/* glDetailTexFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glDetailTexFuncSGIS) (GLenum target, GLsizei n, const GLfloat *points);
GdkGLProc    gdk_gl_get_glDetailTexFuncSGIS (void);
#define      gdk_gl_glDetailTexFuncSGIS(proc, target, n, points) \
  ( ((GdkGLProc_glDetailTexFuncSGIS) (proc)) (target, n, points) )

/* glGetDetailTexFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glGetDetailTexFuncSGIS) (GLenum target, GLfloat *points);
GdkGLProc    gdk_gl_get_glGetDetailTexFuncSGIS (void);
#define      gdk_gl_glGetDetailTexFuncSGIS(proc, target, points) \
  ( ((GdkGLProc_glGetDetailTexFuncSGIS) (proc)) (target, points) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_detail_texture GdkGL_GL_SGIS_detail_texture;

struct _GdkGL_GL_SGIS_detail_texture
{
  GdkGLProc_glDetailTexFuncSGIS glDetailTexFuncSGIS;
  GdkGLProc_glGetDetailTexFuncSGIS glGetDetailTexFuncSGIS;
};

GdkGL_GL_SGIS_detail_texture *gdk_gl_get_GL_SGIS_detail_texture (void);

/*
 * GL_SGIS_sharpen_texture
 */

/* glSharpenTexFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glSharpenTexFuncSGIS) (GLenum target, GLsizei n, const GLfloat *points);
GdkGLProc    gdk_gl_get_glSharpenTexFuncSGIS (void);
#define      gdk_gl_glSharpenTexFuncSGIS(proc, target, n, points) \
  ( ((GdkGLProc_glSharpenTexFuncSGIS) (proc)) (target, n, points) )

/* glGetSharpenTexFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glGetSharpenTexFuncSGIS) (GLenum target, GLfloat *points);
GdkGLProc    gdk_gl_get_glGetSharpenTexFuncSGIS (void);
#define      gdk_gl_glGetSharpenTexFuncSGIS(proc, target, points) \
  ( ((GdkGLProc_glGetSharpenTexFuncSGIS) (proc)) (target, points) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_sharpen_texture GdkGL_GL_SGIS_sharpen_texture;

struct _GdkGL_GL_SGIS_sharpen_texture
{
  GdkGLProc_glSharpenTexFuncSGIS glSharpenTexFuncSGIS;
  GdkGLProc_glGetSharpenTexFuncSGIS glGetSharpenTexFuncSGIS;
};

GdkGL_GL_SGIS_sharpen_texture *gdk_gl_get_GL_SGIS_sharpen_texture (void);

/*
 * GL_SGIS_multisample
 */

/* glSampleMaskSGIS */
typedef void (APIENTRYP GdkGLProc_glSampleMaskSGIS) (GLclampf value, GLboolean invert);
GdkGLProc    gdk_gl_get_glSampleMaskSGIS (void);
#define      gdk_gl_glSampleMaskSGIS(proc, value, invert) \
  ( ((GdkGLProc_glSampleMaskSGIS) (proc)) (value, invert) )

/* glSamplePatternSGIS */
typedef void (APIENTRYP GdkGLProc_glSamplePatternSGIS) (GLenum pattern);
GdkGLProc    gdk_gl_get_glSamplePatternSGIS (void);
#define      gdk_gl_glSamplePatternSGIS(proc, pattern) \
  ( ((GdkGLProc_glSamplePatternSGIS) (proc)) (pattern) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_multisample GdkGL_GL_SGIS_multisample;

struct _GdkGL_GL_SGIS_multisample
{
  GdkGLProc_glSampleMaskSGIS glSampleMaskSGIS;
  GdkGLProc_glSamplePatternSGIS glSamplePatternSGIS;
};

GdkGL_GL_SGIS_multisample *gdk_gl_get_GL_SGIS_multisample (void);

/*
 * GL_EXT_vertex_array
 */

/* glArrayElementEXT */
typedef void (APIENTRYP GdkGLProc_glArrayElementEXT) (GLint i);
GdkGLProc    gdk_gl_get_glArrayElementEXT (void);
#define      gdk_gl_glArrayElementEXT(proc, i) \
  ( ((GdkGLProc_glArrayElementEXT) (proc)) (i) )

/* glColorPointerEXT */
typedef void (APIENTRYP GdkGLProc_glColorPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glColorPointerEXT (void);
#define      gdk_gl_glColorPointerEXT(proc, size, type, stride, count, pointer) \
  ( ((GdkGLProc_glColorPointerEXT) (proc)) (size, type, stride, count, pointer) )

/* glDrawArraysEXT */
typedef void (APIENTRYP GdkGLProc_glDrawArraysEXT) (GLenum mode, GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawArraysEXT (void);
#define      gdk_gl_glDrawArraysEXT(proc, mode, first, count) \
  ( ((GdkGLProc_glDrawArraysEXT) (proc)) (mode, first, count) )

/* glEdgeFlagPointerEXT */
typedef void (APIENTRYP GdkGLProc_glEdgeFlagPointerEXT) (GLsizei stride, GLsizei count, const GLboolean *pointer);
GdkGLProc    gdk_gl_get_glEdgeFlagPointerEXT (void);
#define      gdk_gl_glEdgeFlagPointerEXT(proc, stride, count, pointer) \
  ( ((GdkGLProc_glEdgeFlagPointerEXT) (proc)) (stride, count, pointer) )

/* glGetPointervEXT */
typedef void (APIENTRYP GdkGLProc_glGetPointervEXT) (GLenum pname, GLvoid* *params);
GdkGLProc    gdk_gl_get_glGetPointervEXT (void);
#define      gdk_gl_glGetPointervEXT(proc, pname, params) \
  ( ((GdkGLProc_glGetPointervEXT) (proc)) (pname, params) )

/* glIndexPointerEXT */
typedef void (APIENTRYP GdkGLProc_glIndexPointerEXT) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glIndexPointerEXT (void);
#define      gdk_gl_glIndexPointerEXT(proc, type, stride, count, pointer) \
  ( ((GdkGLProc_glIndexPointerEXT) (proc)) (type, stride, count, pointer) )

/* glNormalPointerEXT */
typedef void (APIENTRYP GdkGLProc_glNormalPointerEXT) (GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glNormalPointerEXT (void);
#define      gdk_gl_glNormalPointerEXT(proc, type, stride, count, pointer) \
  ( ((GdkGLProc_glNormalPointerEXT) (proc)) (type, stride, count, pointer) )

/* glTexCoordPointerEXT */
typedef void (APIENTRYP GdkGLProc_glTexCoordPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glTexCoordPointerEXT (void);
#define      gdk_gl_glTexCoordPointerEXT(proc, size, type, stride, count, pointer) \
  ( ((GdkGLProc_glTexCoordPointerEXT) (proc)) (size, type, stride, count, pointer) )

/* glVertexPointerEXT */
typedef void (APIENTRYP GdkGLProc_glVertexPointerEXT) (GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexPointerEXT (void);
#define      gdk_gl_glVertexPointerEXT(proc, size, type, stride, count, pointer) \
  ( ((GdkGLProc_glVertexPointerEXT) (proc)) (size, type, stride, count, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_vertex_array GdkGL_GL_EXT_vertex_array;

struct _GdkGL_GL_EXT_vertex_array
{
  GdkGLProc_glArrayElementEXT glArrayElementEXT;
  GdkGLProc_glColorPointerEXT glColorPointerEXT;
  GdkGLProc_glDrawArraysEXT glDrawArraysEXT;
  GdkGLProc_glEdgeFlagPointerEXT glEdgeFlagPointerEXT;
  GdkGLProc_glGetPointervEXT glGetPointervEXT;
  GdkGLProc_glIndexPointerEXT glIndexPointerEXT;
  GdkGLProc_glNormalPointerEXT glNormalPointerEXT;
  GdkGLProc_glTexCoordPointerEXT glTexCoordPointerEXT;
  GdkGLProc_glVertexPointerEXT glVertexPointerEXT;
};

GdkGL_GL_EXT_vertex_array *gdk_gl_get_GL_EXT_vertex_array (void);

/*
 * GL_EXT_blend_minmax
 */

/* glBlendEquationEXT */
typedef void (APIENTRYP GdkGLProc_glBlendEquationEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glBlendEquationEXT (void);
#define      gdk_gl_glBlendEquationEXT(proc, mode) \
  ( ((GdkGLProc_glBlendEquationEXT) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_blend_minmax GdkGL_GL_EXT_blend_minmax;

struct _GdkGL_GL_EXT_blend_minmax
{
  GdkGLProc_glBlendEquationEXT glBlendEquationEXT;
};

GdkGL_GL_EXT_blend_minmax *gdk_gl_get_GL_EXT_blend_minmax (void);

/*
 * GL_SGIX_sprite
 */

/* glSpriteParameterfSGIX */
typedef void (APIENTRYP GdkGLProc_glSpriteParameterfSGIX) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glSpriteParameterfSGIX (void);
#define      gdk_gl_glSpriteParameterfSGIX(proc, pname, param) \
  ( ((GdkGLProc_glSpriteParameterfSGIX) (proc)) (pname, param) )

/* glSpriteParameterfvSGIX */
typedef void (APIENTRYP GdkGLProc_glSpriteParameterfvSGIX) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glSpriteParameterfvSGIX (void);
#define      gdk_gl_glSpriteParameterfvSGIX(proc, pname, params) \
  ( ((GdkGLProc_glSpriteParameterfvSGIX) (proc)) (pname, params) )

/* glSpriteParameteriSGIX */
typedef void (APIENTRYP GdkGLProc_glSpriteParameteriSGIX) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glSpriteParameteriSGIX (void);
#define      gdk_gl_glSpriteParameteriSGIX(proc, pname, param) \
  ( ((GdkGLProc_glSpriteParameteriSGIX) (proc)) (pname, param) )

/* glSpriteParameterivSGIX */
typedef void (APIENTRYP GdkGLProc_glSpriteParameterivSGIX) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glSpriteParameterivSGIX (void);
#define      gdk_gl_glSpriteParameterivSGIX(proc, pname, params) \
  ( ((GdkGLProc_glSpriteParameterivSGIX) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_sprite GdkGL_GL_SGIX_sprite;

struct _GdkGL_GL_SGIX_sprite
{
  GdkGLProc_glSpriteParameterfSGIX glSpriteParameterfSGIX;
  GdkGLProc_glSpriteParameterfvSGIX glSpriteParameterfvSGIX;
  GdkGLProc_glSpriteParameteriSGIX glSpriteParameteriSGIX;
  GdkGLProc_glSpriteParameterivSGIX glSpriteParameterivSGIX;
};

GdkGL_GL_SGIX_sprite *gdk_gl_get_GL_SGIX_sprite (void);

/*
 * GL_EXT_point_parameters
 */

/* glPointParameterfEXT */
typedef void (APIENTRYP GdkGLProc_glPointParameterfEXT) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPointParameterfEXT (void);
#define      gdk_gl_glPointParameterfEXT(proc, pname, param) \
  ( ((GdkGLProc_glPointParameterfEXT) (proc)) (pname, param) )

/* glPointParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glPointParameterfvEXT) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPointParameterfvEXT (void);
#define      gdk_gl_glPointParameterfvEXT(proc, pname, params) \
  ( ((GdkGLProc_glPointParameterfvEXT) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_point_parameters GdkGL_GL_EXT_point_parameters;

struct _GdkGL_GL_EXT_point_parameters
{
  GdkGLProc_glPointParameterfEXT glPointParameterfEXT;
  GdkGLProc_glPointParameterfvEXT glPointParameterfvEXT;
};

GdkGL_GL_EXT_point_parameters *gdk_gl_get_GL_EXT_point_parameters (void);

/*
 * GL_SGIS_point_parameters
 */

/* glPointParameterfSGIS */
typedef void (APIENTRYP GdkGLProc_glPointParameterfSGIS) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPointParameterfSGIS (void);
#define      gdk_gl_glPointParameterfSGIS(proc, pname, param) \
  ( ((GdkGLProc_glPointParameterfSGIS) (proc)) (pname, param) )

/* glPointParameterfvSGIS */
typedef void (APIENTRYP GdkGLProc_glPointParameterfvSGIS) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPointParameterfvSGIS (void);
#define      gdk_gl_glPointParameterfvSGIS(proc, pname, params) \
  ( ((GdkGLProc_glPointParameterfvSGIS) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_point_parameters GdkGL_GL_SGIS_point_parameters;

struct _GdkGL_GL_SGIS_point_parameters
{
  GdkGLProc_glPointParameterfSGIS glPointParameterfSGIS;
  GdkGLProc_glPointParameterfvSGIS glPointParameterfvSGIS;
};

GdkGL_GL_SGIS_point_parameters *gdk_gl_get_GL_SGIS_point_parameters (void);

/*
 * GL_SGIX_instruments
 */

/* glGetInstrumentsSGIX */
typedef GLint (APIENTRYP GdkGLProc_glGetInstrumentsSGIX) (void);
GdkGLProc    gdk_gl_get_glGetInstrumentsSGIX (void);
#define      gdk_gl_glGetInstrumentsSGIX(proc) \
  ( ((GdkGLProc_glGetInstrumentsSGIX) (proc)) () )

/* glInstrumentsBufferSGIX */
typedef void (APIENTRYP GdkGLProc_glInstrumentsBufferSGIX) (GLsizei size, GLint *buffer);
GdkGLProc    gdk_gl_get_glInstrumentsBufferSGIX (void);
#define      gdk_gl_glInstrumentsBufferSGIX(proc, size, buffer) \
  ( ((GdkGLProc_glInstrumentsBufferSGIX) (proc)) (size, buffer) )

/* glPollInstrumentsSGIX */
typedef GLint (APIENTRYP GdkGLProc_glPollInstrumentsSGIX) (GLint *marker_p);
GdkGLProc    gdk_gl_get_glPollInstrumentsSGIX (void);
#define      gdk_gl_glPollInstrumentsSGIX(proc, marker_p) \
  ( ((GdkGLProc_glPollInstrumentsSGIX) (proc)) (marker_p) )

/* glReadInstrumentsSGIX */
typedef void (APIENTRYP GdkGLProc_glReadInstrumentsSGIX) (GLint marker);
GdkGLProc    gdk_gl_get_glReadInstrumentsSGIX (void);
#define      gdk_gl_glReadInstrumentsSGIX(proc, marker) \
  ( ((GdkGLProc_glReadInstrumentsSGIX) (proc)) (marker) )

/* glStartInstrumentsSGIX */
typedef void (APIENTRYP GdkGLProc_glStartInstrumentsSGIX) (void);
GdkGLProc    gdk_gl_get_glStartInstrumentsSGIX (void);
#define      gdk_gl_glStartInstrumentsSGIX(proc) \
  ( ((GdkGLProc_glStartInstrumentsSGIX) (proc)) () )

/* glStopInstrumentsSGIX */
typedef void (APIENTRYP GdkGLProc_glStopInstrumentsSGIX) (GLint marker);
GdkGLProc    gdk_gl_get_glStopInstrumentsSGIX (void);
#define      gdk_gl_glStopInstrumentsSGIX(proc, marker) \
  ( ((GdkGLProc_glStopInstrumentsSGIX) (proc)) (marker) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_instruments GdkGL_GL_SGIX_instruments;

struct _GdkGL_GL_SGIX_instruments
{
  GdkGLProc_glGetInstrumentsSGIX glGetInstrumentsSGIX;
  GdkGLProc_glInstrumentsBufferSGIX glInstrumentsBufferSGIX;
  GdkGLProc_glPollInstrumentsSGIX glPollInstrumentsSGIX;
  GdkGLProc_glReadInstrumentsSGIX glReadInstrumentsSGIX;
  GdkGLProc_glStartInstrumentsSGIX glStartInstrumentsSGIX;
  GdkGLProc_glStopInstrumentsSGIX glStopInstrumentsSGIX;
};

GdkGL_GL_SGIX_instruments *gdk_gl_get_GL_SGIX_instruments (void);

/*
 * GL_SGIX_framezoom
 */

/* glFrameZoomSGIX */
typedef void (APIENTRYP GdkGLProc_glFrameZoomSGIX) (GLint factor);
GdkGLProc    gdk_gl_get_glFrameZoomSGIX (void);
#define      gdk_gl_glFrameZoomSGIX(proc, factor) \
  ( ((GdkGLProc_glFrameZoomSGIX) (proc)) (factor) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_framezoom GdkGL_GL_SGIX_framezoom;

struct _GdkGL_GL_SGIX_framezoom
{
  GdkGLProc_glFrameZoomSGIX glFrameZoomSGIX;
};

GdkGL_GL_SGIX_framezoom *gdk_gl_get_GL_SGIX_framezoom (void);

/*
 * GL_SGIX_tag_sample_buffer
 */

/* glTagSampleBufferSGIX */
typedef void (APIENTRYP GdkGLProc_glTagSampleBufferSGIX) (void);
GdkGLProc    gdk_gl_get_glTagSampleBufferSGIX (void);
#define      gdk_gl_glTagSampleBufferSGIX(proc) \
  ( ((GdkGLProc_glTagSampleBufferSGIX) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_tag_sample_buffer GdkGL_GL_SGIX_tag_sample_buffer;

struct _GdkGL_GL_SGIX_tag_sample_buffer
{
  GdkGLProc_glTagSampleBufferSGIX glTagSampleBufferSGIX;
};

GdkGL_GL_SGIX_tag_sample_buffer *gdk_gl_get_GL_SGIX_tag_sample_buffer (void);

/*
 * GL_SGIX_polynomial_ffd
 */

/* glDeformationMap3dSGIX */
typedef void (APIENTRYP GdkGLProc_glDeformationMap3dSGIX) (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points);
GdkGLProc    gdk_gl_get_glDeformationMap3dSGIX (void);
#define      gdk_gl_glDeformationMap3dSGIX(proc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, w1, w2, wstride, worder, points) \
  ( ((GdkGLProc_glDeformationMap3dSGIX) (proc)) (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, w1, w2, wstride, worder, points) )

/* glDeformationMap3fSGIX */
typedef void (APIENTRYP GdkGLProc_glDeformationMap3fSGIX) (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points);
GdkGLProc    gdk_gl_get_glDeformationMap3fSGIX (void);
#define      gdk_gl_glDeformationMap3fSGIX(proc, target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, w1, w2, wstride, worder, points) \
  ( ((GdkGLProc_glDeformationMap3fSGIX) (proc)) (target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, w1, w2, wstride, worder, points) )

/* glDeformSGIX */
typedef void (APIENTRYP GdkGLProc_glDeformSGIX) (GLbitfield mask);
GdkGLProc    gdk_gl_get_glDeformSGIX (void);
#define      gdk_gl_glDeformSGIX(proc, mask) \
  ( ((GdkGLProc_glDeformSGIX) (proc)) (mask) )

/* glLoadIdentityDeformationMapSGIX */
typedef void (APIENTRYP GdkGLProc_glLoadIdentityDeformationMapSGIX) (GLbitfield mask);
GdkGLProc    gdk_gl_get_glLoadIdentityDeformationMapSGIX (void);
#define      gdk_gl_glLoadIdentityDeformationMapSGIX(proc, mask) \
  ( ((GdkGLProc_glLoadIdentityDeformationMapSGIX) (proc)) (mask) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_polynomial_ffd GdkGL_GL_SGIX_polynomial_ffd;

struct _GdkGL_GL_SGIX_polynomial_ffd
{
  GdkGLProc_glDeformationMap3dSGIX glDeformationMap3dSGIX;
  GdkGLProc_glDeformationMap3fSGIX glDeformationMap3fSGIX;
  GdkGLProc_glDeformSGIX glDeformSGIX;
  GdkGLProc_glLoadIdentityDeformationMapSGIX glLoadIdentityDeformationMapSGIX;
};

GdkGL_GL_SGIX_polynomial_ffd *gdk_gl_get_GL_SGIX_polynomial_ffd (void);

/*
 * GL_SGIX_reference_plane
 */

/* glReferencePlaneSGIX */
typedef void (APIENTRYP GdkGLProc_glReferencePlaneSGIX) (const GLdouble *equation);
GdkGLProc    gdk_gl_get_glReferencePlaneSGIX (void);
#define      gdk_gl_glReferencePlaneSGIX(proc, equation) \
  ( ((GdkGLProc_glReferencePlaneSGIX) (proc)) (equation) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_reference_plane GdkGL_GL_SGIX_reference_plane;

struct _GdkGL_GL_SGIX_reference_plane
{
  GdkGLProc_glReferencePlaneSGIX glReferencePlaneSGIX;
};

GdkGL_GL_SGIX_reference_plane *gdk_gl_get_GL_SGIX_reference_plane (void);

/*
 * GL_SGIX_flush_raster
 */

/* glFlushRasterSGIX */
typedef void (APIENTRYP GdkGLProc_glFlushRasterSGIX) (void);
GdkGLProc    gdk_gl_get_glFlushRasterSGIX (void);
#define      gdk_gl_glFlushRasterSGIX(proc) \
  ( ((GdkGLProc_glFlushRasterSGIX) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_flush_raster GdkGL_GL_SGIX_flush_raster;

struct _GdkGL_GL_SGIX_flush_raster
{
  GdkGLProc_glFlushRasterSGIX glFlushRasterSGIX;
};

GdkGL_GL_SGIX_flush_raster *gdk_gl_get_GL_SGIX_flush_raster (void);

/*
 * GL_SGIS_fog_function
 */

/* glFogFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glFogFuncSGIS) (GLsizei n, const GLfloat *points);
GdkGLProc    gdk_gl_get_glFogFuncSGIS (void);
#define      gdk_gl_glFogFuncSGIS(proc, n, points) \
  ( ((GdkGLProc_glFogFuncSGIS) (proc)) (n, points) )

/* glGetFogFuncSGIS */
typedef void (APIENTRYP GdkGLProc_glGetFogFuncSGIS) (GLfloat *points);
GdkGLProc    gdk_gl_get_glGetFogFuncSGIS (void);
#define      gdk_gl_glGetFogFuncSGIS(proc, points) \
  ( ((GdkGLProc_glGetFogFuncSGIS) (proc)) (points) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_fog_function GdkGL_GL_SGIS_fog_function;

struct _GdkGL_GL_SGIS_fog_function
{
  GdkGLProc_glFogFuncSGIS glFogFuncSGIS;
  GdkGLProc_glGetFogFuncSGIS glGetFogFuncSGIS;
};

GdkGL_GL_SGIS_fog_function *gdk_gl_get_GL_SGIS_fog_function (void);

/*
 * GL_HP_image_transform
 */

/* glImageTransformParameteriHP */
typedef void (APIENTRYP GdkGLProc_glImageTransformParameteriHP) (GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glImageTransformParameteriHP (void);
#define      gdk_gl_glImageTransformParameteriHP(proc, target, pname, param) \
  ( ((GdkGLProc_glImageTransformParameteriHP) (proc)) (target, pname, param) )

/* glImageTransformParameterfHP */
typedef void (APIENTRYP GdkGLProc_glImageTransformParameterfHP) (GLenum target, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glImageTransformParameterfHP (void);
#define      gdk_gl_glImageTransformParameterfHP(proc, target, pname, param) \
  ( ((GdkGLProc_glImageTransformParameterfHP) (proc)) (target, pname, param) )

/* glImageTransformParameterivHP */
typedef void (APIENTRYP GdkGLProc_glImageTransformParameterivHP) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glImageTransformParameterivHP (void);
#define      gdk_gl_glImageTransformParameterivHP(proc, target, pname, params) \
  ( ((GdkGLProc_glImageTransformParameterivHP) (proc)) (target, pname, params) )

/* glImageTransformParameterfvHP */
typedef void (APIENTRYP GdkGLProc_glImageTransformParameterfvHP) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glImageTransformParameterfvHP (void);
#define      gdk_gl_glImageTransformParameterfvHP(proc, target, pname, params) \
  ( ((GdkGLProc_glImageTransformParameterfvHP) (proc)) (target, pname, params) )

/* glGetImageTransformParameterivHP */
typedef void (APIENTRYP GdkGLProc_glGetImageTransformParameterivHP) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetImageTransformParameterivHP (void);
#define      gdk_gl_glGetImageTransformParameterivHP(proc, target, pname, params) \
  ( ((GdkGLProc_glGetImageTransformParameterivHP) (proc)) (target, pname, params) )

/* glGetImageTransformParameterfvHP */
typedef void (APIENTRYP GdkGLProc_glGetImageTransformParameterfvHP) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetImageTransformParameterfvHP (void);
#define      gdk_gl_glGetImageTransformParameterfvHP(proc, target, pname, params) \
  ( ((GdkGLProc_glGetImageTransformParameterfvHP) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_HP_image_transform GdkGL_GL_HP_image_transform;

struct _GdkGL_GL_HP_image_transform
{
  GdkGLProc_glImageTransformParameteriHP glImageTransformParameteriHP;
  GdkGLProc_glImageTransformParameterfHP glImageTransformParameterfHP;
  GdkGLProc_glImageTransformParameterivHP glImageTransformParameterivHP;
  GdkGLProc_glImageTransformParameterfvHP glImageTransformParameterfvHP;
  GdkGLProc_glGetImageTransformParameterivHP glGetImageTransformParameterivHP;
  GdkGLProc_glGetImageTransformParameterfvHP glGetImageTransformParameterfvHP;
};

GdkGL_GL_HP_image_transform *gdk_gl_get_GL_HP_image_transform (void);

/*
 * GL_EXT_color_subtable
 */

/* glColorSubTableEXT */
typedef void (APIENTRYP GdkGLProc_glColorSubTableEXT) (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
GdkGLProc    gdk_gl_get_glColorSubTableEXT (void);
#define      gdk_gl_glColorSubTableEXT(proc, target, start, count, format, type, data) \
  ( ((GdkGLProc_glColorSubTableEXT) (proc)) (target, start, count, format, type, data) )

/* glCopyColorSubTableEXT */
typedef void (APIENTRYP GdkGLProc_glCopyColorSubTableEXT) (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyColorSubTableEXT (void);
#define      gdk_gl_glCopyColorSubTableEXT(proc, target, start, x, y, width) \
  ( ((GdkGLProc_glCopyColorSubTableEXT) (proc)) (target, start, x, y, width) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_color_subtable GdkGL_GL_EXT_color_subtable;

struct _GdkGL_GL_EXT_color_subtable
{
  GdkGLProc_glColorSubTableEXT glColorSubTableEXT;
  GdkGLProc_glCopyColorSubTableEXT glCopyColorSubTableEXT;
};

GdkGL_GL_EXT_color_subtable *gdk_gl_get_GL_EXT_color_subtable (void);

/*
 * GL_PGI_misc_hints
 */

/* glHintPGI */
typedef void (APIENTRYP GdkGLProc_glHintPGI) (GLenum target, GLint mode);
GdkGLProc    gdk_gl_get_glHintPGI (void);
#define      gdk_gl_glHintPGI(proc, target, mode) \
  ( ((GdkGLProc_glHintPGI) (proc)) (target, mode) )

/* proc struct */

typedef struct _GdkGL_GL_PGI_misc_hints GdkGL_GL_PGI_misc_hints;

struct _GdkGL_GL_PGI_misc_hints
{
  GdkGLProc_glHintPGI glHintPGI;
};

GdkGL_GL_PGI_misc_hints *gdk_gl_get_GL_PGI_misc_hints (void);

/*
 * GL_EXT_paletted_texture
 */

/* glColorTableEXT */
typedef void (APIENTRYP GdkGLProc_glColorTableEXT) (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);
GdkGLProc    gdk_gl_get_glColorTableEXT (void);
#define      gdk_gl_glColorTableEXT(proc, target, internalFormat, width, format, type, table) \
  ( ((GdkGLProc_glColorTableEXT) (proc)) (target, internalFormat, width, format, type, table) )

/* glGetColorTableEXT */
typedef void (APIENTRYP GdkGLProc_glGetColorTableEXT) (GLenum target, GLenum format, GLenum type, GLvoid *data);
GdkGLProc    gdk_gl_get_glGetColorTableEXT (void);
#define      gdk_gl_glGetColorTableEXT(proc, target, format, type, data) \
  ( ((GdkGLProc_glGetColorTableEXT) (proc)) (target, format, type, data) )

/* glGetColorTableParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameterivEXT (void);
#define      gdk_gl_glGetColorTableParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameterivEXT) (proc)) (target, pname, params) )

/* glGetColorTableParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetColorTableParameterfvEXT) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetColorTableParameterfvEXT (void);
#define      gdk_gl_glGetColorTableParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetColorTableParameterfvEXT) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_paletted_texture GdkGL_GL_EXT_paletted_texture;

struct _GdkGL_GL_EXT_paletted_texture
{
  GdkGLProc_glColorTableEXT glColorTableEXT;
  GdkGLProc_glGetColorTableEXT glGetColorTableEXT;
  GdkGLProc_glGetColorTableParameterivEXT glGetColorTableParameterivEXT;
  GdkGLProc_glGetColorTableParameterfvEXT glGetColorTableParameterfvEXT;
};

GdkGL_GL_EXT_paletted_texture *gdk_gl_get_GL_EXT_paletted_texture (void);

/*
 * GL_SGIX_list_priority
 */

/* glGetListParameterfvSGIX */
typedef void (APIENTRYP GdkGLProc_glGetListParameterfvSGIX) (GLuint list, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetListParameterfvSGIX (void);
#define      gdk_gl_glGetListParameterfvSGIX(proc, list, pname, params) \
  ( ((GdkGLProc_glGetListParameterfvSGIX) (proc)) (list, pname, params) )

/* glGetListParameterivSGIX */
typedef void (APIENTRYP GdkGLProc_glGetListParameterivSGIX) (GLuint list, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetListParameterivSGIX (void);
#define      gdk_gl_glGetListParameterivSGIX(proc, list, pname, params) \
  ( ((GdkGLProc_glGetListParameterivSGIX) (proc)) (list, pname, params) )

/* glListParameterfSGIX */
typedef void (APIENTRYP GdkGLProc_glListParameterfSGIX) (GLuint list, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glListParameterfSGIX (void);
#define      gdk_gl_glListParameterfSGIX(proc, list, pname, param) \
  ( ((GdkGLProc_glListParameterfSGIX) (proc)) (list, pname, param) )

/* glListParameterfvSGIX */
typedef void (APIENTRYP GdkGLProc_glListParameterfvSGIX) (GLuint list, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glListParameterfvSGIX (void);
#define      gdk_gl_glListParameterfvSGIX(proc, list, pname, params) \
  ( ((GdkGLProc_glListParameterfvSGIX) (proc)) (list, pname, params) )

/* glListParameteriSGIX */
typedef void (APIENTRYP GdkGLProc_glListParameteriSGIX) (GLuint list, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glListParameteriSGIX (void);
#define      gdk_gl_glListParameteriSGIX(proc, list, pname, param) \
  ( ((GdkGLProc_glListParameteriSGIX) (proc)) (list, pname, param) )

/* glListParameterivSGIX */
typedef void (APIENTRYP GdkGLProc_glListParameterivSGIX) (GLuint list, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glListParameterivSGIX (void);
#define      gdk_gl_glListParameterivSGIX(proc, list, pname, params) \
  ( ((GdkGLProc_glListParameterivSGIX) (proc)) (list, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_list_priority GdkGL_GL_SGIX_list_priority;

struct _GdkGL_GL_SGIX_list_priority
{
  GdkGLProc_glGetListParameterfvSGIX glGetListParameterfvSGIX;
  GdkGLProc_glGetListParameterivSGIX glGetListParameterivSGIX;
  GdkGLProc_glListParameterfSGIX glListParameterfSGIX;
  GdkGLProc_glListParameterfvSGIX glListParameterfvSGIX;
  GdkGLProc_glListParameteriSGIX glListParameteriSGIX;
  GdkGLProc_glListParameterivSGIX glListParameterivSGIX;
};

GdkGL_GL_SGIX_list_priority *gdk_gl_get_GL_SGIX_list_priority (void);

/*
 * GL_EXT_index_material
 */

/* glIndexMaterialEXT */
typedef void (APIENTRYP GdkGLProc_glIndexMaterialEXT) (GLenum face, GLenum mode);
GdkGLProc    gdk_gl_get_glIndexMaterialEXT (void);
#define      gdk_gl_glIndexMaterialEXT(proc, face, mode) \
  ( ((GdkGLProc_glIndexMaterialEXT) (proc)) (face, mode) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_index_material GdkGL_GL_EXT_index_material;

struct _GdkGL_GL_EXT_index_material
{
  GdkGLProc_glIndexMaterialEXT glIndexMaterialEXT;
};

GdkGL_GL_EXT_index_material *gdk_gl_get_GL_EXT_index_material (void);

/*
 * GL_EXT_index_func
 */

/* glIndexFuncEXT */
typedef void (APIENTRYP GdkGLProc_glIndexFuncEXT) (GLenum func, GLclampf ref);
GdkGLProc    gdk_gl_get_glIndexFuncEXT (void);
#define      gdk_gl_glIndexFuncEXT(proc, func, ref) \
  ( ((GdkGLProc_glIndexFuncEXT) (proc)) (func, ref) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_index_func GdkGL_GL_EXT_index_func;

struct _GdkGL_GL_EXT_index_func
{
  GdkGLProc_glIndexFuncEXT glIndexFuncEXT;
};

GdkGL_GL_EXT_index_func *gdk_gl_get_GL_EXT_index_func (void);

/*
 * GL_EXT_compiled_vertex_array
 */

/* glLockArraysEXT */
typedef void (APIENTRYP GdkGLProc_glLockArraysEXT) (GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glLockArraysEXT (void);
#define      gdk_gl_glLockArraysEXT(proc, first, count) \
  ( ((GdkGLProc_glLockArraysEXT) (proc)) (first, count) )

/* glUnlockArraysEXT */
typedef void (APIENTRYP GdkGLProc_glUnlockArraysEXT) (void);
GdkGLProc    gdk_gl_get_glUnlockArraysEXT (void);
#define      gdk_gl_glUnlockArraysEXT(proc) \
  ( ((GdkGLProc_glUnlockArraysEXT) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_EXT_compiled_vertex_array GdkGL_GL_EXT_compiled_vertex_array;

struct _GdkGL_GL_EXT_compiled_vertex_array
{
  GdkGLProc_glLockArraysEXT glLockArraysEXT;
  GdkGLProc_glUnlockArraysEXT glUnlockArraysEXT;
};

GdkGL_GL_EXT_compiled_vertex_array *gdk_gl_get_GL_EXT_compiled_vertex_array (void);

/*
 * GL_EXT_cull_vertex
 */

/* glCullParameterdvEXT */
typedef void (APIENTRYP GdkGLProc_glCullParameterdvEXT) (GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glCullParameterdvEXT (void);
#define      gdk_gl_glCullParameterdvEXT(proc, pname, params) \
  ( ((GdkGLProc_glCullParameterdvEXT) (proc)) (pname, params) )

/* glCullParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glCullParameterfvEXT) (GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glCullParameterfvEXT (void);
#define      gdk_gl_glCullParameterfvEXT(proc, pname, params) \
  ( ((GdkGLProc_glCullParameterfvEXT) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_cull_vertex GdkGL_GL_EXT_cull_vertex;

struct _GdkGL_GL_EXT_cull_vertex
{
  GdkGLProc_glCullParameterdvEXT glCullParameterdvEXT;
  GdkGLProc_glCullParameterfvEXT glCullParameterfvEXT;
};

GdkGL_GL_EXT_cull_vertex *gdk_gl_get_GL_EXT_cull_vertex (void);

/*
 * GL_SGIX_fragment_lighting
 */

/* glFragmentColorMaterialSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentColorMaterialSGIX) (GLenum face, GLenum mode);
GdkGLProc    gdk_gl_get_glFragmentColorMaterialSGIX (void);
#define      gdk_gl_glFragmentColorMaterialSGIX(proc, face, mode) \
  ( ((GdkGLProc_glFragmentColorMaterialSGIX) (proc)) (face, mode) )

/* glFragmentLightfSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightfSGIX) (GLenum light, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentLightfSGIX (void);
#define      gdk_gl_glFragmentLightfSGIX(proc, light, pname, param) \
  ( ((GdkGLProc_glFragmentLightfSGIX) (proc)) (light, pname, param) )

/* glFragmentLightfvSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightfvSGIX) (GLenum light, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentLightfvSGIX (void);
#define      gdk_gl_glFragmentLightfvSGIX(proc, light, pname, params) \
  ( ((GdkGLProc_glFragmentLightfvSGIX) (proc)) (light, pname, params) )

/* glFragmentLightiSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightiSGIX) (GLenum light, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glFragmentLightiSGIX (void);
#define      gdk_gl_glFragmentLightiSGIX(proc, light, pname, param) \
  ( ((GdkGLProc_glFragmentLightiSGIX) (proc)) (light, pname, param) )

/* glFragmentLightivSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightivSGIX) (GLenum light, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glFragmentLightivSGIX (void);
#define      gdk_gl_glFragmentLightivSGIX(proc, light, pname, params) \
  ( ((GdkGLProc_glFragmentLightivSGIX) (proc)) (light, pname, params) )

/* glFragmentLightModelfSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightModelfSGIX) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentLightModelfSGIX (void);
#define      gdk_gl_glFragmentLightModelfSGIX(proc, pname, param) \
  ( ((GdkGLProc_glFragmentLightModelfSGIX) (proc)) (pname, param) )

/* glFragmentLightModelfvSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightModelfvSGIX) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentLightModelfvSGIX (void);
#define      gdk_gl_glFragmentLightModelfvSGIX(proc, pname, params) \
  ( ((GdkGLProc_glFragmentLightModelfvSGIX) (proc)) (pname, params) )

/* glFragmentLightModeliSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightModeliSGIX) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glFragmentLightModeliSGIX (void);
#define      gdk_gl_glFragmentLightModeliSGIX(proc, pname, param) \
  ( ((GdkGLProc_glFragmentLightModeliSGIX) (proc)) (pname, param) )

/* glFragmentLightModelivSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentLightModelivSGIX) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glFragmentLightModelivSGIX (void);
#define      gdk_gl_glFragmentLightModelivSGIX(proc, pname, params) \
  ( ((GdkGLProc_glFragmentLightModelivSGIX) (proc)) (pname, params) )

/* glFragmentMaterialfSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentMaterialfSGIX) (GLenum face, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentMaterialfSGIX (void);
#define      gdk_gl_glFragmentMaterialfSGIX(proc, face, pname, param) \
  ( ((GdkGLProc_glFragmentMaterialfSGIX) (proc)) (face, pname, param) )

/* glFragmentMaterialfvSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentMaterialfvSGIX) (GLenum face, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentMaterialfvSGIX (void);
#define      gdk_gl_glFragmentMaterialfvSGIX(proc, face, pname, params) \
  ( ((GdkGLProc_glFragmentMaterialfvSGIX) (proc)) (face, pname, params) )

/* glFragmentMaterialiSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentMaterialiSGIX) (GLenum face, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glFragmentMaterialiSGIX (void);
#define      gdk_gl_glFragmentMaterialiSGIX(proc, face, pname, param) \
  ( ((GdkGLProc_glFragmentMaterialiSGIX) (proc)) (face, pname, param) )

/* glFragmentMaterialivSGIX */
typedef void (APIENTRYP GdkGLProc_glFragmentMaterialivSGIX) (GLenum face, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glFragmentMaterialivSGIX (void);
#define      gdk_gl_glFragmentMaterialivSGIX(proc, face, pname, params) \
  ( ((GdkGLProc_glFragmentMaterialivSGIX) (proc)) (face, pname, params) )

/* glGetFragmentLightfvSGIX */
typedef void (APIENTRYP GdkGLProc_glGetFragmentLightfvSGIX) (GLenum light, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetFragmentLightfvSGIX (void);
#define      gdk_gl_glGetFragmentLightfvSGIX(proc, light, pname, params) \
  ( ((GdkGLProc_glGetFragmentLightfvSGIX) (proc)) (light, pname, params) )

/* glGetFragmentLightivSGIX */
typedef void (APIENTRYP GdkGLProc_glGetFragmentLightivSGIX) (GLenum light, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFragmentLightivSGIX (void);
#define      gdk_gl_glGetFragmentLightivSGIX(proc, light, pname, params) \
  ( ((GdkGLProc_glGetFragmentLightivSGIX) (proc)) (light, pname, params) )

/* glGetFragmentMaterialfvSGIX */
typedef void (APIENTRYP GdkGLProc_glGetFragmentMaterialfvSGIX) (GLenum face, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetFragmentMaterialfvSGIX (void);
#define      gdk_gl_glGetFragmentMaterialfvSGIX(proc, face, pname, params) \
  ( ((GdkGLProc_glGetFragmentMaterialfvSGIX) (proc)) (face, pname, params) )

/* glGetFragmentMaterialivSGIX */
typedef void (APIENTRYP GdkGLProc_glGetFragmentMaterialivSGIX) (GLenum face, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFragmentMaterialivSGIX (void);
#define      gdk_gl_glGetFragmentMaterialivSGIX(proc, face, pname, params) \
  ( ((GdkGLProc_glGetFragmentMaterialivSGIX) (proc)) (face, pname, params) )

/* glLightEnviSGIX */
typedef void (APIENTRYP GdkGLProc_glLightEnviSGIX) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glLightEnviSGIX (void);
#define      gdk_gl_glLightEnviSGIX(proc, pname, param) \
  ( ((GdkGLProc_glLightEnviSGIX) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_fragment_lighting GdkGL_GL_SGIX_fragment_lighting;

struct _GdkGL_GL_SGIX_fragment_lighting
{
  GdkGLProc_glFragmentColorMaterialSGIX glFragmentColorMaterialSGIX;
  GdkGLProc_glFragmentLightfSGIX glFragmentLightfSGIX;
  GdkGLProc_glFragmentLightfvSGIX glFragmentLightfvSGIX;
  GdkGLProc_glFragmentLightiSGIX glFragmentLightiSGIX;
  GdkGLProc_glFragmentLightivSGIX glFragmentLightivSGIX;
  GdkGLProc_glFragmentLightModelfSGIX glFragmentLightModelfSGIX;
  GdkGLProc_glFragmentLightModelfvSGIX glFragmentLightModelfvSGIX;
  GdkGLProc_glFragmentLightModeliSGIX glFragmentLightModeliSGIX;
  GdkGLProc_glFragmentLightModelivSGIX glFragmentLightModelivSGIX;
  GdkGLProc_glFragmentMaterialfSGIX glFragmentMaterialfSGIX;
  GdkGLProc_glFragmentMaterialfvSGIX glFragmentMaterialfvSGIX;
  GdkGLProc_glFragmentMaterialiSGIX glFragmentMaterialiSGIX;
  GdkGLProc_glFragmentMaterialivSGIX glFragmentMaterialivSGIX;
  GdkGLProc_glGetFragmentLightfvSGIX glGetFragmentLightfvSGIX;
  GdkGLProc_glGetFragmentLightivSGIX glGetFragmentLightivSGIX;
  GdkGLProc_glGetFragmentMaterialfvSGIX glGetFragmentMaterialfvSGIX;
  GdkGLProc_glGetFragmentMaterialivSGIX glGetFragmentMaterialivSGIX;
  GdkGLProc_glLightEnviSGIX glLightEnviSGIX;
};

GdkGL_GL_SGIX_fragment_lighting *gdk_gl_get_GL_SGIX_fragment_lighting (void);

/*
 * GL_EXT_draw_range_elements
 */

/* glDrawRangeElementsEXT */
typedef void (APIENTRYP GdkGLProc_glDrawRangeElementsEXT) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
GdkGLProc    gdk_gl_get_glDrawRangeElementsEXT (void);
#define      gdk_gl_glDrawRangeElementsEXT(proc, mode, start, end, count, type, indices) \
  ( ((GdkGLProc_glDrawRangeElementsEXT) (proc)) (mode, start, end, count, type, indices) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_draw_range_elements GdkGL_GL_EXT_draw_range_elements;

struct _GdkGL_GL_EXT_draw_range_elements
{
  GdkGLProc_glDrawRangeElementsEXT glDrawRangeElementsEXT;
};

GdkGL_GL_EXT_draw_range_elements *gdk_gl_get_GL_EXT_draw_range_elements (void);

/*
 * GL_EXT_light_texture
 */

/* glApplyTextureEXT */
typedef void (APIENTRYP GdkGLProc_glApplyTextureEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glApplyTextureEXT (void);
#define      gdk_gl_glApplyTextureEXT(proc, mode) \
  ( ((GdkGLProc_glApplyTextureEXT) (proc)) (mode) )

/* glTextureLightEXT */
typedef void (APIENTRYP GdkGLProc_glTextureLightEXT) (GLenum pname);
GdkGLProc    gdk_gl_get_glTextureLightEXT (void);
#define      gdk_gl_glTextureLightEXT(proc, pname) \
  ( ((GdkGLProc_glTextureLightEXT) (proc)) (pname) )

/* glTextureMaterialEXT */
typedef void (APIENTRYP GdkGLProc_glTextureMaterialEXT) (GLenum face, GLenum mode);
GdkGLProc    gdk_gl_get_glTextureMaterialEXT (void);
#define      gdk_gl_glTextureMaterialEXT(proc, face, mode) \
  ( ((GdkGLProc_glTextureMaterialEXT) (proc)) (face, mode) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_light_texture GdkGL_GL_EXT_light_texture;

struct _GdkGL_GL_EXT_light_texture
{
  GdkGLProc_glApplyTextureEXT glApplyTextureEXT;
  GdkGLProc_glTextureLightEXT glTextureLightEXT;
  GdkGLProc_glTextureMaterialEXT glTextureMaterialEXT;
};

GdkGL_GL_EXT_light_texture *gdk_gl_get_GL_EXT_light_texture (void);

/*
 * GL_SGIX_async
 */

/* glAsyncMarkerSGIX */
typedef void (APIENTRYP GdkGLProc_glAsyncMarkerSGIX) (GLuint marker);
GdkGLProc    gdk_gl_get_glAsyncMarkerSGIX (void);
#define      gdk_gl_glAsyncMarkerSGIX(proc, marker) \
  ( ((GdkGLProc_glAsyncMarkerSGIX) (proc)) (marker) )

/* glFinishAsyncSGIX */
typedef GLint (APIENTRYP GdkGLProc_glFinishAsyncSGIX) (GLuint *markerp);
GdkGLProc    gdk_gl_get_glFinishAsyncSGIX (void);
#define      gdk_gl_glFinishAsyncSGIX(proc, markerp) \
  ( ((GdkGLProc_glFinishAsyncSGIX) (proc)) (markerp) )

/* glPollAsyncSGIX */
typedef GLint (APIENTRYP GdkGLProc_glPollAsyncSGIX) (GLuint *markerp);
GdkGLProc    gdk_gl_get_glPollAsyncSGIX (void);
#define      gdk_gl_glPollAsyncSGIX(proc, markerp) \
  ( ((GdkGLProc_glPollAsyncSGIX) (proc)) (markerp) )

/* glGenAsyncMarkersSGIX */
typedef GLuint (APIENTRYP GdkGLProc_glGenAsyncMarkersSGIX) (GLsizei range);
GdkGLProc    gdk_gl_get_glGenAsyncMarkersSGIX (void);
#define      gdk_gl_glGenAsyncMarkersSGIX(proc, range) \
  ( ((GdkGLProc_glGenAsyncMarkersSGIX) (proc)) (range) )

/* glDeleteAsyncMarkersSGIX */
typedef void (APIENTRYP GdkGLProc_glDeleteAsyncMarkersSGIX) (GLuint marker, GLsizei range);
GdkGLProc    gdk_gl_get_glDeleteAsyncMarkersSGIX (void);
#define      gdk_gl_glDeleteAsyncMarkersSGIX(proc, marker, range) \
  ( ((GdkGLProc_glDeleteAsyncMarkersSGIX) (proc)) (marker, range) )

/* glIsAsyncMarkerSGIX */
typedef GLboolean (APIENTRYP GdkGLProc_glIsAsyncMarkerSGIX) (GLuint marker);
GdkGLProc    gdk_gl_get_glIsAsyncMarkerSGIX (void);
#define      gdk_gl_glIsAsyncMarkerSGIX(proc, marker) \
  ( ((GdkGLProc_glIsAsyncMarkerSGIX) (proc)) (marker) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_async GdkGL_GL_SGIX_async;

struct _GdkGL_GL_SGIX_async
{
  GdkGLProc_glAsyncMarkerSGIX glAsyncMarkerSGIX;
  GdkGLProc_glFinishAsyncSGIX glFinishAsyncSGIX;
  GdkGLProc_glPollAsyncSGIX glPollAsyncSGIX;
  GdkGLProc_glGenAsyncMarkersSGIX glGenAsyncMarkersSGIX;
  GdkGLProc_glDeleteAsyncMarkersSGIX glDeleteAsyncMarkersSGIX;
  GdkGLProc_glIsAsyncMarkerSGIX glIsAsyncMarkerSGIX;
};

GdkGL_GL_SGIX_async *gdk_gl_get_GL_SGIX_async (void);

/*
 * GL_INTEL_parallel_arrays
 */

/* glVertexPointervINTEL */
typedef void (APIENTRYP GdkGLProc_glVertexPointervINTEL) (GLint size, GLenum type, const GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glVertexPointervINTEL (void);
#define      gdk_gl_glVertexPointervINTEL(proc, size, type, pointer) \
  ( ((GdkGLProc_glVertexPointervINTEL) (proc)) (size, type, pointer) )

/* glNormalPointervINTEL */
typedef void (APIENTRYP GdkGLProc_glNormalPointervINTEL) (GLenum type, const GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glNormalPointervINTEL (void);
#define      gdk_gl_glNormalPointervINTEL(proc, type, pointer) \
  ( ((GdkGLProc_glNormalPointervINTEL) (proc)) (type, pointer) )

/* glColorPointervINTEL */
typedef void (APIENTRYP GdkGLProc_glColorPointervINTEL) (GLint size, GLenum type, const GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glColorPointervINTEL (void);
#define      gdk_gl_glColorPointervINTEL(proc, size, type, pointer) \
  ( ((GdkGLProc_glColorPointervINTEL) (proc)) (size, type, pointer) )

/* glTexCoordPointervINTEL */
typedef void (APIENTRYP GdkGLProc_glTexCoordPointervINTEL) (GLint size, GLenum type, const GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glTexCoordPointervINTEL (void);
#define      gdk_gl_glTexCoordPointervINTEL(proc, size, type, pointer) \
  ( ((GdkGLProc_glTexCoordPointervINTEL) (proc)) (size, type, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_INTEL_parallel_arrays GdkGL_GL_INTEL_parallel_arrays;

struct _GdkGL_GL_INTEL_parallel_arrays
{
  GdkGLProc_glVertexPointervINTEL glVertexPointervINTEL;
  GdkGLProc_glNormalPointervINTEL glNormalPointervINTEL;
  GdkGLProc_glColorPointervINTEL glColorPointervINTEL;
  GdkGLProc_glTexCoordPointervINTEL glTexCoordPointervINTEL;
};

GdkGL_GL_INTEL_parallel_arrays *gdk_gl_get_GL_INTEL_parallel_arrays (void);

/*
 * GL_EXT_pixel_transform
 */

/* glPixelTransformParameteriEXT */
typedef void (APIENTRYP GdkGLProc_glPixelTransformParameteriEXT) (GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPixelTransformParameteriEXT (void);
#define      gdk_gl_glPixelTransformParameteriEXT(proc, target, pname, param) \
  ( ((GdkGLProc_glPixelTransformParameteriEXT) (proc)) (target, pname, param) )

/* glPixelTransformParameterfEXT */
typedef void (APIENTRYP GdkGLProc_glPixelTransformParameterfEXT) (GLenum target, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPixelTransformParameterfEXT (void);
#define      gdk_gl_glPixelTransformParameterfEXT(proc, target, pname, param) \
  ( ((GdkGLProc_glPixelTransformParameterfEXT) (proc)) (target, pname, param) )

/* glPixelTransformParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glPixelTransformParameterivEXT) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glPixelTransformParameterivEXT (void);
#define      gdk_gl_glPixelTransformParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glPixelTransformParameterivEXT) (proc)) (target, pname, params) )

/* glPixelTransformParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glPixelTransformParameterfvEXT) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glPixelTransformParameterfvEXT (void);
#define      gdk_gl_glPixelTransformParameterfvEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glPixelTransformParameterfvEXT) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_pixel_transform GdkGL_GL_EXT_pixel_transform;

struct _GdkGL_GL_EXT_pixel_transform
{
  GdkGLProc_glPixelTransformParameteriEXT glPixelTransformParameteriEXT;
  GdkGLProc_glPixelTransformParameterfEXT glPixelTransformParameterfEXT;
  GdkGLProc_glPixelTransformParameterivEXT glPixelTransformParameterivEXT;
  GdkGLProc_glPixelTransformParameterfvEXT glPixelTransformParameterfvEXT;
};

GdkGL_GL_EXT_pixel_transform *gdk_gl_get_GL_EXT_pixel_transform (void);

/*
 * GL_EXT_secondary_color
 */

/* glSecondaryColor3bEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3bEXT) (GLbyte red, GLbyte green, GLbyte blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3bEXT (void);
#define      gdk_gl_glSecondaryColor3bEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3bEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3bvEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3bvEXT) (const GLbyte *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3bvEXT (void);
#define      gdk_gl_glSecondaryColor3bvEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3bvEXT) (proc)) (v) )

/* glSecondaryColor3dEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3dEXT) (GLdouble red, GLdouble green, GLdouble blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3dEXT (void);
#define      gdk_gl_glSecondaryColor3dEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3dEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3dvEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3dvEXT) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3dvEXT (void);
#define      gdk_gl_glSecondaryColor3dvEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3dvEXT) (proc)) (v) )

/* glSecondaryColor3fEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3fEXT) (GLfloat red, GLfloat green, GLfloat blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3fEXT (void);
#define      gdk_gl_glSecondaryColor3fEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3fEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3fvEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3fvEXT) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3fvEXT (void);
#define      gdk_gl_glSecondaryColor3fvEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3fvEXT) (proc)) (v) )

/* glSecondaryColor3iEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3iEXT) (GLint red, GLint green, GLint blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3iEXT (void);
#define      gdk_gl_glSecondaryColor3iEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3iEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3ivEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ivEXT) (const GLint *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3ivEXT (void);
#define      gdk_gl_glSecondaryColor3ivEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3ivEXT) (proc)) (v) )

/* glSecondaryColor3sEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3sEXT) (GLshort red, GLshort green, GLshort blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3sEXT (void);
#define      gdk_gl_glSecondaryColor3sEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3sEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3svEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3svEXT) (const GLshort *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3svEXT (void);
#define      gdk_gl_glSecondaryColor3svEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3svEXT) (proc)) (v) )

/* glSecondaryColor3ubEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ubEXT) (GLubyte red, GLubyte green, GLubyte blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3ubEXT (void);
#define      gdk_gl_glSecondaryColor3ubEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3ubEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3ubvEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3ubvEXT) (const GLubyte *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3ubvEXT (void);
#define      gdk_gl_glSecondaryColor3ubvEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3ubvEXT) (proc)) (v) )

/* glSecondaryColor3uiEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3uiEXT) (GLuint red, GLuint green, GLuint blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3uiEXT (void);
#define      gdk_gl_glSecondaryColor3uiEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3uiEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3uivEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3uivEXT) (const GLuint *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3uivEXT (void);
#define      gdk_gl_glSecondaryColor3uivEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3uivEXT) (proc)) (v) )

/* glSecondaryColor3usEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3usEXT) (GLushort red, GLushort green, GLushort blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3usEXT (void);
#define      gdk_gl_glSecondaryColor3usEXT(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3usEXT) (proc)) (red, green, blue) )

/* glSecondaryColor3usvEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3usvEXT) (const GLushort *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3usvEXT (void);
#define      gdk_gl_glSecondaryColor3usvEXT(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3usvEXT) (proc)) (v) )

/* glSecondaryColorPointerEXT */
typedef void (APIENTRYP GdkGLProc_glSecondaryColorPointerEXT) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glSecondaryColorPointerEXT (void);
#define      gdk_gl_glSecondaryColorPointerEXT(proc, size, type, stride, pointer) \
  ( ((GdkGLProc_glSecondaryColorPointerEXT) (proc)) (size, type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_secondary_color GdkGL_GL_EXT_secondary_color;

struct _GdkGL_GL_EXT_secondary_color
{
  GdkGLProc_glSecondaryColor3bEXT glSecondaryColor3bEXT;
  GdkGLProc_glSecondaryColor3bvEXT glSecondaryColor3bvEXT;
  GdkGLProc_glSecondaryColor3dEXT glSecondaryColor3dEXT;
  GdkGLProc_glSecondaryColor3dvEXT glSecondaryColor3dvEXT;
  GdkGLProc_glSecondaryColor3fEXT glSecondaryColor3fEXT;
  GdkGLProc_glSecondaryColor3fvEXT glSecondaryColor3fvEXT;
  GdkGLProc_glSecondaryColor3iEXT glSecondaryColor3iEXT;
  GdkGLProc_glSecondaryColor3ivEXT glSecondaryColor3ivEXT;
  GdkGLProc_glSecondaryColor3sEXT glSecondaryColor3sEXT;
  GdkGLProc_glSecondaryColor3svEXT glSecondaryColor3svEXT;
  GdkGLProc_glSecondaryColor3ubEXT glSecondaryColor3ubEXT;
  GdkGLProc_glSecondaryColor3ubvEXT glSecondaryColor3ubvEXT;
  GdkGLProc_glSecondaryColor3uiEXT glSecondaryColor3uiEXT;
  GdkGLProc_glSecondaryColor3uivEXT glSecondaryColor3uivEXT;
  GdkGLProc_glSecondaryColor3usEXT glSecondaryColor3usEXT;
  GdkGLProc_glSecondaryColor3usvEXT glSecondaryColor3usvEXT;
  GdkGLProc_glSecondaryColorPointerEXT glSecondaryColorPointerEXT;
};

GdkGL_GL_EXT_secondary_color *gdk_gl_get_GL_EXT_secondary_color (void);

/*
 * GL_EXT_texture_perturb_normal
 */

/* glTextureNormalEXT */
typedef void (APIENTRYP GdkGLProc_glTextureNormalEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glTextureNormalEXT (void);
#define      gdk_gl_glTextureNormalEXT(proc, mode) \
  ( ((GdkGLProc_glTextureNormalEXT) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_texture_perturb_normal GdkGL_GL_EXT_texture_perturb_normal;

struct _GdkGL_GL_EXT_texture_perturb_normal
{
  GdkGLProc_glTextureNormalEXT glTextureNormalEXT;
};

GdkGL_GL_EXT_texture_perturb_normal *gdk_gl_get_GL_EXT_texture_perturb_normal (void);

/*
 * GL_EXT_multi_draw_arrays
 */

/* glMultiDrawArraysEXT */
typedef void (APIENTRYP GdkGLProc_glMultiDrawArraysEXT) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawArraysEXT (void);
#define      gdk_gl_glMultiDrawArraysEXT(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawArraysEXT) (proc)) (mode, first, count, primcount) )

/* glMultiDrawElementsEXT */
typedef void (APIENTRYP GdkGLProc_glMultiDrawElementsEXT) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawElementsEXT (void);
#define      gdk_gl_glMultiDrawElementsEXT(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glMultiDrawElementsEXT) (proc)) (mode, count, type, indices, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_multi_draw_arrays GdkGL_GL_EXT_multi_draw_arrays;

struct _GdkGL_GL_EXT_multi_draw_arrays
{
  GdkGLProc_glMultiDrawArraysEXT glMultiDrawArraysEXT;
  GdkGLProc_glMultiDrawElementsEXT glMultiDrawElementsEXT;
};

GdkGL_GL_EXT_multi_draw_arrays *gdk_gl_get_GL_EXT_multi_draw_arrays (void);

/*
 * GL_EXT_fog_coord
 */

/* glFogCoordfEXT */
typedef void (APIENTRYP GdkGLProc_glFogCoordfEXT) (GLfloat coord);
GdkGLProc    gdk_gl_get_glFogCoordfEXT (void);
#define      gdk_gl_glFogCoordfEXT(proc, coord) \
  ( ((GdkGLProc_glFogCoordfEXT) (proc)) (coord) )

/* glFogCoordfvEXT */
typedef void (APIENTRYP GdkGLProc_glFogCoordfvEXT) (const GLfloat *coord);
GdkGLProc    gdk_gl_get_glFogCoordfvEXT (void);
#define      gdk_gl_glFogCoordfvEXT(proc, coord) \
  ( ((GdkGLProc_glFogCoordfvEXT) (proc)) (coord) )

/* glFogCoorddEXT */
typedef void (APIENTRYP GdkGLProc_glFogCoorddEXT) (GLdouble coord);
GdkGLProc    gdk_gl_get_glFogCoorddEXT (void);
#define      gdk_gl_glFogCoorddEXT(proc, coord) \
  ( ((GdkGLProc_glFogCoorddEXT) (proc)) (coord) )

/* glFogCoorddvEXT */
typedef void (APIENTRYP GdkGLProc_glFogCoorddvEXT) (const GLdouble *coord);
GdkGLProc    gdk_gl_get_glFogCoorddvEXT (void);
#define      gdk_gl_glFogCoorddvEXT(proc, coord) \
  ( ((GdkGLProc_glFogCoorddvEXT) (proc)) (coord) )

/* glFogCoordPointerEXT */
typedef void (APIENTRYP GdkGLProc_glFogCoordPointerEXT) (GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glFogCoordPointerEXT (void);
#define      gdk_gl_glFogCoordPointerEXT(proc, type, stride, pointer) \
  ( ((GdkGLProc_glFogCoordPointerEXT) (proc)) (type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_fog_coord GdkGL_GL_EXT_fog_coord;

struct _GdkGL_GL_EXT_fog_coord
{
  GdkGLProc_glFogCoordfEXT glFogCoordfEXT;
  GdkGLProc_glFogCoordfvEXT glFogCoordfvEXT;
  GdkGLProc_glFogCoorddEXT glFogCoorddEXT;
  GdkGLProc_glFogCoorddvEXT glFogCoorddvEXT;
  GdkGLProc_glFogCoordPointerEXT glFogCoordPointerEXT;
};

GdkGL_GL_EXT_fog_coord *gdk_gl_get_GL_EXT_fog_coord (void);

/*
 * GL_EXT_coordinate_frame
 */

/* glTangent3bEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3bEXT) (GLbyte tx, GLbyte ty, GLbyte tz);
GdkGLProc    gdk_gl_get_glTangent3bEXT (void);
#define      gdk_gl_glTangent3bEXT(proc, tx, ty, tz) \
  ( ((GdkGLProc_glTangent3bEXT) (proc)) (tx, ty, tz) )

/* glTangent3bvEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3bvEXT) (const GLbyte *v);
GdkGLProc    gdk_gl_get_glTangent3bvEXT (void);
#define      gdk_gl_glTangent3bvEXT(proc, v) \
  ( ((GdkGLProc_glTangent3bvEXT) (proc)) (v) )

/* glTangent3dEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3dEXT) (GLdouble tx, GLdouble ty, GLdouble tz);
GdkGLProc    gdk_gl_get_glTangent3dEXT (void);
#define      gdk_gl_glTangent3dEXT(proc, tx, ty, tz) \
  ( ((GdkGLProc_glTangent3dEXT) (proc)) (tx, ty, tz) )

/* glTangent3dvEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3dvEXT) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glTangent3dvEXT (void);
#define      gdk_gl_glTangent3dvEXT(proc, v) \
  ( ((GdkGLProc_glTangent3dvEXT) (proc)) (v) )

/* glTangent3fEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3fEXT) (GLfloat tx, GLfloat ty, GLfloat tz);
GdkGLProc    gdk_gl_get_glTangent3fEXT (void);
#define      gdk_gl_glTangent3fEXT(proc, tx, ty, tz) \
  ( ((GdkGLProc_glTangent3fEXT) (proc)) (tx, ty, tz) )

/* glTangent3fvEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3fvEXT) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glTangent3fvEXT (void);
#define      gdk_gl_glTangent3fvEXT(proc, v) \
  ( ((GdkGLProc_glTangent3fvEXT) (proc)) (v) )

/* glTangent3iEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3iEXT) (GLint tx, GLint ty, GLint tz);
GdkGLProc    gdk_gl_get_glTangent3iEXT (void);
#define      gdk_gl_glTangent3iEXT(proc, tx, ty, tz) \
  ( ((GdkGLProc_glTangent3iEXT) (proc)) (tx, ty, tz) )

/* glTangent3ivEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3ivEXT) (const GLint *v);
GdkGLProc    gdk_gl_get_glTangent3ivEXT (void);
#define      gdk_gl_glTangent3ivEXT(proc, v) \
  ( ((GdkGLProc_glTangent3ivEXT) (proc)) (v) )

/* glTangent3sEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3sEXT) (GLshort tx, GLshort ty, GLshort tz);
GdkGLProc    gdk_gl_get_glTangent3sEXT (void);
#define      gdk_gl_glTangent3sEXT(proc, tx, ty, tz) \
  ( ((GdkGLProc_glTangent3sEXT) (proc)) (tx, ty, tz) )

/* glTangent3svEXT */
typedef void (APIENTRYP GdkGLProc_glTangent3svEXT) (const GLshort *v);
GdkGLProc    gdk_gl_get_glTangent3svEXT (void);
#define      gdk_gl_glTangent3svEXT(proc, v) \
  ( ((GdkGLProc_glTangent3svEXT) (proc)) (v) )

/* glBinormal3bEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3bEXT) (GLbyte bx, GLbyte by, GLbyte bz);
GdkGLProc    gdk_gl_get_glBinormal3bEXT (void);
#define      gdk_gl_glBinormal3bEXT(proc, bx, by, bz) \
  ( ((GdkGLProc_glBinormal3bEXT) (proc)) (bx, by, bz) )

/* glBinormal3bvEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3bvEXT) (const GLbyte *v);
GdkGLProc    gdk_gl_get_glBinormal3bvEXT (void);
#define      gdk_gl_glBinormal3bvEXT(proc, v) \
  ( ((GdkGLProc_glBinormal3bvEXT) (proc)) (v) )

/* glBinormal3dEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3dEXT) (GLdouble bx, GLdouble by, GLdouble bz);
GdkGLProc    gdk_gl_get_glBinormal3dEXT (void);
#define      gdk_gl_glBinormal3dEXT(proc, bx, by, bz) \
  ( ((GdkGLProc_glBinormal3dEXT) (proc)) (bx, by, bz) )

/* glBinormal3dvEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3dvEXT) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glBinormal3dvEXT (void);
#define      gdk_gl_glBinormal3dvEXT(proc, v) \
  ( ((GdkGLProc_glBinormal3dvEXT) (proc)) (v) )

/* glBinormal3fEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3fEXT) (GLfloat bx, GLfloat by, GLfloat bz);
GdkGLProc    gdk_gl_get_glBinormal3fEXT (void);
#define      gdk_gl_glBinormal3fEXT(proc, bx, by, bz) \
  ( ((GdkGLProc_glBinormal3fEXT) (proc)) (bx, by, bz) )

/* glBinormal3fvEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3fvEXT) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glBinormal3fvEXT (void);
#define      gdk_gl_glBinormal3fvEXT(proc, v) \
  ( ((GdkGLProc_glBinormal3fvEXT) (proc)) (v) )

/* glBinormal3iEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3iEXT) (GLint bx, GLint by, GLint bz);
GdkGLProc    gdk_gl_get_glBinormal3iEXT (void);
#define      gdk_gl_glBinormal3iEXT(proc, bx, by, bz) \
  ( ((GdkGLProc_glBinormal3iEXT) (proc)) (bx, by, bz) )

/* glBinormal3ivEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3ivEXT) (const GLint *v);
GdkGLProc    gdk_gl_get_glBinormal3ivEXT (void);
#define      gdk_gl_glBinormal3ivEXT(proc, v) \
  ( ((GdkGLProc_glBinormal3ivEXT) (proc)) (v) )

/* glBinormal3sEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3sEXT) (GLshort bx, GLshort by, GLshort bz);
GdkGLProc    gdk_gl_get_glBinormal3sEXT (void);
#define      gdk_gl_glBinormal3sEXT(proc, bx, by, bz) \
  ( ((GdkGLProc_glBinormal3sEXT) (proc)) (bx, by, bz) )

/* glBinormal3svEXT */
typedef void (APIENTRYP GdkGLProc_glBinormal3svEXT) (const GLshort *v);
GdkGLProc    gdk_gl_get_glBinormal3svEXT (void);
#define      gdk_gl_glBinormal3svEXT(proc, v) \
  ( ((GdkGLProc_glBinormal3svEXT) (proc)) (v) )

/* glTangentPointerEXT */
typedef void (APIENTRYP GdkGLProc_glTangentPointerEXT) (GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glTangentPointerEXT (void);
#define      gdk_gl_glTangentPointerEXT(proc, type, stride, pointer) \
  ( ((GdkGLProc_glTangentPointerEXT) (proc)) (type, stride, pointer) )

/* glBinormalPointerEXT */
typedef void (APIENTRYP GdkGLProc_glBinormalPointerEXT) (GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glBinormalPointerEXT (void);
#define      gdk_gl_glBinormalPointerEXT(proc, type, stride, pointer) \
  ( ((GdkGLProc_glBinormalPointerEXT) (proc)) (type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_coordinate_frame GdkGL_GL_EXT_coordinate_frame;

struct _GdkGL_GL_EXT_coordinate_frame
{
  GdkGLProc_glTangent3bEXT glTangent3bEXT;
  GdkGLProc_glTangent3bvEXT glTangent3bvEXT;
  GdkGLProc_glTangent3dEXT glTangent3dEXT;
  GdkGLProc_glTangent3dvEXT glTangent3dvEXT;
  GdkGLProc_glTangent3fEXT glTangent3fEXT;
  GdkGLProc_glTangent3fvEXT glTangent3fvEXT;
  GdkGLProc_glTangent3iEXT glTangent3iEXT;
  GdkGLProc_glTangent3ivEXT glTangent3ivEXT;
  GdkGLProc_glTangent3sEXT glTangent3sEXT;
  GdkGLProc_glTangent3svEXT glTangent3svEXT;
  GdkGLProc_glBinormal3bEXT glBinormal3bEXT;
  GdkGLProc_glBinormal3bvEXT glBinormal3bvEXT;
  GdkGLProc_glBinormal3dEXT glBinormal3dEXT;
  GdkGLProc_glBinormal3dvEXT glBinormal3dvEXT;
  GdkGLProc_glBinormal3fEXT glBinormal3fEXT;
  GdkGLProc_glBinormal3fvEXT glBinormal3fvEXT;
  GdkGLProc_glBinormal3iEXT glBinormal3iEXT;
  GdkGLProc_glBinormal3ivEXT glBinormal3ivEXT;
  GdkGLProc_glBinormal3sEXT glBinormal3sEXT;
  GdkGLProc_glBinormal3svEXT glBinormal3svEXT;
  GdkGLProc_glTangentPointerEXT glTangentPointerEXT;
  GdkGLProc_glBinormalPointerEXT glBinormalPointerEXT;
};

GdkGL_GL_EXT_coordinate_frame *gdk_gl_get_GL_EXT_coordinate_frame (void);

/*
 * GL_SUNX_constant_data
 */

/* glFinishTextureSUNX */
typedef void (APIENTRYP GdkGLProc_glFinishTextureSUNX) (void);
GdkGLProc    gdk_gl_get_glFinishTextureSUNX (void);
#define      gdk_gl_glFinishTextureSUNX(proc) \
  ( ((GdkGLProc_glFinishTextureSUNX) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_SUNX_constant_data GdkGL_GL_SUNX_constant_data;

struct _GdkGL_GL_SUNX_constant_data
{
  GdkGLProc_glFinishTextureSUNX glFinishTextureSUNX;
};

GdkGL_GL_SUNX_constant_data *gdk_gl_get_GL_SUNX_constant_data (void);

/*
 * GL_SUN_global_alpha
 */

/* glGlobalAlphaFactorbSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactorbSUN) (GLbyte factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactorbSUN (void);
#define      gdk_gl_glGlobalAlphaFactorbSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactorbSUN) (proc)) (factor) )

/* glGlobalAlphaFactorsSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactorsSUN) (GLshort factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactorsSUN (void);
#define      gdk_gl_glGlobalAlphaFactorsSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactorsSUN) (proc)) (factor) )

/* glGlobalAlphaFactoriSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactoriSUN) (GLint factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactoriSUN (void);
#define      gdk_gl_glGlobalAlphaFactoriSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactoriSUN) (proc)) (factor) )

/* glGlobalAlphaFactorfSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactorfSUN) (GLfloat factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactorfSUN (void);
#define      gdk_gl_glGlobalAlphaFactorfSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactorfSUN) (proc)) (factor) )

/* glGlobalAlphaFactordSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactordSUN) (GLdouble factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactordSUN (void);
#define      gdk_gl_glGlobalAlphaFactordSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactordSUN) (proc)) (factor) )

/* glGlobalAlphaFactorubSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactorubSUN) (GLubyte factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactorubSUN (void);
#define      gdk_gl_glGlobalAlphaFactorubSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactorubSUN) (proc)) (factor) )

/* glGlobalAlphaFactorusSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactorusSUN) (GLushort factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactorusSUN (void);
#define      gdk_gl_glGlobalAlphaFactorusSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactorusSUN) (proc)) (factor) )

/* glGlobalAlphaFactoruiSUN */
typedef void (APIENTRYP GdkGLProc_glGlobalAlphaFactoruiSUN) (GLuint factor);
GdkGLProc    gdk_gl_get_glGlobalAlphaFactoruiSUN (void);
#define      gdk_gl_glGlobalAlphaFactoruiSUN(proc, factor) \
  ( ((GdkGLProc_glGlobalAlphaFactoruiSUN) (proc)) (factor) )

/* proc struct */

typedef struct _GdkGL_GL_SUN_global_alpha GdkGL_GL_SUN_global_alpha;

struct _GdkGL_GL_SUN_global_alpha
{
  GdkGLProc_glGlobalAlphaFactorbSUN glGlobalAlphaFactorbSUN;
  GdkGLProc_glGlobalAlphaFactorsSUN glGlobalAlphaFactorsSUN;
  GdkGLProc_glGlobalAlphaFactoriSUN glGlobalAlphaFactoriSUN;
  GdkGLProc_glGlobalAlphaFactorfSUN glGlobalAlphaFactorfSUN;
  GdkGLProc_glGlobalAlphaFactordSUN glGlobalAlphaFactordSUN;
  GdkGLProc_glGlobalAlphaFactorubSUN glGlobalAlphaFactorubSUN;
  GdkGLProc_glGlobalAlphaFactorusSUN glGlobalAlphaFactorusSUN;
  GdkGLProc_glGlobalAlphaFactoruiSUN glGlobalAlphaFactoruiSUN;
};

GdkGL_GL_SUN_global_alpha *gdk_gl_get_GL_SUN_global_alpha (void);

/*
 * GL_SUN_triangle_list
 */

/* glReplacementCodeuiSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiSUN) (GLuint code);
GdkGLProc    gdk_gl_get_glReplacementCodeuiSUN (void);
#define      gdk_gl_glReplacementCodeuiSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeuiSUN) (proc)) (code) )

/* glReplacementCodeusSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeusSUN) (GLushort code);
GdkGLProc    gdk_gl_get_glReplacementCodeusSUN (void);
#define      gdk_gl_glReplacementCodeusSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeusSUN) (proc)) (code) )

/* glReplacementCodeubSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeubSUN) (GLubyte code);
GdkGLProc    gdk_gl_get_glReplacementCodeubSUN (void);
#define      gdk_gl_glReplacementCodeubSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeubSUN) (proc)) (code) )

/* glReplacementCodeuivSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuivSUN) (const GLuint *code);
GdkGLProc    gdk_gl_get_glReplacementCodeuivSUN (void);
#define      gdk_gl_glReplacementCodeuivSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeuivSUN) (proc)) (code) )

/* glReplacementCodeusvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeusvSUN) (const GLushort *code);
GdkGLProc    gdk_gl_get_glReplacementCodeusvSUN (void);
#define      gdk_gl_glReplacementCodeusvSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeusvSUN) (proc)) (code) )

/* glReplacementCodeubvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeubvSUN) (const GLubyte *code);
GdkGLProc    gdk_gl_get_glReplacementCodeubvSUN (void);
#define      gdk_gl_glReplacementCodeubvSUN(proc, code) \
  ( ((GdkGLProc_glReplacementCodeubvSUN) (proc)) (code) )

/* glReplacementCodePointerSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodePointerSUN) (GLenum type, GLsizei stride, const GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glReplacementCodePointerSUN (void);
#define      gdk_gl_glReplacementCodePointerSUN(proc, type, stride, pointer) \
  ( ((GdkGLProc_glReplacementCodePointerSUN) (proc)) (type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_SUN_triangle_list GdkGL_GL_SUN_triangle_list;

struct _GdkGL_GL_SUN_triangle_list
{
  GdkGLProc_glReplacementCodeuiSUN glReplacementCodeuiSUN;
  GdkGLProc_glReplacementCodeusSUN glReplacementCodeusSUN;
  GdkGLProc_glReplacementCodeubSUN glReplacementCodeubSUN;
  GdkGLProc_glReplacementCodeuivSUN glReplacementCodeuivSUN;
  GdkGLProc_glReplacementCodeusvSUN glReplacementCodeusvSUN;
  GdkGLProc_glReplacementCodeubvSUN glReplacementCodeubvSUN;
  GdkGLProc_glReplacementCodePointerSUN glReplacementCodePointerSUN;
};

GdkGL_GL_SUN_triangle_list *gdk_gl_get_GL_SUN_triangle_list (void);

/*
 * GL_SUN_vertex
 */

/* glColor4ubVertex2fSUN */
typedef void (APIENTRYP GdkGLProc_glColor4ubVertex2fSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glColor4ubVertex2fSUN (void);
#define      gdk_gl_glColor4ubVertex2fSUN(proc, r, g, b, a, x, y) \
  ( ((GdkGLProc_glColor4ubVertex2fSUN) (proc)) (r, g, b, a, x, y) )

/* glColor4ubVertex2fvSUN */
typedef void (APIENTRYP GdkGLProc_glColor4ubVertex2fvSUN) (const GLubyte *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glColor4ubVertex2fvSUN (void);
#define      gdk_gl_glColor4ubVertex2fvSUN(proc, c, v) \
  ( ((GdkGLProc_glColor4ubVertex2fvSUN) (proc)) (c, v) )

/* glColor4ubVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glColor4ubVertex3fSUN) (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glColor4ubVertex3fSUN (void);
#define      gdk_gl_glColor4ubVertex3fSUN(proc, r, g, b, a, x, y, z) \
  ( ((GdkGLProc_glColor4ubVertex3fSUN) (proc)) (r, g, b, a, x, y, z) )

/* glColor4ubVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glColor4ubVertex3fvSUN) (const GLubyte *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glColor4ubVertex3fvSUN (void);
#define      gdk_gl_glColor4ubVertex3fvSUN(proc, c, v) \
  ( ((GdkGLProc_glColor4ubVertex3fvSUN) (proc)) (c, v) )

/* glColor3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glColor3fVertex3fSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glColor3fVertex3fSUN (void);
#define      gdk_gl_glColor3fVertex3fSUN(proc, r, g, b, x, y, z) \
  ( ((GdkGLProc_glColor3fVertex3fSUN) (proc)) (r, g, b, x, y, z) )

/* glColor3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glColor3fVertex3fvSUN) (const GLfloat *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glColor3fVertex3fvSUN (void);
#define      gdk_gl_glColor3fVertex3fvSUN(proc, c, v) \
  ( ((GdkGLProc_glColor3fVertex3fvSUN) (proc)) (c, v) )

/* glNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glNormal3fVertex3fSUN) (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glNormal3fVertex3fSUN (void);
#define      gdk_gl_glNormal3fVertex3fSUN(proc, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glNormal3fVertex3fSUN) (proc)) (nx, ny, nz, x, y, z) )

/* glNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glNormal3fVertex3fvSUN) (const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glNormal3fVertex3fvSUN (void);
#define      gdk_gl_glNormal3fVertex3fvSUN(proc, n, v) \
  ( ((GdkGLProc_glNormal3fVertex3fvSUN) (proc)) (n, v) )

/* glColor4fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glColor4fNormal3fVertex3fSUN) (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glColor4fNormal3fVertex3fSUN (void);
#define      gdk_gl_glColor4fNormal3fVertex3fSUN(proc, r, g, b, a, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glColor4fNormal3fVertex3fSUN) (proc)) (r, g, b, a, nx, ny, nz, x, y, z) )

/* glColor4fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glColor4fNormal3fVertex3fvSUN) (const GLfloat *c, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glColor4fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glColor4fNormal3fVertex3fvSUN(proc, c, n, v) \
  ( ((GdkGLProc_glColor4fNormal3fVertex3fvSUN) (proc)) (c, n, v) )

/* glTexCoord2fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glTexCoord2fVertex3fSUN (void);
#define      gdk_gl_glTexCoord2fVertex3fSUN(proc, s, t, x, y, z) \
  ( ((GdkGLProc_glTexCoord2fVertex3fSUN) (proc)) (s, t, x, y, z) )

/* glTexCoord2fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fVertex3fvSUN) (const GLfloat *tc, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord2fVertex3fvSUN (void);
#define      gdk_gl_glTexCoord2fVertex3fvSUN(proc, tc, v) \
  ( ((GdkGLProc_glTexCoord2fVertex3fvSUN) (proc)) (tc, v) )

/* glTexCoord4fVertex4fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord4fVertex4fSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glTexCoord4fVertex4fSUN (void);
#define      gdk_gl_glTexCoord4fVertex4fSUN(proc, s, t, p, q, x, y, z, w) \
  ( ((GdkGLProc_glTexCoord4fVertex4fSUN) (proc)) (s, t, p, q, x, y, z, w) )

/* glTexCoord4fVertex4fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord4fVertex4fvSUN) (const GLfloat *tc, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord4fVertex4fvSUN (void);
#define      gdk_gl_glTexCoord4fVertex4fvSUN(proc, tc, v) \
  ( ((GdkGLProc_glTexCoord4fVertex4fvSUN) (proc)) (tc, v) )

/* glTexCoord2fColor4ubVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor4ubVertex3fSUN) (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glTexCoord2fColor4ubVertex3fSUN (void);
#define      gdk_gl_glTexCoord2fColor4ubVertex3fSUN(proc, s, t, r, g, b, a, x, y, z) \
  ( ((GdkGLProc_glTexCoord2fColor4ubVertex3fSUN) (proc)) (s, t, r, g, b, a, x, y, z) )

/* glTexCoord2fColor4ubVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor4ubVertex3fvSUN) (const GLfloat *tc, const GLubyte *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord2fColor4ubVertex3fvSUN (void);
#define      gdk_gl_glTexCoord2fColor4ubVertex3fvSUN(proc, tc, c, v) \
  ( ((GdkGLProc_glTexCoord2fColor4ubVertex3fvSUN) (proc)) (tc, c, v) )

/* glTexCoord2fColor3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glTexCoord2fColor3fVertex3fSUN (void);
#define      gdk_gl_glTexCoord2fColor3fVertex3fSUN(proc, s, t, r, g, b, x, y, z) \
  ( ((GdkGLProc_glTexCoord2fColor3fVertex3fSUN) (proc)) (s, t, r, g, b, x, y, z) )

/* glTexCoord2fColor3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord2fColor3fVertex3fvSUN (void);
#define      gdk_gl_glTexCoord2fColor3fVertex3fvSUN(proc, tc, c, v) \
  ( ((GdkGLProc_glTexCoord2fColor3fVertex3fvSUN) (proc)) (tc, c, v) )

/* glTexCoord2fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fNormal3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glTexCoord2fNormal3fVertex3fSUN (void);
#define      gdk_gl_glTexCoord2fNormal3fVertex3fSUN(proc, s, t, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glTexCoord2fNormal3fVertex3fSUN) (proc)) (s, t, nx, ny, nz, x, y, z) )

/* glTexCoord2fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fNormal3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord2fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glTexCoord2fNormal3fVertex3fvSUN(proc, tc, n, v) \
  ( ((GdkGLProc_glTexCoord2fNormal3fVertex3fvSUN) (proc)) (tc, n, v) )

/* glTexCoord2fColor4fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fSUN) (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glTexCoord2fColor4fNormal3fVertex3fSUN (void);
#define      gdk_gl_glTexCoord2fColor4fNormal3fVertex3fSUN(proc, s, t, r, g, b, a, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fSUN) (proc)) (s, t, r, g, b, a, nx, ny, nz, x, y, z) )

/* glTexCoord2fColor4fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord2fColor4fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glTexCoord2fColor4fNormal3fVertex3fvSUN(proc, tc, c, n, v) \
  ( ((GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fvSUN) (proc)) (tc, c, n, v) )

/* glTexCoord4fColor4fNormal3fVertex4fSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fSUN) (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glTexCoord4fColor4fNormal3fVertex4fSUN (void);
#define      gdk_gl_glTexCoord4fColor4fNormal3fVertex4fSUN(proc, s, t, p, q, r, g, b, a, nx, ny, nz, x, y, z, w) \
  ( ((GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fSUN) (proc)) (s, t, p, q, r, g, b, a, nx, ny, nz, x, y, z, w) )

/* glTexCoord4fColor4fNormal3fVertex4fvSUN */
typedef void (APIENTRYP GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fvSUN) (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glTexCoord4fColor4fNormal3fVertex4fvSUN (void);
#define      gdk_gl_glTexCoord4fColor4fNormal3fVertex4fvSUN(proc, tc, c, n, v) \
  ( ((GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fvSUN) (proc)) (tc, c, n, v) )

/* glReplacementCodeuiVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiVertex3fSUN) (GLuint rc, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiVertex3fSUN(proc, rc, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiVertex3fSUN) (proc)) (rc, x, y, z) )

/* glReplacementCodeuiVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiVertex3fvSUN) (const GLuint *rc, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiVertex3fvSUN(proc, rc, v) \
  ( ((GdkGLProc_glReplacementCodeuiVertex3fvSUN) (proc)) (rc, v) )

/* glReplacementCodeuiColor4ubVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor4ubVertex3fSUN) (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor4ubVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiColor4ubVertex3fSUN(proc, rc, r, g, b, a, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiColor4ubVertex3fSUN) (proc)) (rc, r, g, b, a, x, y, z) )

/* glReplacementCodeuiColor4ubVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor4ubVertex3fvSUN) (const GLuint *rc, const GLubyte *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor4ubVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiColor4ubVertex3fvSUN(proc, rc, c, v) \
  ( ((GdkGLProc_glReplacementCodeuiColor4ubVertex3fvSUN) (proc)) (rc, c, v) )

/* glReplacementCodeuiColor3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor3fVertex3fSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor3fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiColor3fVertex3fSUN(proc, rc, r, g, b, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiColor3fVertex3fSUN) (proc)) (rc, r, g, b, x, y, z) )

/* glReplacementCodeuiColor3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor3fVertex3fvSUN) (const GLuint *rc, const GLfloat *c, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor3fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiColor3fVertex3fvSUN(proc, rc, c, v) \
  ( ((GdkGLProc_glReplacementCodeuiColor3fVertex3fvSUN) (proc)) (rc, c, v) )

/* glReplacementCodeuiNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiNormal3fVertex3fSUN) (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiNormal3fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiNormal3fVertex3fSUN(proc, rc, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiNormal3fVertex3fSUN) (proc)) (rc, nx, ny, nz, x, y, z) )

/* glReplacementCodeuiNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiNormal3fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiNormal3fVertex3fvSUN(proc, rc, n, v) \
  ( ((GdkGLProc_glReplacementCodeuiNormal3fVertex3fvSUN) (proc)) (rc, n, v) )

/* glReplacementCodeuiColor4fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fSUN) (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor4fNormal3fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiColor4fNormal3fVertex3fSUN(proc, rc, r, g, b, a, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fSUN) (proc)) (rc, r, g, b, a, nx, ny, nz, x, y, z) )

/* glReplacementCodeuiColor4fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiColor4fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiColor4fNormal3fVertex3fvSUN(proc, rc, c, n, v) \
  ( ((GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fvSUN) (proc)) (rc, c, n, v) )

/* glReplacementCodeuiTexCoord2fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fVertex3fSUN(proc, rc, s, t, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fSUN) (proc)) (rc, s, t, x, y, z) )

/* glReplacementCodeuiTexCoord2fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fVertex3fvSUN(proc, rc, tc, v) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fvSUN) (proc)) (rc, tc, v) )

/* glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(proc, rc, s, t, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN) (proc)) (rc, s, t, nx, ny, nz, x, y, z) )

/* glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(proc, rc, tc, n, v) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN) (proc)) (rc, tc, n, v) )

/* glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN) (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(proc, rc, s, t, r, g, b, a, nx, ny, nz, x, y, z) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN) (proc)) (rc, s, t, r, g, b, a, nx, ny, nz, x, y, z) )

/* glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN */
typedef void (APIENTRYP GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v);
GdkGLProc    gdk_gl_get_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN (void);
#define      gdk_gl_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(proc, rc, tc, c, n, v) \
  ( ((GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN) (proc)) (rc, tc, c, n, v) )

/* proc struct */

typedef struct _GdkGL_GL_SUN_vertex GdkGL_GL_SUN_vertex;

struct _GdkGL_GL_SUN_vertex
{
  GdkGLProc_glColor4ubVertex2fSUN glColor4ubVertex2fSUN;
  GdkGLProc_glColor4ubVertex2fvSUN glColor4ubVertex2fvSUN;
  GdkGLProc_glColor4ubVertex3fSUN glColor4ubVertex3fSUN;
  GdkGLProc_glColor4ubVertex3fvSUN glColor4ubVertex3fvSUN;
  GdkGLProc_glColor3fVertex3fSUN glColor3fVertex3fSUN;
  GdkGLProc_glColor3fVertex3fvSUN glColor3fVertex3fvSUN;
  GdkGLProc_glNormal3fVertex3fSUN glNormal3fVertex3fSUN;
  GdkGLProc_glNormal3fVertex3fvSUN glNormal3fVertex3fvSUN;
  GdkGLProc_glColor4fNormal3fVertex3fSUN glColor4fNormal3fVertex3fSUN;
  GdkGLProc_glColor4fNormal3fVertex3fvSUN glColor4fNormal3fVertex3fvSUN;
  GdkGLProc_glTexCoord2fVertex3fSUN glTexCoord2fVertex3fSUN;
  GdkGLProc_glTexCoord2fVertex3fvSUN glTexCoord2fVertex3fvSUN;
  GdkGLProc_glTexCoord4fVertex4fSUN glTexCoord4fVertex4fSUN;
  GdkGLProc_glTexCoord4fVertex4fvSUN glTexCoord4fVertex4fvSUN;
  GdkGLProc_glTexCoord2fColor4ubVertex3fSUN glTexCoord2fColor4ubVertex3fSUN;
  GdkGLProc_glTexCoord2fColor4ubVertex3fvSUN glTexCoord2fColor4ubVertex3fvSUN;
  GdkGLProc_glTexCoord2fColor3fVertex3fSUN glTexCoord2fColor3fVertex3fSUN;
  GdkGLProc_glTexCoord2fColor3fVertex3fvSUN glTexCoord2fColor3fVertex3fvSUN;
  GdkGLProc_glTexCoord2fNormal3fVertex3fSUN glTexCoord2fNormal3fVertex3fSUN;
  GdkGLProc_glTexCoord2fNormal3fVertex3fvSUN glTexCoord2fNormal3fVertex3fvSUN;
  GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fSUN glTexCoord2fColor4fNormal3fVertex3fSUN;
  GdkGLProc_glTexCoord2fColor4fNormal3fVertex3fvSUN glTexCoord2fColor4fNormal3fVertex3fvSUN;
  GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fSUN glTexCoord4fColor4fNormal3fVertex4fSUN;
  GdkGLProc_glTexCoord4fColor4fNormal3fVertex4fvSUN glTexCoord4fColor4fNormal3fVertex4fvSUN;
  GdkGLProc_glReplacementCodeuiVertex3fSUN glReplacementCodeuiVertex3fSUN;
  GdkGLProc_glReplacementCodeuiVertex3fvSUN glReplacementCodeuiVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiColor4ubVertex3fSUN glReplacementCodeuiColor4ubVertex3fSUN;
  GdkGLProc_glReplacementCodeuiColor4ubVertex3fvSUN glReplacementCodeuiColor4ubVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiColor3fVertex3fSUN glReplacementCodeuiColor3fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiColor3fVertex3fvSUN glReplacementCodeuiColor3fVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiNormal3fVertex3fSUN glReplacementCodeuiNormal3fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiNormal3fVertex3fvSUN glReplacementCodeuiNormal3fVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fSUN glReplacementCodeuiColor4fNormal3fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiColor4fNormal3fVertex3fvSUN glReplacementCodeuiColor4fNormal3fVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fSUN glReplacementCodeuiTexCoord2fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fVertex3fvSUN glReplacementCodeuiTexCoord2fVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN;
  GdkGLProc_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN;
};

GdkGL_GL_SUN_vertex *gdk_gl_get_GL_SUN_vertex (void);

/*
 * GL_EXT_blend_func_separate
 */

/* glBlendFuncSeparateEXT */
typedef void (APIENTRYP GdkGLProc_glBlendFuncSeparateEXT) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GdkGLProc    gdk_gl_get_glBlendFuncSeparateEXT (void);
#define      gdk_gl_glBlendFuncSeparateEXT(proc, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) \
  ( ((GdkGLProc_glBlendFuncSeparateEXT) (proc)) (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_blend_func_separate GdkGL_GL_EXT_blend_func_separate;

struct _GdkGL_GL_EXT_blend_func_separate
{
  GdkGLProc_glBlendFuncSeparateEXT glBlendFuncSeparateEXT;
};

GdkGL_GL_EXT_blend_func_separate *gdk_gl_get_GL_EXT_blend_func_separate (void);

/*
 * GL_INGR_blend_func_separate
 */

/* glBlendFuncSeparateINGR */
typedef void (APIENTRYP GdkGLProc_glBlendFuncSeparateINGR) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
GdkGLProc    gdk_gl_get_glBlendFuncSeparateINGR (void);
#define      gdk_gl_glBlendFuncSeparateINGR(proc, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) \
  ( ((GdkGLProc_glBlendFuncSeparateINGR) (proc)) (sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_INGR_blend_func_separate GdkGL_GL_INGR_blend_func_separate;

struct _GdkGL_GL_INGR_blend_func_separate
{
  GdkGLProc_glBlendFuncSeparateINGR glBlendFuncSeparateINGR;
};

GdkGL_GL_INGR_blend_func_separate *gdk_gl_get_GL_INGR_blend_func_separate (void);

/*
 * GL_EXT_vertex_weighting
 */

/* glVertexWeightfEXT */
typedef void (APIENTRYP GdkGLProc_glVertexWeightfEXT) (GLfloat weight);
GdkGLProc    gdk_gl_get_glVertexWeightfEXT (void);
#define      gdk_gl_glVertexWeightfEXT(proc, weight) \
  ( ((GdkGLProc_glVertexWeightfEXT) (proc)) (weight) )

/* glVertexWeightfvEXT */
typedef void (APIENTRYP GdkGLProc_glVertexWeightfvEXT) (const GLfloat *weight);
GdkGLProc    gdk_gl_get_glVertexWeightfvEXT (void);
#define      gdk_gl_glVertexWeightfvEXT(proc, weight) \
  ( ((GdkGLProc_glVertexWeightfvEXT) (proc)) (weight) )

/* glVertexWeightPointerEXT */
typedef void (APIENTRYP GdkGLProc_glVertexWeightPointerEXT) (GLsizei size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexWeightPointerEXT (void);
#define      gdk_gl_glVertexWeightPointerEXT(proc, size, type, stride, pointer) \
  ( ((GdkGLProc_glVertexWeightPointerEXT) (proc)) (size, type, stride, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_vertex_weighting GdkGL_GL_EXT_vertex_weighting;

struct _GdkGL_GL_EXT_vertex_weighting
{
  GdkGLProc_glVertexWeightfEXT glVertexWeightfEXT;
  GdkGLProc_glVertexWeightfvEXT glVertexWeightfvEXT;
  GdkGLProc_glVertexWeightPointerEXT glVertexWeightPointerEXT;
};

GdkGL_GL_EXT_vertex_weighting *gdk_gl_get_GL_EXT_vertex_weighting (void);

/*
 * GL_NV_vertex_array_range
 */

/* glFlushVertexArrayRangeNV */
typedef void (APIENTRYP GdkGLProc_glFlushVertexArrayRangeNV) (void);
GdkGLProc    gdk_gl_get_glFlushVertexArrayRangeNV (void);
#define      gdk_gl_glFlushVertexArrayRangeNV(proc) \
  ( ((GdkGLProc_glFlushVertexArrayRangeNV) (proc)) () )

/* glVertexArrayRangeNV */
typedef void (APIENTRYP GdkGLProc_glVertexArrayRangeNV) (GLsizei length, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexArrayRangeNV (void);
#define      gdk_gl_glVertexArrayRangeNV(proc, length, pointer) \
  ( ((GdkGLProc_glVertexArrayRangeNV) (proc)) (length, pointer) )

/* proc struct */

typedef struct _GdkGL_GL_NV_vertex_array_range GdkGL_GL_NV_vertex_array_range;

struct _GdkGL_GL_NV_vertex_array_range
{
  GdkGLProc_glFlushVertexArrayRangeNV glFlushVertexArrayRangeNV;
  GdkGLProc_glVertexArrayRangeNV glVertexArrayRangeNV;
};

GdkGL_GL_NV_vertex_array_range *gdk_gl_get_GL_NV_vertex_array_range (void);

/*
 * GL_NV_register_combiners
 */

/* glCombinerParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glCombinerParameterfvNV) (GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glCombinerParameterfvNV (void);
#define      gdk_gl_glCombinerParameterfvNV(proc, pname, params) \
  ( ((GdkGLProc_glCombinerParameterfvNV) (proc)) (pname, params) )

/* glCombinerParameterfNV */
typedef void (APIENTRYP GdkGLProc_glCombinerParameterfNV) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glCombinerParameterfNV (void);
#define      gdk_gl_glCombinerParameterfNV(proc, pname, param) \
  ( ((GdkGLProc_glCombinerParameterfNV) (proc)) (pname, param) )

/* glCombinerParameterivNV */
typedef void (APIENTRYP GdkGLProc_glCombinerParameterivNV) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glCombinerParameterivNV (void);
#define      gdk_gl_glCombinerParameterivNV(proc, pname, params) \
  ( ((GdkGLProc_glCombinerParameterivNV) (proc)) (pname, params) )

/* glCombinerParameteriNV */
typedef void (APIENTRYP GdkGLProc_glCombinerParameteriNV) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glCombinerParameteriNV (void);
#define      gdk_gl_glCombinerParameteriNV(proc, pname, param) \
  ( ((GdkGLProc_glCombinerParameteriNV) (proc)) (pname, param) )

/* glCombinerInputNV */
typedef void (APIENTRYP GdkGLProc_glCombinerInputNV) (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
GdkGLProc    gdk_gl_get_glCombinerInputNV (void);
#define      gdk_gl_glCombinerInputNV(proc, stage, portion, variable, input, mapping, componentUsage) \
  ( ((GdkGLProc_glCombinerInputNV) (proc)) (stage, portion, variable, input, mapping, componentUsage) )

/* glCombinerOutputNV */
typedef void (APIENTRYP GdkGLProc_glCombinerOutputNV) (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum);
GdkGLProc    gdk_gl_get_glCombinerOutputNV (void);
#define      gdk_gl_glCombinerOutputNV(proc, stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum) \
  ( ((GdkGLProc_glCombinerOutputNV) (proc)) (stage, portion, abOutput, cdOutput, sumOutput, scale, bias, abDotProduct, cdDotProduct, muxSum) )

/* glFinalCombinerInputNV */
typedef void (APIENTRYP GdkGLProc_glFinalCombinerInputNV) (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage);
GdkGLProc    gdk_gl_get_glFinalCombinerInputNV (void);
#define      gdk_gl_glFinalCombinerInputNV(proc, variable, input, mapping, componentUsage) \
  ( ((GdkGLProc_glFinalCombinerInputNV) (proc)) (variable, input, mapping, componentUsage) )

/* glGetCombinerInputParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetCombinerInputParameterfvNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetCombinerInputParameterfvNV (void);
#define      gdk_gl_glGetCombinerInputParameterfvNV(proc, stage, portion, variable, pname, params) \
  ( ((GdkGLProc_glGetCombinerInputParameterfvNV) (proc)) (stage, portion, variable, pname, params) )

/* glGetCombinerInputParameterivNV */
typedef void (APIENTRYP GdkGLProc_glGetCombinerInputParameterivNV) (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetCombinerInputParameterivNV (void);
#define      gdk_gl_glGetCombinerInputParameterivNV(proc, stage, portion, variable, pname, params) \
  ( ((GdkGLProc_glGetCombinerInputParameterivNV) (proc)) (stage, portion, variable, pname, params) )

/* glGetCombinerOutputParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetCombinerOutputParameterfvNV) (GLenum stage, GLenum portion, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetCombinerOutputParameterfvNV (void);
#define      gdk_gl_glGetCombinerOutputParameterfvNV(proc, stage, portion, pname, params) \
  ( ((GdkGLProc_glGetCombinerOutputParameterfvNV) (proc)) (stage, portion, pname, params) )

/* glGetCombinerOutputParameterivNV */
typedef void (APIENTRYP GdkGLProc_glGetCombinerOutputParameterivNV) (GLenum stage, GLenum portion, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetCombinerOutputParameterivNV (void);
#define      gdk_gl_glGetCombinerOutputParameterivNV(proc, stage, portion, pname, params) \
  ( ((GdkGLProc_glGetCombinerOutputParameterivNV) (proc)) (stage, portion, pname, params) )

/* glGetFinalCombinerInputParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetFinalCombinerInputParameterfvNV) (GLenum variable, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetFinalCombinerInputParameterfvNV (void);
#define      gdk_gl_glGetFinalCombinerInputParameterfvNV(proc, variable, pname, params) \
  ( ((GdkGLProc_glGetFinalCombinerInputParameterfvNV) (proc)) (variable, pname, params) )

/* glGetFinalCombinerInputParameterivNV */
typedef void (APIENTRYP GdkGLProc_glGetFinalCombinerInputParameterivNV) (GLenum variable, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFinalCombinerInputParameterivNV (void);
#define      gdk_gl_glGetFinalCombinerInputParameterivNV(proc, variable, pname, params) \
  ( ((GdkGLProc_glGetFinalCombinerInputParameterivNV) (proc)) (variable, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_register_combiners GdkGL_GL_NV_register_combiners;

struct _GdkGL_GL_NV_register_combiners
{
  GdkGLProc_glCombinerParameterfvNV glCombinerParameterfvNV;
  GdkGLProc_glCombinerParameterfNV glCombinerParameterfNV;
  GdkGLProc_glCombinerParameterivNV glCombinerParameterivNV;
  GdkGLProc_glCombinerParameteriNV glCombinerParameteriNV;
  GdkGLProc_glCombinerInputNV glCombinerInputNV;
  GdkGLProc_glCombinerOutputNV glCombinerOutputNV;
  GdkGLProc_glFinalCombinerInputNV glFinalCombinerInputNV;
  GdkGLProc_glGetCombinerInputParameterfvNV glGetCombinerInputParameterfvNV;
  GdkGLProc_glGetCombinerInputParameterivNV glGetCombinerInputParameterivNV;
  GdkGLProc_glGetCombinerOutputParameterfvNV glGetCombinerOutputParameterfvNV;
  GdkGLProc_glGetCombinerOutputParameterivNV glGetCombinerOutputParameterivNV;
  GdkGLProc_glGetFinalCombinerInputParameterfvNV glGetFinalCombinerInputParameterfvNV;
  GdkGLProc_glGetFinalCombinerInputParameterivNV glGetFinalCombinerInputParameterivNV;
};

GdkGL_GL_NV_register_combiners *gdk_gl_get_GL_NV_register_combiners (void);

/*
 * GL_MESA_resize_buffers
 */

/* glResizeBuffersMESA */
typedef void (APIENTRYP GdkGLProc_glResizeBuffersMESA) (void);
GdkGLProc    gdk_gl_get_glResizeBuffersMESA (void);
#define      gdk_gl_glResizeBuffersMESA(proc) \
  ( ((GdkGLProc_glResizeBuffersMESA) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_MESA_resize_buffers GdkGL_GL_MESA_resize_buffers;

struct _GdkGL_GL_MESA_resize_buffers
{
  GdkGLProc_glResizeBuffersMESA glResizeBuffersMESA;
};

GdkGL_GL_MESA_resize_buffers *gdk_gl_get_GL_MESA_resize_buffers (void);

/*
 * GL_MESA_window_pos
 */

/* glWindowPos2dMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2dMESA) (GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glWindowPos2dMESA (void);
#define      gdk_gl_glWindowPos2dMESA(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2dMESA) (proc)) (x, y) )

/* glWindowPos2dvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2dvMESA) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos2dvMESA (void);
#define      gdk_gl_glWindowPos2dvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos2dvMESA) (proc)) (v) )

/* glWindowPos2fMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2fMESA) (GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glWindowPos2fMESA (void);
#define      gdk_gl_glWindowPos2fMESA(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2fMESA) (proc)) (x, y) )

/* glWindowPos2fvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2fvMESA) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos2fvMESA (void);
#define      gdk_gl_glWindowPos2fvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos2fvMESA) (proc)) (v) )

/* glWindowPos2iMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2iMESA) (GLint x, GLint y);
GdkGLProc    gdk_gl_get_glWindowPos2iMESA (void);
#define      gdk_gl_glWindowPos2iMESA(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2iMESA) (proc)) (x, y) )

/* glWindowPos2ivMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2ivMESA) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos2ivMESA (void);
#define      gdk_gl_glWindowPos2ivMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos2ivMESA) (proc)) (v) )

/* glWindowPos2sMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2sMESA) (GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glWindowPos2sMESA (void);
#define      gdk_gl_glWindowPos2sMESA(proc, x, y) \
  ( ((GdkGLProc_glWindowPos2sMESA) (proc)) (x, y) )

/* glWindowPos2svMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos2svMESA) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos2svMESA (void);
#define      gdk_gl_glWindowPos2svMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos2svMESA) (proc)) (v) )

/* glWindowPos3dMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3dMESA) (GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glWindowPos3dMESA (void);
#define      gdk_gl_glWindowPos3dMESA(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3dMESA) (proc)) (x, y, z) )

/* glWindowPos3dvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3dvMESA) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos3dvMESA (void);
#define      gdk_gl_glWindowPos3dvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos3dvMESA) (proc)) (v) )

/* glWindowPos3fMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3fMESA) (GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glWindowPos3fMESA (void);
#define      gdk_gl_glWindowPos3fMESA(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3fMESA) (proc)) (x, y, z) )

/* glWindowPos3fvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3fvMESA) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos3fvMESA (void);
#define      gdk_gl_glWindowPos3fvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos3fvMESA) (proc)) (v) )

/* glWindowPos3iMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3iMESA) (GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glWindowPos3iMESA (void);
#define      gdk_gl_glWindowPos3iMESA(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3iMESA) (proc)) (x, y, z) )

/* glWindowPos3ivMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3ivMESA) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos3ivMESA (void);
#define      gdk_gl_glWindowPos3ivMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos3ivMESA) (proc)) (v) )

/* glWindowPos3sMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3sMESA) (GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glWindowPos3sMESA (void);
#define      gdk_gl_glWindowPos3sMESA(proc, x, y, z) \
  ( ((GdkGLProc_glWindowPos3sMESA) (proc)) (x, y, z) )

/* glWindowPos3svMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos3svMESA) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos3svMESA (void);
#define      gdk_gl_glWindowPos3svMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos3svMESA) (proc)) (v) )

/* glWindowPos4dMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4dMESA) (GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glWindowPos4dMESA (void);
#define      gdk_gl_glWindowPos4dMESA(proc, x, y, z, w) \
  ( ((GdkGLProc_glWindowPos4dMESA) (proc)) (x, y, z, w) )

/* glWindowPos4dvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4dvMESA) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glWindowPos4dvMESA (void);
#define      gdk_gl_glWindowPos4dvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos4dvMESA) (proc)) (v) )

/* glWindowPos4fMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4fMESA) (GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glWindowPos4fMESA (void);
#define      gdk_gl_glWindowPos4fMESA(proc, x, y, z, w) \
  ( ((GdkGLProc_glWindowPos4fMESA) (proc)) (x, y, z, w) )

/* glWindowPos4fvMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4fvMESA) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glWindowPos4fvMESA (void);
#define      gdk_gl_glWindowPos4fvMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos4fvMESA) (proc)) (v) )

/* glWindowPos4iMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4iMESA) (GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glWindowPos4iMESA (void);
#define      gdk_gl_glWindowPos4iMESA(proc, x, y, z, w) \
  ( ((GdkGLProc_glWindowPos4iMESA) (proc)) (x, y, z, w) )

/* glWindowPos4ivMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4ivMESA) (const GLint *v);
GdkGLProc    gdk_gl_get_glWindowPos4ivMESA (void);
#define      gdk_gl_glWindowPos4ivMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos4ivMESA) (proc)) (v) )

/* glWindowPos4sMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4sMESA) (GLshort x, GLshort y, GLshort z, GLshort w);
GdkGLProc    gdk_gl_get_glWindowPos4sMESA (void);
#define      gdk_gl_glWindowPos4sMESA(proc, x, y, z, w) \
  ( ((GdkGLProc_glWindowPos4sMESA) (proc)) (x, y, z, w) )

/* glWindowPos4svMESA */
typedef void (APIENTRYP GdkGLProc_glWindowPos4svMESA) (const GLshort *v);
GdkGLProc    gdk_gl_get_glWindowPos4svMESA (void);
#define      gdk_gl_glWindowPos4svMESA(proc, v) \
  ( ((GdkGLProc_glWindowPos4svMESA) (proc)) (v) )

/* proc struct */

typedef struct _GdkGL_GL_MESA_window_pos GdkGL_GL_MESA_window_pos;

struct _GdkGL_GL_MESA_window_pos
{
  GdkGLProc_glWindowPos2dMESA glWindowPos2dMESA;
  GdkGLProc_glWindowPos2dvMESA glWindowPos2dvMESA;
  GdkGLProc_glWindowPos2fMESA glWindowPos2fMESA;
  GdkGLProc_glWindowPos2fvMESA glWindowPos2fvMESA;
  GdkGLProc_glWindowPos2iMESA glWindowPos2iMESA;
  GdkGLProc_glWindowPos2ivMESA glWindowPos2ivMESA;
  GdkGLProc_glWindowPos2sMESA glWindowPos2sMESA;
  GdkGLProc_glWindowPos2svMESA glWindowPos2svMESA;
  GdkGLProc_glWindowPos3dMESA glWindowPos3dMESA;
  GdkGLProc_glWindowPos3dvMESA glWindowPos3dvMESA;
  GdkGLProc_glWindowPos3fMESA glWindowPos3fMESA;
  GdkGLProc_glWindowPos3fvMESA glWindowPos3fvMESA;
  GdkGLProc_glWindowPos3iMESA glWindowPos3iMESA;
  GdkGLProc_glWindowPos3ivMESA glWindowPos3ivMESA;
  GdkGLProc_glWindowPos3sMESA glWindowPos3sMESA;
  GdkGLProc_glWindowPos3svMESA glWindowPos3svMESA;
  GdkGLProc_glWindowPos4dMESA glWindowPos4dMESA;
  GdkGLProc_glWindowPos4dvMESA glWindowPos4dvMESA;
  GdkGLProc_glWindowPos4fMESA glWindowPos4fMESA;
  GdkGLProc_glWindowPos4fvMESA glWindowPos4fvMESA;
  GdkGLProc_glWindowPos4iMESA glWindowPos4iMESA;
  GdkGLProc_glWindowPos4ivMESA glWindowPos4ivMESA;
  GdkGLProc_glWindowPos4sMESA glWindowPos4sMESA;
  GdkGLProc_glWindowPos4svMESA glWindowPos4svMESA;
};

GdkGL_GL_MESA_window_pos *gdk_gl_get_GL_MESA_window_pos (void);

/*
 * GL_IBM_multimode_draw_arrays
 */

/* glMultiModeDrawArraysIBM */
typedef void (APIENTRYP GdkGLProc_glMultiModeDrawArraysIBM) (const GLenum *mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride);
GdkGLProc    gdk_gl_get_glMultiModeDrawArraysIBM (void);
#define      gdk_gl_glMultiModeDrawArraysIBM(proc, mode, first, count, primcount, modestride) \
  ( ((GdkGLProc_glMultiModeDrawArraysIBM) (proc)) (mode, first, count, primcount, modestride) )

/* glMultiModeDrawElementsIBM */
typedef void (APIENTRYP GdkGLProc_glMultiModeDrawElementsIBM) (const GLenum *mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei primcount, GLint modestride);
GdkGLProc    gdk_gl_get_glMultiModeDrawElementsIBM (void);
#define      gdk_gl_glMultiModeDrawElementsIBM(proc, mode, count, type, indices, primcount, modestride) \
  ( ((GdkGLProc_glMultiModeDrawElementsIBM) (proc)) (mode, count, type, indices, primcount, modestride) )

/* proc struct */

typedef struct _GdkGL_GL_IBM_multimode_draw_arrays GdkGL_GL_IBM_multimode_draw_arrays;

struct _GdkGL_GL_IBM_multimode_draw_arrays
{
  GdkGLProc_glMultiModeDrawArraysIBM glMultiModeDrawArraysIBM;
  GdkGLProc_glMultiModeDrawElementsIBM glMultiModeDrawElementsIBM;
};

GdkGL_GL_IBM_multimode_draw_arrays *gdk_gl_get_GL_IBM_multimode_draw_arrays (void);

/*
 * GL_IBM_vertex_array_lists
 */

/* glColorPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glColorPointerListIBM) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glColorPointerListIBM (void);
#define      gdk_gl_glColorPointerListIBM(proc, size, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glColorPointerListIBM) (proc)) (size, type, stride, pointer, ptrstride) )

/* glSecondaryColorPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glSecondaryColorPointerListIBM) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glSecondaryColorPointerListIBM (void);
#define      gdk_gl_glSecondaryColorPointerListIBM(proc, size, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glSecondaryColorPointerListIBM) (proc)) (size, type, stride, pointer, ptrstride) )

/* glEdgeFlagPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glEdgeFlagPointerListIBM) (GLint stride, const GLboolean* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glEdgeFlagPointerListIBM (void);
#define      gdk_gl_glEdgeFlagPointerListIBM(proc, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glEdgeFlagPointerListIBM) (proc)) (stride, pointer, ptrstride) )

/* glFogCoordPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glFogCoordPointerListIBM) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glFogCoordPointerListIBM (void);
#define      gdk_gl_glFogCoordPointerListIBM(proc, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glFogCoordPointerListIBM) (proc)) (type, stride, pointer, ptrstride) )

/* glIndexPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glIndexPointerListIBM) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glIndexPointerListIBM (void);
#define      gdk_gl_glIndexPointerListIBM(proc, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glIndexPointerListIBM) (proc)) (type, stride, pointer, ptrstride) )

/* glNormalPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glNormalPointerListIBM) (GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glNormalPointerListIBM (void);
#define      gdk_gl_glNormalPointerListIBM(proc, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glNormalPointerListIBM) (proc)) (type, stride, pointer, ptrstride) )

/* glTexCoordPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glTexCoordPointerListIBM) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glTexCoordPointerListIBM (void);
#define      gdk_gl_glTexCoordPointerListIBM(proc, size, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glTexCoordPointerListIBM) (proc)) (size, type, stride, pointer, ptrstride) )

/* glVertexPointerListIBM */
typedef void (APIENTRYP GdkGLProc_glVertexPointerListIBM) (GLint size, GLenum type, GLint stride, const GLvoid* *pointer, GLint ptrstride);
GdkGLProc    gdk_gl_get_glVertexPointerListIBM (void);
#define      gdk_gl_glVertexPointerListIBM(proc, size, type, stride, pointer, ptrstride) \
  ( ((GdkGLProc_glVertexPointerListIBM) (proc)) (size, type, stride, pointer, ptrstride) )

/* proc struct */

typedef struct _GdkGL_GL_IBM_vertex_array_lists GdkGL_GL_IBM_vertex_array_lists;

struct _GdkGL_GL_IBM_vertex_array_lists
{
  GdkGLProc_glColorPointerListIBM glColorPointerListIBM;
  GdkGLProc_glSecondaryColorPointerListIBM glSecondaryColorPointerListIBM;
  GdkGLProc_glEdgeFlagPointerListIBM glEdgeFlagPointerListIBM;
  GdkGLProc_glFogCoordPointerListIBM glFogCoordPointerListIBM;
  GdkGLProc_glIndexPointerListIBM glIndexPointerListIBM;
  GdkGLProc_glNormalPointerListIBM glNormalPointerListIBM;
  GdkGLProc_glTexCoordPointerListIBM glTexCoordPointerListIBM;
  GdkGLProc_glVertexPointerListIBM glVertexPointerListIBM;
};

GdkGL_GL_IBM_vertex_array_lists *gdk_gl_get_GL_IBM_vertex_array_lists (void);

/*
 * GL_3DFX_tbuffer
 */

/* glTbufferMask3DFX */
typedef void (APIENTRYP GdkGLProc_glTbufferMask3DFX) (GLuint mask);
GdkGLProc    gdk_gl_get_glTbufferMask3DFX (void);
#define      gdk_gl_glTbufferMask3DFX(proc, mask) \
  ( ((GdkGLProc_glTbufferMask3DFX) (proc)) (mask) )

/* proc struct */

typedef struct _GdkGL_GL_3DFX_tbuffer GdkGL_GL_3DFX_tbuffer;

struct _GdkGL_GL_3DFX_tbuffer
{
  GdkGLProc_glTbufferMask3DFX glTbufferMask3DFX;
};

GdkGL_GL_3DFX_tbuffer *gdk_gl_get_GL_3DFX_tbuffer (void);

/*
 * GL_EXT_multisample
 */

/* glSampleMaskEXT */
typedef void (APIENTRYP GdkGLProc_glSampleMaskEXT) (GLclampf value, GLboolean invert);
GdkGLProc    gdk_gl_get_glSampleMaskEXT (void);
#define      gdk_gl_glSampleMaskEXT(proc, value, invert) \
  ( ((GdkGLProc_glSampleMaskEXT) (proc)) (value, invert) )

/* glSamplePatternEXT */
typedef void (APIENTRYP GdkGLProc_glSamplePatternEXT) (GLenum pattern);
GdkGLProc    gdk_gl_get_glSamplePatternEXT (void);
#define      gdk_gl_glSamplePatternEXT(proc, pattern) \
  ( ((GdkGLProc_glSamplePatternEXT) (proc)) (pattern) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_multisample GdkGL_GL_EXT_multisample;

struct _GdkGL_GL_EXT_multisample
{
  GdkGLProc_glSampleMaskEXT glSampleMaskEXT;
  GdkGLProc_glSamplePatternEXT glSamplePatternEXT;
};

GdkGL_GL_EXT_multisample *gdk_gl_get_GL_EXT_multisample (void);

/*
 * GL_SGIS_texture_color_mask
 */

/* glTextureColorMaskSGIS */
typedef void (APIENTRYP GdkGLProc_glTextureColorMaskSGIS) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
GdkGLProc    gdk_gl_get_glTextureColorMaskSGIS (void);
#define      gdk_gl_glTextureColorMaskSGIS(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glTextureColorMaskSGIS) (proc)) (red, green, blue, alpha) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_texture_color_mask GdkGL_GL_SGIS_texture_color_mask;

struct _GdkGL_GL_SGIS_texture_color_mask
{
  GdkGLProc_glTextureColorMaskSGIS glTextureColorMaskSGIS;
};

GdkGL_GL_SGIS_texture_color_mask *gdk_gl_get_GL_SGIS_texture_color_mask (void);

/*
 * GL_SGIX_igloo_interface
 */

/* glIglooInterfaceSGIX */
typedef void (APIENTRYP GdkGLProc_glIglooInterfaceSGIX) (GLenum pname, const GLvoid *params);
GdkGLProc    gdk_gl_get_glIglooInterfaceSGIX (void);
#define      gdk_gl_glIglooInterfaceSGIX(proc, pname, params) \
  ( ((GdkGLProc_glIglooInterfaceSGIX) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_igloo_interface GdkGL_GL_SGIX_igloo_interface;

struct _GdkGL_GL_SGIX_igloo_interface
{
  GdkGLProc_glIglooInterfaceSGIX glIglooInterfaceSGIX;
};

GdkGL_GL_SGIX_igloo_interface *gdk_gl_get_GL_SGIX_igloo_interface (void);

/*
 * GL_NV_fence
 */

/* glDeleteFencesNV */
typedef void (APIENTRYP GdkGLProc_glDeleteFencesNV) (GLsizei n, const GLuint *fences);
GdkGLProc    gdk_gl_get_glDeleteFencesNV (void);
#define      gdk_gl_glDeleteFencesNV(proc, n, fences) \
  ( ((GdkGLProc_glDeleteFencesNV) (proc)) (n, fences) )

/* glGenFencesNV */
typedef void (APIENTRYP GdkGLProc_glGenFencesNV) (GLsizei n, GLuint *fences);
GdkGLProc    gdk_gl_get_glGenFencesNV (void);
#define      gdk_gl_glGenFencesNV(proc, n, fences) \
  ( ((GdkGLProc_glGenFencesNV) (proc)) (n, fences) )

/* glIsFenceNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsFenceNV) (GLuint fence);
GdkGLProc    gdk_gl_get_glIsFenceNV (void);
#define      gdk_gl_glIsFenceNV(proc, fence) \
  ( ((GdkGLProc_glIsFenceNV) (proc)) (fence) )

/* glTestFenceNV */
typedef GLboolean (APIENTRYP GdkGLProc_glTestFenceNV) (GLuint fence);
GdkGLProc    gdk_gl_get_glTestFenceNV (void);
#define      gdk_gl_glTestFenceNV(proc, fence) \
  ( ((GdkGLProc_glTestFenceNV) (proc)) (fence) )

/* glGetFenceivNV */
typedef void (APIENTRYP GdkGLProc_glGetFenceivNV) (GLuint fence, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFenceivNV (void);
#define      gdk_gl_glGetFenceivNV(proc, fence, pname, params) \
  ( ((GdkGLProc_glGetFenceivNV) (proc)) (fence, pname, params) )

/* glFinishFenceNV */
typedef void (APIENTRYP GdkGLProc_glFinishFenceNV) (GLuint fence);
GdkGLProc    gdk_gl_get_glFinishFenceNV (void);
#define      gdk_gl_glFinishFenceNV(proc, fence) \
  ( ((GdkGLProc_glFinishFenceNV) (proc)) (fence) )

/* glSetFenceNV */
typedef void (APIENTRYP GdkGLProc_glSetFenceNV) (GLuint fence, GLenum condition);
GdkGLProc    gdk_gl_get_glSetFenceNV (void);
#define      gdk_gl_glSetFenceNV(proc, fence, condition) \
  ( ((GdkGLProc_glSetFenceNV) (proc)) (fence, condition) )

/* proc struct */

typedef struct _GdkGL_GL_NV_fence GdkGL_GL_NV_fence;

struct _GdkGL_GL_NV_fence
{
  GdkGLProc_glDeleteFencesNV glDeleteFencesNV;
  GdkGLProc_glGenFencesNV glGenFencesNV;
  GdkGLProc_glIsFenceNV glIsFenceNV;
  GdkGLProc_glTestFenceNV glTestFenceNV;
  GdkGLProc_glGetFenceivNV glGetFenceivNV;
  GdkGLProc_glFinishFenceNV glFinishFenceNV;
  GdkGLProc_glSetFenceNV glSetFenceNV;
};

GdkGL_GL_NV_fence *gdk_gl_get_GL_NV_fence (void);

/*
 * GL_NV_evaluators
 */

/* glMapControlPointsNV */
typedef void (APIENTRYP GdkGLProc_glMapControlPointsNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const GLvoid *points);
GdkGLProc    gdk_gl_get_glMapControlPointsNV (void);
#define      gdk_gl_glMapControlPointsNV(proc, target, index, type, ustride, vstride, uorder, vorder, packed, points) \
  ( ((GdkGLProc_glMapControlPointsNV) (proc)) (target, index, type, ustride, vstride, uorder, vorder, packed, points) )

/* glMapParameterivNV */
typedef void (APIENTRYP GdkGLProc_glMapParameterivNV) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glMapParameterivNV (void);
#define      gdk_gl_glMapParameterivNV(proc, target, pname, params) \
  ( ((GdkGLProc_glMapParameterivNV) (proc)) (target, pname, params) )

/* glMapParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glMapParameterfvNV) (GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glMapParameterfvNV (void);
#define      gdk_gl_glMapParameterfvNV(proc, target, pname, params) \
  ( ((GdkGLProc_glMapParameterfvNV) (proc)) (target, pname, params) )

/* glGetMapControlPointsNV */
typedef void (APIENTRYP GdkGLProc_glGetMapControlPointsNV) (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, GLvoid *points);
GdkGLProc    gdk_gl_get_glGetMapControlPointsNV (void);
#define      gdk_gl_glGetMapControlPointsNV(proc, target, index, type, ustride, vstride, packed, points) \
  ( ((GdkGLProc_glGetMapControlPointsNV) (proc)) (target, index, type, ustride, vstride, packed, points) )

/* glGetMapParameterivNV */
typedef void (APIENTRYP GdkGLProc_glGetMapParameterivNV) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMapParameterivNV (void);
#define      gdk_gl_glGetMapParameterivNV(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMapParameterivNV) (proc)) (target, pname, params) )

/* glGetMapParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetMapParameterfvNV) (GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMapParameterfvNV (void);
#define      gdk_gl_glGetMapParameterfvNV(proc, target, pname, params) \
  ( ((GdkGLProc_glGetMapParameterfvNV) (proc)) (target, pname, params) )

/* glGetMapAttribParameterivNV */
typedef void (APIENTRYP GdkGLProc_glGetMapAttribParameterivNV) (GLenum target, GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMapAttribParameterivNV (void);
#define      gdk_gl_glGetMapAttribParameterivNV(proc, target, index, pname, params) \
  ( ((GdkGLProc_glGetMapAttribParameterivNV) (proc)) (target, index, pname, params) )

/* glGetMapAttribParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetMapAttribParameterfvNV) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMapAttribParameterfvNV (void);
#define      gdk_gl_glGetMapAttribParameterfvNV(proc, target, index, pname, params) \
  ( ((GdkGLProc_glGetMapAttribParameterfvNV) (proc)) (target, index, pname, params) )

/* glEvalMapsNV */
typedef void (APIENTRYP GdkGLProc_glEvalMapsNV) (GLenum target, GLenum mode);
GdkGLProc    gdk_gl_get_glEvalMapsNV (void);
#define      gdk_gl_glEvalMapsNV(proc, target, mode) \
  ( ((GdkGLProc_glEvalMapsNV) (proc)) (target, mode) )

/* proc struct */

typedef struct _GdkGL_GL_NV_evaluators GdkGL_GL_NV_evaluators;

struct _GdkGL_GL_NV_evaluators
{
  GdkGLProc_glMapControlPointsNV glMapControlPointsNV;
  GdkGLProc_glMapParameterivNV glMapParameterivNV;
  GdkGLProc_glMapParameterfvNV glMapParameterfvNV;
  GdkGLProc_glGetMapControlPointsNV glGetMapControlPointsNV;
  GdkGLProc_glGetMapParameterivNV glGetMapParameterivNV;
  GdkGLProc_glGetMapParameterfvNV glGetMapParameterfvNV;
  GdkGLProc_glGetMapAttribParameterivNV glGetMapAttribParameterivNV;
  GdkGLProc_glGetMapAttribParameterfvNV glGetMapAttribParameterfvNV;
  GdkGLProc_glEvalMapsNV glEvalMapsNV;
};

GdkGL_GL_NV_evaluators *gdk_gl_get_GL_NV_evaluators (void);

/*
 * GL_NV_register_combiners2
 */

/* glCombinerStageParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glCombinerStageParameterfvNV) (GLenum stage, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glCombinerStageParameterfvNV (void);
#define      gdk_gl_glCombinerStageParameterfvNV(proc, stage, pname, params) \
  ( ((GdkGLProc_glCombinerStageParameterfvNV) (proc)) (stage, pname, params) )

/* glGetCombinerStageParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetCombinerStageParameterfvNV) (GLenum stage, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetCombinerStageParameterfvNV (void);
#define      gdk_gl_glGetCombinerStageParameterfvNV(proc, stage, pname, params) \
  ( ((GdkGLProc_glGetCombinerStageParameterfvNV) (proc)) (stage, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_register_combiners2 GdkGL_GL_NV_register_combiners2;

struct _GdkGL_GL_NV_register_combiners2
{
  GdkGLProc_glCombinerStageParameterfvNV glCombinerStageParameterfvNV;
  GdkGLProc_glGetCombinerStageParameterfvNV glGetCombinerStageParameterfvNV;
};

GdkGL_GL_NV_register_combiners2 *gdk_gl_get_GL_NV_register_combiners2 (void);

/*
 * GL_NV_vertex_program
 */

/* glAreProgramsResidentNV */
typedef GLboolean (APIENTRYP GdkGLProc_glAreProgramsResidentNV) (GLsizei n, const GLuint *programs, GLboolean *residences);
GdkGLProc    gdk_gl_get_glAreProgramsResidentNV (void);
#define      gdk_gl_glAreProgramsResidentNV(proc, n, programs, residences) \
  ( ((GdkGLProc_glAreProgramsResidentNV) (proc)) (n, programs, residences) )

/* glBindProgramNV */
typedef void (APIENTRYP GdkGLProc_glBindProgramNV) (GLenum target, GLuint id);
GdkGLProc    gdk_gl_get_glBindProgramNV (void);
#define      gdk_gl_glBindProgramNV(proc, target, id) \
  ( ((GdkGLProc_glBindProgramNV) (proc)) (target, id) )

/* glDeleteProgramsNV */
typedef void (APIENTRYP GdkGLProc_glDeleteProgramsNV) (GLsizei n, const GLuint *programs);
GdkGLProc    gdk_gl_get_glDeleteProgramsNV (void);
#define      gdk_gl_glDeleteProgramsNV(proc, n, programs) \
  ( ((GdkGLProc_glDeleteProgramsNV) (proc)) (n, programs) )

/* glExecuteProgramNV */
typedef void (APIENTRYP GdkGLProc_glExecuteProgramNV) (GLenum target, GLuint id, const GLfloat *params);
GdkGLProc    gdk_gl_get_glExecuteProgramNV (void);
#define      gdk_gl_glExecuteProgramNV(proc, target, id, params) \
  ( ((GdkGLProc_glExecuteProgramNV) (proc)) (target, id, params) )

/* glGenProgramsNV */
typedef void (APIENTRYP GdkGLProc_glGenProgramsNV) (GLsizei n, GLuint *programs);
GdkGLProc    gdk_gl_get_glGenProgramsNV (void);
#define      gdk_gl_glGenProgramsNV(proc, n, programs) \
  ( ((GdkGLProc_glGenProgramsNV) (proc)) (n, programs) )

/* glGetProgramParameterdvNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramParameterdvNV) (GLenum target, GLuint index, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetProgramParameterdvNV (void);
#define      gdk_gl_glGetProgramParameterdvNV(proc, target, index, pname, params) \
  ( ((GdkGLProc_glGetProgramParameterdvNV) (proc)) (target, index, pname, params) )

/* glGetProgramParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramParameterfvNV) (GLenum target, GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetProgramParameterfvNV (void);
#define      gdk_gl_glGetProgramParameterfvNV(proc, target, index, pname, params) \
  ( ((GdkGLProc_glGetProgramParameterfvNV) (proc)) (target, index, pname, params) )

/* glGetProgramivNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramivNV) (GLuint id, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetProgramivNV (void);
#define      gdk_gl_glGetProgramivNV(proc, id, pname, params) \
  ( ((GdkGLProc_glGetProgramivNV) (proc)) (id, pname, params) )

/* glGetProgramStringNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramStringNV) (GLuint id, GLenum pname, GLubyte *program);
GdkGLProc    gdk_gl_get_glGetProgramStringNV (void);
#define      gdk_gl_glGetProgramStringNV(proc, id, pname, program) \
  ( ((GdkGLProc_glGetProgramStringNV) (proc)) (id, pname, program) )

/* glGetTrackMatrixivNV */
typedef void (APIENTRYP GdkGLProc_glGetTrackMatrixivNV) (GLenum target, GLuint address, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTrackMatrixivNV (void);
#define      gdk_gl_glGetTrackMatrixivNV(proc, target, address, pname, params) \
  ( ((GdkGLProc_glGetTrackMatrixivNV) (proc)) (target, address, pname, params) )

/* glGetVertexAttribdvNV */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribdvNV) (GLuint index, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribdvNV (void);
#define      gdk_gl_glGetVertexAttribdvNV(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribdvNV) (proc)) (index, pname, params) )

/* glGetVertexAttribfvNV */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribfvNV) (GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribfvNV (void);
#define      gdk_gl_glGetVertexAttribfvNV(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribfvNV) (proc)) (index, pname, params) )

/* glGetVertexAttribivNV */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribivNV) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribivNV (void);
#define      gdk_gl_glGetVertexAttribivNV(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribivNV) (proc)) (index, pname, params) )

/* glGetVertexAttribPointervNV */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribPointervNV) (GLuint index, GLenum pname, GLvoid* *pointer);
GdkGLProc    gdk_gl_get_glGetVertexAttribPointervNV (void);
#define      gdk_gl_glGetVertexAttribPointervNV(proc, index, pname, pointer) \
  ( ((GdkGLProc_glGetVertexAttribPointervNV) (proc)) (index, pname, pointer) )

/* glIsProgramNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsProgramNV) (GLuint id);
GdkGLProc    gdk_gl_get_glIsProgramNV (void);
#define      gdk_gl_glIsProgramNV(proc, id) \
  ( ((GdkGLProc_glIsProgramNV) (proc)) (id) )

/* glLoadProgramNV */
typedef void (APIENTRYP GdkGLProc_glLoadProgramNV) (GLenum target, GLuint id, GLsizei len, const GLubyte *program);
GdkGLProc    gdk_gl_get_glLoadProgramNV (void);
#define      gdk_gl_glLoadProgramNV(proc, target, id, len, program) \
  ( ((GdkGLProc_glLoadProgramNV) (proc)) (target, id, len, program) )

/* glProgramParameter4dNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameter4dNV) (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glProgramParameter4dNV (void);
#define      gdk_gl_glProgramParameter4dNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramParameter4dNV) (proc)) (target, index, x, y, z, w) )

/* glProgramParameter4dvNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameter4dvNV) (GLenum target, GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glProgramParameter4dvNV (void);
#define      gdk_gl_glProgramParameter4dvNV(proc, target, index, v) \
  ( ((GdkGLProc_glProgramParameter4dvNV) (proc)) (target, index, v) )

/* glProgramParameter4fNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameter4fNV) (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glProgramParameter4fNV (void);
#define      gdk_gl_glProgramParameter4fNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramParameter4fNV) (proc)) (target, index, x, y, z, w) )

/* glProgramParameter4fvNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameter4fvNV) (GLenum target, GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glProgramParameter4fvNV (void);
#define      gdk_gl_glProgramParameter4fvNV(proc, target, index, v) \
  ( ((GdkGLProc_glProgramParameter4fvNV) (proc)) (target, index, v) )

/* glProgramParameters4dvNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameters4dvNV) (GLenum target, GLuint index, GLuint count, const GLdouble *v);
GdkGLProc    gdk_gl_get_glProgramParameters4dvNV (void);
#define      gdk_gl_glProgramParameters4dvNV(proc, target, index, count, v) \
  ( ((GdkGLProc_glProgramParameters4dvNV) (proc)) (target, index, count, v) )

/* glProgramParameters4fvNV */
typedef void (APIENTRYP GdkGLProc_glProgramParameters4fvNV) (GLenum target, GLuint index, GLuint count, const GLfloat *v);
GdkGLProc    gdk_gl_get_glProgramParameters4fvNV (void);
#define      gdk_gl_glProgramParameters4fvNV(proc, target, index, count, v) \
  ( ((GdkGLProc_glProgramParameters4fvNV) (proc)) (target, index, count, v) )

/* glRequestResidentProgramsNV */
typedef void (APIENTRYP GdkGLProc_glRequestResidentProgramsNV) (GLsizei n, const GLuint *programs);
GdkGLProc    gdk_gl_get_glRequestResidentProgramsNV (void);
#define      gdk_gl_glRequestResidentProgramsNV(proc, n, programs) \
  ( ((GdkGLProc_glRequestResidentProgramsNV) (proc)) (n, programs) )

/* glTrackMatrixNV */
typedef void (APIENTRYP GdkGLProc_glTrackMatrixNV) (GLenum target, GLuint address, GLenum matrix, GLenum transform);
GdkGLProc    gdk_gl_get_glTrackMatrixNV (void);
#define      gdk_gl_glTrackMatrixNV(proc, target, address, matrix, transform) \
  ( ((GdkGLProc_glTrackMatrixNV) (proc)) (target, address, matrix, transform) )

/* glVertexAttribPointerNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribPointerNV) (GLuint index, GLint fsize, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexAttribPointerNV (void);
#define      gdk_gl_glVertexAttribPointerNV(proc, index, fsize, type, stride, pointer) \
  ( ((GdkGLProc_glVertexAttribPointerNV) (proc)) (index, fsize, type, stride, pointer) )

/* glVertexAttrib1dNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1dNV) (GLuint index, GLdouble x);
GdkGLProc    gdk_gl_get_glVertexAttrib1dNV (void);
#define      gdk_gl_glVertexAttrib1dNV(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1dNV) (proc)) (index, x) )

/* glVertexAttrib1dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1dvNV) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1dvNV (void);
#define      gdk_gl_glVertexAttrib1dvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1dvNV) (proc)) (index, v) )

/* glVertexAttrib1fNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1fNV) (GLuint index, GLfloat x);
GdkGLProc    gdk_gl_get_glVertexAttrib1fNV (void);
#define      gdk_gl_glVertexAttrib1fNV(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1fNV) (proc)) (index, x) )

/* glVertexAttrib1fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1fvNV) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1fvNV (void);
#define      gdk_gl_glVertexAttrib1fvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1fvNV) (proc)) (index, v) )

/* glVertexAttrib1sNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1sNV) (GLuint index, GLshort x);
GdkGLProc    gdk_gl_get_glVertexAttrib1sNV (void);
#define      gdk_gl_glVertexAttrib1sNV(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1sNV) (proc)) (index, x) )

/* glVertexAttrib1svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1svNV) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1svNV (void);
#define      gdk_gl_glVertexAttrib1svNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1svNV) (proc)) (index, v) )

/* glVertexAttrib2dNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2dNV) (GLuint index, GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glVertexAttrib2dNV (void);
#define      gdk_gl_glVertexAttrib2dNV(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2dNV) (proc)) (index, x, y) )

/* glVertexAttrib2dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2dvNV) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2dvNV (void);
#define      gdk_gl_glVertexAttrib2dvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2dvNV) (proc)) (index, v) )

/* glVertexAttrib2fNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2fNV) (GLuint index, GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glVertexAttrib2fNV (void);
#define      gdk_gl_glVertexAttrib2fNV(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2fNV) (proc)) (index, x, y) )

/* glVertexAttrib2fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2fvNV) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2fvNV (void);
#define      gdk_gl_glVertexAttrib2fvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2fvNV) (proc)) (index, v) )

/* glVertexAttrib2sNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2sNV) (GLuint index, GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glVertexAttrib2sNV (void);
#define      gdk_gl_glVertexAttrib2sNV(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2sNV) (proc)) (index, x, y) )

/* glVertexAttrib2svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2svNV) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2svNV (void);
#define      gdk_gl_glVertexAttrib2svNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2svNV) (proc)) (index, v) )

/* glVertexAttrib3dNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3dNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glVertexAttrib3dNV (void);
#define      gdk_gl_glVertexAttrib3dNV(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3dNV) (proc)) (index, x, y, z) )

/* glVertexAttrib3dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3dvNV) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3dvNV (void);
#define      gdk_gl_glVertexAttrib3dvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3dvNV) (proc)) (index, v) )

/* glVertexAttrib3fNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3fNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glVertexAttrib3fNV (void);
#define      gdk_gl_glVertexAttrib3fNV(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3fNV) (proc)) (index, x, y, z) )

/* glVertexAttrib3fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3fvNV) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3fvNV (void);
#define      gdk_gl_glVertexAttrib3fvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3fvNV) (proc)) (index, v) )

/* glVertexAttrib3sNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3sNV) (GLuint index, GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glVertexAttrib3sNV (void);
#define      gdk_gl_glVertexAttrib3sNV(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3sNV) (proc)) (index, x, y, z) )

/* glVertexAttrib3svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3svNV) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3svNV (void);
#define      gdk_gl_glVertexAttrib3svNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3svNV) (proc)) (index, v) )

/* glVertexAttrib4dNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4dNV) (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glVertexAttrib4dNV (void);
#define      gdk_gl_glVertexAttrib4dNV(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4dNV) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4dvNV) (GLuint index, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4dvNV (void);
#define      gdk_gl_glVertexAttrib4dvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4dvNV) (proc)) (index, v) )

/* glVertexAttrib4fNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4fNV) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glVertexAttrib4fNV (void);
#define      gdk_gl_glVertexAttrib4fNV(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4fNV) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4fvNV) (GLuint index, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4fvNV (void);
#define      gdk_gl_glVertexAttrib4fvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4fvNV) (proc)) (index, v) )

/* glVertexAttrib4sNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4sNV) (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
GdkGLProc    gdk_gl_get_glVertexAttrib4sNV (void);
#define      gdk_gl_glVertexAttrib4sNV(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4sNV) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4svNV) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4svNV (void);
#define      gdk_gl_glVertexAttrib4svNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4svNV) (proc)) (index, v) )

/* glVertexAttrib4ubNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4ubNV) (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
GdkGLProc    gdk_gl_get_glVertexAttrib4ubNV (void);
#define      gdk_gl_glVertexAttrib4ubNV(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4ubNV) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4ubvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4ubvNV) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4ubvNV (void);
#define      gdk_gl_glVertexAttrib4ubvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4ubvNV) (proc)) (index, v) )

/* glVertexAttribs1dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs1dvNV) (GLuint index, GLsizei count, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttribs1dvNV (void);
#define      gdk_gl_glVertexAttribs1dvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs1dvNV) (proc)) (index, count, v) )

/* glVertexAttribs1fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs1fvNV) (GLuint index, GLsizei count, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttribs1fvNV (void);
#define      gdk_gl_glVertexAttribs1fvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs1fvNV) (proc)) (index, count, v) )

/* glVertexAttribs1svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs1svNV) (GLuint index, GLsizei count, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribs1svNV (void);
#define      gdk_gl_glVertexAttribs1svNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs1svNV) (proc)) (index, count, v) )

/* glVertexAttribs2dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs2dvNV) (GLuint index, GLsizei count, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttribs2dvNV (void);
#define      gdk_gl_glVertexAttribs2dvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs2dvNV) (proc)) (index, count, v) )

/* glVertexAttribs2fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs2fvNV) (GLuint index, GLsizei count, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttribs2fvNV (void);
#define      gdk_gl_glVertexAttribs2fvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs2fvNV) (proc)) (index, count, v) )

/* glVertexAttribs2svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs2svNV) (GLuint index, GLsizei count, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribs2svNV (void);
#define      gdk_gl_glVertexAttribs2svNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs2svNV) (proc)) (index, count, v) )

/* glVertexAttribs3dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs3dvNV) (GLuint index, GLsizei count, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttribs3dvNV (void);
#define      gdk_gl_glVertexAttribs3dvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs3dvNV) (proc)) (index, count, v) )

/* glVertexAttribs3fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs3fvNV) (GLuint index, GLsizei count, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttribs3fvNV (void);
#define      gdk_gl_glVertexAttribs3fvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs3fvNV) (proc)) (index, count, v) )

/* glVertexAttribs3svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs3svNV) (GLuint index, GLsizei count, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribs3svNV (void);
#define      gdk_gl_glVertexAttribs3svNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs3svNV) (proc)) (index, count, v) )

/* glVertexAttribs4dvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs4dvNV) (GLuint index, GLsizei count, const GLdouble *v);
GdkGLProc    gdk_gl_get_glVertexAttribs4dvNV (void);
#define      gdk_gl_glVertexAttribs4dvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs4dvNV) (proc)) (index, count, v) )

/* glVertexAttribs4fvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs4fvNV) (GLuint index, GLsizei count, const GLfloat *v);
GdkGLProc    gdk_gl_get_glVertexAttribs4fvNV (void);
#define      gdk_gl_glVertexAttribs4fvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs4fvNV) (proc)) (index, count, v) )

/* glVertexAttribs4svNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs4svNV) (GLuint index, GLsizei count, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribs4svNV (void);
#define      gdk_gl_glVertexAttribs4svNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs4svNV) (proc)) (index, count, v) )

/* glVertexAttribs4ubvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs4ubvNV) (GLuint index, GLsizei count, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttribs4ubvNV (void);
#define      gdk_gl_glVertexAttribs4ubvNV(proc, index, count, v) \
  ( ((GdkGLProc_glVertexAttribs4ubvNV) (proc)) (index, count, v) )

/* proc struct */

typedef struct _GdkGL_GL_NV_vertex_program GdkGL_GL_NV_vertex_program;

struct _GdkGL_GL_NV_vertex_program
{
  GdkGLProc_glAreProgramsResidentNV glAreProgramsResidentNV;
  GdkGLProc_glBindProgramNV glBindProgramNV;
  GdkGLProc_glDeleteProgramsNV glDeleteProgramsNV;
  GdkGLProc_glExecuteProgramNV glExecuteProgramNV;
  GdkGLProc_glGenProgramsNV glGenProgramsNV;
  GdkGLProc_glGetProgramParameterdvNV glGetProgramParameterdvNV;
  GdkGLProc_glGetProgramParameterfvNV glGetProgramParameterfvNV;
  GdkGLProc_glGetProgramivNV glGetProgramivNV;
  GdkGLProc_glGetProgramStringNV glGetProgramStringNV;
  GdkGLProc_glGetTrackMatrixivNV glGetTrackMatrixivNV;
  GdkGLProc_glGetVertexAttribdvNV glGetVertexAttribdvNV;
  GdkGLProc_glGetVertexAttribfvNV glGetVertexAttribfvNV;
  GdkGLProc_glGetVertexAttribivNV glGetVertexAttribivNV;
  GdkGLProc_glGetVertexAttribPointervNV glGetVertexAttribPointervNV;
  GdkGLProc_glIsProgramNV glIsProgramNV;
  GdkGLProc_glLoadProgramNV glLoadProgramNV;
  GdkGLProc_glProgramParameter4dNV glProgramParameter4dNV;
  GdkGLProc_glProgramParameter4dvNV glProgramParameter4dvNV;
  GdkGLProc_glProgramParameter4fNV glProgramParameter4fNV;
  GdkGLProc_glProgramParameter4fvNV glProgramParameter4fvNV;
  GdkGLProc_glProgramParameters4dvNV glProgramParameters4dvNV;
  GdkGLProc_glProgramParameters4fvNV glProgramParameters4fvNV;
  GdkGLProc_glRequestResidentProgramsNV glRequestResidentProgramsNV;
  GdkGLProc_glTrackMatrixNV glTrackMatrixNV;
  GdkGLProc_glVertexAttribPointerNV glVertexAttribPointerNV;
  GdkGLProc_glVertexAttrib1dNV glVertexAttrib1dNV;
  GdkGLProc_glVertexAttrib1dvNV glVertexAttrib1dvNV;
  GdkGLProc_glVertexAttrib1fNV glVertexAttrib1fNV;
  GdkGLProc_glVertexAttrib1fvNV glVertexAttrib1fvNV;
  GdkGLProc_glVertexAttrib1sNV glVertexAttrib1sNV;
  GdkGLProc_glVertexAttrib1svNV glVertexAttrib1svNV;
  GdkGLProc_glVertexAttrib2dNV glVertexAttrib2dNV;
  GdkGLProc_glVertexAttrib2dvNV glVertexAttrib2dvNV;
  GdkGLProc_glVertexAttrib2fNV glVertexAttrib2fNV;
  GdkGLProc_glVertexAttrib2fvNV glVertexAttrib2fvNV;
  GdkGLProc_glVertexAttrib2sNV glVertexAttrib2sNV;
  GdkGLProc_glVertexAttrib2svNV glVertexAttrib2svNV;
  GdkGLProc_glVertexAttrib3dNV glVertexAttrib3dNV;
  GdkGLProc_glVertexAttrib3dvNV glVertexAttrib3dvNV;
  GdkGLProc_glVertexAttrib3fNV glVertexAttrib3fNV;
  GdkGLProc_glVertexAttrib3fvNV glVertexAttrib3fvNV;
  GdkGLProc_glVertexAttrib3sNV glVertexAttrib3sNV;
  GdkGLProc_glVertexAttrib3svNV glVertexAttrib3svNV;
  GdkGLProc_glVertexAttrib4dNV glVertexAttrib4dNV;
  GdkGLProc_glVertexAttrib4dvNV glVertexAttrib4dvNV;
  GdkGLProc_glVertexAttrib4fNV glVertexAttrib4fNV;
  GdkGLProc_glVertexAttrib4fvNV glVertexAttrib4fvNV;
  GdkGLProc_glVertexAttrib4sNV glVertexAttrib4sNV;
  GdkGLProc_glVertexAttrib4svNV glVertexAttrib4svNV;
  GdkGLProc_glVertexAttrib4ubNV glVertexAttrib4ubNV;
  GdkGLProc_glVertexAttrib4ubvNV glVertexAttrib4ubvNV;
  GdkGLProc_glVertexAttribs1dvNV glVertexAttribs1dvNV;
  GdkGLProc_glVertexAttribs1fvNV glVertexAttribs1fvNV;
  GdkGLProc_glVertexAttribs1svNV glVertexAttribs1svNV;
  GdkGLProc_glVertexAttribs2dvNV glVertexAttribs2dvNV;
  GdkGLProc_glVertexAttribs2fvNV glVertexAttribs2fvNV;
  GdkGLProc_glVertexAttribs2svNV glVertexAttribs2svNV;
  GdkGLProc_glVertexAttribs3dvNV glVertexAttribs3dvNV;
  GdkGLProc_glVertexAttribs3fvNV glVertexAttribs3fvNV;
  GdkGLProc_glVertexAttribs3svNV glVertexAttribs3svNV;
  GdkGLProc_glVertexAttribs4dvNV glVertexAttribs4dvNV;
  GdkGLProc_glVertexAttribs4fvNV glVertexAttribs4fvNV;
  GdkGLProc_glVertexAttribs4svNV glVertexAttribs4svNV;
  GdkGLProc_glVertexAttribs4ubvNV glVertexAttribs4ubvNV;
};

GdkGL_GL_NV_vertex_program *gdk_gl_get_GL_NV_vertex_program (void);

/*
 * GL_ATI_envmap_bumpmap
 */

/* glTexBumpParameterivATI */
typedef void (APIENTRYP GdkGLProc_glTexBumpParameterivATI) (GLenum pname, const GLint *param);
GdkGLProc    gdk_gl_get_glTexBumpParameterivATI (void);
#define      gdk_gl_glTexBumpParameterivATI(proc, pname, param) \
  ( ((GdkGLProc_glTexBumpParameterivATI) (proc)) (pname, param) )

/* glTexBumpParameterfvATI */
typedef void (APIENTRYP GdkGLProc_glTexBumpParameterfvATI) (GLenum pname, const GLfloat *param);
GdkGLProc    gdk_gl_get_glTexBumpParameterfvATI (void);
#define      gdk_gl_glTexBumpParameterfvATI(proc, pname, param) \
  ( ((GdkGLProc_glTexBumpParameterfvATI) (proc)) (pname, param) )

/* glGetTexBumpParameterivATI */
typedef void (APIENTRYP GdkGLProc_glGetTexBumpParameterivATI) (GLenum pname, GLint *param);
GdkGLProc    gdk_gl_get_glGetTexBumpParameterivATI (void);
#define      gdk_gl_glGetTexBumpParameterivATI(proc, pname, param) \
  ( ((GdkGLProc_glGetTexBumpParameterivATI) (proc)) (pname, param) )

/* glGetTexBumpParameterfvATI */
typedef void (APIENTRYP GdkGLProc_glGetTexBumpParameterfvATI) (GLenum pname, GLfloat *param);
GdkGLProc    gdk_gl_get_glGetTexBumpParameterfvATI (void);
#define      gdk_gl_glGetTexBumpParameterfvATI(proc, pname, param) \
  ( ((GdkGLProc_glGetTexBumpParameterfvATI) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_envmap_bumpmap GdkGL_GL_ATI_envmap_bumpmap;

struct _GdkGL_GL_ATI_envmap_bumpmap
{
  GdkGLProc_glTexBumpParameterivATI glTexBumpParameterivATI;
  GdkGLProc_glTexBumpParameterfvATI glTexBumpParameterfvATI;
  GdkGLProc_glGetTexBumpParameterivATI glGetTexBumpParameterivATI;
  GdkGLProc_glGetTexBumpParameterfvATI glGetTexBumpParameterfvATI;
};

GdkGL_GL_ATI_envmap_bumpmap *gdk_gl_get_GL_ATI_envmap_bumpmap (void);

/*
 * GL_ATI_fragment_shader
 */

/* glGenFragmentShadersATI */
typedef GLuint (APIENTRYP GdkGLProc_glGenFragmentShadersATI) (GLuint range);
GdkGLProc    gdk_gl_get_glGenFragmentShadersATI (void);
#define      gdk_gl_glGenFragmentShadersATI(proc, range) \
  ( ((GdkGLProc_glGenFragmentShadersATI) (proc)) (range) )

/* glBindFragmentShaderATI */
typedef void (APIENTRYP GdkGLProc_glBindFragmentShaderATI) (GLuint id);
GdkGLProc    gdk_gl_get_glBindFragmentShaderATI (void);
#define      gdk_gl_glBindFragmentShaderATI(proc, id) \
  ( ((GdkGLProc_glBindFragmentShaderATI) (proc)) (id) )

/* glDeleteFragmentShaderATI */
typedef void (APIENTRYP GdkGLProc_glDeleteFragmentShaderATI) (GLuint id);
GdkGLProc    gdk_gl_get_glDeleteFragmentShaderATI (void);
#define      gdk_gl_glDeleteFragmentShaderATI(proc, id) \
  ( ((GdkGLProc_glDeleteFragmentShaderATI) (proc)) (id) )

/* glBeginFragmentShaderATI */
typedef void (APIENTRYP GdkGLProc_glBeginFragmentShaderATI) (void);
GdkGLProc    gdk_gl_get_glBeginFragmentShaderATI (void);
#define      gdk_gl_glBeginFragmentShaderATI(proc) \
  ( ((GdkGLProc_glBeginFragmentShaderATI) (proc)) () )

/* glEndFragmentShaderATI */
typedef void (APIENTRYP GdkGLProc_glEndFragmentShaderATI) (void);
GdkGLProc    gdk_gl_get_glEndFragmentShaderATI (void);
#define      gdk_gl_glEndFragmentShaderATI(proc) \
  ( ((GdkGLProc_glEndFragmentShaderATI) (proc)) () )

/* glPassTexCoordATI */
typedef void (APIENTRYP GdkGLProc_glPassTexCoordATI) (GLuint dst, GLuint coord, GLenum swizzle);
GdkGLProc    gdk_gl_get_glPassTexCoordATI (void);
#define      gdk_gl_glPassTexCoordATI(proc, dst, coord, swizzle) \
  ( ((GdkGLProc_glPassTexCoordATI) (proc)) (dst, coord, swizzle) )

/* glSampleMapATI */
typedef void (APIENTRYP GdkGLProc_glSampleMapATI) (GLuint dst, GLuint interp, GLenum swizzle);
GdkGLProc    gdk_gl_get_glSampleMapATI (void);
#define      gdk_gl_glSampleMapATI(proc, dst, interp, swizzle) \
  ( ((GdkGLProc_glSampleMapATI) (proc)) (dst, interp, swizzle) )

/* glColorFragmentOp1ATI */
typedef void (APIENTRYP GdkGLProc_glColorFragmentOp1ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
GdkGLProc    gdk_gl_get_glColorFragmentOp1ATI (void);
#define      gdk_gl_glColorFragmentOp1ATI(proc, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod) \
  ( ((GdkGLProc_glColorFragmentOp1ATI) (proc)) (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod) )

/* glColorFragmentOp2ATI */
typedef void (APIENTRYP GdkGLProc_glColorFragmentOp2ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
GdkGLProc    gdk_gl_get_glColorFragmentOp2ATI (void);
#define      gdk_gl_glColorFragmentOp2ATI(proc, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod) \
  ( ((GdkGLProc_glColorFragmentOp2ATI) (proc)) (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod) )

/* glColorFragmentOp3ATI */
typedef void (APIENTRYP GdkGLProc_glColorFragmentOp3ATI) (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
GdkGLProc    gdk_gl_get_glColorFragmentOp3ATI (void);
#define      gdk_gl_glColorFragmentOp3ATI(proc, op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod) \
  ( ((GdkGLProc_glColorFragmentOp3ATI) (proc)) (op, dst, dstMask, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod) )

/* glAlphaFragmentOp1ATI */
typedef void (APIENTRYP GdkGLProc_glAlphaFragmentOp1ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod);
GdkGLProc    gdk_gl_get_glAlphaFragmentOp1ATI (void);
#define      gdk_gl_glAlphaFragmentOp1ATI(proc, op, dst, dstMod, arg1, arg1Rep, arg1Mod) \
  ( ((GdkGLProc_glAlphaFragmentOp1ATI) (proc)) (op, dst, dstMod, arg1, arg1Rep, arg1Mod) )

/* glAlphaFragmentOp2ATI */
typedef void (APIENTRYP GdkGLProc_glAlphaFragmentOp2ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod);
GdkGLProc    gdk_gl_get_glAlphaFragmentOp2ATI (void);
#define      gdk_gl_glAlphaFragmentOp2ATI(proc, op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod) \
  ( ((GdkGLProc_glAlphaFragmentOp2ATI) (proc)) (op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod) )

/* glAlphaFragmentOp3ATI */
typedef void (APIENTRYP GdkGLProc_glAlphaFragmentOp3ATI) (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod);
GdkGLProc    gdk_gl_get_glAlphaFragmentOp3ATI (void);
#define      gdk_gl_glAlphaFragmentOp3ATI(proc, op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod) \
  ( ((GdkGLProc_glAlphaFragmentOp3ATI) (proc)) (op, dst, dstMod, arg1, arg1Rep, arg1Mod, arg2, arg2Rep, arg2Mod, arg3, arg3Rep, arg3Mod) )

/* glSetFragmentShaderConstantATI */
typedef void (APIENTRYP GdkGLProc_glSetFragmentShaderConstantATI) (GLuint dst, const GLfloat *value);
GdkGLProc    gdk_gl_get_glSetFragmentShaderConstantATI (void);
#define      gdk_gl_glSetFragmentShaderConstantATI(proc, dst, value) \
  ( ((GdkGLProc_glSetFragmentShaderConstantATI) (proc)) (dst, value) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_fragment_shader GdkGL_GL_ATI_fragment_shader;

struct _GdkGL_GL_ATI_fragment_shader
{
  GdkGLProc_glGenFragmentShadersATI glGenFragmentShadersATI;
  GdkGLProc_glBindFragmentShaderATI glBindFragmentShaderATI;
  GdkGLProc_glDeleteFragmentShaderATI glDeleteFragmentShaderATI;
  GdkGLProc_glBeginFragmentShaderATI glBeginFragmentShaderATI;
  GdkGLProc_glEndFragmentShaderATI glEndFragmentShaderATI;
  GdkGLProc_glPassTexCoordATI glPassTexCoordATI;
  GdkGLProc_glSampleMapATI glSampleMapATI;
  GdkGLProc_glColorFragmentOp1ATI glColorFragmentOp1ATI;
  GdkGLProc_glColorFragmentOp2ATI glColorFragmentOp2ATI;
  GdkGLProc_glColorFragmentOp3ATI glColorFragmentOp3ATI;
  GdkGLProc_glAlphaFragmentOp1ATI glAlphaFragmentOp1ATI;
  GdkGLProc_glAlphaFragmentOp2ATI glAlphaFragmentOp2ATI;
  GdkGLProc_glAlphaFragmentOp3ATI glAlphaFragmentOp3ATI;
  GdkGLProc_glSetFragmentShaderConstantATI glSetFragmentShaderConstantATI;
};

GdkGL_GL_ATI_fragment_shader *gdk_gl_get_GL_ATI_fragment_shader (void);

/*
 * GL_ATI_pn_triangles
 */

/* glPNTrianglesiATI */
typedef void (APIENTRYP GdkGLProc_glPNTrianglesiATI) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPNTrianglesiATI (void);
#define      gdk_gl_glPNTrianglesiATI(proc, pname, param) \
  ( ((GdkGLProc_glPNTrianglesiATI) (proc)) (pname, param) )

/* glPNTrianglesfATI */
typedef void (APIENTRYP GdkGLProc_glPNTrianglesfATI) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPNTrianglesfATI (void);
#define      gdk_gl_glPNTrianglesfATI(proc, pname, param) \
  ( ((GdkGLProc_glPNTrianglesfATI) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_pn_triangles GdkGL_GL_ATI_pn_triangles;

struct _GdkGL_GL_ATI_pn_triangles
{
  GdkGLProc_glPNTrianglesiATI glPNTrianglesiATI;
  GdkGLProc_glPNTrianglesfATI glPNTrianglesfATI;
};

GdkGL_GL_ATI_pn_triangles *gdk_gl_get_GL_ATI_pn_triangles (void);

/*
 * GL_ATI_vertex_array_object
 */

/* glNewObjectBufferATI */
typedef GLuint (APIENTRYP GdkGLProc_glNewObjectBufferATI) (GLsizei size, const GLvoid *pointer, GLenum usage);
GdkGLProc    gdk_gl_get_glNewObjectBufferATI (void);
#define      gdk_gl_glNewObjectBufferATI(proc, size, pointer, usage) \
  ( ((GdkGLProc_glNewObjectBufferATI) (proc)) (size, pointer, usage) )

/* glIsObjectBufferATI */
typedef GLboolean (APIENTRYP GdkGLProc_glIsObjectBufferATI) (GLuint buffer);
GdkGLProc    gdk_gl_get_glIsObjectBufferATI (void);
#define      gdk_gl_glIsObjectBufferATI(proc, buffer) \
  ( ((GdkGLProc_glIsObjectBufferATI) (proc)) (buffer) )

/* glUpdateObjectBufferATI */
typedef void (APIENTRYP GdkGLProc_glUpdateObjectBufferATI) (GLuint buffer, GLuint offset, GLsizei size, const GLvoid *pointer, GLenum preserve);
GdkGLProc    gdk_gl_get_glUpdateObjectBufferATI (void);
#define      gdk_gl_glUpdateObjectBufferATI(proc, buffer, offset, size, pointer, preserve) \
  ( ((GdkGLProc_glUpdateObjectBufferATI) (proc)) (buffer, offset, size, pointer, preserve) )

/* glGetObjectBufferfvATI */
typedef void (APIENTRYP GdkGLProc_glGetObjectBufferfvATI) (GLuint buffer, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetObjectBufferfvATI (void);
#define      gdk_gl_glGetObjectBufferfvATI(proc, buffer, pname, params) \
  ( ((GdkGLProc_glGetObjectBufferfvATI) (proc)) (buffer, pname, params) )

/* glGetObjectBufferivATI */
typedef void (APIENTRYP GdkGLProc_glGetObjectBufferivATI) (GLuint buffer, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetObjectBufferivATI (void);
#define      gdk_gl_glGetObjectBufferivATI(proc, buffer, pname, params) \
  ( ((GdkGLProc_glGetObjectBufferivATI) (proc)) (buffer, pname, params) )

/* glFreeObjectBufferATI */
typedef void (APIENTRYP GdkGLProc_glFreeObjectBufferATI) (GLuint buffer);
GdkGLProc    gdk_gl_get_glFreeObjectBufferATI (void);
#define      gdk_gl_glFreeObjectBufferATI(proc, buffer) \
  ( ((GdkGLProc_glFreeObjectBufferATI) (proc)) (buffer) )

/* glArrayObjectATI */
typedef void (APIENTRYP GdkGLProc_glArrayObjectATI) (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
GdkGLProc    gdk_gl_get_glArrayObjectATI (void);
#define      gdk_gl_glArrayObjectATI(proc, array, size, type, stride, buffer, offset) \
  ( ((GdkGLProc_glArrayObjectATI) (proc)) (array, size, type, stride, buffer, offset) )

/* glGetArrayObjectfvATI */
typedef void (APIENTRYP GdkGLProc_glGetArrayObjectfvATI) (GLenum array, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetArrayObjectfvATI (void);
#define      gdk_gl_glGetArrayObjectfvATI(proc, array, pname, params) \
  ( ((GdkGLProc_glGetArrayObjectfvATI) (proc)) (array, pname, params) )

/* glGetArrayObjectivATI */
typedef void (APIENTRYP GdkGLProc_glGetArrayObjectivATI) (GLenum array, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetArrayObjectivATI (void);
#define      gdk_gl_glGetArrayObjectivATI(proc, array, pname, params) \
  ( ((GdkGLProc_glGetArrayObjectivATI) (proc)) (array, pname, params) )

/* glVariantArrayObjectATI */
typedef void (APIENTRYP GdkGLProc_glVariantArrayObjectATI) (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset);
GdkGLProc    gdk_gl_get_glVariantArrayObjectATI (void);
#define      gdk_gl_glVariantArrayObjectATI(proc, id, type, stride, buffer, offset) \
  ( ((GdkGLProc_glVariantArrayObjectATI) (proc)) (id, type, stride, buffer, offset) )

/* glGetVariantArrayObjectfvATI */
typedef void (APIENTRYP GdkGLProc_glGetVariantArrayObjectfvATI) (GLuint id, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVariantArrayObjectfvATI (void);
#define      gdk_gl_glGetVariantArrayObjectfvATI(proc, id, pname, params) \
  ( ((GdkGLProc_glGetVariantArrayObjectfvATI) (proc)) (id, pname, params) )

/* glGetVariantArrayObjectivATI */
typedef void (APIENTRYP GdkGLProc_glGetVariantArrayObjectivATI) (GLuint id, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVariantArrayObjectivATI (void);
#define      gdk_gl_glGetVariantArrayObjectivATI(proc, id, pname, params) \
  ( ((GdkGLProc_glGetVariantArrayObjectivATI) (proc)) (id, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_vertex_array_object GdkGL_GL_ATI_vertex_array_object;

struct _GdkGL_GL_ATI_vertex_array_object
{
  GdkGLProc_glNewObjectBufferATI glNewObjectBufferATI;
  GdkGLProc_glIsObjectBufferATI glIsObjectBufferATI;
  GdkGLProc_glUpdateObjectBufferATI glUpdateObjectBufferATI;
  GdkGLProc_glGetObjectBufferfvATI glGetObjectBufferfvATI;
  GdkGLProc_glGetObjectBufferivATI glGetObjectBufferivATI;
  GdkGLProc_glFreeObjectBufferATI glFreeObjectBufferATI;
  GdkGLProc_glArrayObjectATI glArrayObjectATI;
  GdkGLProc_glGetArrayObjectfvATI glGetArrayObjectfvATI;
  GdkGLProc_glGetArrayObjectivATI glGetArrayObjectivATI;
  GdkGLProc_glVariantArrayObjectATI glVariantArrayObjectATI;
  GdkGLProc_glGetVariantArrayObjectfvATI glGetVariantArrayObjectfvATI;
  GdkGLProc_glGetVariantArrayObjectivATI glGetVariantArrayObjectivATI;
};

GdkGL_GL_ATI_vertex_array_object *gdk_gl_get_GL_ATI_vertex_array_object (void);

/*
 * GL_EXT_vertex_shader
 */

/* glBeginVertexShaderEXT */
typedef void (APIENTRYP GdkGLProc_glBeginVertexShaderEXT) (void);
GdkGLProc    gdk_gl_get_glBeginVertexShaderEXT (void);
#define      gdk_gl_glBeginVertexShaderEXT(proc) \
  ( ((GdkGLProc_glBeginVertexShaderEXT) (proc)) () )

/* glEndVertexShaderEXT */
typedef void (APIENTRYP GdkGLProc_glEndVertexShaderEXT) (void);
GdkGLProc    gdk_gl_get_glEndVertexShaderEXT (void);
#define      gdk_gl_glEndVertexShaderEXT(proc) \
  ( ((GdkGLProc_glEndVertexShaderEXT) (proc)) () )

/* glBindVertexShaderEXT */
typedef void (APIENTRYP GdkGLProc_glBindVertexShaderEXT) (GLuint id);
GdkGLProc    gdk_gl_get_glBindVertexShaderEXT (void);
#define      gdk_gl_glBindVertexShaderEXT(proc, id) \
  ( ((GdkGLProc_glBindVertexShaderEXT) (proc)) (id) )

/* glGenVertexShadersEXT */
typedef GLuint (APIENTRYP GdkGLProc_glGenVertexShadersEXT) (GLuint range);
GdkGLProc    gdk_gl_get_glGenVertexShadersEXT (void);
#define      gdk_gl_glGenVertexShadersEXT(proc, range) \
  ( ((GdkGLProc_glGenVertexShadersEXT) (proc)) (range) )

/* glDeleteVertexShaderEXT */
typedef void (APIENTRYP GdkGLProc_glDeleteVertexShaderEXT) (GLuint id);
GdkGLProc    gdk_gl_get_glDeleteVertexShaderEXT (void);
#define      gdk_gl_glDeleteVertexShaderEXT(proc, id) \
  ( ((GdkGLProc_glDeleteVertexShaderEXT) (proc)) (id) )

/* glShaderOp1EXT */
typedef void (APIENTRYP GdkGLProc_glShaderOp1EXT) (GLenum op, GLuint res, GLuint arg1);
GdkGLProc    gdk_gl_get_glShaderOp1EXT (void);
#define      gdk_gl_glShaderOp1EXT(proc, op, res, arg1) \
  ( ((GdkGLProc_glShaderOp1EXT) (proc)) (op, res, arg1) )

/* glShaderOp2EXT */
typedef void (APIENTRYP GdkGLProc_glShaderOp2EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2);
GdkGLProc    gdk_gl_get_glShaderOp2EXT (void);
#define      gdk_gl_glShaderOp2EXT(proc, op, res, arg1, arg2) \
  ( ((GdkGLProc_glShaderOp2EXT) (proc)) (op, res, arg1, arg2) )

/* glShaderOp3EXT */
typedef void (APIENTRYP GdkGLProc_glShaderOp3EXT) (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3);
GdkGLProc    gdk_gl_get_glShaderOp3EXT (void);
#define      gdk_gl_glShaderOp3EXT(proc, op, res, arg1, arg2, arg3) \
  ( ((GdkGLProc_glShaderOp3EXT) (proc)) (op, res, arg1, arg2, arg3) )

/* glSwizzleEXT */
typedef void (APIENTRYP GdkGLProc_glSwizzleEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
GdkGLProc    gdk_gl_get_glSwizzleEXT (void);
#define      gdk_gl_glSwizzleEXT(proc, res, in, outX, outY, outZ, outW) \
  ( ((GdkGLProc_glSwizzleEXT) (proc)) (res, in, outX, outY, outZ, outW) )

/* glWriteMaskEXT */
typedef void (APIENTRYP GdkGLProc_glWriteMaskEXT) (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW);
GdkGLProc    gdk_gl_get_glWriteMaskEXT (void);
#define      gdk_gl_glWriteMaskEXT(proc, res, in, outX, outY, outZ, outW) \
  ( ((GdkGLProc_glWriteMaskEXT) (proc)) (res, in, outX, outY, outZ, outW) )

/* glInsertComponentEXT */
typedef void (APIENTRYP GdkGLProc_glInsertComponentEXT) (GLuint res, GLuint src, GLuint num);
GdkGLProc    gdk_gl_get_glInsertComponentEXT (void);
#define      gdk_gl_glInsertComponentEXT(proc, res, src, num) \
  ( ((GdkGLProc_glInsertComponentEXT) (proc)) (res, src, num) )

/* glExtractComponentEXT */
typedef void (APIENTRYP GdkGLProc_glExtractComponentEXT) (GLuint res, GLuint src, GLuint num);
GdkGLProc    gdk_gl_get_glExtractComponentEXT (void);
#define      gdk_gl_glExtractComponentEXT(proc, res, src, num) \
  ( ((GdkGLProc_glExtractComponentEXT) (proc)) (res, src, num) )

/* glGenSymbolsEXT */
typedef GLuint (APIENTRYP GdkGLProc_glGenSymbolsEXT) (GLenum datatype, GLenum storagetype, GLenum range, GLuint components);
GdkGLProc    gdk_gl_get_glGenSymbolsEXT (void);
#define      gdk_gl_glGenSymbolsEXT(proc, datatype, storagetype, range, components) \
  ( ((GdkGLProc_glGenSymbolsEXT) (proc)) (datatype, storagetype, range, components) )

/* glSetInvariantEXT */
typedef void (APIENTRYP GdkGLProc_glSetInvariantEXT) (GLuint id, GLenum type, const GLvoid *addr);
GdkGLProc    gdk_gl_get_glSetInvariantEXT (void);
#define      gdk_gl_glSetInvariantEXT(proc, id, type, addr) \
  ( ((GdkGLProc_glSetInvariantEXT) (proc)) (id, type, addr) )

/* glSetLocalConstantEXT */
typedef void (APIENTRYP GdkGLProc_glSetLocalConstantEXT) (GLuint id, GLenum type, const GLvoid *addr);
GdkGLProc    gdk_gl_get_glSetLocalConstantEXT (void);
#define      gdk_gl_glSetLocalConstantEXT(proc, id, type, addr) \
  ( ((GdkGLProc_glSetLocalConstantEXT) (proc)) (id, type, addr) )

/* glVariantbvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantbvEXT) (GLuint id, const GLbyte *addr);
GdkGLProc    gdk_gl_get_glVariantbvEXT (void);
#define      gdk_gl_glVariantbvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantbvEXT) (proc)) (id, addr) )

/* glVariantsvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantsvEXT) (GLuint id, const GLshort *addr);
GdkGLProc    gdk_gl_get_glVariantsvEXT (void);
#define      gdk_gl_glVariantsvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantsvEXT) (proc)) (id, addr) )

/* glVariantivEXT */
typedef void (APIENTRYP GdkGLProc_glVariantivEXT) (GLuint id, const GLint *addr);
GdkGLProc    gdk_gl_get_glVariantivEXT (void);
#define      gdk_gl_glVariantivEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantivEXT) (proc)) (id, addr) )

/* glVariantfvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantfvEXT) (GLuint id, const GLfloat *addr);
GdkGLProc    gdk_gl_get_glVariantfvEXT (void);
#define      gdk_gl_glVariantfvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantfvEXT) (proc)) (id, addr) )

/* glVariantdvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantdvEXT) (GLuint id, const GLdouble *addr);
GdkGLProc    gdk_gl_get_glVariantdvEXT (void);
#define      gdk_gl_glVariantdvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantdvEXT) (proc)) (id, addr) )

/* glVariantubvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantubvEXT) (GLuint id, const GLubyte *addr);
GdkGLProc    gdk_gl_get_glVariantubvEXT (void);
#define      gdk_gl_glVariantubvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantubvEXT) (proc)) (id, addr) )

/* glVariantusvEXT */
typedef void (APIENTRYP GdkGLProc_glVariantusvEXT) (GLuint id, const GLushort *addr);
GdkGLProc    gdk_gl_get_glVariantusvEXT (void);
#define      gdk_gl_glVariantusvEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantusvEXT) (proc)) (id, addr) )

/* glVariantuivEXT */
typedef void (APIENTRYP GdkGLProc_glVariantuivEXT) (GLuint id, const GLuint *addr);
GdkGLProc    gdk_gl_get_glVariantuivEXT (void);
#define      gdk_gl_glVariantuivEXT(proc, id, addr) \
  ( ((GdkGLProc_glVariantuivEXT) (proc)) (id, addr) )

/* glVariantPointerEXT */
typedef void (APIENTRYP GdkGLProc_glVariantPointerEXT) (GLuint id, GLenum type, GLuint stride, const GLvoid *addr);
GdkGLProc    gdk_gl_get_glVariantPointerEXT (void);
#define      gdk_gl_glVariantPointerEXT(proc, id, type, stride, addr) \
  ( ((GdkGLProc_glVariantPointerEXT) (proc)) (id, type, stride, addr) )

/* glEnableVariantClientStateEXT */
typedef void (APIENTRYP GdkGLProc_glEnableVariantClientStateEXT) (GLuint id);
GdkGLProc    gdk_gl_get_glEnableVariantClientStateEXT (void);
#define      gdk_gl_glEnableVariantClientStateEXT(proc, id) \
  ( ((GdkGLProc_glEnableVariantClientStateEXT) (proc)) (id) )

/* glDisableVariantClientStateEXT */
typedef void (APIENTRYP GdkGLProc_glDisableVariantClientStateEXT) (GLuint id);
GdkGLProc    gdk_gl_get_glDisableVariantClientStateEXT (void);
#define      gdk_gl_glDisableVariantClientStateEXT(proc, id) \
  ( ((GdkGLProc_glDisableVariantClientStateEXT) (proc)) (id) )

/* glBindLightParameterEXT */
typedef GLuint (APIENTRYP GdkGLProc_glBindLightParameterEXT) (GLenum light, GLenum value);
GdkGLProc    gdk_gl_get_glBindLightParameterEXT (void);
#define      gdk_gl_glBindLightParameterEXT(proc, light, value) \
  ( ((GdkGLProc_glBindLightParameterEXT) (proc)) (light, value) )

/* glBindMaterialParameterEXT */
typedef GLuint (APIENTRYP GdkGLProc_glBindMaterialParameterEXT) (GLenum face, GLenum value);
GdkGLProc    gdk_gl_get_glBindMaterialParameterEXT (void);
#define      gdk_gl_glBindMaterialParameterEXT(proc, face, value) \
  ( ((GdkGLProc_glBindMaterialParameterEXT) (proc)) (face, value) )

/* glBindTexGenParameterEXT */
typedef GLuint (APIENTRYP GdkGLProc_glBindTexGenParameterEXT) (GLenum unit, GLenum coord, GLenum value);
GdkGLProc    gdk_gl_get_glBindTexGenParameterEXT (void);
#define      gdk_gl_glBindTexGenParameterEXT(proc, unit, coord, value) \
  ( ((GdkGLProc_glBindTexGenParameterEXT) (proc)) (unit, coord, value) )

/* glBindTextureUnitParameterEXT */
typedef GLuint (APIENTRYP GdkGLProc_glBindTextureUnitParameterEXT) (GLenum unit, GLenum value);
GdkGLProc    gdk_gl_get_glBindTextureUnitParameterEXT (void);
#define      gdk_gl_glBindTextureUnitParameterEXT(proc, unit, value) \
  ( ((GdkGLProc_glBindTextureUnitParameterEXT) (proc)) (unit, value) )

/* glBindParameterEXT */
typedef GLuint (APIENTRYP GdkGLProc_glBindParameterEXT) (GLenum value);
GdkGLProc    gdk_gl_get_glBindParameterEXT (void);
#define      gdk_gl_glBindParameterEXT(proc, value) \
  ( ((GdkGLProc_glBindParameterEXT) (proc)) (value) )

/* glIsVariantEnabledEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glIsVariantEnabledEXT) (GLuint id, GLenum cap);
GdkGLProc    gdk_gl_get_glIsVariantEnabledEXT (void);
#define      gdk_gl_glIsVariantEnabledEXT(proc, id, cap) \
  ( ((GdkGLProc_glIsVariantEnabledEXT) (proc)) (id, cap) )

/* glGetVariantBooleanvEXT */
typedef void (APIENTRYP GdkGLProc_glGetVariantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
GdkGLProc    gdk_gl_get_glGetVariantBooleanvEXT (void);
#define      gdk_gl_glGetVariantBooleanvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetVariantBooleanvEXT) (proc)) (id, value, data) )

/* glGetVariantIntegervEXT */
typedef void (APIENTRYP GdkGLProc_glGetVariantIntegervEXT) (GLuint id, GLenum value, GLint *data);
GdkGLProc    gdk_gl_get_glGetVariantIntegervEXT (void);
#define      gdk_gl_glGetVariantIntegervEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetVariantIntegervEXT) (proc)) (id, value, data) )

/* glGetVariantFloatvEXT */
typedef void (APIENTRYP GdkGLProc_glGetVariantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
GdkGLProc    gdk_gl_get_glGetVariantFloatvEXT (void);
#define      gdk_gl_glGetVariantFloatvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetVariantFloatvEXT) (proc)) (id, value, data) )

/* glGetVariantPointervEXT */
typedef void (APIENTRYP GdkGLProc_glGetVariantPointervEXT) (GLuint id, GLenum value, GLvoid* *data);
GdkGLProc    gdk_gl_get_glGetVariantPointervEXT (void);
#define      gdk_gl_glGetVariantPointervEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetVariantPointervEXT) (proc)) (id, value, data) )

/* glGetInvariantBooleanvEXT */
typedef void (APIENTRYP GdkGLProc_glGetInvariantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
GdkGLProc    gdk_gl_get_glGetInvariantBooleanvEXT (void);
#define      gdk_gl_glGetInvariantBooleanvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetInvariantBooleanvEXT) (proc)) (id, value, data) )

/* glGetInvariantIntegervEXT */
typedef void (APIENTRYP GdkGLProc_glGetInvariantIntegervEXT) (GLuint id, GLenum value, GLint *data);
GdkGLProc    gdk_gl_get_glGetInvariantIntegervEXT (void);
#define      gdk_gl_glGetInvariantIntegervEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetInvariantIntegervEXT) (proc)) (id, value, data) )

/* glGetInvariantFloatvEXT */
typedef void (APIENTRYP GdkGLProc_glGetInvariantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
GdkGLProc    gdk_gl_get_glGetInvariantFloatvEXT (void);
#define      gdk_gl_glGetInvariantFloatvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetInvariantFloatvEXT) (proc)) (id, value, data) )

/* glGetLocalConstantBooleanvEXT */
typedef void (APIENTRYP GdkGLProc_glGetLocalConstantBooleanvEXT) (GLuint id, GLenum value, GLboolean *data);
GdkGLProc    gdk_gl_get_glGetLocalConstantBooleanvEXT (void);
#define      gdk_gl_glGetLocalConstantBooleanvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetLocalConstantBooleanvEXT) (proc)) (id, value, data) )

/* glGetLocalConstantIntegervEXT */
typedef void (APIENTRYP GdkGLProc_glGetLocalConstantIntegervEXT) (GLuint id, GLenum value, GLint *data);
GdkGLProc    gdk_gl_get_glGetLocalConstantIntegervEXT (void);
#define      gdk_gl_glGetLocalConstantIntegervEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetLocalConstantIntegervEXT) (proc)) (id, value, data) )

/* glGetLocalConstantFloatvEXT */
typedef void (APIENTRYP GdkGLProc_glGetLocalConstantFloatvEXT) (GLuint id, GLenum value, GLfloat *data);
GdkGLProc    gdk_gl_get_glGetLocalConstantFloatvEXT (void);
#define      gdk_gl_glGetLocalConstantFloatvEXT(proc, id, value, data) \
  ( ((GdkGLProc_glGetLocalConstantFloatvEXT) (proc)) (id, value, data) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_vertex_shader GdkGL_GL_EXT_vertex_shader;

struct _GdkGL_GL_EXT_vertex_shader
{
  GdkGLProc_glBeginVertexShaderEXT glBeginVertexShaderEXT;
  GdkGLProc_glEndVertexShaderEXT glEndVertexShaderEXT;
  GdkGLProc_glBindVertexShaderEXT glBindVertexShaderEXT;
  GdkGLProc_glGenVertexShadersEXT glGenVertexShadersEXT;
  GdkGLProc_glDeleteVertexShaderEXT glDeleteVertexShaderEXT;
  GdkGLProc_glShaderOp1EXT glShaderOp1EXT;
  GdkGLProc_glShaderOp2EXT glShaderOp2EXT;
  GdkGLProc_glShaderOp3EXT glShaderOp3EXT;
  GdkGLProc_glSwizzleEXT glSwizzleEXT;
  GdkGLProc_glWriteMaskEXT glWriteMaskEXT;
  GdkGLProc_glInsertComponentEXT glInsertComponentEXT;
  GdkGLProc_glExtractComponentEXT glExtractComponentEXT;
  GdkGLProc_glGenSymbolsEXT glGenSymbolsEXT;
  GdkGLProc_glSetInvariantEXT glSetInvariantEXT;
  GdkGLProc_glSetLocalConstantEXT glSetLocalConstantEXT;
  GdkGLProc_glVariantbvEXT glVariantbvEXT;
  GdkGLProc_glVariantsvEXT glVariantsvEXT;
  GdkGLProc_glVariantivEXT glVariantivEXT;
  GdkGLProc_glVariantfvEXT glVariantfvEXT;
  GdkGLProc_glVariantdvEXT glVariantdvEXT;
  GdkGLProc_glVariantubvEXT glVariantubvEXT;
  GdkGLProc_glVariantusvEXT glVariantusvEXT;
  GdkGLProc_glVariantuivEXT glVariantuivEXT;
  GdkGLProc_glVariantPointerEXT glVariantPointerEXT;
  GdkGLProc_glEnableVariantClientStateEXT glEnableVariantClientStateEXT;
  GdkGLProc_glDisableVariantClientStateEXT glDisableVariantClientStateEXT;
  GdkGLProc_glBindLightParameterEXT glBindLightParameterEXT;
  GdkGLProc_glBindMaterialParameterEXT glBindMaterialParameterEXT;
  GdkGLProc_glBindTexGenParameterEXT glBindTexGenParameterEXT;
  GdkGLProc_glBindTextureUnitParameterEXT glBindTextureUnitParameterEXT;
  GdkGLProc_glBindParameterEXT glBindParameterEXT;
  GdkGLProc_glIsVariantEnabledEXT glIsVariantEnabledEXT;
  GdkGLProc_glGetVariantBooleanvEXT glGetVariantBooleanvEXT;
  GdkGLProc_glGetVariantIntegervEXT glGetVariantIntegervEXT;
  GdkGLProc_glGetVariantFloatvEXT glGetVariantFloatvEXT;
  GdkGLProc_glGetVariantPointervEXT glGetVariantPointervEXT;
  GdkGLProc_glGetInvariantBooleanvEXT glGetInvariantBooleanvEXT;
  GdkGLProc_glGetInvariantIntegervEXT glGetInvariantIntegervEXT;
  GdkGLProc_glGetInvariantFloatvEXT glGetInvariantFloatvEXT;
  GdkGLProc_glGetLocalConstantBooleanvEXT glGetLocalConstantBooleanvEXT;
  GdkGLProc_glGetLocalConstantIntegervEXT glGetLocalConstantIntegervEXT;
  GdkGLProc_glGetLocalConstantFloatvEXT glGetLocalConstantFloatvEXT;
};

GdkGL_GL_EXT_vertex_shader *gdk_gl_get_GL_EXT_vertex_shader (void);

/*
 * GL_ATI_vertex_streams
 */

/* glVertexStream1sATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1sATI) (GLenum stream, GLshort x);
GdkGLProc    gdk_gl_get_glVertexStream1sATI (void);
#define      gdk_gl_glVertexStream1sATI(proc, stream, x) \
  ( ((GdkGLProc_glVertexStream1sATI) (proc)) (stream, x) )

/* glVertexStream1svATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1svATI) (GLenum stream, const GLshort *coords);
GdkGLProc    gdk_gl_get_glVertexStream1svATI (void);
#define      gdk_gl_glVertexStream1svATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream1svATI) (proc)) (stream, coords) )

/* glVertexStream1iATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1iATI) (GLenum stream, GLint x);
GdkGLProc    gdk_gl_get_glVertexStream1iATI (void);
#define      gdk_gl_glVertexStream1iATI(proc, stream, x) \
  ( ((GdkGLProc_glVertexStream1iATI) (proc)) (stream, x) )

/* glVertexStream1ivATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1ivATI) (GLenum stream, const GLint *coords);
GdkGLProc    gdk_gl_get_glVertexStream1ivATI (void);
#define      gdk_gl_glVertexStream1ivATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream1ivATI) (proc)) (stream, coords) )

/* glVertexStream1fATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1fATI) (GLenum stream, GLfloat x);
GdkGLProc    gdk_gl_get_glVertexStream1fATI (void);
#define      gdk_gl_glVertexStream1fATI(proc, stream, x) \
  ( ((GdkGLProc_glVertexStream1fATI) (proc)) (stream, x) )

/* glVertexStream1fvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1fvATI) (GLenum stream, const GLfloat *coords);
GdkGLProc    gdk_gl_get_glVertexStream1fvATI (void);
#define      gdk_gl_glVertexStream1fvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream1fvATI) (proc)) (stream, coords) )

/* glVertexStream1dATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1dATI) (GLenum stream, GLdouble x);
GdkGLProc    gdk_gl_get_glVertexStream1dATI (void);
#define      gdk_gl_glVertexStream1dATI(proc, stream, x) \
  ( ((GdkGLProc_glVertexStream1dATI) (proc)) (stream, x) )

/* glVertexStream1dvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream1dvATI) (GLenum stream, const GLdouble *coords);
GdkGLProc    gdk_gl_get_glVertexStream1dvATI (void);
#define      gdk_gl_glVertexStream1dvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream1dvATI) (proc)) (stream, coords) )

/* glVertexStream2sATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2sATI) (GLenum stream, GLshort x, GLshort y);
GdkGLProc    gdk_gl_get_glVertexStream2sATI (void);
#define      gdk_gl_glVertexStream2sATI(proc, stream, x, y) \
  ( ((GdkGLProc_glVertexStream2sATI) (proc)) (stream, x, y) )

/* glVertexStream2svATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2svATI) (GLenum stream, const GLshort *coords);
GdkGLProc    gdk_gl_get_glVertexStream2svATI (void);
#define      gdk_gl_glVertexStream2svATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream2svATI) (proc)) (stream, coords) )

/* glVertexStream2iATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2iATI) (GLenum stream, GLint x, GLint y);
GdkGLProc    gdk_gl_get_glVertexStream2iATI (void);
#define      gdk_gl_glVertexStream2iATI(proc, stream, x, y) \
  ( ((GdkGLProc_glVertexStream2iATI) (proc)) (stream, x, y) )

/* glVertexStream2ivATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2ivATI) (GLenum stream, const GLint *coords);
GdkGLProc    gdk_gl_get_glVertexStream2ivATI (void);
#define      gdk_gl_glVertexStream2ivATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream2ivATI) (proc)) (stream, coords) )

/* glVertexStream2fATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2fATI) (GLenum stream, GLfloat x, GLfloat y);
GdkGLProc    gdk_gl_get_glVertexStream2fATI (void);
#define      gdk_gl_glVertexStream2fATI(proc, stream, x, y) \
  ( ((GdkGLProc_glVertexStream2fATI) (proc)) (stream, x, y) )

/* glVertexStream2fvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2fvATI) (GLenum stream, const GLfloat *coords);
GdkGLProc    gdk_gl_get_glVertexStream2fvATI (void);
#define      gdk_gl_glVertexStream2fvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream2fvATI) (proc)) (stream, coords) )

/* glVertexStream2dATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2dATI) (GLenum stream, GLdouble x, GLdouble y);
GdkGLProc    gdk_gl_get_glVertexStream2dATI (void);
#define      gdk_gl_glVertexStream2dATI(proc, stream, x, y) \
  ( ((GdkGLProc_glVertexStream2dATI) (proc)) (stream, x, y) )

/* glVertexStream2dvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream2dvATI) (GLenum stream, const GLdouble *coords);
GdkGLProc    gdk_gl_get_glVertexStream2dvATI (void);
#define      gdk_gl_glVertexStream2dvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream2dvATI) (proc)) (stream, coords) )

/* glVertexStream3sATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3sATI) (GLenum stream, GLshort x, GLshort y, GLshort z);
GdkGLProc    gdk_gl_get_glVertexStream3sATI (void);
#define      gdk_gl_glVertexStream3sATI(proc, stream, x, y, z) \
  ( ((GdkGLProc_glVertexStream3sATI) (proc)) (stream, x, y, z) )

/* glVertexStream3svATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3svATI) (GLenum stream, const GLshort *coords);
GdkGLProc    gdk_gl_get_glVertexStream3svATI (void);
#define      gdk_gl_glVertexStream3svATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream3svATI) (proc)) (stream, coords) )

/* glVertexStream3iATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3iATI) (GLenum stream, GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glVertexStream3iATI (void);
#define      gdk_gl_glVertexStream3iATI(proc, stream, x, y, z) \
  ( ((GdkGLProc_glVertexStream3iATI) (proc)) (stream, x, y, z) )

/* glVertexStream3ivATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3ivATI) (GLenum stream, const GLint *coords);
GdkGLProc    gdk_gl_get_glVertexStream3ivATI (void);
#define      gdk_gl_glVertexStream3ivATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream3ivATI) (proc)) (stream, coords) )

/* glVertexStream3fATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3fATI) (GLenum stream, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glVertexStream3fATI (void);
#define      gdk_gl_glVertexStream3fATI(proc, stream, x, y, z) \
  ( ((GdkGLProc_glVertexStream3fATI) (proc)) (stream, x, y, z) )

/* glVertexStream3fvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3fvATI) (GLenum stream, const GLfloat *coords);
GdkGLProc    gdk_gl_get_glVertexStream3fvATI (void);
#define      gdk_gl_glVertexStream3fvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream3fvATI) (proc)) (stream, coords) )

/* glVertexStream3dATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3dATI) (GLenum stream, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glVertexStream3dATI (void);
#define      gdk_gl_glVertexStream3dATI(proc, stream, x, y, z) \
  ( ((GdkGLProc_glVertexStream3dATI) (proc)) (stream, x, y, z) )

/* glVertexStream3dvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream3dvATI) (GLenum stream, const GLdouble *coords);
GdkGLProc    gdk_gl_get_glVertexStream3dvATI (void);
#define      gdk_gl_glVertexStream3dvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream3dvATI) (proc)) (stream, coords) )

/* glVertexStream4sATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4sATI) (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w);
GdkGLProc    gdk_gl_get_glVertexStream4sATI (void);
#define      gdk_gl_glVertexStream4sATI(proc, stream, x, y, z, w) \
  ( ((GdkGLProc_glVertexStream4sATI) (proc)) (stream, x, y, z, w) )

/* glVertexStream4svATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4svATI) (GLenum stream, const GLshort *coords);
GdkGLProc    gdk_gl_get_glVertexStream4svATI (void);
#define      gdk_gl_glVertexStream4svATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream4svATI) (proc)) (stream, coords) )

/* glVertexStream4iATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4iATI) (GLenum stream, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glVertexStream4iATI (void);
#define      gdk_gl_glVertexStream4iATI(proc, stream, x, y, z, w) \
  ( ((GdkGLProc_glVertexStream4iATI) (proc)) (stream, x, y, z, w) )

/* glVertexStream4ivATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4ivATI) (GLenum stream, const GLint *coords);
GdkGLProc    gdk_gl_get_glVertexStream4ivATI (void);
#define      gdk_gl_glVertexStream4ivATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream4ivATI) (proc)) (stream, coords) )

/* glVertexStream4fATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4fATI) (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glVertexStream4fATI (void);
#define      gdk_gl_glVertexStream4fATI(proc, stream, x, y, z, w) \
  ( ((GdkGLProc_glVertexStream4fATI) (proc)) (stream, x, y, z, w) )

/* glVertexStream4fvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4fvATI) (GLenum stream, const GLfloat *coords);
GdkGLProc    gdk_gl_get_glVertexStream4fvATI (void);
#define      gdk_gl_glVertexStream4fvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream4fvATI) (proc)) (stream, coords) )

/* glVertexStream4dATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4dATI) (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glVertexStream4dATI (void);
#define      gdk_gl_glVertexStream4dATI(proc, stream, x, y, z, w) \
  ( ((GdkGLProc_glVertexStream4dATI) (proc)) (stream, x, y, z, w) )

/* glVertexStream4dvATI */
typedef void (APIENTRYP GdkGLProc_glVertexStream4dvATI) (GLenum stream, const GLdouble *coords);
GdkGLProc    gdk_gl_get_glVertexStream4dvATI (void);
#define      gdk_gl_glVertexStream4dvATI(proc, stream, coords) \
  ( ((GdkGLProc_glVertexStream4dvATI) (proc)) (stream, coords) )

/* glNormalStream3bATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3bATI) (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz);
GdkGLProc    gdk_gl_get_glNormalStream3bATI (void);
#define      gdk_gl_glNormalStream3bATI(proc, stream, nx, ny, nz) \
  ( ((GdkGLProc_glNormalStream3bATI) (proc)) (stream, nx, ny, nz) )

/* glNormalStream3bvATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3bvATI) (GLenum stream, const GLbyte *coords);
GdkGLProc    gdk_gl_get_glNormalStream3bvATI (void);
#define      gdk_gl_glNormalStream3bvATI(proc, stream, coords) \
  ( ((GdkGLProc_glNormalStream3bvATI) (proc)) (stream, coords) )

/* glNormalStream3sATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3sATI) (GLenum stream, GLshort nx, GLshort ny, GLshort nz);
GdkGLProc    gdk_gl_get_glNormalStream3sATI (void);
#define      gdk_gl_glNormalStream3sATI(proc, stream, nx, ny, nz) \
  ( ((GdkGLProc_glNormalStream3sATI) (proc)) (stream, nx, ny, nz) )

/* glNormalStream3svATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3svATI) (GLenum stream, const GLshort *coords);
GdkGLProc    gdk_gl_get_glNormalStream3svATI (void);
#define      gdk_gl_glNormalStream3svATI(proc, stream, coords) \
  ( ((GdkGLProc_glNormalStream3svATI) (proc)) (stream, coords) )

/* glNormalStream3iATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3iATI) (GLenum stream, GLint nx, GLint ny, GLint nz);
GdkGLProc    gdk_gl_get_glNormalStream3iATI (void);
#define      gdk_gl_glNormalStream3iATI(proc, stream, nx, ny, nz) \
  ( ((GdkGLProc_glNormalStream3iATI) (proc)) (stream, nx, ny, nz) )

/* glNormalStream3ivATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3ivATI) (GLenum stream, const GLint *coords);
GdkGLProc    gdk_gl_get_glNormalStream3ivATI (void);
#define      gdk_gl_glNormalStream3ivATI(proc, stream, coords) \
  ( ((GdkGLProc_glNormalStream3ivATI) (proc)) (stream, coords) )

/* glNormalStream3fATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3fATI) (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz);
GdkGLProc    gdk_gl_get_glNormalStream3fATI (void);
#define      gdk_gl_glNormalStream3fATI(proc, stream, nx, ny, nz) \
  ( ((GdkGLProc_glNormalStream3fATI) (proc)) (stream, nx, ny, nz) )

/* glNormalStream3fvATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3fvATI) (GLenum stream, const GLfloat *coords);
GdkGLProc    gdk_gl_get_glNormalStream3fvATI (void);
#define      gdk_gl_glNormalStream3fvATI(proc, stream, coords) \
  ( ((GdkGLProc_glNormalStream3fvATI) (proc)) (stream, coords) )

/* glNormalStream3dATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3dATI) (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz);
GdkGLProc    gdk_gl_get_glNormalStream3dATI (void);
#define      gdk_gl_glNormalStream3dATI(proc, stream, nx, ny, nz) \
  ( ((GdkGLProc_glNormalStream3dATI) (proc)) (stream, nx, ny, nz) )

/* glNormalStream3dvATI */
typedef void (APIENTRYP GdkGLProc_glNormalStream3dvATI) (GLenum stream, const GLdouble *coords);
GdkGLProc    gdk_gl_get_glNormalStream3dvATI (void);
#define      gdk_gl_glNormalStream3dvATI(proc, stream, coords) \
  ( ((GdkGLProc_glNormalStream3dvATI) (proc)) (stream, coords) )

/* glClientActiveVertexStreamATI */
typedef void (APIENTRYP GdkGLProc_glClientActiveVertexStreamATI) (GLenum stream);
GdkGLProc    gdk_gl_get_glClientActiveVertexStreamATI (void);
#define      gdk_gl_glClientActiveVertexStreamATI(proc, stream) \
  ( ((GdkGLProc_glClientActiveVertexStreamATI) (proc)) (stream) )

/* glVertexBlendEnviATI */
typedef void (APIENTRYP GdkGLProc_glVertexBlendEnviATI) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glVertexBlendEnviATI (void);
#define      gdk_gl_glVertexBlendEnviATI(proc, pname, param) \
  ( ((GdkGLProc_glVertexBlendEnviATI) (proc)) (pname, param) )

/* glVertexBlendEnvfATI */
typedef void (APIENTRYP GdkGLProc_glVertexBlendEnvfATI) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glVertexBlendEnvfATI (void);
#define      gdk_gl_glVertexBlendEnvfATI(proc, pname, param) \
  ( ((GdkGLProc_glVertexBlendEnvfATI) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_vertex_streams GdkGL_GL_ATI_vertex_streams;

struct _GdkGL_GL_ATI_vertex_streams
{
  GdkGLProc_glVertexStream1sATI glVertexStream1sATI;
  GdkGLProc_glVertexStream1svATI glVertexStream1svATI;
  GdkGLProc_glVertexStream1iATI glVertexStream1iATI;
  GdkGLProc_glVertexStream1ivATI glVertexStream1ivATI;
  GdkGLProc_glVertexStream1fATI glVertexStream1fATI;
  GdkGLProc_glVertexStream1fvATI glVertexStream1fvATI;
  GdkGLProc_glVertexStream1dATI glVertexStream1dATI;
  GdkGLProc_glVertexStream1dvATI glVertexStream1dvATI;
  GdkGLProc_glVertexStream2sATI glVertexStream2sATI;
  GdkGLProc_glVertexStream2svATI glVertexStream2svATI;
  GdkGLProc_glVertexStream2iATI glVertexStream2iATI;
  GdkGLProc_glVertexStream2ivATI glVertexStream2ivATI;
  GdkGLProc_glVertexStream2fATI glVertexStream2fATI;
  GdkGLProc_glVertexStream2fvATI glVertexStream2fvATI;
  GdkGLProc_glVertexStream2dATI glVertexStream2dATI;
  GdkGLProc_glVertexStream2dvATI glVertexStream2dvATI;
  GdkGLProc_glVertexStream3sATI glVertexStream3sATI;
  GdkGLProc_glVertexStream3svATI glVertexStream3svATI;
  GdkGLProc_glVertexStream3iATI glVertexStream3iATI;
  GdkGLProc_glVertexStream3ivATI glVertexStream3ivATI;
  GdkGLProc_glVertexStream3fATI glVertexStream3fATI;
  GdkGLProc_glVertexStream3fvATI glVertexStream3fvATI;
  GdkGLProc_glVertexStream3dATI glVertexStream3dATI;
  GdkGLProc_glVertexStream3dvATI glVertexStream3dvATI;
  GdkGLProc_glVertexStream4sATI glVertexStream4sATI;
  GdkGLProc_glVertexStream4svATI glVertexStream4svATI;
  GdkGLProc_glVertexStream4iATI glVertexStream4iATI;
  GdkGLProc_glVertexStream4ivATI glVertexStream4ivATI;
  GdkGLProc_glVertexStream4fATI glVertexStream4fATI;
  GdkGLProc_glVertexStream4fvATI glVertexStream4fvATI;
  GdkGLProc_glVertexStream4dATI glVertexStream4dATI;
  GdkGLProc_glVertexStream4dvATI glVertexStream4dvATI;
  GdkGLProc_glNormalStream3bATI glNormalStream3bATI;
  GdkGLProc_glNormalStream3bvATI glNormalStream3bvATI;
  GdkGLProc_glNormalStream3sATI glNormalStream3sATI;
  GdkGLProc_glNormalStream3svATI glNormalStream3svATI;
  GdkGLProc_glNormalStream3iATI glNormalStream3iATI;
  GdkGLProc_glNormalStream3ivATI glNormalStream3ivATI;
  GdkGLProc_glNormalStream3fATI glNormalStream3fATI;
  GdkGLProc_glNormalStream3fvATI glNormalStream3fvATI;
  GdkGLProc_glNormalStream3dATI glNormalStream3dATI;
  GdkGLProc_glNormalStream3dvATI glNormalStream3dvATI;
  GdkGLProc_glClientActiveVertexStreamATI glClientActiveVertexStreamATI;
  GdkGLProc_glVertexBlendEnviATI glVertexBlendEnviATI;
  GdkGLProc_glVertexBlendEnvfATI glVertexBlendEnvfATI;
};

GdkGL_GL_ATI_vertex_streams *gdk_gl_get_GL_ATI_vertex_streams (void);

/*
 * GL_ATI_element_array
 */

/* glElementPointerATI */
typedef void (APIENTRYP GdkGLProc_glElementPointerATI) (GLenum type, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glElementPointerATI (void);
#define      gdk_gl_glElementPointerATI(proc, type, pointer) \
  ( ((GdkGLProc_glElementPointerATI) (proc)) (type, pointer) )

/* glDrawElementArrayATI */
typedef void (APIENTRYP GdkGLProc_glDrawElementArrayATI) (GLenum mode, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawElementArrayATI (void);
#define      gdk_gl_glDrawElementArrayATI(proc, mode, count) \
  ( ((GdkGLProc_glDrawElementArrayATI) (proc)) (mode, count) )

/* glDrawRangeElementArrayATI */
typedef void (APIENTRYP GdkGLProc_glDrawRangeElementArrayATI) (GLenum mode, GLuint start, GLuint end, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawRangeElementArrayATI (void);
#define      gdk_gl_glDrawRangeElementArrayATI(proc, mode, start, end, count) \
  ( ((GdkGLProc_glDrawRangeElementArrayATI) (proc)) (mode, start, end, count) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_element_array GdkGL_GL_ATI_element_array;

struct _GdkGL_GL_ATI_element_array
{
  GdkGLProc_glElementPointerATI glElementPointerATI;
  GdkGLProc_glDrawElementArrayATI glDrawElementArrayATI;
  GdkGLProc_glDrawRangeElementArrayATI glDrawRangeElementArrayATI;
};

GdkGL_GL_ATI_element_array *gdk_gl_get_GL_ATI_element_array (void);

/*
 * GL_SUN_mesh_array
 */

/* glDrawMeshArraysSUN */
typedef void (APIENTRYP GdkGLProc_glDrawMeshArraysSUN) (GLenum mode, GLint first, GLsizei count, GLsizei width);
GdkGLProc    gdk_gl_get_glDrawMeshArraysSUN (void);
#define      gdk_gl_glDrawMeshArraysSUN(proc, mode, first, count, width) \
  ( ((GdkGLProc_glDrawMeshArraysSUN) (proc)) (mode, first, count, width) )

/* proc struct */

typedef struct _GdkGL_GL_SUN_mesh_array GdkGL_GL_SUN_mesh_array;

struct _GdkGL_GL_SUN_mesh_array
{
  GdkGLProc_glDrawMeshArraysSUN glDrawMeshArraysSUN;
};

GdkGL_GL_SUN_mesh_array *gdk_gl_get_GL_SUN_mesh_array (void);

/*
 * GL_NV_occlusion_query
 */

/* glGenOcclusionQueriesNV */
typedef void (APIENTRYP GdkGLProc_glGenOcclusionQueriesNV) (GLsizei n, GLuint *ids);
GdkGLProc    gdk_gl_get_glGenOcclusionQueriesNV (void);
#define      gdk_gl_glGenOcclusionQueriesNV(proc, n, ids) \
  ( ((GdkGLProc_glGenOcclusionQueriesNV) (proc)) (n, ids) )

/* glDeleteOcclusionQueriesNV */
typedef void (APIENTRYP GdkGLProc_glDeleteOcclusionQueriesNV) (GLsizei n, const GLuint *ids);
GdkGLProc    gdk_gl_get_glDeleteOcclusionQueriesNV (void);
#define      gdk_gl_glDeleteOcclusionQueriesNV(proc, n, ids) \
  ( ((GdkGLProc_glDeleteOcclusionQueriesNV) (proc)) (n, ids) )

/* glIsOcclusionQueryNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsOcclusionQueryNV) (GLuint id);
GdkGLProc    gdk_gl_get_glIsOcclusionQueryNV (void);
#define      gdk_gl_glIsOcclusionQueryNV(proc, id) \
  ( ((GdkGLProc_glIsOcclusionQueryNV) (proc)) (id) )

/* glBeginOcclusionQueryNV */
typedef void (APIENTRYP GdkGLProc_glBeginOcclusionQueryNV) (GLuint id);
GdkGLProc    gdk_gl_get_glBeginOcclusionQueryNV (void);
#define      gdk_gl_glBeginOcclusionQueryNV(proc, id) \
  ( ((GdkGLProc_glBeginOcclusionQueryNV) (proc)) (id) )

/* glEndOcclusionQueryNV */
typedef void (APIENTRYP GdkGLProc_glEndOcclusionQueryNV) (void);
GdkGLProc    gdk_gl_get_glEndOcclusionQueryNV (void);
#define      gdk_gl_glEndOcclusionQueryNV(proc) \
  ( ((GdkGLProc_glEndOcclusionQueryNV) (proc)) () )

/* glGetOcclusionQueryivNV */
typedef void (APIENTRYP GdkGLProc_glGetOcclusionQueryivNV) (GLuint id, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetOcclusionQueryivNV (void);
#define      gdk_gl_glGetOcclusionQueryivNV(proc, id, pname, params) \
  ( ((GdkGLProc_glGetOcclusionQueryivNV) (proc)) (id, pname, params) )

/* glGetOcclusionQueryuivNV */
typedef void (APIENTRYP GdkGLProc_glGetOcclusionQueryuivNV) (GLuint id, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetOcclusionQueryuivNV (void);
#define      gdk_gl_glGetOcclusionQueryuivNV(proc, id, pname, params) \
  ( ((GdkGLProc_glGetOcclusionQueryuivNV) (proc)) (id, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_occlusion_query GdkGL_GL_NV_occlusion_query;

struct _GdkGL_GL_NV_occlusion_query
{
  GdkGLProc_glGenOcclusionQueriesNV glGenOcclusionQueriesNV;
  GdkGLProc_glDeleteOcclusionQueriesNV glDeleteOcclusionQueriesNV;
  GdkGLProc_glIsOcclusionQueryNV glIsOcclusionQueryNV;
  GdkGLProc_glBeginOcclusionQueryNV glBeginOcclusionQueryNV;
  GdkGLProc_glEndOcclusionQueryNV glEndOcclusionQueryNV;
  GdkGLProc_glGetOcclusionQueryivNV glGetOcclusionQueryivNV;
  GdkGLProc_glGetOcclusionQueryuivNV glGetOcclusionQueryuivNV;
};

GdkGL_GL_NV_occlusion_query *gdk_gl_get_GL_NV_occlusion_query (void);

/*
 * GL_NV_point_sprite
 */

/* glPointParameteriNV */
typedef void (APIENTRYP GdkGLProc_glPointParameteriNV) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPointParameteriNV (void);
#define      gdk_gl_glPointParameteriNV(proc, pname, param) \
  ( ((GdkGLProc_glPointParameteriNV) (proc)) (pname, param) )

/* glPointParameterivNV */
typedef void (APIENTRYP GdkGLProc_glPointParameterivNV) (GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glPointParameterivNV (void);
#define      gdk_gl_glPointParameterivNV(proc, pname, params) \
  ( ((GdkGLProc_glPointParameterivNV) (proc)) (pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_point_sprite GdkGL_GL_NV_point_sprite;

struct _GdkGL_GL_NV_point_sprite
{
  GdkGLProc_glPointParameteriNV glPointParameteriNV;
  GdkGLProc_glPointParameterivNV glPointParameterivNV;
};

GdkGL_GL_NV_point_sprite *gdk_gl_get_GL_NV_point_sprite (void);

/*
 * GL_EXT_stencil_two_side
 */

/* glActiveStencilFaceEXT */
typedef void (APIENTRYP GdkGLProc_glActiveStencilFaceEXT) (GLenum face);
GdkGLProc    gdk_gl_get_glActiveStencilFaceEXT (void);
#define      gdk_gl_glActiveStencilFaceEXT(proc, face) \
  ( ((GdkGLProc_glActiveStencilFaceEXT) (proc)) (face) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_stencil_two_side GdkGL_GL_EXT_stencil_two_side;

struct _GdkGL_GL_EXT_stencil_two_side
{
  GdkGLProc_glActiveStencilFaceEXT glActiveStencilFaceEXT;
};

GdkGL_GL_EXT_stencil_two_side *gdk_gl_get_GL_EXT_stencil_two_side (void);

/*
 * GL_APPLE_element_array
 */

/* glElementPointerAPPLE */
typedef void (APIENTRYP GdkGLProc_glElementPointerAPPLE) (GLenum type, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glElementPointerAPPLE (void);
#define      gdk_gl_glElementPointerAPPLE(proc, type, pointer) \
  ( ((GdkGLProc_glElementPointerAPPLE) (proc)) (type, pointer) )

/* glDrawElementArrayAPPLE */
typedef void (APIENTRYP GdkGLProc_glDrawElementArrayAPPLE) (GLenum mode, GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawElementArrayAPPLE (void);
#define      gdk_gl_glDrawElementArrayAPPLE(proc, mode, first, count) \
  ( ((GdkGLProc_glDrawElementArrayAPPLE) (proc)) (mode, first, count) )

/* glDrawRangeElementArrayAPPLE */
typedef void (APIENTRYP GdkGLProc_glDrawRangeElementArrayAPPLE) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawRangeElementArrayAPPLE (void);
#define      gdk_gl_glDrawRangeElementArrayAPPLE(proc, mode, start, end, first, count) \
  ( ((GdkGLProc_glDrawRangeElementArrayAPPLE) (proc)) (mode, start, end, first, count) )

/* glMultiDrawElementArrayAPPLE */
typedef void (APIENTRYP GdkGLProc_glMultiDrawElementArrayAPPLE) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawElementArrayAPPLE (void);
#define      gdk_gl_glMultiDrawElementArrayAPPLE(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawElementArrayAPPLE) (proc)) (mode, first, count, primcount) )

/* glMultiDrawRangeElementArrayAPPLE */
typedef void (APIENTRYP GdkGLProc_glMultiDrawRangeElementArrayAPPLE) (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawRangeElementArrayAPPLE (void);
#define      gdk_gl_glMultiDrawRangeElementArrayAPPLE(proc, mode, start, end, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawRangeElementArrayAPPLE) (proc)) (mode, start, end, first, count, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_element_array GdkGL_GL_APPLE_element_array;

struct _GdkGL_GL_APPLE_element_array
{
  GdkGLProc_glElementPointerAPPLE glElementPointerAPPLE;
  GdkGLProc_glDrawElementArrayAPPLE glDrawElementArrayAPPLE;
  GdkGLProc_glDrawRangeElementArrayAPPLE glDrawRangeElementArrayAPPLE;
  GdkGLProc_glMultiDrawElementArrayAPPLE glMultiDrawElementArrayAPPLE;
  GdkGLProc_glMultiDrawRangeElementArrayAPPLE glMultiDrawRangeElementArrayAPPLE;
};

GdkGL_GL_APPLE_element_array *gdk_gl_get_GL_APPLE_element_array (void);

/*
 * GL_APPLE_fence
 */

/* glGenFencesAPPLE */
typedef void (APIENTRYP GdkGLProc_glGenFencesAPPLE) (GLsizei n, GLuint *fences);
GdkGLProc    gdk_gl_get_glGenFencesAPPLE (void);
#define      gdk_gl_glGenFencesAPPLE(proc, n, fences) \
  ( ((GdkGLProc_glGenFencesAPPLE) (proc)) (n, fences) )

/* glDeleteFencesAPPLE */
typedef void (APIENTRYP GdkGLProc_glDeleteFencesAPPLE) (GLsizei n, const GLuint *fences);
GdkGLProc    gdk_gl_get_glDeleteFencesAPPLE (void);
#define      gdk_gl_glDeleteFencesAPPLE(proc, n, fences) \
  ( ((GdkGLProc_glDeleteFencesAPPLE) (proc)) (n, fences) )

/* glSetFenceAPPLE */
typedef void (APIENTRYP GdkGLProc_glSetFenceAPPLE) (GLuint fence);
GdkGLProc    gdk_gl_get_glSetFenceAPPLE (void);
#define      gdk_gl_glSetFenceAPPLE(proc, fence) \
  ( ((GdkGLProc_glSetFenceAPPLE) (proc)) (fence) )

/* glIsFenceAPPLE */
typedef GLboolean (APIENTRYP GdkGLProc_glIsFenceAPPLE) (GLuint fence);
GdkGLProc    gdk_gl_get_glIsFenceAPPLE (void);
#define      gdk_gl_glIsFenceAPPLE(proc, fence) \
  ( ((GdkGLProc_glIsFenceAPPLE) (proc)) (fence) )

/* glTestFenceAPPLE */
typedef GLboolean (APIENTRYP GdkGLProc_glTestFenceAPPLE) (GLuint fence);
GdkGLProc    gdk_gl_get_glTestFenceAPPLE (void);
#define      gdk_gl_glTestFenceAPPLE(proc, fence) \
  ( ((GdkGLProc_glTestFenceAPPLE) (proc)) (fence) )

/* glFinishFenceAPPLE */
typedef void (APIENTRYP GdkGLProc_glFinishFenceAPPLE) (GLuint fence);
GdkGLProc    gdk_gl_get_glFinishFenceAPPLE (void);
#define      gdk_gl_glFinishFenceAPPLE(proc, fence) \
  ( ((GdkGLProc_glFinishFenceAPPLE) (proc)) (fence) )

/* glTestObjectAPPLE */
typedef GLboolean (APIENTRYP GdkGLProc_glTestObjectAPPLE) (GLenum object, GLuint name);
GdkGLProc    gdk_gl_get_glTestObjectAPPLE (void);
#define      gdk_gl_glTestObjectAPPLE(proc, object, name) \
  ( ((GdkGLProc_glTestObjectAPPLE) (proc)) (object, name) )

/* glFinishObjectAPPLE */
typedef void (APIENTRYP GdkGLProc_glFinishObjectAPPLE) (GLenum object, GLint name);
GdkGLProc    gdk_gl_get_glFinishObjectAPPLE (void);
#define      gdk_gl_glFinishObjectAPPLE(proc, object, name) \
  ( ((GdkGLProc_glFinishObjectAPPLE) (proc)) (object, name) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_fence GdkGL_GL_APPLE_fence;

struct _GdkGL_GL_APPLE_fence
{
  GdkGLProc_glGenFencesAPPLE glGenFencesAPPLE;
  GdkGLProc_glDeleteFencesAPPLE glDeleteFencesAPPLE;
  GdkGLProc_glSetFenceAPPLE glSetFenceAPPLE;
  GdkGLProc_glIsFenceAPPLE glIsFenceAPPLE;
  GdkGLProc_glTestFenceAPPLE glTestFenceAPPLE;
  GdkGLProc_glFinishFenceAPPLE glFinishFenceAPPLE;
  GdkGLProc_glTestObjectAPPLE glTestObjectAPPLE;
  GdkGLProc_glFinishObjectAPPLE glFinishObjectAPPLE;
};

GdkGL_GL_APPLE_fence *gdk_gl_get_GL_APPLE_fence (void);

/*
 * GL_APPLE_vertex_array_object
 */

/* glBindVertexArrayAPPLE */
typedef void (APIENTRYP GdkGLProc_glBindVertexArrayAPPLE) (GLuint array);
GdkGLProc    gdk_gl_get_glBindVertexArrayAPPLE (void);
#define      gdk_gl_glBindVertexArrayAPPLE(proc, array) \
  ( ((GdkGLProc_glBindVertexArrayAPPLE) (proc)) (array) )

/* glDeleteVertexArraysAPPLE */
typedef void (APIENTRYP GdkGLProc_glDeleteVertexArraysAPPLE) (GLsizei n, const GLuint *arrays);
GdkGLProc    gdk_gl_get_glDeleteVertexArraysAPPLE (void);
#define      gdk_gl_glDeleteVertexArraysAPPLE(proc, n, arrays) \
  ( ((GdkGLProc_glDeleteVertexArraysAPPLE) (proc)) (n, arrays) )

/* glGenVertexArraysAPPLE */
typedef void (APIENTRYP GdkGLProc_glGenVertexArraysAPPLE) (GLsizei n, GLuint *arrays);
GdkGLProc    gdk_gl_get_glGenVertexArraysAPPLE (void);
#define      gdk_gl_glGenVertexArraysAPPLE(proc, n, arrays) \
  ( ((GdkGLProc_glGenVertexArraysAPPLE) (proc)) (n, arrays) )

/* glIsVertexArrayAPPLE */
typedef GLboolean (APIENTRYP GdkGLProc_glIsVertexArrayAPPLE) (GLuint array);
GdkGLProc    gdk_gl_get_glIsVertexArrayAPPLE (void);
#define      gdk_gl_glIsVertexArrayAPPLE(proc, array) \
  ( ((GdkGLProc_glIsVertexArrayAPPLE) (proc)) (array) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_vertex_array_object GdkGL_GL_APPLE_vertex_array_object;

struct _GdkGL_GL_APPLE_vertex_array_object
{
  GdkGLProc_glBindVertexArrayAPPLE glBindVertexArrayAPPLE;
  GdkGLProc_glDeleteVertexArraysAPPLE glDeleteVertexArraysAPPLE;
  GdkGLProc_glGenVertexArraysAPPLE glGenVertexArraysAPPLE;
  GdkGLProc_glIsVertexArrayAPPLE glIsVertexArrayAPPLE;
};

GdkGL_GL_APPLE_vertex_array_object *gdk_gl_get_GL_APPLE_vertex_array_object (void);

/*
 * GL_APPLE_vertex_array_range
 */

/* glVertexArrayRangeAPPLE */
typedef void (APIENTRYP GdkGLProc_glVertexArrayRangeAPPLE) (GLsizei length, GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexArrayRangeAPPLE (void);
#define      gdk_gl_glVertexArrayRangeAPPLE(proc, length, pointer) \
  ( ((GdkGLProc_glVertexArrayRangeAPPLE) (proc)) (length, pointer) )

/* glFlushVertexArrayRangeAPPLE */
typedef void (APIENTRYP GdkGLProc_glFlushVertexArrayRangeAPPLE) (GLsizei length, GLvoid *pointer);
GdkGLProc    gdk_gl_get_glFlushVertexArrayRangeAPPLE (void);
#define      gdk_gl_glFlushVertexArrayRangeAPPLE(proc, length, pointer) \
  ( ((GdkGLProc_glFlushVertexArrayRangeAPPLE) (proc)) (length, pointer) )

/* glVertexArrayParameteriAPPLE */
typedef void (APIENTRYP GdkGLProc_glVertexArrayParameteriAPPLE) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glVertexArrayParameteriAPPLE (void);
#define      gdk_gl_glVertexArrayParameteriAPPLE(proc, pname, param) \
  ( ((GdkGLProc_glVertexArrayParameteriAPPLE) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_vertex_array_range GdkGL_GL_APPLE_vertex_array_range;

struct _GdkGL_GL_APPLE_vertex_array_range
{
  GdkGLProc_glVertexArrayRangeAPPLE glVertexArrayRangeAPPLE;
  GdkGLProc_glFlushVertexArrayRangeAPPLE glFlushVertexArrayRangeAPPLE;
  GdkGLProc_glVertexArrayParameteriAPPLE glVertexArrayParameteriAPPLE;
};

GdkGL_GL_APPLE_vertex_array_range *gdk_gl_get_GL_APPLE_vertex_array_range (void);

/*
 * GL_ATI_draw_buffers
 */

/* glDrawBuffersATI */
typedef void (APIENTRYP GdkGLProc_glDrawBuffersATI) (GLsizei n, const GLenum *bufs);
GdkGLProc    gdk_gl_get_glDrawBuffersATI (void);
#define      gdk_gl_glDrawBuffersATI(proc, n, bufs) \
  ( ((GdkGLProc_glDrawBuffersATI) (proc)) (n, bufs) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_draw_buffers GdkGL_GL_ATI_draw_buffers;

struct _GdkGL_GL_ATI_draw_buffers
{
  GdkGLProc_glDrawBuffersATI glDrawBuffersATI;
};

GdkGL_GL_ATI_draw_buffers *gdk_gl_get_GL_ATI_draw_buffers (void);

/*
 * GL_NV_fragment_program
 */

/* glProgramNamedParameter4fNV */
typedef void (APIENTRYP GdkGLProc_glProgramNamedParameter4fNV) (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glProgramNamedParameter4fNV (void);
#define      gdk_gl_glProgramNamedParameter4fNV(proc, id, len, name, x, y, z, w) \
  ( ((GdkGLProc_glProgramNamedParameter4fNV) (proc)) (id, len, name, x, y, z, w) )

/* glProgramNamedParameter4dNV */
typedef void (APIENTRYP GdkGLProc_glProgramNamedParameter4dNV) (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glProgramNamedParameter4dNV (void);
#define      gdk_gl_glProgramNamedParameter4dNV(proc, id, len, name, x, y, z, w) \
  ( ((GdkGLProc_glProgramNamedParameter4dNV) (proc)) (id, len, name, x, y, z, w) )

/* glProgramNamedParameter4fvNV */
typedef void (APIENTRYP GdkGLProc_glProgramNamedParameter4fvNV) (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v);
GdkGLProc    gdk_gl_get_glProgramNamedParameter4fvNV (void);
#define      gdk_gl_glProgramNamedParameter4fvNV(proc, id, len, name, v) \
  ( ((GdkGLProc_glProgramNamedParameter4fvNV) (proc)) (id, len, name, v) )

/* glProgramNamedParameter4dvNV */
typedef void (APIENTRYP GdkGLProc_glProgramNamedParameter4dvNV) (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v);
GdkGLProc    gdk_gl_get_glProgramNamedParameter4dvNV (void);
#define      gdk_gl_glProgramNamedParameter4dvNV(proc, id, len, name, v) \
  ( ((GdkGLProc_glProgramNamedParameter4dvNV) (proc)) (id, len, name, v) )

/* glGetProgramNamedParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramNamedParameterfvNV) (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetProgramNamedParameterfvNV (void);
#define      gdk_gl_glGetProgramNamedParameterfvNV(proc, id, len, name, params) \
  ( ((GdkGLProc_glGetProgramNamedParameterfvNV) (proc)) (id, len, name, params) )

/* glGetProgramNamedParameterdvNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramNamedParameterdvNV) (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetProgramNamedParameterdvNV (void);
#define      gdk_gl_glGetProgramNamedParameterdvNV(proc, id, len, name, params) \
  ( ((GdkGLProc_glGetProgramNamedParameterdvNV) (proc)) (id, len, name, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_fragment_program GdkGL_GL_NV_fragment_program;

struct _GdkGL_GL_NV_fragment_program
{
  GdkGLProc_glProgramNamedParameter4fNV glProgramNamedParameter4fNV;
  GdkGLProc_glProgramNamedParameter4dNV glProgramNamedParameter4dNV;
  GdkGLProc_glProgramNamedParameter4fvNV glProgramNamedParameter4fvNV;
  GdkGLProc_glProgramNamedParameter4dvNV glProgramNamedParameter4dvNV;
  GdkGLProc_glGetProgramNamedParameterfvNV glGetProgramNamedParameterfvNV;
  GdkGLProc_glGetProgramNamedParameterdvNV glGetProgramNamedParameterdvNV;
};

GdkGL_GL_NV_fragment_program *gdk_gl_get_GL_NV_fragment_program (void);

/*
 * GL_NV_half_float
 */

/* glVertex2hNV */
typedef void (APIENTRYP GdkGLProc_glVertex2hNV) (GLhalfNV x, GLhalfNV y);
GdkGLProc    gdk_gl_get_glVertex2hNV (void);
#define      gdk_gl_glVertex2hNV(proc, x, y) \
  ( ((GdkGLProc_glVertex2hNV) (proc)) (x, y) )

/* glVertex2hvNV */
typedef void (APIENTRYP GdkGLProc_glVertex2hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertex2hvNV (void);
#define      gdk_gl_glVertex2hvNV(proc, v) \
  ( ((GdkGLProc_glVertex2hvNV) (proc)) (v) )

/* glVertex3hNV */
typedef void (APIENTRYP GdkGLProc_glVertex3hNV) (GLhalfNV x, GLhalfNV y, GLhalfNV z);
GdkGLProc    gdk_gl_get_glVertex3hNV (void);
#define      gdk_gl_glVertex3hNV(proc, x, y, z) \
  ( ((GdkGLProc_glVertex3hNV) (proc)) (x, y, z) )

/* glVertex3hvNV */
typedef void (APIENTRYP GdkGLProc_glVertex3hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertex3hvNV (void);
#define      gdk_gl_glVertex3hvNV(proc, v) \
  ( ((GdkGLProc_glVertex3hvNV) (proc)) (v) )

/* glVertex4hNV */
typedef void (APIENTRYP GdkGLProc_glVertex4hNV) (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
GdkGLProc    gdk_gl_get_glVertex4hNV (void);
#define      gdk_gl_glVertex4hNV(proc, x, y, z, w) \
  ( ((GdkGLProc_glVertex4hNV) (proc)) (x, y, z, w) )

/* glVertex4hvNV */
typedef void (APIENTRYP GdkGLProc_glVertex4hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertex4hvNV (void);
#define      gdk_gl_glVertex4hvNV(proc, v) \
  ( ((GdkGLProc_glVertex4hvNV) (proc)) (v) )

/* glNormal3hNV */
typedef void (APIENTRYP GdkGLProc_glNormal3hNV) (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz);
GdkGLProc    gdk_gl_get_glNormal3hNV (void);
#define      gdk_gl_glNormal3hNV(proc, nx, ny, nz) \
  ( ((GdkGLProc_glNormal3hNV) (proc)) (nx, ny, nz) )

/* glNormal3hvNV */
typedef void (APIENTRYP GdkGLProc_glNormal3hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glNormal3hvNV (void);
#define      gdk_gl_glNormal3hvNV(proc, v) \
  ( ((GdkGLProc_glNormal3hvNV) (proc)) (v) )

/* glColor3hNV */
typedef void (APIENTRYP GdkGLProc_glColor3hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
GdkGLProc    gdk_gl_get_glColor3hNV (void);
#define      gdk_gl_glColor3hNV(proc, red, green, blue) \
  ( ((GdkGLProc_glColor3hNV) (proc)) (red, green, blue) )

/* glColor3hvNV */
typedef void (APIENTRYP GdkGLProc_glColor3hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glColor3hvNV (void);
#define      gdk_gl_glColor3hvNV(proc, v) \
  ( ((GdkGLProc_glColor3hvNV) (proc)) (v) )

/* glColor4hNV */
typedef void (APIENTRYP GdkGLProc_glColor4hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha);
GdkGLProc    gdk_gl_get_glColor4hNV (void);
#define      gdk_gl_glColor4hNV(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glColor4hNV) (proc)) (red, green, blue, alpha) )

/* glColor4hvNV */
typedef void (APIENTRYP GdkGLProc_glColor4hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glColor4hvNV (void);
#define      gdk_gl_glColor4hvNV(proc, v) \
  ( ((GdkGLProc_glColor4hvNV) (proc)) (v) )

/* glTexCoord1hNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord1hNV) (GLhalfNV s);
GdkGLProc    gdk_gl_get_glTexCoord1hNV (void);
#define      gdk_gl_glTexCoord1hNV(proc, s) \
  ( ((GdkGLProc_glTexCoord1hNV) (proc)) (s) )

/* glTexCoord1hvNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord1hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glTexCoord1hvNV (void);
#define      gdk_gl_glTexCoord1hvNV(proc, v) \
  ( ((GdkGLProc_glTexCoord1hvNV) (proc)) (v) )

/* glTexCoord2hNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord2hNV) (GLhalfNV s, GLhalfNV t);
GdkGLProc    gdk_gl_get_glTexCoord2hNV (void);
#define      gdk_gl_glTexCoord2hNV(proc, s, t) \
  ( ((GdkGLProc_glTexCoord2hNV) (proc)) (s, t) )

/* glTexCoord2hvNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord2hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glTexCoord2hvNV (void);
#define      gdk_gl_glTexCoord2hvNV(proc, v) \
  ( ((GdkGLProc_glTexCoord2hvNV) (proc)) (v) )

/* glTexCoord3hNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord3hNV) (GLhalfNV s, GLhalfNV t, GLhalfNV r);
GdkGLProc    gdk_gl_get_glTexCoord3hNV (void);
#define      gdk_gl_glTexCoord3hNV(proc, s, t, r) \
  ( ((GdkGLProc_glTexCoord3hNV) (proc)) (s, t, r) )

/* glTexCoord3hvNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord3hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glTexCoord3hvNV (void);
#define      gdk_gl_glTexCoord3hvNV(proc, v) \
  ( ((GdkGLProc_glTexCoord3hvNV) (proc)) (v) )

/* glTexCoord4hNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord4hNV) (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
GdkGLProc    gdk_gl_get_glTexCoord4hNV (void);
#define      gdk_gl_glTexCoord4hNV(proc, s, t, r, q) \
  ( ((GdkGLProc_glTexCoord4hNV) (proc)) (s, t, r, q) )

/* glTexCoord4hvNV */
typedef void (APIENTRYP GdkGLProc_glTexCoord4hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glTexCoord4hvNV (void);
#define      gdk_gl_glTexCoord4hvNV(proc, v) \
  ( ((GdkGLProc_glTexCoord4hvNV) (proc)) (v) )

/* glMultiTexCoord1hNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1hNV) (GLenum target, GLhalfNV s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1hNV (void);
#define      gdk_gl_glMultiTexCoord1hNV(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1hNV) (proc)) (target, s) )

/* glMultiTexCoord1hvNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord1hvNV) (GLenum target, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1hvNV (void);
#define      gdk_gl_glMultiTexCoord1hvNV(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1hvNV) (proc)) (target, v) )

/* glMultiTexCoord2hNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2hNV) (GLenum target, GLhalfNV s, GLhalfNV t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2hNV (void);
#define      gdk_gl_glMultiTexCoord2hNV(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2hNV) (proc)) (target, s, t) )

/* glMultiTexCoord2hvNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord2hvNV) (GLenum target, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2hvNV (void);
#define      gdk_gl_glMultiTexCoord2hvNV(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2hvNV) (proc)) (target, v) )

/* glMultiTexCoord3hNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3hNV) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3hNV (void);
#define      gdk_gl_glMultiTexCoord3hNV(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3hNV) (proc)) (target, s, t, r) )

/* glMultiTexCoord3hvNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord3hvNV) (GLenum target, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3hvNV (void);
#define      gdk_gl_glMultiTexCoord3hvNV(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3hvNV) (proc)) (target, v) )

/* glMultiTexCoord4hNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4hNV) (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4hNV (void);
#define      gdk_gl_glMultiTexCoord4hNV(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4hNV) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4hvNV */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoord4hvNV) (GLenum target, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4hvNV (void);
#define      gdk_gl_glMultiTexCoord4hvNV(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4hvNV) (proc)) (target, v) )

/* glFogCoordhNV */
typedef void (APIENTRYP GdkGLProc_glFogCoordhNV) (GLhalfNV fog);
GdkGLProc    gdk_gl_get_glFogCoordhNV (void);
#define      gdk_gl_glFogCoordhNV(proc, fog) \
  ( ((GdkGLProc_glFogCoordhNV) (proc)) (fog) )

/* glFogCoordhvNV */
typedef void (APIENTRYP GdkGLProc_glFogCoordhvNV) (const GLhalfNV *fog);
GdkGLProc    gdk_gl_get_glFogCoordhvNV (void);
#define      gdk_gl_glFogCoordhvNV(proc, fog) \
  ( ((GdkGLProc_glFogCoordhvNV) (proc)) (fog) )

/* glSecondaryColor3hNV */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3hNV) (GLhalfNV red, GLhalfNV green, GLhalfNV blue);
GdkGLProc    gdk_gl_get_glSecondaryColor3hNV (void);
#define      gdk_gl_glSecondaryColor3hNV(proc, red, green, blue) \
  ( ((GdkGLProc_glSecondaryColor3hNV) (proc)) (red, green, blue) )

/* glSecondaryColor3hvNV */
typedef void (APIENTRYP GdkGLProc_glSecondaryColor3hvNV) (const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glSecondaryColor3hvNV (void);
#define      gdk_gl_glSecondaryColor3hvNV(proc, v) \
  ( ((GdkGLProc_glSecondaryColor3hvNV) (proc)) (v) )

/* glVertexWeighthNV */
typedef void (APIENTRYP GdkGLProc_glVertexWeighthNV) (GLhalfNV weight);
GdkGLProc    gdk_gl_get_glVertexWeighthNV (void);
#define      gdk_gl_glVertexWeighthNV(proc, weight) \
  ( ((GdkGLProc_glVertexWeighthNV) (proc)) (weight) )

/* glVertexWeighthvNV */
typedef void (APIENTRYP GdkGLProc_glVertexWeighthvNV) (const GLhalfNV *weight);
GdkGLProc    gdk_gl_get_glVertexWeighthvNV (void);
#define      gdk_gl_glVertexWeighthvNV(proc, weight) \
  ( ((GdkGLProc_glVertexWeighthvNV) (proc)) (weight) )

/* glVertexAttrib1hNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1hNV) (GLuint index, GLhalfNV x);
GdkGLProc    gdk_gl_get_glVertexAttrib1hNV (void);
#define      gdk_gl_glVertexAttrib1hNV(proc, index, x) \
  ( ((GdkGLProc_glVertexAttrib1hNV) (proc)) (index, x) )

/* glVertexAttrib1hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib1hvNV) (GLuint index, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttrib1hvNV (void);
#define      gdk_gl_glVertexAttrib1hvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib1hvNV) (proc)) (index, v) )

/* glVertexAttrib2hNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2hNV) (GLuint index, GLhalfNV x, GLhalfNV y);
GdkGLProc    gdk_gl_get_glVertexAttrib2hNV (void);
#define      gdk_gl_glVertexAttrib2hNV(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttrib2hNV) (proc)) (index, x, y) )

/* glVertexAttrib2hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib2hvNV) (GLuint index, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttrib2hvNV (void);
#define      gdk_gl_glVertexAttrib2hvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib2hvNV) (proc)) (index, v) )

/* glVertexAttrib3hNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3hNV) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z);
GdkGLProc    gdk_gl_get_glVertexAttrib3hNV (void);
#define      gdk_gl_glVertexAttrib3hNV(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttrib3hNV) (proc)) (index, x, y, z) )

/* glVertexAttrib3hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib3hvNV) (GLuint index, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttrib3hvNV (void);
#define      gdk_gl_glVertexAttrib3hvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib3hvNV) (proc)) (index, v) )

/* glVertexAttrib4hNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4hNV) (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w);
GdkGLProc    gdk_gl_get_glVertexAttrib4hNV (void);
#define      gdk_gl_glVertexAttrib4hNV(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttrib4hNV) (proc)) (index, x, y, z, w) )

/* glVertexAttrib4hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttrib4hvNV) (GLuint index, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttrib4hvNV (void);
#define      gdk_gl_glVertexAttrib4hvNV(proc, index, v) \
  ( ((GdkGLProc_glVertexAttrib4hvNV) (proc)) (index, v) )

/* glVertexAttribs1hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs1hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttribs1hvNV (void);
#define      gdk_gl_glVertexAttribs1hvNV(proc, index, n, v) \
  ( ((GdkGLProc_glVertexAttribs1hvNV) (proc)) (index, n, v) )

/* glVertexAttribs2hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs2hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttribs2hvNV (void);
#define      gdk_gl_glVertexAttribs2hvNV(proc, index, n, v) \
  ( ((GdkGLProc_glVertexAttribs2hvNV) (proc)) (index, n, v) )

/* glVertexAttribs3hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs3hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttribs3hvNV (void);
#define      gdk_gl_glVertexAttribs3hvNV(proc, index, n, v) \
  ( ((GdkGLProc_glVertexAttribs3hvNV) (proc)) (index, n, v) )

/* glVertexAttribs4hvNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribs4hvNV) (GLuint index, GLsizei n, const GLhalfNV *v);
GdkGLProc    gdk_gl_get_glVertexAttribs4hvNV (void);
#define      gdk_gl_glVertexAttribs4hvNV(proc, index, n, v) \
  ( ((GdkGLProc_glVertexAttribs4hvNV) (proc)) (index, n, v) )

/* proc struct */

typedef struct _GdkGL_GL_NV_half_float GdkGL_GL_NV_half_float;

struct _GdkGL_GL_NV_half_float
{
  GdkGLProc_glVertex2hNV glVertex2hNV;
  GdkGLProc_glVertex2hvNV glVertex2hvNV;
  GdkGLProc_glVertex3hNV glVertex3hNV;
  GdkGLProc_glVertex3hvNV glVertex3hvNV;
  GdkGLProc_glVertex4hNV glVertex4hNV;
  GdkGLProc_glVertex4hvNV glVertex4hvNV;
  GdkGLProc_glNormal3hNV glNormal3hNV;
  GdkGLProc_glNormal3hvNV glNormal3hvNV;
  GdkGLProc_glColor3hNV glColor3hNV;
  GdkGLProc_glColor3hvNV glColor3hvNV;
  GdkGLProc_glColor4hNV glColor4hNV;
  GdkGLProc_glColor4hvNV glColor4hvNV;
  GdkGLProc_glTexCoord1hNV glTexCoord1hNV;
  GdkGLProc_glTexCoord1hvNV glTexCoord1hvNV;
  GdkGLProc_glTexCoord2hNV glTexCoord2hNV;
  GdkGLProc_glTexCoord2hvNV glTexCoord2hvNV;
  GdkGLProc_glTexCoord3hNV glTexCoord3hNV;
  GdkGLProc_glTexCoord3hvNV glTexCoord3hvNV;
  GdkGLProc_glTexCoord4hNV glTexCoord4hNV;
  GdkGLProc_glTexCoord4hvNV glTexCoord4hvNV;
  GdkGLProc_glMultiTexCoord1hNV glMultiTexCoord1hNV;
  GdkGLProc_glMultiTexCoord1hvNV glMultiTexCoord1hvNV;
  GdkGLProc_glMultiTexCoord2hNV glMultiTexCoord2hNV;
  GdkGLProc_glMultiTexCoord2hvNV glMultiTexCoord2hvNV;
  GdkGLProc_glMultiTexCoord3hNV glMultiTexCoord3hNV;
  GdkGLProc_glMultiTexCoord3hvNV glMultiTexCoord3hvNV;
  GdkGLProc_glMultiTexCoord4hNV glMultiTexCoord4hNV;
  GdkGLProc_glMultiTexCoord4hvNV glMultiTexCoord4hvNV;
  GdkGLProc_glFogCoordhNV glFogCoordhNV;
  GdkGLProc_glFogCoordhvNV glFogCoordhvNV;
  GdkGLProc_glSecondaryColor3hNV glSecondaryColor3hNV;
  GdkGLProc_glSecondaryColor3hvNV glSecondaryColor3hvNV;
  GdkGLProc_glVertexWeighthNV glVertexWeighthNV;
  GdkGLProc_glVertexWeighthvNV glVertexWeighthvNV;
  GdkGLProc_glVertexAttrib1hNV glVertexAttrib1hNV;
  GdkGLProc_glVertexAttrib1hvNV glVertexAttrib1hvNV;
  GdkGLProc_glVertexAttrib2hNV glVertexAttrib2hNV;
  GdkGLProc_glVertexAttrib2hvNV glVertexAttrib2hvNV;
  GdkGLProc_glVertexAttrib3hNV glVertexAttrib3hNV;
  GdkGLProc_glVertexAttrib3hvNV glVertexAttrib3hvNV;
  GdkGLProc_glVertexAttrib4hNV glVertexAttrib4hNV;
  GdkGLProc_glVertexAttrib4hvNV glVertexAttrib4hvNV;
  GdkGLProc_glVertexAttribs1hvNV glVertexAttribs1hvNV;
  GdkGLProc_glVertexAttribs2hvNV glVertexAttribs2hvNV;
  GdkGLProc_glVertexAttribs3hvNV glVertexAttribs3hvNV;
  GdkGLProc_glVertexAttribs4hvNV glVertexAttribs4hvNV;
};

GdkGL_GL_NV_half_float *gdk_gl_get_GL_NV_half_float (void);

/*
 * GL_NV_pixel_data_range
 */

/* glPixelDataRangeNV */
typedef void (APIENTRYP GdkGLProc_glPixelDataRangeNV) (GLenum target, GLsizei length, GLvoid *pointer);
GdkGLProc    gdk_gl_get_glPixelDataRangeNV (void);
#define      gdk_gl_glPixelDataRangeNV(proc, target, length, pointer) \
  ( ((GdkGLProc_glPixelDataRangeNV) (proc)) (target, length, pointer) )

/* glFlushPixelDataRangeNV */
typedef void (APIENTRYP GdkGLProc_glFlushPixelDataRangeNV) (GLenum target);
GdkGLProc    gdk_gl_get_glFlushPixelDataRangeNV (void);
#define      gdk_gl_glFlushPixelDataRangeNV(proc, target) \
  ( ((GdkGLProc_glFlushPixelDataRangeNV) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_NV_pixel_data_range GdkGL_GL_NV_pixel_data_range;

struct _GdkGL_GL_NV_pixel_data_range
{
  GdkGLProc_glPixelDataRangeNV glPixelDataRangeNV;
  GdkGLProc_glFlushPixelDataRangeNV glFlushPixelDataRangeNV;
};

GdkGL_GL_NV_pixel_data_range *gdk_gl_get_GL_NV_pixel_data_range (void);

/*
 * GL_NV_primitive_restart
 */

/* glPrimitiveRestartNV */
typedef void (APIENTRYP GdkGLProc_glPrimitiveRestartNV) (void);
GdkGLProc    gdk_gl_get_glPrimitiveRestartNV (void);
#define      gdk_gl_glPrimitiveRestartNV(proc) \
  ( ((GdkGLProc_glPrimitiveRestartNV) (proc)) () )

/* glPrimitiveRestartIndexNV */
typedef void (APIENTRYP GdkGLProc_glPrimitiveRestartIndexNV) (GLuint index);
GdkGLProc    gdk_gl_get_glPrimitiveRestartIndexNV (void);
#define      gdk_gl_glPrimitiveRestartIndexNV(proc, index) \
  ( ((GdkGLProc_glPrimitiveRestartIndexNV) (proc)) (index) )

/* proc struct */

typedef struct _GdkGL_GL_NV_primitive_restart GdkGL_GL_NV_primitive_restart;

struct _GdkGL_GL_NV_primitive_restart
{
  GdkGLProc_glPrimitiveRestartNV glPrimitiveRestartNV;
  GdkGLProc_glPrimitiveRestartIndexNV glPrimitiveRestartIndexNV;
};

GdkGL_GL_NV_primitive_restart *gdk_gl_get_GL_NV_primitive_restart (void);

/*
 * GL_ATI_map_object_buffer
 */

/* glMapObjectBufferATI */
typedef GLvoid* (APIENTRYP GdkGLProc_glMapObjectBufferATI) (GLuint buffer);
GdkGLProc    gdk_gl_get_glMapObjectBufferATI (void);
#define      gdk_gl_glMapObjectBufferATI(proc, buffer) \
  ( ((GdkGLProc_glMapObjectBufferATI) (proc)) (buffer) )

/* glUnmapObjectBufferATI */
typedef void (APIENTRYP GdkGLProc_glUnmapObjectBufferATI) (GLuint buffer);
GdkGLProc    gdk_gl_get_glUnmapObjectBufferATI (void);
#define      gdk_gl_glUnmapObjectBufferATI(proc, buffer) \
  ( ((GdkGLProc_glUnmapObjectBufferATI) (proc)) (buffer) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_map_object_buffer GdkGL_GL_ATI_map_object_buffer;

struct _GdkGL_GL_ATI_map_object_buffer
{
  GdkGLProc_glMapObjectBufferATI glMapObjectBufferATI;
  GdkGLProc_glUnmapObjectBufferATI glUnmapObjectBufferATI;
};

GdkGL_GL_ATI_map_object_buffer *gdk_gl_get_GL_ATI_map_object_buffer (void);

/*
 * GL_ATI_separate_stencil
 */

/* glStencilOpSeparateATI */
typedef void (APIENTRYP GdkGLProc_glStencilOpSeparateATI) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
GdkGLProc    gdk_gl_get_glStencilOpSeparateATI (void);
#define      gdk_gl_glStencilOpSeparateATI(proc, face, sfail, dpfail, dppass) \
  ( ((GdkGLProc_glStencilOpSeparateATI) (proc)) (face, sfail, dpfail, dppass) )

/* glStencilFuncSeparateATI */
typedef void (APIENTRYP GdkGLProc_glStencilFuncSeparateATI) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
GdkGLProc    gdk_gl_get_glStencilFuncSeparateATI (void);
#define      gdk_gl_glStencilFuncSeparateATI(proc, frontfunc, backfunc, ref, mask) \
  ( ((GdkGLProc_glStencilFuncSeparateATI) (proc)) (frontfunc, backfunc, ref, mask) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_separate_stencil GdkGL_GL_ATI_separate_stencil;

struct _GdkGL_GL_ATI_separate_stencil
{
  GdkGLProc_glStencilOpSeparateATI glStencilOpSeparateATI;
  GdkGLProc_glStencilFuncSeparateATI glStencilFuncSeparateATI;
};

GdkGL_GL_ATI_separate_stencil *gdk_gl_get_GL_ATI_separate_stencil (void);

/*
 * GL_ATI_vertex_attrib_array_object
 */

/* glVertexAttribArrayObjectATI */
typedef void (APIENTRYP GdkGLProc_glVertexAttribArrayObjectATI) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset);
GdkGLProc    gdk_gl_get_glVertexAttribArrayObjectATI (void);
#define      gdk_gl_glVertexAttribArrayObjectATI(proc, index, size, type, normalized, stride, buffer, offset) \
  ( ((GdkGLProc_glVertexAttribArrayObjectATI) (proc)) (index, size, type, normalized, stride, buffer, offset) )

/* glGetVertexAttribArrayObjectfvATI */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribArrayObjectfvATI) (GLuint index, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribArrayObjectfvATI (void);
#define      gdk_gl_glGetVertexAttribArrayObjectfvATI(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribArrayObjectfvATI) (proc)) (index, pname, params) )

/* glGetVertexAttribArrayObjectivATI */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribArrayObjectivATI) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribArrayObjectivATI (void);
#define      gdk_gl_glGetVertexAttribArrayObjectivATI(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribArrayObjectivATI) (proc)) (index, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_vertex_attrib_array_object GdkGL_GL_ATI_vertex_attrib_array_object;

struct _GdkGL_GL_ATI_vertex_attrib_array_object
{
  GdkGLProc_glVertexAttribArrayObjectATI glVertexAttribArrayObjectATI;
  GdkGLProc_glGetVertexAttribArrayObjectfvATI glGetVertexAttribArrayObjectfvATI;
  GdkGLProc_glGetVertexAttribArrayObjectivATI glGetVertexAttribArrayObjectivATI;
};

GdkGL_GL_ATI_vertex_attrib_array_object *gdk_gl_get_GL_ATI_vertex_attrib_array_object (void);

/*
 * GL_EXT_depth_bounds_test
 */

/* glDepthBoundsEXT */
typedef void (APIENTRYP GdkGLProc_glDepthBoundsEXT) (GLclampd zmin, GLclampd zmax);
GdkGLProc    gdk_gl_get_glDepthBoundsEXT (void);
#define      gdk_gl_glDepthBoundsEXT(proc, zmin, zmax) \
  ( ((GdkGLProc_glDepthBoundsEXT) (proc)) (zmin, zmax) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_depth_bounds_test GdkGL_GL_EXT_depth_bounds_test;

struct _GdkGL_GL_EXT_depth_bounds_test
{
  GdkGLProc_glDepthBoundsEXT glDepthBoundsEXT;
};

GdkGL_GL_EXT_depth_bounds_test *gdk_gl_get_GL_EXT_depth_bounds_test (void);

/*
 * GL_EXT_blend_equation_separate
 */

/* glBlendEquationSeparateEXT */
typedef void (APIENTRYP GdkGLProc_glBlendEquationSeparateEXT) (GLenum modeRGB, GLenum modeAlpha);
GdkGLProc    gdk_gl_get_glBlendEquationSeparateEXT (void);
#define      gdk_gl_glBlendEquationSeparateEXT(proc, modeRGB, modeAlpha) \
  ( ((GdkGLProc_glBlendEquationSeparateEXT) (proc)) (modeRGB, modeAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_blend_equation_separate GdkGL_GL_EXT_blend_equation_separate;

struct _GdkGL_GL_EXT_blend_equation_separate
{
  GdkGLProc_glBlendEquationSeparateEXT glBlendEquationSeparateEXT;
};

GdkGL_GL_EXT_blend_equation_separate *gdk_gl_get_GL_EXT_blend_equation_separate (void);

/*
 * GL_EXT_framebuffer_object
 */

/* glIsRenderbufferEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glIsRenderbufferEXT) (GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glIsRenderbufferEXT (void);
#define      gdk_gl_glIsRenderbufferEXT(proc, renderbuffer) \
  ( ((GdkGLProc_glIsRenderbufferEXT) (proc)) (renderbuffer) )

/* glBindRenderbufferEXT */
typedef void (APIENTRYP GdkGLProc_glBindRenderbufferEXT) (GLenum target, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glBindRenderbufferEXT (void);
#define      gdk_gl_glBindRenderbufferEXT(proc, target, renderbuffer) \
  ( ((GdkGLProc_glBindRenderbufferEXT) (proc)) (target, renderbuffer) )

/* glDeleteRenderbuffersEXT */
typedef void (APIENTRYP GdkGLProc_glDeleteRenderbuffersEXT) (GLsizei n, const GLuint *renderbuffers);
GdkGLProc    gdk_gl_get_glDeleteRenderbuffersEXT (void);
#define      gdk_gl_glDeleteRenderbuffersEXT(proc, n, renderbuffers) \
  ( ((GdkGLProc_glDeleteRenderbuffersEXT) (proc)) (n, renderbuffers) )

/* glGenRenderbuffersEXT */
typedef void (APIENTRYP GdkGLProc_glGenRenderbuffersEXT) (GLsizei n, GLuint *renderbuffers);
GdkGLProc    gdk_gl_get_glGenRenderbuffersEXT (void);
#define      gdk_gl_glGenRenderbuffersEXT(proc, n, renderbuffers) \
  ( ((GdkGLProc_glGenRenderbuffersEXT) (proc)) (n, renderbuffers) )

/* glRenderbufferStorageEXT */
typedef void (APIENTRYP GdkGLProc_glRenderbufferStorageEXT) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glRenderbufferStorageEXT (void);
#define      gdk_gl_glRenderbufferStorageEXT(proc, target, internalformat, width, height) \
  ( ((GdkGLProc_glRenderbufferStorageEXT) (proc)) (target, internalformat, width, height) )

/* glGetRenderbufferParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetRenderbufferParameterivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetRenderbufferParameterivEXT (void);
#define      gdk_gl_glGetRenderbufferParameterivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetRenderbufferParameterivEXT) (proc)) (target, pname, params) )

/* glIsFramebufferEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glIsFramebufferEXT) (GLuint framebuffer);
GdkGLProc    gdk_gl_get_glIsFramebufferEXT (void);
#define      gdk_gl_glIsFramebufferEXT(proc, framebuffer) \
  ( ((GdkGLProc_glIsFramebufferEXT) (proc)) (framebuffer) )

/* glBindFramebufferEXT */
typedef void (APIENTRYP GdkGLProc_glBindFramebufferEXT) (GLenum target, GLuint framebuffer);
GdkGLProc    gdk_gl_get_glBindFramebufferEXT (void);
#define      gdk_gl_glBindFramebufferEXT(proc, target, framebuffer) \
  ( ((GdkGLProc_glBindFramebufferEXT) (proc)) (target, framebuffer) )

/* glDeleteFramebuffersEXT */
typedef void (APIENTRYP GdkGLProc_glDeleteFramebuffersEXT) (GLsizei n, const GLuint *framebuffers);
GdkGLProc    gdk_gl_get_glDeleteFramebuffersEXT (void);
#define      gdk_gl_glDeleteFramebuffersEXT(proc, n, framebuffers) \
  ( ((GdkGLProc_glDeleteFramebuffersEXT) (proc)) (n, framebuffers) )

/* glGenFramebuffersEXT */
typedef void (APIENTRYP GdkGLProc_glGenFramebuffersEXT) (GLsizei n, GLuint *framebuffers);
GdkGLProc    gdk_gl_get_glGenFramebuffersEXT (void);
#define      gdk_gl_glGenFramebuffersEXT(proc, n, framebuffers) \
  ( ((GdkGLProc_glGenFramebuffersEXT) (proc)) (n, framebuffers) )

/* glCheckFramebufferStatusEXT */
typedef GLenum (APIENTRYP GdkGLProc_glCheckFramebufferStatusEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glCheckFramebufferStatusEXT (void);
#define      gdk_gl_glCheckFramebufferStatusEXT(proc, target) \
  ( ((GdkGLProc_glCheckFramebufferStatusEXT) (proc)) (target) )

/* glFramebufferTexture1DEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture1DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTexture1DEXT (void);
#define      gdk_gl_glFramebufferTexture1DEXT(proc, target, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glFramebufferTexture1DEXT) (proc)) (target, attachment, textarget, texture, level) )

/* glFramebufferTexture2DEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture2DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTexture2DEXT (void);
#define      gdk_gl_glFramebufferTexture2DEXT(proc, target, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glFramebufferTexture2DEXT) (proc)) (target, attachment, textarget, texture, level) )

/* glFramebufferTexture3DEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTexture3DEXT) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GdkGLProc    gdk_gl_get_glFramebufferTexture3DEXT (void);
#define      gdk_gl_glFramebufferTexture3DEXT(proc, target, attachment, textarget, texture, level, zoffset) \
  ( ((GdkGLProc_glFramebufferTexture3DEXT) (proc)) (target, attachment, textarget, texture, level, zoffset) )

/* glFramebufferRenderbufferEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferRenderbufferEXT) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glFramebufferRenderbufferEXT (void);
#define      gdk_gl_glFramebufferRenderbufferEXT(proc, target, attachment, renderbuffertarget, renderbuffer) \
  ( ((GdkGLProc_glFramebufferRenderbufferEXT) (proc)) (target, attachment, renderbuffertarget, renderbuffer) )

/* glGetFramebufferAttachmentParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetFramebufferAttachmentParameterivEXT) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFramebufferAttachmentParameterivEXT (void);
#define      gdk_gl_glGetFramebufferAttachmentParameterivEXT(proc, target, attachment, pname, params) \
  ( ((GdkGLProc_glGetFramebufferAttachmentParameterivEXT) (proc)) (target, attachment, pname, params) )

/* glGenerateMipmapEXT */
typedef void (APIENTRYP GdkGLProc_glGenerateMipmapEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glGenerateMipmapEXT (void);
#define      gdk_gl_glGenerateMipmapEXT(proc, target) \
  ( ((GdkGLProc_glGenerateMipmapEXT) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_framebuffer_object GdkGL_GL_EXT_framebuffer_object;

struct _GdkGL_GL_EXT_framebuffer_object
{
  GdkGLProc_glIsRenderbufferEXT glIsRenderbufferEXT;
  GdkGLProc_glBindRenderbufferEXT glBindRenderbufferEXT;
  GdkGLProc_glDeleteRenderbuffersEXT glDeleteRenderbuffersEXT;
  GdkGLProc_glGenRenderbuffersEXT glGenRenderbuffersEXT;
  GdkGLProc_glRenderbufferStorageEXT glRenderbufferStorageEXT;
  GdkGLProc_glGetRenderbufferParameterivEXT glGetRenderbufferParameterivEXT;
  GdkGLProc_glIsFramebufferEXT glIsFramebufferEXT;
  GdkGLProc_glBindFramebufferEXT glBindFramebufferEXT;
  GdkGLProc_glDeleteFramebuffersEXT glDeleteFramebuffersEXT;
  GdkGLProc_glGenFramebuffersEXT glGenFramebuffersEXT;
  GdkGLProc_glCheckFramebufferStatusEXT glCheckFramebufferStatusEXT;
  GdkGLProc_glFramebufferTexture1DEXT glFramebufferTexture1DEXT;
  GdkGLProc_glFramebufferTexture2DEXT glFramebufferTexture2DEXT;
  GdkGLProc_glFramebufferTexture3DEXT glFramebufferTexture3DEXT;
  GdkGLProc_glFramebufferRenderbufferEXT glFramebufferRenderbufferEXT;
  GdkGLProc_glGetFramebufferAttachmentParameterivEXT glGetFramebufferAttachmentParameterivEXT;
  GdkGLProc_glGenerateMipmapEXT glGenerateMipmapEXT;
};

GdkGL_GL_EXT_framebuffer_object *gdk_gl_get_GL_EXT_framebuffer_object (void);

/*
 * GL_GREMEDY_string_marker
 */

/* glStringMarkerGREMEDY */
typedef void (APIENTRYP GdkGLProc_glStringMarkerGREMEDY) (GLsizei len, const GLvoid *string);
GdkGLProc    gdk_gl_get_glStringMarkerGREMEDY (void);
#define      gdk_gl_glStringMarkerGREMEDY(proc, len, string) \
  ( ((GdkGLProc_glStringMarkerGREMEDY) (proc)) (len, string) )

/* proc struct */

typedef struct _GdkGL_GL_GREMEDY_string_marker GdkGL_GL_GREMEDY_string_marker;

struct _GdkGL_GL_GREMEDY_string_marker
{
  GdkGLProc_glStringMarkerGREMEDY glStringMarkerGREMEDY;
};

GdkGL_GL_GREMEDY_string_marker *gdk_gl_get_GL_GREMEDY_string_marker (void);

/*
 * GL_EXT_stencil_clear_tag
 */

/* glStencilClearTagEXT */
typedef void (APIENTRYP GdkGLProc_glStencilClearTagEXT) (GLsizei stencilTagBits, GLuint stencilClearTag);
GdkGLProc    gdk_gl_get_glStencilClearTagEXT (void);
#define      gdk_gl_glStencilClearTagEXT(proc, stencilTagBits, stencilClearTag) \
  ( ((GdkGLProc_glStencilClearTagEXT) (proc)) (stencilTagBits, stencilClearTag) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_stencil_clear_tag GdkGL_GL_EXT_stencil_clear_tag;

struct _GdkGL_GL_EXT_stencil_clear_tag
{
  GdkGLProc_glStencilClearTagEXT glStencilClearTagEXT;
};

GdkGL_GL_EXT_stencil_clear_tag *gdk_gl_get_GL_EXT_stencil_clear_tag (void);

/*
 * GL_EXT_framebuffer_blit
 */

/* glBlitFramebufferEXT */
typedef void (APIENTRYP GdkGLProc_glBlitFramebufferEXT) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
GdkGLProc    gdk_gl_get_glBlitFramebufferEXT (void);
#define      gdk_gl_glBlitFramebufferEXT(proc, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) \
  ( ((GdkGLProc_glBlitFramebufferEXT) (proc)) (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_framebuffer_blit GdkGL_GL_EXT_framebuffer_blit;

struct _GdkGL_GL_EXT_framebuffer_blit
{
  GdkGLProc_glBlitFramebufferEXT glBlitFramebufferEXT;
};

GdkGL_GL_EXT_framebuffer_blit *gdk_gl_get_GL_EXT_framebuffer_blit (void);

/*
 * GL_EXT_framebuffer_multisample
 */

/* glRenderbufferStorageMultisampleEXT */
typedef void (APIENTRYP GdkGLProc_glRenderbufferStorageMultisampleEXT) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glRenderbufferStorageMultisampleEXT (void);
#define      gdk_gl_glRenderbufferStorageMultisampleEXT(proc, target, samples, internalformat, width, height) \
  ( ((GdkGLProc_glRenderbufferStorageMultisampleEXT) (proc)) (target, samples, internalformat, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_framebuffer_multisample GdkGL_GL_EXT_framebuffer_multisample;

struct _GdkGL_GL_EXT_framebuffer_multisample
{
  GdkGLProc_glRenderbufferStorageMultisampleEXT glRenderbufferStorageMultisampleEXT;
};

GdkGL_GL_EXT_framebuffer_multisample *gdk_gl_get_GL_EXT_framebuffer_multisample (void);

/*
 * GL_EXT_timer_query
 */

/* glGetQueryObjecti64vEXT */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjecti64vEXT) (GLuint id, GLenum pname, GLint64EXT *params);
GdkGLProc    gdk_gl_get_glGetQueryObjecti64vEXT (void);
#define      gdk_gl_glGetQueryObjecti64vEXT(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjecti64vEXT) (proc)) (id, pname, params) )

/* glGetQueryObjectui64vEXT */
typedef void (APIENTRYP GdkGLProc_glGetQueryObjectui64vEXT) (GLuint id, GLenum pname, GLuint64EXT *params);
GdkGLProc    gdk_gl_get_glGetQueryObjectui64vEXT (void);
#define      gdk_gl_glGetQueryObjectui64vEXT(proc, id, pname, params) \
  ( ((GdkGLProc_glGetQueryObjectui64vEXT) (proc)) (id, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_timer_query GdkGL_GL_EXT_timer_query;

struct _GdkGL_GL_EXT_timer_query
{
  GdkGLProc_glGetQueryObjecti64vEXT glGetQueryObjecti64vEXT;
  GdkGLProc_glGetQueryObjectui64vEXT glGetQueryObjectui64vEXT;
};

GdkGL_GL_EXT_timer_query *gdk_gl_get_GL_EXT_timer_query (void);

/*
 * GL_EXT_gpu_program_parameters
 */

/* glProgramEnvParameters4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameters4fvEXT) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
GdkGLProc    gdk_gl_get_glProgramEnvParameters4fvEXT (void);
#define      gdk_gl_glProgramEnvParameters4fvEXT(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramEnvParameters4fvEXT) (proc)) (target, index, count, params) )

/* glProgramLocalParameters4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameters4fvEXT) (GLenum target, GLuint index, GLsizei count, const GLfloat *params);
GdkGLProc    gdk_gl_get_glProgramLocalParameters4fvEXT (void);
#define      gdk_gl_glProgramLocalParameters4fvEXT(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramLocalParameters4fvEXT) (proc)) (target, index, count, params) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_gpu_program_parameters GdkGL_GL_EXT_gpu_program_parameters;

struct _GdkGL_GL_EXT_gpu_program_parameters
{
  GdkGLProc_glProgramEnvParameters4fvEXT glProgramEnvParameters4fvEXT;
  GdkGLProc_glProgramLocalParameters4fvEXT glProgramLocalParameters4fvEXT;
};

GdkGL_GL_EXT_gpu_program_parameters *gdk_gl_get_GL_EXT_gpu_program_parameters (void);

/*
 * GL_APPLE_flush_buffer_range
 */

/* glBufferParameteriAPPLE */
typedef void (APIENTRYP GdkGLProc_glBufferParameteriAPPLE) (GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glBufferParameteriAPPLE (void);
#define      gdk_gl_glBufferParameteriAPPLE(proc, target, pname, param) \
  ( ((GdkGLProc_glBufferParameteriAPPLE) (proc)) (target, pname, param) )

/* glFlushMappedBufferRangeAPPLE */
typedef void (APIENTRYP GdkGLProc_glFlushMappedBufferRangeAPPLE) (GLenum target, GLintptr offset, GLsizeiptr size);
GdkGLProc    gdk_gl_get_glFlushMappedBufferRangeAPPLE (void);
#define      gdk_gl_glFlushMappedBufferRangeAPPLE(proc, target, offset, size) \
  ( ((GdkGLProc_glFlushMappedBufferRangeAPPLE) (proc)) (target, offset, size) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_flush_buffer_range GdkGL_GL_APPLE_flush_buffer_range;

struct _GdkGL_GL_APPLE_flush_buffer_range
{
  GdkGLProc_glBufferParameteriAPPLE glBufferParameteriAPPLE;
  GdkGLProc_glFlushMappedBufferRangeAPPLE glFlushMappedBufferRangeAPPLE;
};

GdkGL_GL_APPLE_flush_buffer_range *gdk_gl_get_GL_APPLE_flush_buffer_range (void);

/*
 * GL_NV_gpu_program4
 */

/* glProgramLocalParameterI4iNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameterI4iNV) (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glProgramLocalParameterI4iNV (void);
#define      gdk_gl_glProgramLocalParameterI4iNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramLocalParameterI4iNV) (proc)) (target, index, x, y, z, w) )

/* glProgramLocalParameterI4ivNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameterI4ivNV) (GLenum target, GLuint index, const GLint *params);
GdkGLProc    gdk_gl_get_glProgramLocalParameterI4ivNV (void);
#define      gdk_gl_glProgramLocalParameterI4ivNV(proc, target, index, params) \
  ( ((GdkGLProc_glProgramLocalParameterI4ivNV) (proc)) (target, index, params) )

/* glProgramLocalParametersI4ivNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParametersI4ivNV) (GLenum target, GLuint index, GLsizei count, const GLint *params);
GdkGLProc    gdk_gl_get_glProgramLocalParametersI4ivNV (void);
#define      gdk_gl_glProgramLocalParametersI4ivNV(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramLocalParametersI4ivNV) (proc)) (target, index, count, params) )

/* glProgramLocalParameterI4uiNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameterI4uiNV) (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GdkGLProc    gdk_gl_get_glProgramLocalParameterI4uiNV (void);
#define      gdk_gl_glProgramLocalParameterI4uiNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramLocalParameterI4uiNV) (proc)) (target, index, x, y, z, w) )

/* glProgramLocalParameterI4uivNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParameterI4uivNV) (GLenum target, GLuint index, const GLuint *params);
GdkGLProc    gdk_gl_get_glProgramLocalParameterI4uivNV (void);
#define      gdk_gl_glProgramLocalParameterI4uivNV(proc, target, index, params) \
  ( ((GdkGLProc_glProgramLocalParameterI4uivNV) (proc)) (target, index, params) )

/* glProgramLocalParametersI4uivNV */
typedef void (APIENTRYP GdkGLProc_glProgramLocalParametersI4uivNV) (GLenum target, GLuint index, GLsizei count, const GLuint *params);
GdkGLProc    gdk_gl_get_glProgramLocalParametersI4uivNV (void);
#define      gdk_gl_glProgramLocalParametersI4uivNV(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramLocalParametersI4uivNV) (proc)) (target, index, count, params) )

/* glProgramEnvParameterI4iNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameterI4iNV) (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glProgramEnvParameterI4iNV (void);
#define      gdk_gl_glProgramEnvParameterI4iNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramEnvParameterI4iNV) (proc)) (target, index, x, y, z, w) )

/* glProgramEnvParameterI4ivNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameterI4ivNV) (GLenum target, GLuint index, const GLint *params);
GdkGLProc    gdk_gl_get_glProgramEnvParameterI4ivNV (void);
#define      gdk_gl_glProgramEnvParameterI4ivNV(proc, target, index, params) \
  ( ((GdkGLProc_glProgramEnvParameterI4ivNV) (proc)) (target, index, params) )

/* glProgramEnvParametersI4ivNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParametersI4ivNV) (GLenum target, GLuint index, GLsizei count, const GLint *params);
GdkGLProc    gdk_gl_get_glProgramEnvParametersI4ivNV (void);
#define      gdk_gl_glProgramEnvParametersI4ivNV(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramEnvParametersI4ivNV) (proc)) (target, index, count, params) )

/* glProgramEnvParameterI4uiNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameterI4uiNV) (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GdkGLProc    gdk_gl_get_glProgramEnvParameterI4uiNV (void);
#define      gdk_gl_glProgramEnvParameterI4uiNV(proc, target, index, x, y, z, w) \
  ( ((GdkGLProc_glProgramEnvParameterI4uiNV) (proc)) (target, index, x, y, z, w) )

/* glProgramEnvParameterI4uivNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParameterI4uivNV) (GLenum target, GLuint index, const GLuint *params);
GdkGLProc    gdk_gl_get_glProgramEnvParameterI4uivNV (void);
#define      gdk_gl_glProgramEnvParameterI4uivNV(proc, target, index, params) \
  ( ((GdkGLProc_glProgramEnvParameterI4uivNV) (proc)) (target, index, params) )

/* glProgramEnvParametersI4uivNV */
typedef void (APIENTRYP GdkGLProc_glProgramEnvParametersI4uivNV) (GLenum target, GLuint index, GLsizei count, const GLuint *params);
GdkGLProc    gdk_gl_get_glProgramEnvParametersI4uivNV (void);
#define      gdk_gl_glProgramEnvParametersI4uivNV(proc, target, index, count, params) \
  ( ((GdkGLProc_glProgramEnvParametersI4uivNV) (proc)) (target, index, count, params) )

/* glGetProgramLocalParameterIivNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramLocalParameterIivNV) (GLenum target, GLuint index, GLint *params);
GdkGLProc    gdk_gl_get_glGetProgramLocalParameterIivNV (void);
#define      gdk_gl_glGetProgramLocalParameterIivNV(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramLocalParameterIivNV) (proc)) (target, index, params) )

/* glGetProgramLocalParameterIuivNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramLocalParameterIuivNV) (GLenum target, GLuint index, GLuint *params);
GdkGLProc    gdk_gl_get_glGetProgramLocalParameterIuivNV (void);
#define      gdk_gl_glGetProgramLocalParameterIuivNV(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramLocalParameterIuivNV) (proc)) (target, index, params) )

/* glGetProgramEnvParameterIivNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramEnvParameterIivNV) (GLenum target, GLuint index, GLint *params);
GdkGLProc    gdk_gl_get_glGetProgramEnvParameterIivNV (void);
#define      gdk_gl_glGetProgramEnvParameterIivNV(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramEnvParameterIivNV) (proc)) (target, index, params) )

/* glGetProgramEnvParameterIuivNV */
typedef void (APIENTRYP GdkGLProc_glGetProgramEnvParameterIuivNV) (GLenum target, GLuint index, GLuint *params);
GdkGLProc    gdk_gl_get_glGetProgramEnvParameterIuivNV (void);
#define      gdk_gl_glGetProgramEnvParameterIuivNV(proc, target, index, params) \
  ( ((GdkGLProc_glGetProgramEnvParameterIuivNV) (proc)) (target, index, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_gpu_program4 GdkGL_GL_NV_gpu_program4;

struct _GdkGL_GL_NV_gpu_program4
{
  GdkGLProc_glProgramLocalParameterI4iNV glProgramLocalParameterI4iNV;
  GdkGLProc_glProgramLocalParameterI4ivNV glProgramLocalParameterI4ivNV;
  GdkGLProc_glProgramLocalParametersI4ivNV glProgramLocalParametersI4ivNV;
  GdkGLProc_glProgramLocalParameterI4uiNV glProgramLocalParameterI4uiNV;
  GdkGLProc_glProgramLocalParameterI4uivNV glProgramLocalParameterI4uivNV;
  GdkGLProc_glProgramLocalParametersI4uivNV glProgramLocalParametersI4uivNV;
  GdkGLProc_glProgramEnvParameterI4iNV glProgramEnvParameterI4iNV;
  GdkGLProc_glProgramEnvParameterI4ivNV glProgramEnvParameterI4ivNV;
  GdkGLProc_glProgramEnvParametersI4ivNV glProgramEnvParametersI4ivNV;
  GdkGLProc_glProgramEnvParameterI4uiNV glProgramEnvParameterI4uiNV;
  GdkGLProc_glProgramEnvParameterI4uivNV glProgramEnvParameterI4uivNV;
  GdkGLProc_glProgramEnvParametersI4uivNV glProgramEnvParametersI4uivNV;
  GdkGLProc_glGetProgramLocalParameterIivNV glGetProgramLocalParameterIivNV;
  GdkGLProc_glGetProgramLocalParameterIuivNV glGetProgramLocalParameterIuivNV;
  GdkGLProc_glGetProgramEnvParameterIivNV glGetProgramEnvParameterIivNV;
  GdkGLProc_glGetProgramEnvParameterIuivNV glGetProgramEnvParameterIuivNV;
};

GdkGL_GL_NV_gpu_program4 *gdk_gl_get_GL_NV_gpu_program4 (void);

/*
 * GL_NV_geometry_program4
 */

/* glProgramVertexLimitNV */
typedef void (APIENTRYP GdkGLProc_glProgramVertexLimitNV) (GLenum target, GLint limit);
GdkGLProc    gdk_gl_get_glProgramVertexLimitNV (void);
#define      gdk_gl_glProgramVertexLimitNV(proc, target, limit) \
  ( ((GdkGLProc_glProgramVertexLimitNV) (proc)) (target, limit) )

/* glFramebufferTextureEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glFramebufferTextureEXT (void);
#define      gdk_gl_glFramebufferTextureEXT(proc, target, attachment, texture, level) \
  ( ((GdkGLProc_glFramebufferTextureEXT) (proc)) (target, attachment, texture, level) )

/* glFramebufferTextureLayerEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureLayerEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
GdkGLProc    gdk_gl_get_glFramebufferTextureLayerEXT (void);
#define      gdk_gl_glFramebufferTextureLayerEXT(proc, target, attachment, texture, level, layer) \
  ( ((GdkGLProc_glFramebufferTextureLayerEXT) (proc)) (target, attachment, texture, level, layer) )

/* glFramebufferTextureFaceEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferTextureFaceEXT) (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face);
GdkGLProc    gdk_gl_get_glFramebufferTextureFaceEXT (void);
#define      gdk_gl_glFramebufferTextureFaceEXT(proc, target, attachment, texture, level, face) \
  ( ((GdkGLProc_glFramebufferTextureFaceEXT) (proc)) (target, attachment, texture, level, face) )

/* proc struct */

typedef struct _GdkGL_GL_NV_geometry_program4 GdkGL_GL_NV_geometry_program4;

struct _GdkGL_GL_NV_geometry_program4
{
  GdkGLProc_glProgramVertexLimitNV glProgramVertexLimitNV;
  GdkGLProc_glFramebufferTextureEXT glFramebufferTextureEXT;
  GdkGLProc_glFramebufferTextureLayerEXT glFramebufferTextureLayerEXT;
  GdkGLProc_glFramebufferTextureFaceEXT glFramebufferTextureFaceEXT;
};

GdkGL_GL_NV_geometry_program4 *gdk_gl_get_GL_NV_geometry_program4 (void);

/*
 * GL_EXT_geometry_shader4
 */

/* glProgramParameteriEXT */
typedef void (APIENTRYP GdkGLProc_glProgramParameteriEXT) (GLuint program, GLenum pname, GLint value);
GdkGLProc    gdk_gl_get_glProgramParameteriEXT (void);
#define      gdk_gl_glProgramParameteriEXT(proc, program, pname, value) \
  ( ((GdkGLProc_glProgramParameteriEXT) (proc)) (program, pname, value) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_geometry_shader4 GdkGL_GL_EXT_geometry_shader4;

struct _GdkGL_GL_EXT_geometry_shader4
{
  GdkGLProc_glProgramParameteriEXT glProgramParameteriEXT;
};

GdkGL_GL_EXT_geometry_shader4 *gdk_gl_get_GL_EXT_geometry_shader4 (void);

/*
 * GL_NV_vertex_program4
 */

/* glVertexAttribI1iEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1iEXT) (GLuint index, GLint x);
GdkGLProc    gdk_gl_get_glVertexAttribI1iEXT (void);
#define      gdk_gl_glVertexAttribI1iEXT(proc, index, x) \
  ( ((GdkGLProc_glVertexAttribI1iEXT) (proc)) (index, x) )

/* glVertexAttribI2iEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2iEXT) (GLuint index, GLint x, GLint y);
GdkGLProc    gdk_gl_get_glVertexAttribI2iEXT (void);
#define      gdk_gl_glVertexAttribI2iEXT(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttribI2iEXT) (proc)) (index, x, y) )

/* glVertexAttribI3iEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3iEXT) (GLuint index, GLint x, GLint y, GLint z);
GdkGLProc    gdk_gl_get_glVertexAttribI3iEXT (void);
#define      gdk_gl_glVertexAttribI3iEXT(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttribI3iEXT) (proc)) (index, x, y, z) )

/* glVertexAttribI4iEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4iEXT) (GLuint index, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glVertexAttribI4iEXT (void);
#define      gdk_gl_glVertexAttribI4iEXT(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttribI4iEXT) (proc)) (index, x, y, z, w) )

/* glVertexAttribI1uiEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1uiEXT) (GLuint index, GLuint x);
GdkGLProc    gdk_gl_get_glVertexAttribI1uiEXT (void);
#define      gdk_gl_glVertexAttribI1uiEXT(proc, index, x) \
  ( ((GdkGLProc_glVertexAttribI1uiEXT) (proc)) (index, x) )

/* glVertexAttribI2uiEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2uiEXT) (GLuint index, GLuint x, GLuint y);
GdkGLProc    gdk_gl_get_glVertexAttribI2uiEXT (void);
#define      gdk_gl_glVertexAttribI2uiEXT(proc, index, x, y) \
  ( ((GdkGLProc_glVertexAttribI2uiEXT) (proc)) (index, x, y) )

/* glVertexAttribI3uiEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3uiEXT) (GLuint index, GLuint x, GLuint y, GLuint z);
GdkGLProc    gdk_gl_get_glVertexAttribI3uiEXT (void);
#define      gdk_gl_glVertexAttribI3uiEXT(proc, index, x, y, z) \
  ( ((GdkGLProc_glVertexAttribI3uiEXT) (proc)) (index, x, y, z) )

/* glVertexAttribI4uiEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4uiEXT) (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GdkGLProc    gdk_gl_get_glVertexAttribI4uiEXT (void);
#define      gdk_gl_glVertexAttribI4uiEXT(proc, index, x, y, z, w) \
  ( ((GdkGLProc_glVertexAttribI4uiEXT) (proc)) (index, x, y, z, w) )

/* glVertexAttribI1ivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1ivEXT) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI1ivEXT (void);
#define      gdk_gl_glVertexAttribI1ivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI1ivEXT) (proc)) (index, v) )

/* glVertexAttribI2ivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2ivEXT) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI2ivEXT (void);
#define      gdk_gl_glVertexAttribI2ivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI2ivEXT) (proc)) (index, v) )

/* glVertexAttribI3ivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3ivEXT) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI3ivEXT (void);
#define      gdk_gl_glVertexAttribI3ivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI3ivEXT) (proc)) (index, v) )

/* glVertexAttribI4ivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4ivEXT) (GLuint index, const GLint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4ivEXT (void);
#define      gdk_gl_glVertexAttribI4ivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4ivEXT) (proc)) (index, v) )

/* glVertexAttribI1uivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI1uivEXT) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI1uivEXT (void);
#define      gdk_gl_glVertexAttribI1uivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI1uivEXT) (proc)) (index, v) )

/* glVertexAttribI2uivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI2uivEXT) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI2uivEXT (void);
#define      gdk_gl_glVertexAttribI2uivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI2uivEXT) (proc)) (index, v) )

/* glVertexAttribI3uivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI3uivEXT) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI3uivEXT (void);
#define      gdk_gl_glVertexAttribI3uivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI3uivEXT) (proc)) (index, v) )

/* glVertexAttribI4uivEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4uivEXT) (GLuint index, const GLuint *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4uivEXT (void);
#define      gdk_gl_glVertexAttribI4uivEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4uivEXT) (proc)) (index, v) )

/* glVertexAttribI4bvEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4bvEXT) (GLuint index, const GLbyte *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4bvEXT (void);
#define      gdk_gl_glVertexAttribI4bvEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4bvEXT) (proc)) (index, v) )

/* glVertexAttribI4svEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4svEXT) (GLuint index, const GLshort *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4svEXT (void);
#define      gdk_gl_glVertexAttribI4svEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4svEXT) (proc)) (index, v) )

/* glVertexAttribI4ubvEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4ubvEXT) (GLuint index, const GLubyte *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4ubvEXT (void);
#define      gdk_gl_glVertexAttribI4ubvEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4ubvEXT) (proc)) (index, v) )

/* glVertexAttribI4usvEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribI4usvEXT) (GLuint index, const GLushort *v);
GdkGLProc    gdk_gl_get_glVertexAttribI4usvEXT (void);
#define      gdk_gl_glVertexAttribI4usvEXT(proc, index, v) \
  ( ((GdkGLProc_glVertexAttribI4usvEXT) (proc)) (index, v) )

/* glVertexAttribIPointerEXT */
typedef void (APIENTRYP GdkGLProc_glVertexAttribIPointerEXT) (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glVertexAttribIPointerEXT (void);
#define      gdk_gl_glVertexAttribIPointerEXT(proc, index, size, type, stride, pointer) \
  ( ((GdkGLProc_glVertexAttribIPointerEXT) (proc)) (index, size, type, stride, pointer) )

/* glGetVertexAttribIivEXT */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribIivEXT) (GLuint index, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribIivEXT (void);
#define      gdk_gl_glGetVertexAttribIivEXT(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribIivEXT) (proc)) (index, pname, params) )

/* glGetVertexAttribIuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetVertexAttribIuivEXT) (GLuint index, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetVertexAttribIuivEXT (void);
#define      gdk_gl_glGetVertexAttribIuivEXT(proc, index, pname, params) \
  ( ((GdkGLProc_glGetVertexAttribIuivEXT) (proc)) (index, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_vertex_program4 GdkGL_GL_NV_vertex_program4;

struct _GdkGL_GL_NV_vertex_program4
{
  GdkGLProc_glVertexAttribI1iEXT glVertexAttribI1iEXT;
  GdkGLProc_glVertexAttribI2iEXT glVertexAttribI2iEXT;
  GdkGLProc_glVertexAttribI3iEXT glVertexAttribI3iEXT;
  GdkGLProc_glVertexAttribI4iEXT glVertexAttribI4iEXT;
  GdkGLProc_glVertexAttribI1uiEXT glVertexAttribI1uiEXT;
  GdkGLProc_glVertexAttribI2uiEXT glVertexAttribI2uiEXT;
  GdkGLProc_glVertexAttribI3uiEXT glVertexAttribI3uiEXT;
  GdkGLProc_glVertexAttribI4uiEXT glVertexAttribI4uiEXT;
  GdkGLProc_glVertexAttribI1ivEXT glVertexAttribI1ivEXT;
  GdkGLProc_glVertexAttribI2ivEXT glVertexAttribI2ivEXT;
  GdkGLProc_glVertexAttribI3ivEXT glVertexAttribI3ivEXT;
  GdkGLProc_glVertexAttribI4ivEXT glVertexAttribI4ivEXT;
  GdkGLProc_glVertexAttribI1uivEXT glVertexAttribI1uivEXT;
  GdkGLProc_glVertexAttribI2uivEXT glVertexAttribI2uivEXT;
  GdkGLProc_glVertexAttribI3uivEXT glVertexAttribI3uivEXT;
  GdkGLProc_glVertexAttribI4uivEXT glVertexAttribI4uivEXT;
  GdkGLProc_glVertexAttribI4bvEXT glVertexAttribI4bvEXT;
  GdkGLProc_glVertexAttribI4svEXT glVertexAttribI4svEXT;
  GdkGLProc_glVertexAttribI4ubvEXT glVertexAttribI4ubvEXT;
  GdkGLProc_glVertexAttribI4usvEXT glVertexAttribI4usvEXT;
  GdkGLProc_glVertexAttribIPointerEXT glVertexAttribIPointerEXT;
  GdkGLProc_glGetVertexAttribIivEXT glGetVertexAttribIivEXT;
  GdkGLProc_glGetVertexAttribIuivEXT glGetVertexAttribIuivEXT;
};

GdkGL_GL_NV_vertex_program4 *gdk_gl_get_GL_NV_vertex_program4 (void);

/*
 * GL_EXT_gpu_shader4
 */

/* glGetUniformuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetUniformuivEXT) (GLuint program, GLint location, GLuint *params);
GdkGLProc    gdk_gl_get_glGetUniformuivEXT (void);
#define      gdk_gl_glGetUniformuivEXT(proc, program, location, params) \
  ( ((GdkGLProc_glGetUniformuivEXT) (proc)) (program, location, params) )

/* glBindFragDataLocationEXT */
typedef void (APIENTRYP GdkGLProc_glBindFragDataLocationEXT) (GLuint program, GLuint color, const GLchar *name);
GdkGLProc    gdk_gl_get_glBindFragDataLocationEXT (void);
#define      gdk_gl_glBindFragDataLocationEXT(proc, program, color, name) \
  ( ((GdkGLProc_glBindFragDataLocationEXT) (proc)) (program, color, name) )

/* glGetFragDataLocationEXT */
typedef GLint (APIENTRYP GdkGLProc_glGetFragDataLocationEXT) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glGetFragDataLocationEXT (void);
#define      gdk_gl_glGetFragDataLocationEXT(proc, program, name) \
  ( ((GdkGLProc_glGetFragDataLocationEXT) (proc)) (program, name) )

/* glUniform1uiEXT */
typedef void (APIENTRYP GdkGLProc_glUniform1uiEXT) (GLint location, GLuint v0);
GdkGLProc    gdk_gl_get_glUniform1uiEXT (void);
#define      gdk_gl_glUniform1uiEXT(proc, location, v0) \
  ( ((GdkGLProc_glUniform1uiEXT) (proc)) (location, v0) )

/* glUniform2uiEXT */
typedef void (APIENTRYP GdkGLProc_glUniform2uiEXT) (GLint location, GLuint v0, GLuint v1);
GdkGLProc    gdk_gl_get_glUniform2uiEXT (void);
#define      gdk_gl_glUniform2uiEXT(proc, location, v0, v1) \
  ( ((GdkGLProc_glUniform2uiEXT) (proc)) (location, v0, v1) )

/* glUniform3uiEXT */
typedef void (APIENTRYP GdkGLProc_glUniform3uiEXT) (GLint location, GLuint v0, GLuint v1, GLuint v2);
GdkGLProc    gdk_gl_get_glUniform3uiEXT (void);
#define      gdk_gl_glUniform3uiEXT(proc, location, v0, v1, v2) \
  ( ((GdkGLProc_glUniform3uiEXT) (proc)) (location, v0, v1, v2) )

/* glUniform4uiEXT */
typedef void (APIENTRYP GdkGLProc_glUniform4uiEXT) (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GdkGLProc    gdk_gl_get_glUniform4uiEXT (void);
#define      gdk_gl_glUniform4uiEXT(proc, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glUniform4uiEXT) (proc)) (location, v0, v1, v2, v3) )

/* glUniform1uivEXT */
typedef void (APIENTRYP GdkGLProc_glUniform1uivEXT) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform1uivEXT (void);
#define      gdk_gl_glUniform1uivEXT(proc, location, count, value) \
  ( ((GdkGLProc_glUniform1uivEXT) (proc)) (location, count, value) )

/* glUniform2uivEXT */
typedef void (APIENTRYP GdkGLProc_glUniform2uivEXT) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform2uivEXT (void);
#define      gdk_gl_glUniform2uivEXT(proc, location, count, value) \
  ( ((GdkGLProc_glUniform2uivEXT) (proc)) (location, count, value) )

/* glUniform3uivEXT */
typedef void (APIENTRYP GdkGLProc_glUniform3uivEXT) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform3uivEXT (void);
#define      gdk_gl_glUniform3uivEXT(proc, location, count, value) \
  ( ((GdkGLProc_glUniform3uivEXT) (proc)) (location, count, value) )

/* glUniform4uivEXT */
typedef void (APIENTRYP GdkGLProc_glUniform4uivEXT) (GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glUniform4uivEXT (void);
#define      gdk_gl_glUniform4uivEXT(proc, location, count, value) \
  ( ((GdkGLProc_glUniform4uivEXT) (proc)) (location, count, value) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_gpu_shader4 GdkGL_GL_EXT_gpu_shader4;

struct _GdkGL_GL_EXT_gpu_shader4
{
  GdkGLProc_glGetUniformuivEXT glGetUniformuivEXT;
  GdkGLProc_glBindFragDataLocationEXT glBindFragDataLocationEXT;
  GdkGLProc_glGetFragDataLocationEXT glGetFragDataLocationEXT;
  GdkGLProc_glUniform1uiEXT glUniform1uiEXT;
  GdkGLProc_glUniform2uiEXT glUniform2uiEXT;
  GdkGLProc_glUniform3uiEXT glUniform3uiEXT;
  GdkGLProc_glUniform4uiEXT glUniform4uiEXT;
  GdkGLProc_glUniform1uivEXT glUniform1uivEXT;
  GdkGLProc_glUniform2uivEXT glUniform2uivEXT;
  GdkGLProc_glUniform3uivEXT glUniform3uivEXT;
  GdkGLProc_glUniform4uivEXT glUniform4uivEXT;
};

GdkGL_GL_EXT_gpu_shader4 *gdk_gl_get_GL_EXT_gpu_shader4 (void);

/*
 * GL_EXT_draw_instanced
 */

/* glDrawArraysInstancedEXT */
typedef void (APIENTRYP GdkGLProc_glDrawArraysInstancedEXT) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawArraysInstancedEXT (void);
#define      gdk_gl_glDrawArraysInstancedEXT(proc, mode, start, count, primcount) \
  ( ((GdkGLProc_glDrawArraysInstancedEXT) (proc)) (mode, start, count, primcount) )

/* glDrawElementsInstancedEXT */
typedef void (APIENTRYP GdkGLProc_glDrawElementsInstancedEXT) (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glDrawElementsInstancedEXT (void);
#define      gdk_gl_glDrawElementsInstancedEXT(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glDrawElementsInstancedEXT) (proc)) (mode, count, type, indices, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_draw_instanced GdkGL_GL_EXT_draw_instanced;

struct _GdkGL_GL_EXT_draw_instanced
{
  GdkGLProc_glDrawArraysInstancedEXT glDrawArraysInstancedEXT;
  GdkGLProc_glDrawElementsInstancedEXT glDrawElementsInstancedEXT;
};

GdkGL_GL_EXT_draw_instanced *gdk_gl_get_GL_EXT_draw_instanced (void);

/*
 * GL_EXT_texture_buffer_object
 */

/* glTexBufferEXT */
typedef void (APIENTRYP GdkGLProc_glTexBufferEXT) (GLenum target, GLenum internalformat, GLuint buffer);
GdkGLProc    gdk_gl_get_glTexBufferEXT (void);
#define      gdk_gl_glTexBufferEXT(proc, target, internalformat, buffer) \
  ( ((GdkGLProc_glTexBufferEXT) (proc)) (target, internalformat, buffer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_texture_buffer_object GdkGL_GL_EXT_texture_buffer_object;

struct _GdkGL_GL_EXT_texture_buffer_object
{
  GdkGLProc_glTexBufferEXT glTexBufferEXT;
};

GdkGL_GL_EXT_texture_buffer_object *gdk_gl_get_GL_EXT_texture_buffer_object (void);

/*
 * GL_NV_depth_buffer_float
 */

/* glDepthRangedNV */
typedef void (APIENTRYP GdkGLProc_glDepthRangedNV) (GLdouble zNear, GLdouble zFar);
GdkGLProc    gdk_gl_get_glDepthRangedNV (void);
#define      gdk_gl_glDepthRangedNV(proc, zNear, zFar) \
  ( ((GdkGLProc_glDepthRangedNV) (proc)) (zNear, zFar) )

/* glClearDepthdNV */
typedef void (APIENTRYP GdkGLProc_glClearDepthdNV) (GLdouble depth);
GdkGLProc    gdk_gl_get_glClearDepthdNV (void);
#define      gdk_gl_glClearDepthdNV(proc, depth) \
  ( ((GdkGLProc_glClearDepthdNV) (proc)) (depth) )

/* glDepthBoundsdNV */
typedef void (APIENTRYP GdkGLProc_glDepthBoundsdNV) (GLdouble zmin, GLdouble zmax);
GdkGLProc    gdk_gl_get_glDepthBoundsdNV (void);
#define      gdk_gl_glDepthBoundsdNV(proc, zmin, zmax) \
  ( ((GdkGLProc_glDepthBoundsdNV) (proc)) (zmin, zmax) )

/* proc struct */

typedef struct _GdkGL_GL_NV_depth_buffer_float GdkGL_GL_NV_depth_buffer_float;

struct _GdkGL_GL_NV_depth_buffer_float
{
  GdkGLProc_glDepthRangedNV glDepthRangedNV;
  GdkGLProc_glClearDepthdNV glClearDepthdNV;
  GdkGLProc_glDepthBoundsdNV glDepthBoundsdNV;
};

GdkGL_GL_NV_depth_buffer_float *gdk_gl_get_GL_NV_depth_buffer_float (void);

/*
 * GL_NV_framebuffer_multisample_coverage
 */

/* glRenderbufferStorageMultisampleCoverageNV */
typedef void (APIENTRYP GdkGLProc_glRenderbufferStorageMultisampleCoverageNV) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glRenderbufferStorageMultisampleCoverageNV (void);
#define      gdk_gl_glRenderbufferStorageMultisampleCoverageNV(proc, target, coverageSamples, colorSamples, internalformat, width, height) \
  ( ((GdkGLProc_glRenderbufferStorageMultisampleCoverageNV) (proc)) (target, coverageSamples, colorSamples, internalformat, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_NV_framebuffer_multisample_coverage GdkGL_GL_NV_framebuffer_multisample_coverage;

struct _GdkGL_GL_NV_framebuffer_multisample_coverage
{
  GdkGLProc_glRenderbufferStorageMultisampleCoverageNV glRenderbufferStorageMultisampleCoverageNV;
};

GdkGL_GL_NV_framebuffer_multisample_coverage *gdk_gl_get_GL_NV_framebuffer_multisample_coverage (void);

/*
 * GL_NV_parameter_buffer_object
 */

/* glProgramBufferParametersfvNV */
typedef void (APIENTRYP GdkGLProc_glProgramBufferParametersfvNV) (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLfloat *params);
GdkGLProc    gdk_gl_get_glProgramBufferParametersfvNV (void);
#define      gdk_gl_glProgramBufferParametersfvNV(proc, target, buffer, index, count, params) \
  ( ((GdkGLProc_glProgramBufferParametersfvNV) (proc)) (target, buffer, index, count, params) )

/* glProgramBufferParametersIivNV */
typedef void (APIENTRYP GdkGLProc_glProgramBufferParametersIivNV) (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLint *params);
GdkGLProc    gdk_gl_get_glProgramBufferParametersIivNV (void);
#define      gdk_gl_glProgramBufferParametersIivNV(proc, target, buffer, index, count, params) \
  ( ((GdkGLProc_glProgramBufferParametersIivNV) (proc)) (target, buffer, index, count, params) )

/* glProgramBufferParametersIuivNV */
typedef void (APIENTRYP GdkGLProc_glProgramBufferParametersIuivNV) (GLenum target, GLuint buffer, GLuint index, GLsizei count, const GLuint *params);
GdkGLProc    gdk_gl_get_glProgramBufferParametersIuivNV (void);
#define      gdk_gl_glProgramBufferParametersIuivNV(proc, target, buffer, index, count, params) \
  ( ((GdkGLProc_glProgramBufferParametersIuivNV) (proc)) (target, buffer, index, count, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_parameter_buffer_object GdkGL_GL_NV_parameter_buffer_object;

struct _GdkGL_GL_NV_parameter_buffer_object
{
  GdkGLProc_glProgramBufferParametersfvNV glProgramBufferParametersfvNV;
  GdkGLProc_glProgramBufferParametersIivNV glProgramBufferParametersIivNV;
  GdkGLProc_glProgramBufferParametersIuivNV glProgramBufferParametersIuivNV;
};

GdkGL_GL_NV_parameter_buffer_object *gdk_gl_get_GL_NV_parameter_buffer_object (void);

/*
 * GL_EXT_draw_buffers2
 */

/* glColorMaskIndexedEXT */
typedef void (APIENTRYP GdkGLProc_glColorMaskIndexedEXT) (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
GdkGLProc    gdk_gl_get_glColorMaskIndexedEXT (void);
#define      gdk_gl_glColorMaskIndexedEXT(proc, index, r, g, b, a) \
  ( ((GdkGLProc_glColorMaskIndexedEXT) (proc)) (index, r, g, b, a) )

/* glGetBooleanIndexedvEXT */
typedef void (APIENTRYP GdkGLProc_glGetBooleanIndexedvEXT) (GLenum target, GLuint index, GLboolean *data);
GdkGLProc    gdk_gl_get_glGetBooleanIndexedvEXT (void);
#define      gdk_gl_glGetBooleanIndexedvEXT(proc, target, index, data) \
  ( ((GdkGLProc_glGetBooleanIndexedvEXT) (proc)) (target, index, data) )

/* glGetIntegerIndexedvEXT */
typedef void (APIENTRYP GdkGLProc_glGetIntegerIndexedvEXT) (GLenum target, GLuint index, GLint *data);
GdkGLProc    gdk_gl_get_glGetIntegerIndexedvEXT (void);
#define      gdk_gl_glGetIntegerIndexedvEXT(proc, target, index, data) \
  ( ((GdkGLProc_glGetIntegerIndexedvEXT) (proc)) (target, index, data) )

/* glEnableIndexedEXT */
typedef void (APIENTRYP GdkGLProc_glEnableIndexedEXT) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glEnableIndexedEXT (void);
#define      gdk_gl_glEnableIndexedEXT(proc, target, index) \
  ( ((GdkGLProc_glEnableIndexedEXT) (proc)) (target, index) )

/* glDisableIndexedEXT */
typedef void (APIENTRYP GdkGLProc_glDisableIndexedEXT) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glDisableIndexedEXT (void);
#define      gdk_gl_glDisableIndexedEXT(proc, target, index) \
  ( ((GdkGLProc_glDisableIndexedEXT) (proc)) (target, index) )

/* glIsEnabledIndexedEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glIsEnabledIndexedEXT) (GLenum target, GLuint index);
GdkGLProc    gdk_gl_get_glIsEnabledIndexedEXT (void);
#define      gdk_gl_glIsEnabledIndexedEXT(proc, target, index) \
  ( ((GdkGLProc_glIsEnabledIndexedEXT) (proc)) (target, index) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_draw_buffers2 GdkGL_GL_EXT_draw_buffers2;

struct _GdkGL_GL_EXT_draw_buffers2
{
  GdkGLProc_glColorMaskIndexedEXT glColorMaskIndexedEXT;
  GdkGLProc_glGetBooleanIndexedvEXT glGetBooleanIndexedvEXT;
  GdkGLProc_glGetIntegerIndexedvEXT glGetIntegerIndexedvEXT;
  GdkGLProc_glEnableIndexedEXT glEnableIndexedEXT;
  GdkGLProc_glDisableIndexedEXT glDisableIndexedEXT;
  GdkGLProc_glIsEnabledIndexedEXT glIsEnabledIndexedEXT;
};

GdkGL_GL_EXT_draw_buffers2 *gdk_gl_get_GL_EXT_draw_buffers2 (void);

/*
 * GL_NV_transform_feedback
 */

/* glBeginTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glBeginTransformFeedbackNV) (GLenum primitiveMode);
GdkGLProc    gdk_gl_get_glBeginTransformFeedbackNV (void);
#define      gdk_gl_glBeginTransformFeedbackNV(proc, primitiveMode) \
  ( ((GdkGLProc_glBeginTransformFeedbackNV) (proc)) (primitiveMode) )

/* glEndTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glEndTransformFeedbackNV) (void);
GdkGLProc    gdk_gl_get_glEndTransformFeedbackNV (void);
#define      gdk_gl_glEndTransformFeedbackNV(proc) \
  ( ((GdkGLProc_glEndTransformFeedbackNV) (proc)) () )

/* glTransformFeedbackAttribsNV */
typedef void (APIENTRYP GdkGLProc_glTransformFeedbackAttribsNV) (GLuint count, const GLint *attribs, GLenum bufferMode);
GdkGLProc    gdk_gl_get_glTransformFeedbackAttribsNV (void);
#define      gdk_gl_glTransformFeedbackAttribsNV(proc, count, attribs, bufferMode) \
  ( ((GdkGLProc_glTransformFeedbackAttribsNV) (proc)) (count, attribs, bufferMode) )

/* glBindBufferRangeNV */
typedef void (APIENTRYP GdkGLProc_glBindBufferRangeNV) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GdkGLProc    gdk_gl_get_glBindBufferRangeNV (void);
#define      gdk_gl_glBindBufferRangeNV(proc, target, index, buffer, offset, size) \
  ( ((GdkGLProc_glBindBufferRangeNV) (proc)) (target, index, buffer, offset, size) )

/* glBindBufferOffsetNV */
typedef void (APIENTRYP GdkGLProc_glBindBufferOffsetNV) (GLenum target, GLuint index, GLuint buffer, GLintptr offset);
GdkGLProc    gdk_gl_get_glBindBufferOffsetNV (void);
#define      gdk_gl_glBindBufferOffsetNV(proc, target, index, buffer, offset) \
  ( ((GdkGLProc_glBindBufferOffsetNV) (proc)) (target, index, buffer, offset) )

/* glBindBufferBaseNV */
typedef void (APIENTRYP GdkGLProc_glBindBufferBaseNV) (GLenum target, GLuint index, GLuint buffer);
GdkGLProc    gdk_gl_get_glBindBufferBaseNV (void);
#define      gdk_gl_glBindBufferBaseNV(proc, target, index, buffer) \
  ( ((GdkGLProc_glBindBufferBaseNV) (proc)) (target, index, buffer) )

/* glTransformFeedbackVaryingsNV */
typedef void (APIENTRYP GdkGLProc_glTransformFeedbackVaryingsNV) (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode);
GdkGLProc    gdk_gl_get_glTransformFeedbackVaryingsNV (void);
#define      gdk_gl_glTransformFeedbackVaryingsNV(proc, program, count, varyings, bufferMode) \
  ( ((GdkGLProc_glTransformFeedbackVaryingsNV) (proc)) (program, count, varyings, bufferMode) )

/* glActiveVaryingNV */
typedef void (APIENTRYP GdkGLProc_glActiveVaryingNV) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glActiveVaryingNV (void);
#define      gdk_gl_glActiveVaryingNV(proc, program, name) \
  ( ((GdkGLProc_glActiveVaryingNV) (proc)) (program, name) )

/* glGetVaryingLocationNV */
typedef GLint (APIENTRYP GdkGLProc_glGetVaryingLocationNV) (GLuint program, const GLchar *name);
GdkGLProc    gdk_gl_get_glGetVaryingLocationNV (void);
#define      gdk_gl_glGetVaryingLocationNV(proc, program, name) \
  ( ((GdkGLProc_glGetVaryingLocationNV) (proc)) (program, name) )

/* glGetActiveVaryingNV */
typedef void (APIENTRYP GdkGLProc_glGetActiveVaryingNV) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GdkGLProc    gdk_gl_get_glGetActiveVaryingNV (void);
#define      gdk_gl_glGetActiveVaryingNV(proc, program, index, bufSize, length, size, type, name) \
  ( ((GdkGLProc_glGetActiveVaryingNV) (proc)) (program, index, bufSize, length, size, type, name) )

/* glGetTransformFeedbackVaryingNV */
typedef void (APIENTRYP GdkGLProc_glGetTransformFeedbackVaryingNV) (GLuint program, GLuint index, GLint *location);
GdkGLProc    gdk_gl_get_glGetTransformFeedbackVaryingNV (void);
#define      gdk_gl_glGetTransformFeedbackVaryingNV(proc, program, index, location) \
  ( ((GdkGLProc_glGetTransformFeedbackVaryingNV) (proc)) (program, index, location) )

/* proc struct */

typedef struct _GdkGL_GL_NV_transform_feedback GdkGL_GL_NV_transform_feedback;

struct _GdkGL_GL_NV_transform_feedback
{
  GdkGLProc_glBeginTransformFeedbackNV glBeginTransformFeedbackNV;
  GdkGLProc_glEndTransformFeedbackNV glEndTransformFeedbackNV;
  GdkGLProc_glTransformFeedbackAttribsNV glTransformFeedbackAttribsNV;
  GdkGLProc_glBindBufferRangeNV glBindBufferRangeNV;
  GdkGLProc_glBindBufferOffsetNV glBindBufferOffsetNV;
  GdkGLProc_glBindBufferBaseNV glBindBufferBaseNV;
  GdkGLProc_glTransformFeedbackVaryingsNV glTransformFeedbackVaryingsNV;
  GdkGLProc_glActiveVaryingNV glActiveVaryingNV;
  GdkGLProc_glGetVaryingLocationNV glGetVaryingLocationNV;
  GdkGLProc_glGetActiveVaryingNV glGetActiveVaryingNV;
  GdkGLProc_glGetTransformFeedbackVaryingNV glGetTransformFeedbackVaryingNV;
};

GdkGL_GL_NV_transform_feedback *gdk_gl_get_GL_NV_transform_feedback (void);

/*
 * GL_EXT_bindable_uniform
 */

/* glUniformBufferEXT */
typedef void (APIENTRYP GdkGLProc_glUniformBufferEXT) (GLuint program, GLint location, GLuint buffer);
GdkGLProc    gdk_gl_get_glUniformBufferEXT (void);
#define      gdk_gl_glUniformBufferEXT(proc, program, location, buffer) \
  ( ((GdkGLProc_glUniformBufferEXT) (proc)) (program, location, buffer) )

/* glGetUniformBufferSizeEXT */
typedef GLint (APIENTRYP GdkGLProc_glGetUniformBufferSizeEXT) (GLuint program, GLint location);
GdkGLProc    gdk_gl_get_glGetUniformBufferSizeEXT (void);
#define      gdk_gl_glGetUniformBufferSizeEXT(proc, program, location) \
  ( ((GdkGLProc_glGetUniformBufferSizeEXT) (proc)) (program, location) )

/* glGetUniformOffsetEXT */
typedef GLintptr (APIENTRYP GdkGLProc_glGetUniformOffsetEXT) (GLuint program, GLint location);
GdkGLProc    gdk_gl_get_glGetUniformOffsetEXT (void);
#define      gdk_gl_glGetUniformOffsetEXT(proc, program, location) \
  ( ((GdkGLProc_glGetUniformOffsetEXT) (proc)) (program, location) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_bindable_uniform GdkGL_GL_EXT_bindable_uniform;

struct _GdkGL_GL_EXT_bindable_uniform
{
  GdkGLProc_glUniformBufferEXT glUniformBufferEXT;
  GdkGLProc_glGetUniformBufferSizeEXT glGetUniformBufferSizeEXT;
  GdkGLProc_glGetUniformOffsetEXT glGetUniformOffsetEXT;
};

GdkGL_GL_EXT_bindable_uniform *gdk_gl_get_GL_EXT_bindable_uniform (void);

/*
 * GL_EXT_texture_integer
 */

/* glTexParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glTexParameterIivEXT) (GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glTexParameterIivEXT (void);
#define      gdk_gl_glTexParameterIivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glTexParameterIivEXT) (proc)) (target, pname, params) )

/* glTexParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glTexParameterIuivEXT) (GLenum target, GLenum pname, const GLuint *params);
GdkGLProc    gdk_gl_get_glTexParameterIuivEXT (void);
#define      gdk_gl_glTexParameterIuivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glTexParameterIuivEXT) (proc)) (target, pname, params) )

/* glGetTexParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTexParameterIivEXT) (GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTexParameterIivEXT (void);
#define      gdk_gl_glGetTexParameterIivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetTexParameterIivEXT) (proc)) (target, pname, params) )

/* glGetTexParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTexParameterIuivEXT) (GLenum target, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetTexParameterIuivEXT (void);
#define      gdk_gl_glGetTexParameterIuivEXT(proc, target, pname, params) \
  ( ((GdkGLProc_glGetTexParameterIuivEXT) (proc)) (target, pname, params) )

/* glClearColorIiEXT */
typedef void (APIENTRYP GdkGLProc_glClearColorIiEXT) (GLint red, GLint green, GLint blue, GLint alpha);
GdkGLProc    gdk_gl_get_glClearColorIiEXT (void);
#define      gdk_gl_glClearColorIiEXT(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glClearColorIiEXT) (proc)) (red, green, blue, alpha) )

/* glClearColorIuiEXT */
typedef void (APIENTRYP GdkGLProc_glClearColorIuiEXT) (GLuint red, GLuint green, GLuint blue, GLuint alpha);
GdkGLProc    gdk_gl_get_glClearColorIuiEXT (void);
#define      gdk_gl_glClearColorIuiEXT(proc, red, green, blue, alpha) \
  ( ((GdkGLProc_glClearColorIuiEXT) (proc)) (red, green, blue, alpha) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_texture_integer GdkGL_GL_EXT_texture_integer;

struct _GdkGL_GL_EXT_texture_integer
{
  GdkGLProc_glTexParameterIivEXT glTexParameterIivEXT;
  GdkGLProc_glTexParameterIuivEXT glTexParameterIuivEXT;
  GdkGLProc_glGetTexParameterIivEXT glGetTexParameterIivEXT;
  GdkGLProc_glGetTexParameterIuivEXT glGetTexParameterIuivEXT;
  GdkGLProc_glClearColorIiEXT glClearColorIiEXT;
  GdkGLProc_glClearColorIuiEXT glClearColorIuiEXT;
};

GdkGL_GL_EXT_texture_integer *gdk_gl_get_GL_EXT_texture_integer (void);

/*
 * GL_GREMEDY_frame_terminator
 */

/* glFrameTerminatorGREMEDY */
typedef void (APIENTRYP GdkGLProc_glFrameTerminatorGREMEDY) (void);
GdkGLProc    gdk_gl_get_glFrameTerminatorGREMEDY (void);
#define      gdk_gl_glFrameTerminatorGREMEDY(proc) \
  ( ((GdkGLProc_glFrameTerminatorGREMEDY) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_GREMEDY_frame_terminator GdkGL_GL_GREMEDY_frame_terminator;

struct _GdkGL_GL_GREMEDY_frame_terminator
{
  GdkGLProc_glFrameTerminatorGREMEDY glFrameTerminatorGREMEDY;
};

GdkGL_GL_GREMEDY_frame_terminator *gdk_gl_get_GL_GREMEDY_frame_terminator (void);

/*
 * GL_NV_conditional_render
 */

/* glBeginConditionalRenderNV */
typedef void (APIENTRYP GdkGLProc_glBeginConditionalRenderNV) (GLuint id, GLenum mode);
GdkGLProc    gdk_gl_get_glBeginConditionalRenderNV (void);
#define      gdk_gl_glBeginConditionalRenderNV(proc, id, mode) \
  ( ((GdkGLProc_glBeginConditionalRenderNV) (proc)) (id, mode) )

/* glEndConditionalRenderNV */
typedef void (APIENTRYP GdkGLProc_glEndConditionalRenderNV) (void);
GdkGLProc    gdk_gl_get_glEndConditionalRenderNV (void);
#define      gdk_gl_glEndConditionalRenderNV(proc) \
  ( ((GdkGLProc_glEndConditionalRenderNV) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_NV_conditional_render GdkGL_GL_NV_conditional_render;

struct _GdkGL_GL_NV_conditional_render
{
  GdkGLProc_glBeginConditionalRenderNV glBeginConditionalRenderNV;
  GdkGLProc_glEndConditionalRenderNV glEndConditionalRenderNV;
};

GdkGL_GL_NV_conditional_render *gdk_gl_get_GL_NV_conditional_render (void);

/*
 * GL_NV_present_video
 */

/* glPresentFrameKeyedNV */
typedef void (APIENTRYP GdkGLProc_glPresentFrameKeyedNV) (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1);
GdkGLProc    gdk_gl_get_glPresentFrameKeyedNV (void);
#define      gdk_gl_glPresentFrameKeyedNV(proc, video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, key0, target1, fill1, key1) \
  ( ((GdkGLProc_glPresentFrameKeyedNV) (proc)) (video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, key0, target1, fill1, key1) )

/* glPresentFrameDualFillNV */
typedef void (APIENTRYP GdkGLProc_glPresentFrameDualFillNV) (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3);
GdkGLProc    gdk_gl_get_glPresentFrameDualFillNV (void);
#define      gdk_gl_glPresentFrameDualFillNV(proc, video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, target1, fill1, target2, fill2, target3, fill3) \
  ( ((GdkGLProc_glPresentFrameDualFillNV) (proc)) (video_slot, minPresentTime, beginPresentTimeId, presentDurationId, type, target0, fill0, target1, fill1, target2, fill2, target3, fill3) )

/* glGetVideoivNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoivNV) (GLuint video_slot, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVideoivNV (void);
#define      gdk_gl_glGetVideoivNV(proc, video_slot, pname, params) \
  ( ((GdkGLProc_glGetVideoivNV) (proc)) (video_slot, pname, params) )

/* glGetVideouivNV */
typedef void (APIENTRYP GdkGLProc_glGetVideouivNV) (GLuint video_slot, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetVideouivNV (void);
#define      gdk_gl_glGetVideouivNV(proc, video_slot, pname, params) \
  ( ((GdkGLProc_glGetVideouivNV) (proc)) (video_slot, pname, params) )

/* glGetVideoi64vNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoi64vNV) (GLuint video_slot, GLenum pname, GLint64EXT *params);
GdkGLProc    gdk_gl_get_glGetVideoi64vNV (void);
#define      gdk_gl_glGetVideoi64vNV(proc, video_slot, pname, params) \
  ( ((GdkGLProc_glGetVideoi64vNV) (proc)) (video_slot, pname, params) )

/* glGetVideoui64vNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoui64vNV) (GLuint video_slot, GLenum pname, GLuint64EXT *params);
GdkGLProc    gdk_gl_get_glGetVideoui64vNV (void);
#define      gdk_gl_glGetVideoui64vNV(proc, video_slot, pname, params) \
  ( ((GdkGLProc_glGetVideoui64vNV) (proc)) (video_slot, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_present_video GdkGL_GL_NV_present_video;

struct _GdkGL_GL_NV_present_video
{
  GdkGLProc_glPresentFrameKeyedNV glPresentFrameKeyedNV;
  GdkGLProc_glPresentFrameDualFillNV glPresentFrameDualFillNV;
  GdkGLProc_glGetVideoivNV glGetVideoivNV;
  GdkGLProc_glGetVideouivNV glGetVideouivNV;
  GdkGLProc_glGetVideoi64vNV glGetVideoi64vNV;
  GdkGLProc_glGetVideoui64vNV glGetVideoui64vNV;
};

GdkGL_GL_NV_present_video *gdk_gl_get_GL_NV_present_video (void);

/*
 * GL_EXT_transform_feedback
 */

/* glBeginTransformFeedbackEXT */
typedef void (APIENTRYP GdkGLProc_glBeginTransformFeedbackEXT) (GLenum primitiveMode);
GdkGLProc    gdk_gl_get_glBeginTransformFeedbackEXT (void);
#define      gdk_gl_glBeginTransformFeedbackEXT(proc, primitiveMode) \
  ( ((GdkGLProc_glBeginTransformFeedbackEXT) (proc)) (primitiveMode) )

/* glEndTransformFeedbackEXT */
typedef void (APIENTRYP GdkGLProc_glEndTransformFeedbackEXT) (void);
GdkGLProc    gdk_gl_get_glEndTransformFeedbackEXT (void);
#define      gdk_gl_glEndTransformFeedbackEXT(proc) \
  ( ((GdkGLProc_glEndTransformFeedbackEXT) (proc)) () )

/* glBindBufferRangeEXT */
typedef void (APIENTRYP GdkGLProc_glBindBufferRangeEXT) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
GdkGLProc    gdk_gl_get_glBindBufferRangeEXT (void);
#define      gdk_gl_glBindBufferRangeEXT(proc, target, index, buffer, offset, size) \
  ( ((GdkGLProc_glBindBufferRangeEXT) (proc)) (target, index, buffer, offset, size) )

/* glBindBufferOffsetEXT */
typedef void (APIENTRYP GdkGLProc_glBindBufferOffsetEXT) (GLenum target, GLuint index, GLuint buffer, GLintptr offset);
GdkGLProc    gdk_gl_get_glBindBufferOffsetEXT (void);
#define      gdk_gl_glBindBufferOffsetEXT(proc, target, index, buffer, offset) \
  ( ((GdkGLProc_glBindBufferOffsetEXT) (proc)) (target, index, buffer, offset) )

/* glBindBufferBaseEXT */
typedef void (APIENTRYP GdkGLProc_glBindBufferBaseEXT) (GLenum target, GLuint index, GLuint buffer);
GdkGLProc    gdk_gl_get_glBindBufferBaseEXT (void);
#define      gdk_gl_glBindBufferBaseEXT(proc, target, index, buffer) \
  ( ((GdkGLProc_glBindBufferBaseEXT) (proc)) (target, index, buffer) )

/* glTransformFeedbackVaryingsEXT */
typedef void (APIENTRYP GdkGLProc_glTransformFeedbackVaryingsEXT) (GLuint program, GLsizei count, const GLchar* *varyings, GLenum bufferMode);
GdkGLProc    gdk_gl_get_glTransformFeedbackVaryingsEXT (void);
#define      gdk_gl_glTransformFeedbackVaryingsEXT(proc, program, count, varyings, bufferMode) \
  ( ((GdkGLProc_glTransformFeedbackVaryingsEXT) (proc)) (program, count, varyings, bufferMode) )

/* glGetTransformFeedbackVaryingEXT */
typedef void (APIENTRYP GdkGLProc_glGetTransformFeedbackVaryingEXT) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
GdkGLProc    gdk_gl_get_glGetTransformFeedbackVaryingEXT (void);
#define      gdk_gl_glGetTransformFeedbackVaryingEXT(proc, program, index, bufSize, length, size, type, name) \
  ( ((GdkGLProc_glGetTransformFeedbackVaryingEXT) (proc)) (program, index, bufSize, length, size, type, name) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_transform_feedback GdkGL_GL_EXT_transform_feedback;

struct _GdkGL_GL_EXT_transform_feedback
{
  GdkGLProc_glBeginTransformFeedbackEXT glBeginTransformFeedbackEXT;
  GdkGLProc_glEndTransformFeedbackEXT glEndTransformFeedbackEXT;
  GdkGLProc_glBindBufferRangeEXT glBindBufferRangeEXT;
  GdkGLProc_glBindBufferOffsetEXT glBindBufferOffsetEXT;
  GdkGLProc_glBindBufferBaseEXT glBindBufferBaseEXT;
  GdkGLProc_glTransformFeedbackVaryingsEXT glTransformFeedbackVaryingsEXT;
  GdkGLProc_glGetTransformFeedbackVaryingEXT glGetTransformFeedbackVaryingEXT;
};

GdkGL_GL_EXT_transform_feedback *gdk_gl_get_GL_EXT_transform_feedback (void);

/*
 * GL_EXT_direct_state_access
 */

/* glClientAttribDefaultEXT */
typedef void (APIENTRYP GdkGLProc_glClientAttribDefaultEXT) (GLbitfield mask);
GdkGLProc    gdk_gl_get_glClientAttribDefaultEXT (void);
#define      gdk_gl_glClientAttribDefaultEXT(proc, mask) \
  ( ((GdkGLProc_glClientAttribDefaultEXT) (proc)) (mask) )

/* glPushClientAttribDefaultEXT */
typedef void (APIENTRYP GdkGLProc_glPushClientAttribDefaultEXT) (GLbitfield mask);
GdkGLProc    gdk_gl_get_glPushClientAttribDefaultEXT (void);
#define      gdk_gl_glPushClientAttribDefaultEXT(proc, mask) \
  ( ((GdkGLProc_glPushClientAttribDefaultEXT) (proc)) (mask) )

/* glMatrixLoadfEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixLoadfEXT) (GLenum mode, const GLfloat *m);
GdkGLProc    gdk_gl_get_glMatrixLoadfEXT (void);
#define      gdk_gl_glMatrixLoadfEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixLoadfEXT) (proc)) (mode, m) )

/* glMatrixLoaddEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixLoaddEXT) (GLenum mode, const GLdouble *m);
GdkGLProc    gdk_gl_get_glMatrixLoaddEXT (void);
#define      gdk_gl_glMatrixLoaddEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixLoaddEXT) (proc)) (mode, m) )

/* glMatrixMultfEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixMultfEXT) (GLenum mode, const GLfloat *m);
GdkGLProc    gdk_gl_get_glMatrixMultfEXT (void);
#define      gdk_gl_glMatrixMultfEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixMultfEXT) (proc)) (mode, m) )

/* glMatrixMultdEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixMultdEXT) (GLenum mode, const GLdouble *m);
GdkGLProc    gdk_gl_get_glMatrixMultdEXT (void);
#define      gdk_gl_glMatrixMultdEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixMultdEXT) (proc)) (mode, m) )

/* glMatrixLoadIdentityEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixLoadIdentityEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glMatrixLoadIdentityEXT (void);
#define      gdk_gl_glMatrixLoadIdentityEXT(proc, mode) \
  ( ((GdkGLProc_glMatrixLoadIdentityEXT) (proc)) (mode) )

/* glMatrixRotatefEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixRotatefEXT) (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glMatrixRotatefEXT (void);
#define      gdk_gl_glMatrixRotatefEXT(proc, mode, angle, x, y, z) \
  ( ((GdkGLProc_glMatrixRotatefEXT) (proc)) (mode, angle, x, y, z) )

/* glMatrixRotatedEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixRotatedEXT) (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glMatrixRotatedEXT (void);
#define      gdk_gl_glMatrixRotatedEXT(proc, mode, angle, x, y, z) \
  ( ((GdkGLProc_glMatrixRotatedEXT) (proc)) (mode, angle, x, y, z) )

/* glMatrixScalefEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixScalefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glMatrixScalefEXT (void);
#define      gdk_gl_glMatrixScalefEXT(proc, mode, x, y, z) \
  ( ((GdkGLProc_glMatrixScalefEXT) (proc)) (mode, x, y, z) )

/* glMatrixScaledEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixScaledEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glMatrixScaledEXT (void);
#define      gdk_gl_glMatrixScaledEXT(proc, mode, x, y, z) \
  ( ((GdkGLProc_glMatrixScaledEXT) (proc)) (mode, x, y, z) )

/* glMatrixTranslatefEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixTranslatefEXT) (GLenum mode, GLfloat x, GLfloat y, GLfloat z);
GdkGLProc    gdk_gl_get_glMatrixTranslatefEXT (void);
#define      gdk_gl_glMatrixTranslatefEXT(proc, mode, x, y, z) \
  ( ((GdkGLProc_glMatrixTranslatefEXT) (proc)) (mode, x, y, z) )

/* glMatrixTranslatedEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixTranslatedEXT) (GLenum mode, GLdouble x, GLdouble y, GLdouble z);
GdkGLProc    gdk_gl_get_glMatrixTranslatedEXT (void);
#define      gdk_gl_glMatrixTranslatedEXT(proc, mode, x, y, z) \
  ( ((GdkGLProc_glMatrixTranslatedEXT) (proc)) (mode, x, y, z) )

/* glMatrixFrustumEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixFrustumEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GdkGLProc    gdk_gl_get_glMatrixFrustumEXT (void);
#define      gdk_gl_glMatrixFrustumEXT(proc, mode, left, right, bottom, top, zNear, zFar) \
  ( ((GdkGLProc_glMatrixFrustumEXT) (proc)) (mode, left, right, bottom, top, zNear, zFar) )

/* glMatrixOrthoEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixOrthoEXT) (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
GdkGLProc    gdk_gl_get_glMatrixOrthoEXT (void);
#define      gdk_gl_glMatrixOrthoEXT(proc, mode, left, right, bottom, top, zNear, zFar) \
  ( ((GdkGLProc_glMatrixOrthoEXT) (proc)) (mode, left, right, bottom, top, zNear, zFar) )

/* glMatrixPopEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixPopEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glMatrixPopEXT (void);
#define      gdk_gl_glMatrixPopEXT(proc, mode) \
  ( ((GdkGLProc_glMatrixPopEXT) (proc)) (mode) )

/* glMatrixPushEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixPushEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glMatrixPushEXT (void);
#define      gdk_gl_glMatrixPushEXT(proc, mode) \
  ( ((GdkGLProc_glMatrixPushEXT) (proc)) (mode) )

/* glMatrixLoadTransposefEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixLoadTransposefEXT) (GLenum mode, const GLfloat *m);
GdkGLProc    gdk_gl_get_glMatrixLoadTransposefEXT (void);
#define      gdk_gl_glMatrixLoadTransposefEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixLoadTransposefEXT) (proc)) (mode, m) )

/* glMatrixLoadTransposedEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixLoadTransposedEXT) (GLenum mode, const GLdouble *m);
GdkGLProc    gdk_gl_get_glMatrixLoadTransposedEXT (void);
#define      gdk_gl_glMatrixLoadTransposedEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixLoadTransposedEXT) (proc)) (mode, m) )

/* glMatrixMultTransposefEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixMultTransposefEXT) (GLenum mode, const GLfloat *m);
GdkGLProc    gdk_gl_get_glMatrixMultTransposefEXT (void);
#define      gdk_gl_glMatrixMultTransposefEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixMultTransposefEXT) (proc)) (mode, m) )

/* glMatrixMultTransposedEXT */
typedef void (APIENTRYP GdkGLProc_glMatrixMultTransposedEXT) (GLenum mode, const GLdouble *m);
GdkGLProc    gdk_gl_get_glMatrixMultTransposedEXT (void);
#define      gdk_gl_glMatrixMultTransposedEXT(proc, mode, m) \
  ( ((GdkGLProc_glMatrixMultTransposedEXT) (proc)) (mode, m) )

/* glTextureParameterfEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameterfEXT) (GLuint texture, GLenum target, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glTextureParameterfEXT (void);
#define      gdk_gl_glTextureParameterfEXT(proc, texture, target, pname, param) \
  ( ((GdkGLProc_glTextureParameterfEXT) (proc)) (texture, target, pname, param) )

/* glTextureParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameterfvEXT) (GLuint texture, GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glTextureParameterfvEXT (void);
#define      gdk_gl_glTextureParameterfvEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glTextureParameterfvEXT) (proc)) (texture, target, pname, params) )

/* glTextureParameteriEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameteriEXT) (GLuint texture, GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glTextureParameteriEXT (void);
#define      gdk_gl_glTextureParameteriEXT(proc, texture, target, pname, param) \
  ( ((GdkGLProc_glTextureParameteriEXT) (proc)) (texture, target, pname, param) )

/* glTextureParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameterivEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glTextureParameterivEXT (void);
#define      gdk_gl_glTextureParameterivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glTextureParameterivEXT) (proc)) (texture, target, pname, params) )

/* glTextureImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureImage1DEXT (void);
#define      gdk_gl_glTextureImage1DEXT(proc, texture, target, level, internalformat, width, border, format, type, pixels) \
  ( ((GdkGLProc_glTextureImage1DEXT) (proc)) (texture, target, level, internalformat, width, border, format, type, pixels) )

/* glTextureImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureImage2DEXT (void);
#define      gdk_gl_glTextureImage2DEXT(proc, texture, target, level, internalformat, width, height, border, format, type, pixels) \
  ( ((GdkGLProc_glTextureImage2DEXT) (proc)) (texture, target, level, internalformat, width, height, border, format, type, pixels) )

/* glTextureSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureSubImage1DEXT (void);
#define      gdk_gl_glTextureSubImage1DEXT(proc, texture, target, level, xoffset, width, format, type, pixels) \
  ( ((GdkGLProc_glTextureSubImage1DEXT) (proc)) (texture, target, level, xoffset, width, format, type, pixels) )

/* glTextureSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureSubImage2DEXT (void);
#define      gdk_gl_glTextureSubImage2DEXT(proc, texture, target, level, xoffset, yoffset, width, height, format, type, pixels) \
  ( ((GdkGLProc_glTextureSubImage2DEXT) (proc)) (texture, target, level, xoffset, yoffset, width, height, format, type, pixels) )

/* glCopyTextureImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GdkGLProc    gdk_gl_get_glCopyTextureImage1DEXT (void);
#define      gdk_gl_glCopyTextureImage1DEXT(proc, texture, target, level, internalformat, x, y, width, border) \
  ( ((GdkGLProc_glCopyTextureImage1DEXT) (proc)) (texture, target, level, internalformat, x, y, width, border) )

/* glCopyTextureImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GdkGLProc    gdk_gl_get_glCopyTextureImage2DEXT (void);
#define      gdk_gl_glCopyTextureImage2DEXT(proc, texture, target, level, internalformat, x, y, width, height, border) \
  ( ((GdkGLProc_glCopyTextureImage2DEXT) (proc)) (texture, target, level, internalformat, x, y, width, height, border) )

/* glCopyTextureSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyTextureSubImage1DEXT (void);
#define      gdk_gl_glCopyTextureSubImage1DEXT(proc, texture, target, level, xoffset, x, y, width) \
  ( ((GdkGLProc_glCopyTextureSubImage1DEXT) (proc)) (texture, target, level, xoffset, x, y, width) )

/* glCopyTextureSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyTextureSubImage2DEXT (void);
#define      gdk_gl_glCopyTextureSubImage2DEXT(proc, texture, target, level, xoffset, yoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyTextureSubImage2DEXT) (proc)) (texture, target, level, xoffset, yoffset, x, y, width, height) )

/* glGetTextureImageEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureImageEXT) (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
GdkGLProc    gdk_gl_get_glGetTextureImageEXT (void);
#define      gdk_gl_glGetTextureImageEXT(proc, texture, target, level, format, type, pixels) \
  ( ((GdkGLProc_glGetTextureImageEXT) (proc)) (texture, target, level, format, type, pixels) )

/* glGetTextureParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureParameterfvEXT) (GLuint texture, GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetTextureParameterfvEXT (void);
#define      gdk_gl_glGetTextureParameterfvEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glGetTextureParameterfvEXT) (proc)) (texture, target, pname, params) )

/* glGetTextureParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureParameterivEXT) (GLuint texture, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTextureParameterivEXT (void);
#define      gdk_gl_glGetTextureParameterivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glGetTextureParameterivEXT) (proc)) (texture, target, pname, params) )

/* glGetTextureLevelParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureLevelParameterfvEXT) (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetTextureLevelParameterfvEXT (void);
#define      gdk_gl_glGetTextureLevelParameterfvEXT(proc, texture, target, level, pname, params) \
  ( ((GdkGLProc_glGetTextureLevelParameterfvEXT) (proc)) (texture, target, level, pname, params) )

/* glGetTextureLevelParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureLevelParameterivEXT) (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTextureLevelParameterivEXT (void);
#define      gdk_gl_glGetTextureLevelParameterivEXT(proc, texture, target, level, pname, params) \
  ( ((GdkGLProc_glGetTextureLevelParameterivEXT) (proc)) (texture, target, level, pname, params) )

/* glTextureImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureImage3DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureImage3DEXT (void);
#define      gdk_gl_glTextureImage3DEXT(proc, texture, target, level, internalformat, width, height, depth, border, format, type, pixels) \
  ( ((GdkGLProc_glTextureImage3DEXT) (proc)) (texture, target, level, internalformat, width, height, depth, border, format, type, pixels) )

/* glTextureSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glTextureSubImage3DEXT (void);
#define      gdk_gl_glTextureSubImage3DEXT(proc, texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) \
  ( ((GdkGLProc_glTextureSubImage3DEXT) (proc)) (texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) )

/* glCopyTextureSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyTextureSubImage3DEXT (void);
#define      gdk_gl_glCopyTextureSubImage3DEXT(proc, texture, target, level, xoffset, yoffset, zoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyTextureSubImage3DEXT) (proc)) (texture, target, level, xoffset, yoffset, zoffset, x, y, width, height) )

/* glMultiTexParameterfEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameterfEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glMultiTexParameterfEXT (void);
#define      gdk_gl_glMultiTexParameterfEXT(proc, texunit, target, pname, param) \
  ( ((GdkGLProc_glMultiTexParameterfEXT) (proc)) (texunit, target, pname, param) )

/* glMultiTexParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameterfvEXT) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glMultiTexParameterfvEXT (void);
#define      gdk_gl_glMultiTexParameterfvEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexParameterfvEXT) (proc)) (texunit, target, pname, params) )

/* glMultiTexParameteriEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameteriEXT) (GLenum texunit, GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glMultiTexParameteriEXT (void);
#define      gdk_gl_glMultiTexParameteriEXT(proc, texunit, target, pname, param) \
  ( ((GdkGLProc_glMultiTexParameteriEXT) (proc)) (texunit, target, pname, param) )

/* glMultiTexParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameterivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glMultiTexParameterivEXT (void);
#define      gdk_gl_glMultiTexParameterivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexParameterivEXT) (proc)) (texunit, target, pname, params) )

/* glMultiTexImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexImage1DEXT (void);
#define      gdk_gl_glMultiTexImage1DEXT(proc, texunit, target, level, internalformat, width, border, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexImage1DEXT) (proc)) (texunit, target, level, internalformat, width, border, format, type, pixels) )

/* glMultiTexImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexImage2DEXT (void);
#define      gdk_gl_glMultiTexImage2DEXT(proc, texunit, target, level, internalformat, width, height, border, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexImage2DEXT) (proc)) (texunit, target, level, internalformat, width, height, border, format, type, pixels) )

/* glMultiTexSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexSubImage1DEXT (void);
#define      gdk_gl_glMultiTexSubImage1DEXT(proc, texunit, target, level, xoffset, width, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexSubImage1DEXT) (proc)) (texunit, target, level, xoffset, width, format, type, pixels) )

/* glMultiTexSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexSubImage2DEXT (void);
#define      gdk_gl_glMultiTexSubImage2DEXT(proc, texunit, target, level, xoffset, yoffset, width, height, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexSubImage2DEXT) (proc)) (texunit, target, level, xoffset, yoffset, width, height, format, type, pixels) )

/* glCopyMultiTexImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
GdkGLProc    gdk_gl_get_glCopyMultiTexImage1DEXT (void);
#define      gdk_gl_glCopyMultiTexImage1DEXT(proc, texunit, target, level, internalformat, x, y, width, border) \
  ( ((GdkGLProc_glCopyMultiTexImage1DEXT) (proc)) (texunit, target, level, internalformat, x, y, width, border) )

/* glCopyMultiTexImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
GdkGLProc    gdk_gl_get_glCopyMultiTexImage2DEXT (void);
#define      gdk_gl_glCopyMultiTexImage2DEXT(proc, texunit, target, level, internalformat, x, y, width, height, border) \
  ( ((GdkGLProc_glCopyMultiTexImage2DEXT) (proc)) (texunit, target, level, internalformat, x, y, width, height, border) )

/* glCopyMultiTexSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
GdkGLProc    gdk_gl_get_glCopyMultiTexSubImage1DEXT (void);
#define      gdk_gl_glCopyMultiTexSubImage1DEXT(proc, texunit, target, level, xoffset, x, y, width) \
  ( ((GdkGLProc_glCopyMultiTexSubImage1DEXT) (proc)) (texunit, target, level, xoffset, x, y, width) )

/* glCopyMultiTexSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyMultiTexSubImage2DEXT (void);
#define      gdk_gl_glCopyMultiTexSubImage2DEXT(proc, texunit, target, level, xoffset, yoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyMultiTexSubImage2DEXT) (proc)) (texunit, target, level, xoffset, yoffset, x, y, width, height) )

/* glGetMultiTexImageEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexImageEXT) (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
GdkGLProc    gdk_gl_get_glGetMultiTexImageEXT (void);
#define      gdk_gl_glGetMultiTexImageEXT(proc, texunit, target, level, format, type, pixels) \
  ( ((GdkGLProc_glGetMultiTexImageEXT) (proc)) (texunit, target, level, format, type, pixels) )

/* glGetMultiTexParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexParameterfvEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMultiTexParameterfvEXT (void);
#define      gdk_gl_glGetMultiTexParameterfvEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexParameterfvEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexParameterivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexParameterivEXT (void);
#define      gdk_gl_glGetMultiTexParameterivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexParameterivEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexLevelParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexLevelParameterfvEXT) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMultiTexLevelParameterfvEXT (void);
#define      gdk_gl_glGetMultiTexLevelParameterfvEXT(proc, texunit, target, level, pname, params) \
  ( ((GdkGLProc_glGetMultiTexLevelParameterfvEXT) (proc)) (texunit, target, level, pname, params) )

/* glGetMultiTexLevelParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexLevelParameterivEXT) (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexLevelParameterivEXT (void);
#define      gdk_gl_glGetMultiTexLevelParameterivEXT(proc, texunit, target, level, pname, params) \
  ( ((GdkGLProc_glGetMultiTexLevelParameterivEXT) (proc)) (texunit, target, level, pname, params) )

/* glMultiTexImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexImage3DEXT (void);
#define      gdk_gl_glMultiTexImage3DEXT(proc, texunit, target, level, internalformat, width, height, depth, border, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexImage3DEXT) (proc)) (texunit, target, level, internalformat, width, height, depth, border, format, type, pixels) )

/* glMultiTexSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
GdkGLProc    gdk_gl_get_glMultiTexSubImage3DEXT (void);
#define      gdk_gl_glMultiTexSubImage3DEXT(proc, texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) \
  ( ((GdkGLProc_glMultiTexSubImage3DEXT) (proc)) (texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels) )

/* glCopyMultiTexSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCopyMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glCopyMultiTexSubImage3DEXT (void);
#define      gdk_gl_glCopyMultiTexSubImage3DEXT(proc, texunit, target, level, xoffset, yoffset, zoffset, x, y, width, height) \
  ( ((GdkGLProc_glCopyMultiTexSubImage3DEXT) (proc)) (texunit, target, level, xoffset, yoffset, zoffset, x, y, width, height) )

/* glBindMultiTextureEXT */
typedef void (APIENTRYP GdkGLProc_glBindMultiTextureEXT) (GLenum texunit, GLenum target, GLuint texture);
GdkGLProc    gdk_gl_get_glBindMultiTextureEXT (void);
#define      gdk_gl_glBindMultiTextureEXT(proc, texunit, target, texture) \
  ( ((GdkGLProc_glBindMultiTextureEXT) (proc)) (texunit, target, texture) )

/* glEnableClientStateIndexedEXT */
typedef void (APIENTRYP GdkGLProc_glEnableClientStateIndexedEXT) (GLenum array, GLuint index);
GdkGLProc    gdk_gl_get_glEnableClientStateIndexedEXT (void);
#define      gdk_gl_glEnableClientStateIndexedEXT(proc, array, index) \
  ( ((GdkGLProc_glEnableClientStateIndexedEXT) (proc)) (array, index) )

/* glDisableClientStateIndexedEXT */
typedef void (APIENTRYP GdkGLProc_glDisableClientStateIndexedEXT) (GLenum array, GLuint index);
GdkGLProc    gdk_gl_get_glDisableClientStateIndexedEXT (void);
#define      gdk_gl_glDisableClientStateIndexedEXT(proc, array, index) \
  ( ((GdkGLProc_glDisableClientStateIndexedEXT) (proc)) (array, index) )

/* glMultiTexCoordPointerEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexCoordPointerEXT) (GLenum texunit, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glMultiTexCoordPointerEXT (void);
#define      gdk_gl_glMultiTexCoordPointerEXT(proc, texunit, size, type, stride, pointer) \
  ( ((GdkGLProc_glMultiTexCoordPointerEXT) (proc)) (texunit, size, type, stride, pointer) )

/* glMultiTexEnvfEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexEnvfEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glMultiTexEnvfEXT (void);
#define      gdk_gl_glMultiTexEnvfEXT(proc, texunit, target, pname, param) \
  ( ((GdkGLProc_glMultiTexEnvfEXT) (proc)) (texunit, target, pname, param) )

/* glMultiTexEnvfvEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexEnvfvEXT) (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glMultiTexEnvfvEXT (void);
#define      gdk_gl_glMultiTexEnvfvEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexEnvfvEXT) (proc)) (texunit, target, pname, params) )

/* glMultiTexEnviEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexEnviEXT) (GLenum texunit, GLenum target, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glMultiTexEnviEXT (void);
#define      gdk_gl_glMultiTexEnviEXT(proc, texunit, target, pname, param) \
  ( ((GdkGLProc_glMultiTexEnviEXT) (proc)) (texunit, target, pname, param) )

/* glMultiTexEnvivEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexEnvivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glMultiTexEnvivEXT (void);
#define      gdk_gl_glMultiTexEnvivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexEnvivEXT) (proc)) (texunit, target, pname, params) )

/* glMultiTexGendEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGendEXT) (GLenum texunit, GLenum coord, GLenum pname, GLdouble param);
GdkGLProc    gdk_gl_get_glMultiTexGendEXT (void);
#define      gdk_gl_glMultiTexGendEXT(proc, texunit, coord, pname, param) \
  ( ((GdkGLProc_glMultiTexGendEXT) (proc)) (texunit, coord, pname, param) )

/* glMultiTexGendvEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGendvEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params);
GdkGLProc    gdk_gl_get_glMultiTexGendvEXT (void);
#define      gdk_gl_glMultiTexGendvEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glMultiTexGendvEXT) (proc)) (texunit, coord, pname, params) )

/* glMultiTexGenfEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGenfEXT) (GLenum texunit, GLenum coord, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glMultiTexGenfEXT (void);
#define      gdk_gl_glMultiTexGenfEXT(proc, texunit, coord, pname, param) \
  ( ((GdkGLProc_glMultiTexGenfEXT) (proc)) (texunit, coord, pname, param) )

/* glMultiTexGenfvEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGenfvEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glMultiTexGenfvEXT (void);
#define      gdk_gl_glMultiTexGenfvEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glMultiTexGenfvEXT) (proc)) (texunit, coord, pname, params) )

/* glMultiTexGeniEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGeniEXT) (GLenum texunit, GLenum coord, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glMultiTexGeniEXT (void);
#define      gdk_gl_glMultiTexGeniEXT(proc, texunit, coord, pname, param) \
  ( ((GdkGLProc_glMultiTexGeniEXT) (proc)) (texunit, coord, pname, param) )

/* glMultiTexGenivEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexGenivEXT) (GLenum texunit, GLenum coord, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glMultiTexGenivEXT (void);
#define      gdk_gl_glMultiTexGenivEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glMultiTexGenivEXT) (proc)) (texunit, coord, pname, params) )

/* glGetMultiTexEnvfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexEnvfvEXT) (GLenum texunit, GLenum target, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMultiTexEnvfvEXT (void);
#define      gdk_gl_glGetMultiTexEnvfvEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexEnvfvEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexEnvivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexEnvivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexEnvivEXT (void);
#define      gdk_gl_glGetMultiTexEnvivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexEnvivEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexGendvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexGendvEXT) (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetMultiTexGendvEXT (void);
#define      gdk_gl_glGetMultiTexGendvEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glGetMultiTexGendvEXT) (proc)) (texunit, coord, pname, params) )

/* glGetMultiTexGenfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexGenfvEXT) (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetMultiTexGenfvEXT (void);
#define      gdk_gl_glGetMultiTexGenfvEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glGetMultiTexGenfvEXT) (proc)) (texunit, coord, pname, params) )

/* glGetMultiTexGenivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexGenivEXT) (GLenum texunit, GLenum coord, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexGenivEXT (void);
#define      gdk_gl_glGetMultiTexGenivEXT(proc, texunit, coord, pname, params) \
  ( ((GdkGLProc_glGetMultiTexGenivEXT) (proc)) (texunit, coord, pname, params) )

/* glGetFloatIndexedvEXT */
typedef void (APIENTRYP GdkGLProc_glGetFloatIndexedvEXT) (GLenum target, GLuint index, GLfloat *data);
GdkGLProc    gdk_gl_get_glGetFloatIndexedvEXT (void);
#define      gdk_gl_glGetFloatIndexedvEXT(proc, target, index, data) \
  ( ((GdkGLProc_glGetFloatIndexedvEXT) (proc)) (target, index, data) )

/* glGetDoubleIndexedvEXT */
typedef void (APIENTRYP GdkGLProc_glGetDoubleIndexedvEXT) (GLenum target, GLuint index, GLdouble *data);
GdkGLProc    gdk_gl_get_glGetDoubleIndexedvEXT (void);
#define      gdk_gl_glGetDoubleIndexedvEXT(proc, target, index, data) \
  ( ((GdkGLProc_glGetDoubleIndexedvEXT) (proc)) (target, index, data) )

/* glGetPointerIndexedvEXT */
typedef void (APIENTRYP GdkGLProc_glGetPointerIndexedvEXT) (GLenum target, GLuint index, GLvoid* *data);
GdkGLProc    gdk_gl_get_glGetPointerIndexedvEXT (void);
#define      gdk_gl_glGetPointerIndexedvEXT(proc, target, index, data) \
  ( ((GdkGLProc_glGetPointerIndexedvEXT) (proc)) (target, index, data) )

/* glCompressedTextureImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureImage3DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureImage3DEXT (void);
#define      gdk_gl_glCompressedTextureImage3DEXT(proc, texture, target, level, internalformat, width, height, depth, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureImage3DEXT) (proc)) (texture, target, level, internalformat, width, height, depth, border, imageSize, bits) )

/* glCompressedTextureImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureImage2DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureImage2DEXT (void);
#define      gdk_gl_glCompressedTextureImage2DEXT(proc, texture, target, level, internalformat, width, height, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureImage2DEXT) (proc)) (texture, target, level, internalformat, width, height, border, imageSize, bits) )

/* glCompressedTextureImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureImage1DEXT) (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureImage1DEXT (void);
#define      gdk_gl_glCompressedTextureImage1DEXT(proc, texture, target, level, internalformat, width, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureImage1DEXT) (proc)) (texture, target, level, internalformat, width, border, imageSize, bits) )

/* glCompressedTextureSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureSubImage3DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureSubImage3DEXT (void);
#define      gdk_gl_glCompressedTextureSubImage3DEXT(proc, texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureSubImage3DEXT) (proc)) (texture, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits) )

/* glCompressedTextureSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureSubImage2DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureSubImage2DEXT (void);
#define      gdk_gl_glCompressedTextureSubImage2DEXT(proc, texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureSubImage2DEXT) (proc)) (texture, target, level, xoffset, yoffset, width, height, format, imageSize, bits) )

/* glCompressedTextureSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedTextureSubImage1DEXT) (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedTextureSubImage1DEXT (void);
#define      gdk_gl_glCompressedTextureSubImage1DEXT(proc, texture, target, level, xoffset, width, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedTextureSubImage1DEXT) (proc)) (texture, target, level, xoffset, width, format, imageSize, bits) )

/* glGetCompressedTextureImageEXT */
typedef void (APIENTRYP GdkGLProc_glGetCompressedTextureImageEXT) (GLuint texture, GLenum target, GLint lod, GLvoid *img);
GdkGLProc    gdk_gl_get_glGetCompressedTextureImageEXT (void);
#define      gdk_gl_glGetCompressedTextureImageEXT(proc, texture, target, lod, img) \
  ( ((GdkGLProc_glGetCompressedTextureImageEXT) (proc)) (texture, target, lod, img) )

/* glCompressedMultiTexImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexImage3DEXT (void);
#define      gdk_gl_glCompressedMultiTexImage3DEXT(proc, texunit, target, level, internalformat, width, height, depth, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexImage3DEXT) (proc)) (texunit, target, level, internalformat, width, height, depth, border, imageSize, bits) )

/* glCompressedMultiTexImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexImage2DEXT (void);
#define      gdk_gl_glCompressedMultiTexImage2DEXT(proc, texunit, target, level, internalformat, width, height, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexImage2DEXT) (proc)) (texunit, target, level, internalformat, width, height, border, imageSize, bits) )

/* glCompressedMultiTexImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexImage1DEXT (void);
#define      gdk_gl_glCompressedMultiTexImage1DEXT(proc, texunit, target, level, internalformat, width, border, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexImage1DEXT) (proc)) (texunit, target, level, internalformat, width, border, imageSize, bits) )

/* glCompressedMultiTexSubImage3DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexSubImage3DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexSubImage3DEXT (void);
#define      gdk_gl_glCompressedMultiTexSubImage3DEXT(proc, texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexSubImage3DEXT) (proc)) (texunit, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, bits) )

/* glCompressedMultiTexSubImage2DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexSubImage2DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexSubImage2DEXT (void);
#define      gdk_gl_glCompressedMultiTexSubImage2DEXT(proc, texunit, target, level, xoffset, yoffset, width, height, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexSubImage2DEXT) (proc)) (texunit, target, level, xoffset, yoffset, width, height, format, imageSize, bits) )

/* glCompressedMultiTexSubImage1DEXT */
typedef void (APIENTRYP GdkGLProc_glCompressedMultiTexSubImage1DEXT) (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *bits);
GdkGLProc    gdk_gl_get_glCompressedMultiTexSubImage1DEXT (void);
#define      gdk_gl_glCompressedMultiTexSubImage1DEXT(proc, texunit, target, level, xoffset, width, format, imageSize, bits) \
  ( ((GdkGLProc_glCompressedMultiTexSubImage1DEXT) (proc)) (texunit, target, level, xoffset, width, format, imageSize, bits) )

/* glGetCompressedMultiTexImageEXT */
typedef void (APIENTRYP GdkGLProc_glGetCompressedMultiTexImageEXT) (GLenum texunit, GLenum target, GLint lod, GLvoid *img);
GdkGLProc    gdk_gl_get_glGetCompressedMultiTexImageEXT (void);
#define      gdk_gl_glGetCompressedMultiTexImageEXT(proc, texunit, target, lod, img) \
  ( ((GdkGLProc_glGetCompressedMultiTexImageEXT) (proc)) (texunit, target, lod, img) )

/* glNamedProgramStringEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramStringEXT) (GLuint program, GLenum target, GLenum format, GLsizei len, const GLvoid *string);
GdkGLProc    gdk_gl_get_glNamedProgramStringEXT (void);
#define      gdk_gl_glNamedProgramStringEXT(proc, program, target, format, len, string) \
  ( ((GdkGLProc_glNamedProgramStringEXT) (proc)) (program, target, format, len, string) )

/* glNamedProgramLocalParameter4dEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameter4dEXT) (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameter4dEXT (void);
#define      gdk_gl_glNamedProgramLocalParameter4dEXT(proc, program, target, index, x, y, z, w) \
  ( ((GdkGLProc_glNamedProgramLocalParameter4dEXT) (proc)) (program, target, index, x, y, z, w) )

/* glNamedProgramLocalParameter4dvEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameter4dvEXT) (GLuint program, GLenum target, GLuint index, const GLdouble *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameter4dvEXT (void);
#define      gdk_gl_glNamedProgramLocalParameter4dvEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glNamedProgramLocalParameter4dvEXT) (proc)) (program, target, index, params) )

/* glNamedProgramLocalParameter4fEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameter4fEXT) (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameter4fEXT (void);
#define      gdk_gl_glNamedProgramLocalParameter4fEXT(proc, program, target, index, x, y, z, w) \
  ( ((GdkGLProc_glNamedProgramLocalParameter4fEXT) (proc)) (program, target, index, x, y, z, w) )

/* glNamedProgramLocalParameter4fvEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameter4fvEXT) (GLuint program, GLenum target, GLuint index, const GLfloat *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameter4fvEXT (void);
#define      gdk_gl_glNamedProgramLocalParameter4fvEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glNamedProgramLocalParameter4fvEXT) (proc)) (program, target, index, params) )

/* glGetNamedProgramLocalParameterdvEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramLocalParameterdvEXT) (GLuint program, GLenum target, GLuint index, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetNamedProgramLocalParameterdvEXT (void);
#define      gdk_gl_glGetNamedProgramLocalParameterdvEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glGetNamedProgramLocalParameterdvEXT) (proc)) (program, target, index, params) )

/* glGetNamedProgramLocalParameterfvEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramLocalParameterfvEXT) (GLuint program, GLenum target, GLuint index, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetNamedProgramLocalParameterfvEXT (void);
#define      gdk_gl_glGetNamedProgramLocalParameterfvEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glGetNamedProgramLocalParameterfvEXT) (proc)) (program, target, index, params) )

/* glGetNamedProgramivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramivEXT) (GLuint program, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetNamedProgramivEXT (void);
#define      gdk_gl_glGetNamedProgramivEXT(proc, program, target, pname, params) \
  ( ((GdkGLProc_glGetNamedProgramivEXT) (proc)) (program, target, pname, params) )

/* glGetNamedProgramStringEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramStringEXT) (GLuint program, GLenum target, GLenum pname, GLvoid *string);
GdkGLProc    gdk_gl_get_glGetNamedProgramStringEXT (void);
#define      gdk_gl_glGetNamedProgramStringEXT(proc, program, target, pname, string) \
  ( ((GdkGLProc_glGetNamedProgramStringEXT) (proc)) (program, target, pname, string) )

/* glNamedProgramLocalParameters4fvEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameters4fvEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameters4fvEXT (void);
#define      gdk_gl_glNamedProgramLocalParameters4fvEXT(proc, program, target, index, count, params) \
  ( ((GdkGLProc_glNamedProgramLocalParameters4fvEXT) (proc)) (program, target, index, count, params) )

/* glNamedProgramLocalParameterI4iEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameterI4iEXT) (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameterI4iEXT (void);
#define      gdk_gl_glNamedProgramLocalParameterI4iEXT(proc, program, target, index, x, y, z, w) \
  ( ((GdkGLProc_glNamedProgramLocalParameterI4iEXT) (proc)) (program, target, index, x, y, z, w) )

/* glNamedProgramLocalParameterI4ivEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameterI4ivEXT) (GLuint program, GLenum target, GLuint index, const GLint *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameterI4ivEXT (void);
#define      gdk_gl_glNamedProgramLocalParameterI4ivEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glNamedProgramLocalParameterI4ivEXT) (proc)) (program, target, index, params) )

/* glNamedProgramLocalParametersI4ivEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParametersI4ivEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParametersI4ivEXT (void);
#define      gdk_gl_glNamedProgramLocalParametersI4ivEXT(proc, program, target, index, count, params) \
  ( ((GdkGLProc_glNamedProgramLocalParametersI4ivEXT) (proc)) (program, target, index, count, params) )

/* glNamedProgramLocalParameterI4uiEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameterI4uiEXT) (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameterI4uiEXT (void);
#define      gdk_gl_glNamedProgramLocalParameterI4uiEXT(proc, program, target, index, x, y, z, w) \
  ( ((GdkGLProc_glNamedProgramLocalParameterI4uiEXT) (proc)) (program, target, index, x, y, z, w) )

/* glNamedProgramLocalParameterI4uivEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParameterI4uivEXT) (GLuint program, GLenum target, GLuint index, const GLuint *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParameterI4uivEXT (void);
#define      gdk_gl_glNamedProgramLocalParameterI4uivEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glNamedProgramLocalParameterI4uivEXT) (proc)) (program, target, index, params) )

/* glNamedProgramLocalParametersI4uivEXT */
typedef void (APIENTRYP GdkGLProc_glNamedProgramLocalParametersI4uivEXT) (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params);
GdkGLProc    gdk_gl_get_glNamedProgramLocalParametersI4uivEXT (void);
#define      gdk_gl_glNamedProgramLocalParametersI4uivEXT(proc, program, target, index, count, params) \
  ( ((GdkGLProc_glNamedProgramLocalParametersI4uivEXT) (proc)) (program, target, index, count, params) )

/* glGetNamedProgramLocalParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramLocalParameterIivEXT) (GLuint program, GLenum target, GLuint index, GLint *params);
GdkGLProc    gdk_gl_get_glGetNamedProgramLocalParameterIivEXT (void);
#define      gdk_gl_glGetNamedProgramLocalParameterIivEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glGetNamedProgramLocalParameterIivEXT) (proc)) (program, target, index, params) )

/* glGetNamedProgramLocalParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedProgramLocalParameterIuivEXT) (GLuint program, GLenum target, GLuint index, GLuint *params);
GdkGLProc    gdk_gl_get_glGetNamedProgramLocalParameterIuivEXT (void);
#define      gdk_gl_glGetNamedProgramLocalParameterIuivEXT(proc, program, target, index, params) \
  ( ((GdkGLProc_glGetNamedProgramLocalParameterIuivEXT) (proc)) (program, target, index, params) )

/* glTextureParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameterIivEXT) (GLuint texture, GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glTextureParameterIivEXT (void);
#define      gdk_gl_glTextureParameterIivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glTextureParameterIivEXT) (proc)) (texture, target, pname, params) )

/* glTextureParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glTextureParameterIuivEXT) (GLuint texture, GLenum target, GLenum pname, const GLuint *params);
GdkGLProc    gdk_gl_get_glTextureParameterIuivEXT (void);
#define      gdk_gl_glTextureParameterIuivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glTextureParameterIuivEXT) (proc)) (texture, target, pname, params) )

/* glGetTextureParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureParameterIivEXT) (GLuint texture, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetTextureParameterIivEXT (void);
#define      gdk_gl_glGetTextureParameterIivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glGetTextureParameterIivEXT) (proc)) (texture, target, pname, params) )

/* glGetTextureParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetTextureParameterIuivEXT) (GLuint texture, GLenum target, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetTextureParameterIuivEXT (void);
#define      gdk_gl_glGetTextureParameterIuivEXT(proc, texture, target, pname, params) \
  ( ((GdkGLProc_glGetTextureParameterIuivEXT) (proc)) (texture, target, pname, params) )

/* glMultiTexParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameterIivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glMultiTexParameterIivEXT (void);
#define      gdk_gl_glMultiTexParameterIivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexParameterIivEXT) (proc)) (texunit, target, pname, params) )

/* glMultiTexParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexParameterIuivEXT) (GLenum texunit, GLenum target, GLenum pname, const GLuint *params);
GdkGLProc    gdk_gl_get_glMultiTexParameterIuivEXT (void);
#define      gdk_gl_glMultiTexParameterIuivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glMultiTexParameterIuivEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexParameterIivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexParameterIivEXT) (GLenum texunit, GLenum target, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexParameterIivEXT (void);
#define      gdk_gl_glGetMultiTexParameterIivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexParameterIivEXT) (proc)) (texunit, target, pname, params) )

/* glGetMultiTexParameterIuivEXT */
typedef void (APIENTRYP GdkGLProc_glGetMultiTexParameterIuivEXT) (GLenum texunit, GLenum target, GLenum pname, GLuint *params);
GdkGLProc    gdk_gl_get_glGetMultiTexParameterIuivEXT (void);
#define      gdk_gl_glGetMultiTexParameterIuivEXT(proc, texunit, target, pname, params) \
  ( ((GdkGLProc_glGetMultiTexParameterIuivEXT) (proc)) (texunit, target, pname, params) )

/* glProgramUniform1fEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1fEXT) (GLuint program, GLint location, GLfloat v0);
GdkGLProc    gdk_gl_get_glProgramUniform1fEXT (void);
#define      gdk_gl_glProgramUniform1fEXT(proc, program, location, v0) \
  ( ((GdkGLProc_glProgramUniform1fEXT) (proc)) (program, location, v0) )

/* glProgramUniform2fEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1);
GdkGLProc    gdk_gl_get_glProgramUniform2fEXT (void);
#define      gdk_gl_glProgramUniform2fEXT(proc, program, location, v0, v1) \
  ( ((GdkGLProc_glProgramUniform2fEXT) (proc)) (program, location, v0, v1) )

/* glProgramUniform3fEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
GdkGLProc    gdk_gl_get_glProgramUniform3fEXT (void);
#define      gdk_gl_glProgramUniform3fEXT(proc, program, location, v0, v1, v2) \
  ( ((GdkGLProc_glProgramUniform3fEXT) (proc)) (program, location, v0, v1, v2) )

/* glProgramUniform4fEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4fEXT) (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
GdkGLProc    gdk_gl_get_glProgramUniform4fEXT (void);
#define      gdk_gl_glProgramUniform4fEXT(proc, program, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glProgramUniform4fEXT) (proc)) (program, location, v0, v1, v2, v3) )

/* glProgramUniform1iEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1iEXT) (GLuint program, GLint location, GLint v0);
GdkGLProc    gdk_gl_get_glProgramUniform1iEXT (void);
#define      gdk_gl_glProgramUniform1iEXT(proc, program, location, v0) \
  ( ((GdkGLProc_glProgramUniform1iEXT) (proc)) (program, location, v0) )

/* glProgramUniform2iEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2iEXT) (GLuint program, GLint location, GLint v0, GLint v1);
GdkGLProc    gdk_gl_get_glProgramUniform2iEXT (void);
#define      gdk_gl_glProgramUniform2iEXT(proc, program, location, v0, v1) \
  ( ((GdkGLProc_glProgramUniform2iEXT) (proc)) (program, location, v0, v1) )

/* glProgramUniform3iEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
GdkGLProc    gdk_gl_get_glProgramUniform3iEXT (void);
#define      gdk_gl_glProgramUniform3iEXT(proc, program, location, v0, v1, v2) \
  ( ((GdkGLProc_glProgramUniform3iEXT) (proc)) (program, location, v0, v1, v2) )

/* glProgramUniform4iEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4iEXT) (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
GdkGLProc    gdk_gl_get_glProgramUniform4iEXT (void);
#define      gdk_gl_glProgramUniform4iEXT(proc, program, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glProgramUniform4iEXT) (proc)) (program, location, v0, v1, v2, v3) )

/* glProgramUniform1fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniform1fvEXT (void);
#define      gdk_gl_glProgramUniform1fvEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform1fvEXT) (proc)) (program, location, count, value) )

/* glProgramUniform2fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniform2fvEXT (void);
#define      gdk_gl_glProgramUniform2fvEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform2fvEXT) (proc)) (program, location, count, value) )

/* glProgramUniform3fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniform3fvEXT (void);
#define      gdk_gl_glProgramUniform3fvEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform3fvEXT) (proc)) (program, location, count, value) )

/* glProgramUniform4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4fvEXT) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniform4fvEXT (void);
#define      gdk_gl_glProgramUniform4fvEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform4fvEXT) (proc)) (program, location, count, value) )

/* glProgramUniform1ivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glProgramUniform1ivEXT (void);
#define      gdk_gl_glProgramUniform1ivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform1ivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform2ivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glProgramUniform2ivEXT (void);
#define      gdk_gl_glProgramUniform2ivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform2ivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform3ivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glProgramUniform3ivEXT (void);
#define      gdk_gl_glProgramUniform3ivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform3ivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform4ivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4ivEXT) (GLuint program, GLint location, GLsizei count, const GLint *value);
GdkGLProc    gdk_gl_get_glProgramUniform4ivEXT (void);
#define      gdk_gl_glProgramUniform4ivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform4ivEXT) (proc)) (program, location, count, value) )

/* glProgramUniformMatrix2fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix2fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix2fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix2fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix3fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix3fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix3fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix3fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix4fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix4fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix4fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix2x3fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix2x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix2x3fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix2x3fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix2x3fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix3x2fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix3x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix3x2fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix3x2fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix3x2fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix2x4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix2x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix2x4fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix2x4fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix2x4fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix4x2fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix4x2fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix4x2fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix4x2fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix4x2fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix3x4fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix3x4fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix3x4fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix3x4fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix3x4fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniformMatrix4x3fvEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniformMatrix4x3fvEXT) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
GdkGLProc    gdk_gl_get_glProgramUniformMatrix4x3fvEXT (void);
#define      gdk_gl_glProgramUniformMatrix4x3fvEXT(proc, program, location, count, transpose, value) \
  ( ((GdkGLProc_glProgramUniformMatrix4x3fvEXT) (proc)) (program, location, count, transpose, value) )

/* glProgramUniform1uiEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1uiEXT) (GLuint program, GLint location, GLuint v0);
GdkGLProc    gdk_gl_get_glProgramUniform1uiEXT (void);
#define      gdk_gl_glProgramUniform1uiEXT(proc, program, location, v0) \
  ( ((GdkGLProc_glProgramUniform1uiEXT) (proc)) (program, location, v0) )

/* glProgramUniform2uiEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1);
GdkGLProc    gdk_gl_get_glProgramUniform2uiEXT (void);
#define      gdk_gl_glProgramUniform2uiEXT(proc, program, location, v0, v1) \
  ( ((GdkGLProc_glProgramUniform2uiEXT) (proc)) (program, location, v0, v1) )

/* glProgramUniform3uiEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
GdkGLProc    gdk_gl_get_glProgramUniform3uiEXT (void);
#define      gdk_gl_glProgramUniform3uiEXT(proc, program, location, v0, v1, v2) \
  ( ((GdkGLProc_glProgramUniform3uiEXT) (proc)) (program, location, v0, v1, v2) )

/* glProgramUniform4uiEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4uiEXT) (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
GdkGLProc    gdk_gl_get_glProgramUniform4uiEXT (void);
#define      gdk_gl_glProgramUniform4uiEXT(proc, program, location, v0, v1, v2, v3) \
  ( ((GdkGLProc_glProgramUniform4uiEXT) (proc)) (program, location, v0, v1, v2, v3) )

/* glProgramUniform1uivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform1uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glProgramUniform1uivEXT (void);
#define      gdk_gl_glProgramUniform1uivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform1uivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform2uivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform2uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glProgramUniform2uivEXT (void);
#define      gdk_gl_glProgramUniform2uivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform2uivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform3uivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform3uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glProgramUniform3uivEXT (void);
#define      gdk_gl_glProgramUniform3uivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform3uivEXT) (proc)) (program, location, count, value) )

/* glProgramUniform4uivEXT */
typedef void (APIENTRYP GdkGLProc_glProgramUniform4uivEXT) (GLuint program, GLint location, GLsizei count, const GLuint *value);
GdkGLProc    gdk_gl_get_glProgramUniform4uivEXT (void);
#define      gdk_gl_glProgramUniform4uivEXT(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniform4uivEXT) (proc)) (program, location, count, value) )

/* glNamedBufferDataEXT */
typedef void (APIENTRYP GdkGLProc_glNamedBufferDataEXT) (GLuint buffer, GLsizeiptr size, const GLvoid *data, GLenum usage);
GdkGLProc    gdk_gl_get_glNamedBufferDataEXT (void);
#define      gdk_gl_glNamedBufferDataEXT(proc, buffer, size, data, usage) \
  ( ((GdkGLProc_glNamedBufferDataEXT) (proc)) (buffer, size, data, usage) )

/* glNamedBufferSubDataEXT */
typedef void (APIENTRYP GdkGLProc_glNamedBufferSubDataEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, const GLvoid *data);
GdkGLProc    gdk_gl_get_glNamedBufferSubDataEXT (void);
#define      gdk_gl_glNamedBufferSubDataEXT(proc, buffer, offset, size, data) \
  ( ((GdkGLProc_glNamedBufferSubDataEXT) (proc)) (buffer, offset, size, data) )

/* glMapNamedBufferEXT */
typedef GLvoid* (APIENTRYP GdkGLProc_glMapNamedBufferEXT) (GLuint buffer, GLenum access);
GdkGLProc    gdk_gl_get_glMapNamedBufferEXT (void);
#define      gdk_gl_glMapNamedBufferEXT(proc, buffer, access) \
  ( ((GdkGLProc_glMapNamedBufferEXT) (proc)) (buffer, access) )

/* glUnmapNamedBufferEXT */
typedef GLboolean (APIENTRYP GdkGLProc_glUnmapNamedBufferEXT) (GLuint buffer);
GdkGLProc    gdk_gl_get_glUnmapNamedBufferEXT (void);
#define      gdk_gl_glUnmapNamedBufferEXT(proc, buffer) \
  ( ((GdkGLProc_glUnmapNamedBufferEXT) (proc)) (buffer) )

/* glGetNamedBufferParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedBufferParameterivEXT) (GLuint buffer, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetNamedBufferParameterivEXT (void);
#define      gdk_gl_glGetNamedBufferParameterivEXT(proc, buffer, pname, params) \
  ( ((GdkGLProc_glGetNamedBufferParameterivEXT) (proc)) (buffer, pname, params) )

/* glGetNamedBufferPointervEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedBufferPointervEXT) (GLuint buffer, GLenum pname, GLvoid* *params);
GdkGLProc    gdk_gl_get_glGetNamedBufferPointervEXT (void);
#define      gdk_gl_glGetNamedBufferPointervEXT(proc, buffer, pname, params) \
  ( ((GdkGLProc_glGetNamedBufferPointervEXT) (proc)) (buffer, pname, params) )

/* glGetNamedBufferSubDataEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedBufferSubDataEXT) (GLuint buffer, GLintptr offset, GLsizeiptr size, GLvoid *data);
GdkGLProc    gdk_gl_get_glGetNamedBufferSubDataEXT (void);
#define      gdk_gl_glGetNamedBufferSubDataEXT(proc, buffer, offset, size, data) \
  ( ((GdkGLProc_glGetNamedBufferSubDataEXT) (proc)) (buffer, offset, size, data) )

/* glTextureBufferEXT */
typedef void (APIENTRYP GdkGLProc_glTextureBufferEXT) (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
GdkGLProc    gdk_gl_get_glTextureBufferEXT (void);
#define      gdk_gl_glTextureBufferEXT(proc, texture, target, internalformat, buffer) \
  ( ((GdkGLProc_glTextureBufferEXT) (proc)) (texture, target, internalformat, buffer) )

/* glMultiTexBufferEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexBufferEXT) (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
GdkGLProc    gdk_gl_get_glMultiTexBufferEXT (void);
#define      gdk_gl_glMultiTexBufferEXT(proc, texunit, target, internalformat, buffer) \
  ( ((GdkGLProc_glMultiTexBufferEXT) (proc)) (texunit, target, internalformat, buffer) )

/* glNamedRenderbufferStorageEXT */
typedef void (APIENTRYP GdkGLProc_glNamedRenderbufferStorageEXT) (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glNamedRenderbufferStorageEXT (void);
#define      gdk_gl_glNamedRenderbufferStorageEXT(proc, renderbuffer, internalformat, width, height) \
  ( ((GdkGLProc_glNamedRenderbufferStorageEXT) (proc)) (renderbuffer, internalformat, width, height) )

/* glGetNamedRenderbufferParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedRenderbufferParameterivEXT) (GLuint renderbuffer, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetNamedRenderbufferParameterivEXT (void);
#define      gdk_gl_glGetNamedRenderbufferParameterivEXT(proc, renderbuffer, pname, params) \
  ( ((GdkGLProc_glGetNamedRenderbufferParameterivEXT) (proc)) (renderbuffer, pname, params) )

/* glCheckNamedFramebufferStatusEXT */
typedef GLenum (APIENTRYP GdkGLProc_glCheckNamedFramebufferStatusEXT) (GLuint framebuffer, GLenum target);
GdkGLProc    gdk_gl_get_glCheckNamedFramebufferStatusEXT (void);
#define      gdk_gl_glCheckNamedFramebufferStatusEXT(proc, framebuffer, target) \
  ( ((GdkGLProc_glCheckNamedFramebufferStatusEXT) (proc)) (framebuffer, target) )

/* glNamedFramebufferTexture1DEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTexture1DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glNamedFramebufferTexture1DEXT (void);
#define      gdk_gl_glNamedFramebufferTexture1DEXT(proc, framebuffer, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glNamedFramebufferTexture1DEXT) (proc)) (framebuffer, attachment, textarget, texture, level) )

/* glNamedFramebufferTexture2DEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTexture2DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glNamedFramebufferTexture2DEXT (void);
#define      gdk_gl_glNamedFramebufferTexture2DEXT(proc, framebuffer, attachment, textarget, texture, level) \
  ( ((GdkGLProc_glNamedFramebufferTexture2DEXT) (proc)) (framebuffer, attachment, textarget, texture, level) )

/* glNamedFramebufferTexture3DEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTexture3DEXT) (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
GdkGLProc    gdk_gl_get_glNamedFramebufferTexture3DEXT (void);
#define      gdk_gl_glNamedFramebufferTexture3DEXT(proc, framebuffer, attachment, textarget, texture, level, zoffset) \
  ( ((GdkGLProc_glNamedFramebufferTexture3DEXT) (proc)) (framebuffer, attachment, textarget, texture, level, zoffset) )

/* glNamedFramebufferRenderbufferEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferRenderbufferEXT) (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glNamedFramebufferRenderbufferEXT (void);
#define      gdk_gl_glNamedFramebufferRenderbufferEXT(proc, framebuffer, attachment, renderbuffertarget, renderbuffer) \
  ( ((GdkGLProc_glNamedFramebufferRenderbufferEXT) (proc)) (framebuffer, attachment, renderbuffertarget, renderbuffer) )

/* glGetNamedFramebufferAttachmentParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetNamedFramebufferAttachmentParameterivEXT) (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetNamedFramebufferAttachmentParameterivEXT (void);
#define      gdk_gl_glGetNamedFramebufferAttachmentParameterivEXT(proc, framebuffer, attachment, pname, params) \
  ( ((GdkGLProc_glGetNamedFramebufferAttachmentParameterivEXT) (proc)) (framebuffer, attachment, pname, params) )

/* glGenerateTextureMipmapEXT */
typedef void (APIENTRYP GdkGLProc_glGenerateTextureMipmapEXT) (GLuint texture, GLenum target);
GdkGLProc    gdk_gl_get_glGenerateTextureMipmapEXT (void);
#define      gdk_gl_glGenerateTextureMipmapEXT(proc, texture, target) \
  ( ((GdkGLProc_glGenerateTextureMipmapEXT) (proc)) (texture, target) )

/* glGenerateMultiTexMipmapEXT */
typedef void (APIENTRYP GdkGLProc_glGenerateMultiTexMipmapEXT) (GLenum texunit, GLenum target);
GdkGLProc    gdk_gl_get_glGenerateMultiTexMipmapEXT (void);
#define      gdk_gl_glGenerateMultiTexMipmapEXT(proc, texunit, target) \
  ( ((GdkGLProc_glGenerateMultiTexMipmapEXT) (proc)) (texunit, target) )

/* glFramebufferDrawBufferEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferDrawBufferEXT) (GLuint framebuffer, GLenum mode);
GdkGLProc    gdk_gl_get_glFramebufferDrawBufferEXT (void);
#define      gdk_gl_glFramebufferDrawBufferEXT(proc, framebuffer, mode) \
  ( ((GdkGLProc_glFramebufferDrawBufferEXT) (proc)) (framebuffer, mode) )

/* glFramebufferDrawBuffersEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferDrawBuffersEXT) (GLuint framebuffer, GLsizei n, const GLenum *bufs);
GdkGLProc    gdk_gl_get_glFramebufferDrawBuffersEXT (void);
#define      gdk_gl_glFramebufferDrawBuffersEXT(proc, framebuffer, n, bufs) \
  ( ((GdkGLProc_glFramebufferDrawBuffersEXT) (proc)) (framebuffer, n, bufs) )

/* glFramebufferReadBufferEXT */
typedef void (APIENTRYP GdkGLProc_glFramebufferReadBufferEXT) (GLuint framebuffer, GLenum mode);
GdkGLProc    gdk_gl_get_glFramebufferReadBufferEXT (void);
#define      gdk_gl_glFramebufferReadBufferEXT(proc, framebuffer, mode) \
  ( ((GdkGLProc_glFramebufferReadBufferEXT) (proc)) (framebuffer, mode) )

/* glGetFramebufferParameterivEXT */
typedef void (APIENTRYP GdkGLProc_glGetFramebufferParameterivEXT) (GLuint framebuffer, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFramebufferParameterivEXT (void);
#define      gdk_gl_glGetFramebufferParameterivEXT(proc, framebuffer, pname, params) \
  ( ((GdkGLProc_glGetFramebufferParameterivEXT) (proc)) (framebuffer, pname, params) )

/* glNamedRenderbufferStorageMultisampleEXT */
typedef void (APIENTRYP GdkGLProc_glNamedRenderbufferStorageMultisampleEXT) (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glNamedRenderbufferStorageMultisampleEXT (void);
#define      gdk_gl_glNamedRenderbufferStorageMultisampleEXT(proc, renderbuffer, samples, internalformat, width, height) \
  ( ((GdkGLProc_glNamedRenderbufferStorageMultisampleEXT) (proc)) (renderbuffer, samples, internalformat, width, height) )

/* glNamedRenderbufferStorageMultisampleCoverageEXT */
typedef void (APIENTRYP GdkGLProc_glNamedRenderbufferStorageMultisampleCoverageEXT) (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glNamedRenderbufferStorageMultisampleCoverageEXT (void);
#define      gdk_gl_glNamedRenderbufferStorageMultisampleCoverageEXT(proc, renderbuffer, coverageSamples, colorSamples, internalformat, width, height) \
  ( ((GdkGLProc_glNamedRenderbufferStorageMultisampleCoverageEXT) (proc)) (renderbuffer, coverageSamples, colorSamples, internalformat, width, height) )

/* glNamedFramebufferTextureEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTextureEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
GdkGLProc    gdk_gl_get_glNamedFramebufferTextureEXT (void);
#define      gdk_gl_glNamedFramebufferTextureEXT(proc, framebuffer, attachment, texture, level) \
  ( ((GdkGLProc_glNamedFramebufferTextureEXT) (proc)) (framebuffer, attachment, texture, level) )

/* glNamedFramebufferTextureLayerEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTextureLayerEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
GdkGLProc    gdk_gl_get_glNamedFramebufferTextureLayerEXT (void);
#define      gdk_gl_glNamedFramebufferTextureLayerEXT(proc, framebuffer, attachment, texture, level, layer) \
  ( ((GdkGLProc_glNamedFramebufferTextureLayerEXT) (proc)) (framebuffer, attachment, texture, level, layer) )

/* glNamedFramebufferTextureFaceEXT */
typedef void (APIENTRYP GdkGLProc_glNamedFramebufferTextureFaceEXT) (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face);
GdkGLProc    gdk_gl_get_glNamedFramebufferTextureFaceEXT (void);
#define      gdk_gl_glNamedFramebufferTextureFaceEXT(proc, framebuffer, attachment, texture, level, face) \
  ( ((GdkGLProc_glNamedFramebufferTextureFaceEXT) (proc)) (framebuffer, attachment, texture, level, face) )

/* glTextureRenderbufferEXT */
typedef void (APIENTRYP GdkGLProc_glTextureRenderbufferEXT) (GLuint texture, GLenum target, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glTextureRenderbufferEXT (void);
#define      gdk_gl_glTextureRenderbufferEXT(proc, texture, target, renderbuffer) \
  ( ((GdkGLProc_glTextureRenderbufferEXT) (proc)) (texture, target, renderbuffer) )

/* glMultiTexRenderbufferEXT */
typedef void (APIENTRYP GdkGLProc_glMultiTexRenderbufferEXT) (GLenum texunit, GLenum target, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glMultiTexRenderbufferEXT (void);
#define      gdk_gl_glMultiTexRenderbufferEXT(proc, texunit, target, renderbuffer) \
  ( ((GdkGLProc_glMultiTexRenderbufferEXT) (proc)) (texunit, target, renderbuffer) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_direct_state_access GdkGL_GL_EXT_direct_state_access;

struct _GdkGL_GL_EXT_direct_state_access
{
  GdkGLProc_glClientAttribDefaultEXT glClientAttribDefaultEXT;
  GdkGLProc_glPushClientAttribDefaultEXT glPushClientAttribDefaultEXT;
  GdkGLProc_glMatrixLoadfEXT glMatrixLoadfEXT;
  GdkGLProc_glMatrixLoaddEXT glMatrixLoaddEXT;
  GdkGLProc_glMatrixMultfEXT glMatrixMultfEXT;
  GdkGLProc_glMatrixMultdEXT glMatrixMultdEXT;
  GdkGLProc_glMatrixLoadIdentityEXT glMatrixLoadIdentityEXT;
  GdkGLProc_glMatrixRotatefEXT glMatrixRotatefEXT;
  GdkGLProc_glMatrixRotatedEXT glMatrixRotatedEXT;
  GdkGLProc_glMatrixScalefEXT glMatrixScalefEXT;
  GdkGLProc_glMatrixScaledEXT glMatrixScaledEXT;
  GdkGLProc_glMatrixTranslatefEXT glMatrixTranslatefEXT;
  GdkGLProc_glMatrixTranslatedEXT glMatrixTranslatedEXT;
  GdkGLProc_glMatrixFrustumEXT glMatrixFrustumEXT;
  GdkGLProc_glMatrixOrthoEXT glMatrixOrthoEXT;
  GdkGLProc_glMatrixPopEXT glMatrixPopEXT;
  GdkGLProc_glMatrixPushEXT glMatrixPushEXT;
  GdkGLProc_glMatrixLoadTransposefEXT glMatrixLoadTransposefEXT;
  GdkGLProc_glMatrixLoadTransposedEXT glMatrixLoadTransposedEXT;
  GdkGLProc_glMatrixMultTransposefEXT glMatrixMultTransposefEXT;
  GdkGLProc_glMatrixMultTransposedEXT glMatrixMultTransposedEXT;
  GdkGLProc_glTextureParameterfEXT glTextureParameterfEXT;
  GdkGLProc_glTextureParameterfvEXT glTextureParameterfvEXT;
  GdkGLProc_glTextureParameteriEXT glTextureParameteriEXT;
  GdkGLProc_glTextureParameterivEXT glTextureParameterivEXT;
  GdkGLProc_glTextureImage1DEXT glTextureImage1DEXT;
  GdkGLProc_glTextureImage2DEXT glTextureImage2DEXT;
  GdkGLProc_glTextureSubImage1DEXT glTextureSubImage1DEXT;
  GdkGLProc_glTextureSubImage2DEXT glTextureSubImage2DEXT;
  GdkGLProc_glCopyTextureImage1DEXT glCopyTextureImage1DEXT;
  GdkGLProc_glCopyTextureImage2DEXT glCopyTextureImage2DEXT;
  GdkGLProc_glCopyTextureSubImage1DEXT glCopyTextureSubImage1DEXT;
  GdkGLProc_glCopyTextureSubImage2DEXT glCopyTextureSubImage2DEXT;
  GdkGLProc_glGetTextureImageEXT glGetTextureImageEXT;
  GdkGLProc_glGetTextureParameterfvEXT glGetTextureParameterfvEXT;
  GdkGLProc_glGetTextureParameterivEXT glGetTextureParameterivEXT;
  GdkGLProc_glGetTextureLevelParameterfvEXT glGetTextureLevelParameterfvEXT;
  GdkGLProc_glGetTextureLevelParameterivEXT glGetTextureLevelParameterivEXT;
  GdkGLProc_glTextureImage3DEXT glTextureImage3DEXT;
  GdkGLProc_glTextureSubImage3DEXT glTextureSubImage3DEXT;
  GdkGLProc_glCopyTextureSubImage3DEXT glCopyTextureSubImage3DEXT;
  GdkGLProc_glMultiTexParameterfEXT glMultiTexParameterfEXT;
  GdkGLProc_glMultiTexParameterfvEXT glMultiTexParameterfvEXT;
  GdkGLProc_glMultiTexParameteriEXT glMultiTexParameteriEXT;
  GdkGLProc_glMultiTexParameterivEXT glMultiTexParameterivEXT;
  GdkGLProc_glMultiTexImage1DEXT glMultiTexImage1DEXT;
  GdkGLProc_glMultiTexImage2DEXT glMultiTexImage2DEXT;
  GdkGLProc_glMultiTexSubImage1DEXT glMultiTexSubImage1DEXT;
  GdkGLProc_glMultiTexSubImage2DEXT glMultiTexSubImage2DEXT;
  GdkGLProc_glCopyMultiTexImage1DEXT glCopyMultiTexImage1DEXT;
  GdkGLProc_glCopyMultiTexImage2DEXT glCopyMultiTexImage2DEXT;
  GdkGLProc_glCopyMultiTexSubImage1DEXT glCopyMultiTexSubImage1DEXT;
  GdkGLProc_glCopyMultiTexSubImage2DEXT glCopyMultiTexSubImage2DEXT;
  GdkGLProc_glGetMultiTexImageEXT glGetMultiTexImageEXT;
  GdkGLProc_glGetMultiTexParameterfvEXT glGetMultiTexParameterfvEXT;
  GdkGLProc_glGetMultiTexParameterivEXT glGetMultiTexParameterivEXT;
  GdkGLProc_glGetMultiTexLevelParameterfvEXT glGetMultiTexLevelParameterfvEXT;
  GdkGLProc_glGetMultiTexLevelParameterivEXT glGetMultiTexLevelParameterivEXT;
  GdkGLProc_glMultiTexImage3DEXT glMultiTexImage3DEXT;
  GdkGLProc_glMultiTexSubImage3DEXT glMultiTexSubImage3DEXT;
  GdkGLProc_glCopyMultiTexSubImage3DEXT glCopyMultiTexSubImage3DEXT;
  GdkGLProc_glBindMultiTextureEXT glBindMultiTextureEXT;
  GdkGLProc_glEnableClientStateIndexedEXT glEnableClientStateIndexedEXT;
  GdkGLProc_glDisableClientStateIndexedEXT glDisableClientStateIndexedEXT;
  GdkGLProc_glMultiTexCoordPointerEXT glMultiTexCoordPointerEXT;
  GdkGLProc_glMultiTexEnvfEXT glMultiTexEnvfEXT;
  GdkGLProc_glMultiTexEnvfvEXT glMultiTexEnvfvEXT;
  GdkGLProc_glMultiTexEnviEXT glMultiTexEnviEXT;
  GdkGLProc_glMultiTexEnvivEXT glMultiTexEnvivEXT;
  GdkGLProc_glMultiTexGendEXT glMultiTexGendEXT;
  GdkGLProc_glMultiTexGendvEXT glMultiTexGendvEXT;
  GdkGLProc_glMultiTexGenfEXT glMultiTexGenfEXT;
  GdkGLProc_glMultiTexGenfvEXT glMultiTexGenfvEXT;
  GdkGLProc_glMultiTexGeniEXT glMultiTexGeniEXT;
  GdkGLProc_glMultiTexGenivEXT glMultiTexGenivEXT;
  GdkGLProc_glGetMultiTexEnvfvEXT glGetMultiTexEnvfvEXT;
  GdkGLProc_glGetMultiTexEnvivEXT glGetMultiTexEnvivEXT;
  GdkGLProc_glGetMultiTexGendvEXT glGetMultiTexGendvEXT;
  GdkGLProc_glGetMultiTexGenfvEXT glGetMultiTexGenfvEXT;
  GdkGLProc_glGetMultiTexGenivEXT glGetMultiTexGenivEXT;
  GdkGLProc_glGetFloatIndexedvEXT glGetFloatIndexedvEXT;
  GdkGLProc_glGetDoubleIndexedvEXT glGetDoubleIndexedvEXT;
  GdkGLProc_glGetPointerIndexedvEXT glGetPointerIndexedvEXT;
  GdkGLProc_glCompressedTextureImage3DEXT glCompressedTextureImage3DEXT;
  GdkGLProc_glCompressedTextureImage2DEXT glCompressedTextureImage2DEXT;
  GdkGLProc_glCompressedTextureImage1DEXT glCompressedTextureImage1DEXT;
  GdkGLProc_glCompressedTextureSubImage3DEXT glCompressedTextureSubImage3DEXT;
  GdkGLProc_glCompressedTextureSubImage2DEXT glCompressedTextureSubImage2DEXT;
  GdkGLProc_glCompressedTextureSubImage1DEXT glCompressedTextureSubImage1DEXT;
  GdkGLProc_glGetCompressedTextureImageEXT glGetCompressedTextureImageEXT;
  GdkGLProc_glCompressedMultiTexImage3DEXT glCompressedMultiTexImage3DEXT;
  GdkGLProc_glCompressedMultiTexImage2DEXT glCompressedMultiTexImage2DEXT;
  GdkGLProc_glCompressedMultiTexImage1DEXT glCompressedMultiTexImage1DEXT;
  GdkGLProc_glCompressedMultiTexSubImage3DEXT glCompressedMultiTexSubImage3DEXT;
  GdkGLProc_glCompressedMultiTexSubImage2DEXT glCompressedMultiTexSubImage2DEXT;
  GdkGLProc_glCompressedMultiTexSubImage1DEXT glCompressedMultiTexSubImage1DEXT;
  GdkGLProc_glGetCompressedMultiTexImageEXT glGetCompressedMultiTexImageEXT;
  GdkGLProc_glNamedProgramStringEXT glNamedProgramStringEXT;
  GdkGLProc_glNamedProgramLocalParameter4dEXT glNamedProgramLocalParameter4dEXT;
  GdkGLProc_glNamedProgramLocalParameter4dvEXT glNamedProgramLocalParameter4dvEXT;
  GdkGLProc_glNamedProgramLocalParameter4fEXT glNamedProgramLocalParameter4fEXT;
  GdkGLProc_glNamedProgramLocalParameter4fvEXT glNamedProgramLocalParameter4fvEXT;
  GdkGLProc_glGetNamedProgramLocalParameterdvEXT glGetNamedProgramLocalParameterdvEXT;
  GdkGLProc_glGetNamedProgramLocalParameterfvEXT glGetNamedProgramLocalParameterfvEXT;
  GdkGLProc_glGetNamedProgramivEXT glGetNamedProgramivEXT;
  GdkGLProc_glGetNamedProgramStringEXT glGetNamedProgramStringEXT;
  GdkGLProc_glNamedProgramLocalParameters4fvEXT glNamedProgramLocalParameters4fvEXT;
  GdkGLProc_glNamedProgramLocalParameterI4iEXT glNamedProgramLocalParameterI4iEXT;
  GdkGLProc_glNamedProgramLocalParameterI4ivEXT glNamedProgramLocalParameterI4ivEXT;
  GdkGLProc_glNamedProgramLocalParametersI4ivEXT glNamedProgramLocalParametersI4ivEXT;
  GdkGLProc_glNamedProgramLocalParameterI4uiEXT glNamedProgramLocalParameterI4uiEXT;
  GdkGLProc_glNamedProgramLocalParameterI4uivEXT glNamedProgramLocalParameterI4uivEXT;
  GdkGLProc_glNamedProgramLocalParametersI4uivEXT glNamedProgramLocalParametersI4uivEXT;
  GdkGLProc_glGetNamedProgramLocalParameterIivEXT glGetNamedProgramLocalParameterIivEXT;
  GdkGLProc_glGetNamedProgramLocalParameterIuivEXT glGetNamedProgramLocalParameterIuivEXT;
  GdkGLProc_glTextureParameterIivEXT glTextureParameterIivEXT;
  GdkGLProc_glTextureParameterIuivEXT glTextureParameterIuivEXT;
  GdkGLProc_glGetTextureParameterIivEXT glGetTextureParameterIivEXT;
  GdkGLProc_glGetTextureParameterIuivEXT glGetTextureParameterIuivEXT;
  GdkGLProc_glMultiTexParameterIivEXT glMultiTexParameterIivEXT;
  GdkGLProc_glMultiTexParameterIuivEXT glMultiTexParameterIuivEXT;
  GdkGLProc_glGetMultiTexParameterIivEXT glGetMultiTexParameterIivEXT;
  GdkGLProc_glGetMultiTexParameterIuivEXT glGetMultiTexParameterIuivEXT;
  GdkGLProc_glProgramUniform1fEXT glProgramUniform1fEXT;
  GdkGLProc_glProgramUniform2fEXT glProgramUniform2fEXT;
  GdkGLProc_glProgramUniform3fEXT glProgramUniform3fEXT;
  GdkGLProc_glProgramUniform4fEXT glProgramUniform4fEXT;
  GdkGLProc_glProgramUniform1iEXT glProgramUniform1iEXT;
  GdkGLProc_glProgramUniform2iEXT glProgramUniform2iEXT;
  GdkGLProc_glProgramUniform3iEXT glProgramUniform3iEXT;
  GdkGLProc_glProgramUniform4iEXT glProgramUniform4iEXT;
  GdkGLProc_glProgramUniform1fvEXT glProgramUniform1fvEXT;
  GdkGLProc_glProgramUniform2fvEXT glProgramUniform2fvEXT;
  GdkGLProc_glProgramUniform3fvEXT glProgramUniform3fvEXT;
  GdkGLProc_glProgramUniform4fvEXT glProgramUniform4fvEXT;
  GdkGLProc_glProgramUniform1ivEXT glProgramUniform1ivEXT;
  GdkGLProc_glProgramUniform2ivEXT glProgramUniform2ivEXT;
  GdkGLProc_glProgramUniform3ivEXT glProgramUniform3ivEXT;
  GdkGLProc_glProgramUniform4ivEXT glProgramUniform4ivEXT;
  GdkGLProc_glProgramUniformMatrix2fvEXT glProgramUniformMatrix2fvEXT;
  GdkGLProc_glProgramUniformMatrix3fvEXT glProgramUniformMatrix3fvEXT;
  GdkGLProc_glProgramUniformMatrix4fvEXT glProgramUniformMatrix4fvEXT;
  GdkGLProc_glProgramUniformMatrix2x3fvEXT glProgramUniformMatrix2x3fvEXT;
  GdkGLProc_glProgramUniformMatrix3x2fvEXT glProgramUniformMatrix3x2fvEXT;
  GdkGLProc_glProgramUniformMatrix2x4fvEXT glProgramUniformMatrix2x4fvEXT;
  GdkGLProc_glProgramUniformMatrix4x2fvEXT glProgramUniformMatrix4x2fvEXT;
  GdkGLProc_glProgramUniformMatrix3x4fvEXT glProgramUniformMatrix3x4fvEXT;
  GdkGLProc_glProgramUniformMatrix4x3fvEXT glProgramUniformMatrix4x3fvEXT;
  GdkGLProc_glProgramUniform1uiEXT glProgramUniform1uiEXT;
  GdkGLProc_glProgramUniform2uiEXT glProgramUniform2uiEXT;
  GdkGLProc_glProgramUniform3uiEXT glProgramUniform3uiEXT;
  GdkGLProc_glProgramUniform4uiEXT glProgramUniform4uiEXT;
  GdkGLProc_glProgramUniform1uivEXT glProgramUniform1uivEXT;
  GdkGLProc_glProgramUniform2uivEXT glProgramUniform2uivEXT;
  GdkGLProc_glProgramUniform3uivEXT glProgramUniform3uivEXT;
  GdkGLProc_glProgramUniform4uivEXT glProgramUniform4uivEXT;
  GdkGLProc_glNamedBufferDataEXT glNamedBufferDataEXT;
  GdkGLProc_glNamedBufferSubDataEXT glNamedBufferSubDataEXT;
  GdkGLProc_glMapNamedBufferEXT glMapNamedBufferEXT;
  GdkGLProc_glUnmapNamedBufferEXT glUnmapNamedBufferEXT;
  GdkGLProc_glGetNamedBufferParameterivEXT glGetNamedBufferParameterivEXT;
  GdkGLProc_glGetNamedBufferPointervEXT glGetNamedBufferPointervEXT;
  GdkGLProc_glGetNamedBufferSubDataEXT glGetNamedBufferSubDataEXT;
  GdkGLProc_glTextureBufferEXT glTextureBufferEXT;
  GdkGLProc_glMultiTexBufferEXT glMultiTexBufferEXT;
  GdkGLProc_glNamedRenderbufferStorageEXT glNamedRenderbufferStorageEXT;
  GdkGLProc_glGetNamedRenderbufferParameterivEXT glGetNamedRenderbufferParameterivEXT;
  GdkGLProc_glCheckNamedFramebufferStatusEXT glCheckNamedFramebufferStatusEXT;
  GdkGLProc_glNamedFramebufferTexture1DEXT glNamedFramebufferTexture1DEXT;
  GdkGLProc_glNamedFramebufferTexture2DEXT glNamedFramebufferTexture2DEXT;
  GdkGLProc_glNamedFramebufferTexture3DEXT glNamedFramebufferTexture3DEXT;
  GdkGLProc_glNamedFramebufferRenderbufferEXT glNamedFramebufferRenderbufferEXT;
  GdkGLProc_glGetNamedFramebufferAttachmentParameterivEXT glGetNamedFramebufferAttachmentParameterivEXT;
  GdkGLProc_glGenerateTextureMipmapEXT glGenerateTextureMipmapEXT;
  GdkGLProc_glGenerateMultiTexMipmapEXT glGenerateMultiTexMipmapEXT;
  GdkGLProc_glFramebufferDrawBufferEXT glFramebufferDrawBufferEXT;
  GdkGLProc_glFramebufferDrawBuffersEXT glFramebufferDrawBuffersEXT;
  GdkGLProc_glFramebufferReadBufferEXT glFramebufferReadBufferEXT;
  GdkGLProc_glGetFramebufferParameterivEXT glGetFramebufferParameterivEXT;
  GdkGLProc_glNamedRenderbufferStorageMultisampleEXT glNamedRenderbufferStorageMultisampleEXT;
  GdkGLProc_glNamedRenderbufferStorageMultisampleCoverageEXT glNamedRenderbufferStorageMultisampleCoverageEXT;
  GdkGLProc_glNamedFramebufferTextureEXT glNamedFramebufferTextureEXT;
  GdkGLProc_glNamedFramebufferTextureLayerEXT glNamedFramebufferTextureLayerEXT;
  GdkGLProc_glNamedFramebufferTextureFaceEXT glNamedFramebufferTextureFaceEXT;
  GdkGLProc_glTextureRenderbufferEXT glTextureRenderbufferEXT;
  GdkGLProc_glMultiTexRenderbufferEXT glMultiTexRenderbufferEXT;
};

GdkGL_GL_EXT_direct_state_access *gdk_gl_get_GL_EXT_direct_state_access (void);

/*
 * GL_NV_explicit_multisample
 */

/* glGetMultisamplefvNV */
typedef void (APIENTRYP GdkGLProc_glGetMultisamplefvNV) (GLenum pname, GLuint index, GLfloat *val);
GdkGLProc    gdk_gl_get_glGetMultisamplefvNV (void);
#define      gdk_gl_glGetMultisamplefvNV(proc, pname, index, val) \
  ( ((GdkGLProc_glGetMultisamplefvNV) (proc)) (pname, index, val) )

/* glSampleMaskIndexedNV */
typedef void (APIENTRYP GdkGLProc_glSampleMaskIndexedNV) (GLuint index, GLbitfield mask);
GdkGLProc    gdk_gl_get_glSampleMaskIndexedNV (void);
#define      gdk_gl_glSampleMaskIndexedNV(proc, index, mask) \
  ( ((GdkGLProc_glSampleMaskIndexedNV) (proc)) (index, mask) )

/* glTexRenderbufferNV */
typedef void (APIENTRYP GdkGLProc_glTexRenderbufferNV) (GLenum target, GLuint renderbuffer);
GdkGLProc    gdk_gl_get_glTexRenderbufferNV (void);
#define      gdk_gl_glTexRenderbufferNV(proc, target, renderbuffer) \
  ( ((GdkGLProc_glTexRenderbufferNV) (proc)) (target, renderbuffer) )

/* proc struct */

typedef struct _GdkGL_GL_NV_explicit_multisample GdkGL_GL_NV_explicit_multisample;

struct _GdkGL_GL_NV_explicit_multisample
{
  GdkGLProc_glGetMultisamplefvNV glGetMultisamplefvNV;
  GdkGLProc_glSampleMaskIndexedNV glSampleMaskIndexedNV;
  GdkGLProc_glTexRenderbufferNV glTexRenderbufferNV;
};

GdkGL_GL_NV_explicit_multisample *gdk_gl_get_GL_NV_explicit_multisample (void);

/*
 * GL_NV_transform_feedback2
 */

/* glBindTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glBindTransformFeedbackNV) (GLenum target, GLuint id);
GdkGLProc    gdk_gl_get_glBindTransformFeedbackNV (void);
#define      gdk_gl_glBindTransformFeedbackNV(proc, target, id) \
  ( ((GdkGLProc_glBindTransformFeedbackNV) (proc)) (target, id) )

/* glDeleteTransformFeedbacksNV */
typedef void (APIENTRYP GdkGLProc_glDeleteTransformFeedbacksNV) (GLsizei n, const GLuint *ids);
GdkGLProc    gdk_gl_get_glDeleteTransformFeedbacksNV (void);
#define      gdk_gl_glDeleteTransformFeedbacksNV(proc, n, ids) \
  ( ((GdkGLProc_glDeleteTransformFeedbacksNV) (proc)) (n, ids) )

/* glGenTransformFeedbacksNV */
typedef void (APIENTRYP GdkGLProc_glGenTransformFeedbacksNV) (GLsizei n, GLuint *ids);
GdkGLProc    gdk_gl_get_glGenTransformFeedbacksNV (void);
#define      gdk_gl_glGenTransformFeedbacksNV(proc, n, ids) \
  ( ((GdkGLProc_glGenTransformFeedbacksNV) (proc)) (n, ids) )

/* glIsTransformFeedbackNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsTransformFeedbackNV) (GLuint id);
GdkGLProc    gdk_gl_get_glIsTransformFeedbackNV (void);
#define      gdk_gl_glIsTransformFeedbackNV(proc, id) \
  ( ((GdkGLProc_glIsTransformFeedbackNV) (proc)) (id) )

/* glPauseTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glPauseTransformFeedbackNV) (void);
GdkGLProc    gdk_gl_get_glPauseTransformFeedbackNV (void);
#define      gdk_gl_glPauseTransformFeedbackNV(proc) \
  ( ((GdkGLProc_glPauseTransformFeedbackNV) (proc)) () )

/* glResumeTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glResumeTransformFeedbackNV) (void);
GdkGLProc    gdk_gl_get_glResumeTransformFeedbackNV (void);
#define      gdk_gl_glResumeTransformFeedbackNV(proc) \
  ( ((GdkGLProc_glResumeTransformFeedbackNV) (proc)) () )

/* glDrawTransformFeedbackNV */
typedef void (APIENTRYP GdkGLProc_glDrawTransformFeedbackNV) (GLenum mode, GLuint id);
GdkGLProc    gdk_gl_get_glDrawTransformFeedbackNV (void);
#define      gdk_gl_glDrawTransformFeedbackNV(proc, mode, id) \
  ( ((GdkGLProc_glDrawTransformFeedbackNV) (proc)) (mode, id) )

/* proc struct */

typedef struct _GdkGL_GL_NV_transform_feedback2 GdkGL_GL_NV_transform_feedback2;

struct _GdkGL_GL_NV_transform_feedback2
{
  GdkGLProc_glBindTransformFeedbackNV glBindTransformFeedbackNV;
  GdkGLProc_glDeleteTransformFeedbacksNV glDeleteTransformFeedbacksNV;
  GdkGLProc_glGenTransformFeedbacksNV glGenTransformFeedbacksNV;
  GdkGLProc_glIsTransformFeedbackNV glIsTransformFeedbackNV;
  GdkGLProc_glPauseTransformFeedbackNV glPauseTransformFeedbackNV;
  GdkGLProc_glResumeTransformFeedbackNV glResumeTransformFeedbackNV;
  GdkGLProc_glDrawTransformFeedbackNV glDrawTransformFeedbackNV;
};

GdkGL_GL_NV_transform_feedback2 *gdk_gl_get_GL_NV_transform_feedback2 (void);

/*
 * GL_AMD_performance_monitor
 */

/* glGetPerfMonitorGroupsAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorGroupsAMD) (GLint *numGroups, GLsizei groupsSize, GLuint *groups);
GdkGLProc    gdk_gl_get_glGetPerfMonitorGroupsAMD (void);
#define      gdk_gl_glGetPerfMonitorGroupsAMD(proc, numGroups, groupsSize, groups) \
  ( ((GdkGLProc_glGetPerfMonitorGroupsAMD) (proc)) (numGroups, groupsSize, groups) )

/* glGetPerfMonitorCountersAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorCountersAMD) (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters);
GdkGLProc    gdk_gl_get_glGetPerfMonitorCountersAMD (void);
#define      gdk_gl_glGetPerfMonitorCountersAMD(proc, group, numCounters, maxActiveCounters, counterSize, counters) \
  ( ((GdkGLProc_glGetPerfMonitorCountersAMD) (proc)) (group, numCounters, maxActiveCounters, counterSize, counters) )

/* glGetPerfMonitorGroupStringAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorGroupStringAMD) (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString);
GdkGLProc    gdk_gl_get_glGetPerfMonitorGroupStringAMD (void);
#define      gdk_gl_glGetPerfMonitorGroupStringAMD(proc, group, bufSize, length, groupString) \
  ( ((GdkGLProc_glGetPerfMonitorGroupStringAMD) (proc)) (group, bufSize, length, groupString) )

/* glGetPerfMonitorCounterStringAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorCounterStringAMD) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString);
GdkGLProc    gdk_gl_get_glGetPerfMonitorCounterStringAMD (void);
#define      gdk_gl_glGetPerfMonitorCounterStringAMD(proc, group, counter, bufSize, length, counterString) \
  ( ((GdkGLProc_glGetPerfMonitorCounterStringAMD) (proc)) (group, counter, bufSize, length, counterString) )

/* glGetPerfMonitorCounterInfoAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorCounterInfoAMD) (GLuint group, GLuint counter, GLenum pname, void *data);
GdkGLProc    gdk_gl_get_glGetPerfMonitorCounterInfoAMD (void);
#define      gdk_gl_glGetPerfMonitorCounterInfoAMD(proc, group, counter, pname, data) \
  ( ((GdkGLProc_glGetPerfMonitorCounterInfoAMD) (proc)) (group, counter, pname, data) )

/* glGenPerfMonitorsAMD */
typedef void (APIENTRYP GdkGLProc_glGenPerfMonitorsAMD) (GLsizei n, GLuint *monitors);
GdkGLProc    gdk_gl_get_glGenPerfMonitorsAMD (void);
#define      gdk_gl_glGenPerfMonitorsAMD(proc, n, monitors) \
  ( ((GdkGLProc_glGenPerfMonitorsAMD) (proc)) (n, monitors) )

/* glDeletePerfMonitorsAMD */
typedef void (APIENTRYP GdkGLProc_glDeletePerfMonitorsAMD) (GLsizei n, GLuint *monitors);
GdkGLProc    gdk_gl_get_glDeletePerfMonitorsAMD (void);
#define      gdk_gl_glDeletePerfMonitorsAMD(proc, n, monitors) \
  ( ((GdkGLProc_glDeletePerfMonitorsAMD) (proc)) (n, monitors) )

/* glSelectPerfMonitorCountersAMD */
typedef void (APIENTRYP GdkGLProc_glSelectPerfMonitorCountersAMD) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *counterList);
GdkGLProc    gdk_gl_get_glSelectPerfMonitorCountersAMD (void);
#define      gdk_gl_glSelectPerfMonitorCountersAMD(proc, monitor, enable, group, numCounters, counterList) \
  ( ((GdkGLProc_glSelectPerfMonitorCountersAMD) (proc)) (monitor, enable, group, numCounters, counterList) )

/* glBeginPerfMonitorAMD */
typedef void (APIENTRYP GdkGLProc_glBeginPerfMonitorAMD) (GLuint monitor);
GdkGLProc    gdk_gl_get_glBeginPerfMonitorAMD (void);
#define      gdk_gl_glBeginPerfMonitorAMD(proc, monitor) \
  ( ((GdkGLProc_glBeginPerfMonitorAMD) (proc)) (monitor) )

/* glEndPerfMonitorAMD */
typedef void (APIENTRYP GdkGLProc_glEndPerfMonitorAMD) (GLuint monitor);
GdkGLProc    gdk_gl_get_glEndPerfMonitorAMD (void);
#define      gdk_gl_glEndPerfMonitorAMD(proc, monitor) \
  ( ((GdkGLProc_glEndPerfMonitorAMD) (proc)) (monitor) )

/* glGetPerfMonitorCounterDataAMD */
typedef void (APIENTRYP GdkGLProc_glGetPerfMonitorCounterDataAMD) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten);
GdkGLProc    gdk_gl_get_glGetPerfMonitorCounterDataAMD (void);
#define      gdk_gl_glGetPerfMonitorCounterDataAMD(proc, monitor, pname, dataSize, data, bytesWritten) \
  ( ((GdkGLProc_glGetPerfMonitorCounterDataAMD) (proc)) (monitor, pname, dataSize, data, bytesWritten) )

/* proc struct */

typedef struct _GdkGL_GL_AMD_performance_monitor GdkGL_GL_AMD_performance_monitor;

struct _GdkGL_GL_AMD_performance_monitor
{
  GdkGLProc_glGetPerfMonitorGroupsAMD glGetPerfMonitorGroupsAMD;
  GdkGLProc_glGetPerfMonitorCountersAMD glGetPerfMonitorCountersAMD;
  GdkGLProc_glGetPerfMonitorGroupStringAMD glGetPerfMonitorGroupStringAMD;
  GdkGLProc_glGetPerfMonitorCounterStringAMD glGetPerfMonitorCounterStringAMD;
  GdkGLProc_glGetPerfMonitorCounterInfoAMD glGetPerfMonitorCounterInfoAMD;
  GdkGLProc_glGenPerfMonitorsAMD glGenPerfMonitorsAMD;
  GdkGLProc_glDeletePerfMonitorsAMD glDeletePerfMonitorsAMD;
  GdkGLProc_glSelectPerfMonitorCountersAMD glSelectPerfMonitorCountersAMD;
  GdkGLProc_glBeginPerfMonitorAMD glBeginPerfMonitorAMD;
  GdkGLProc_glEndPerfMonitorAMD glEndPerfMonitorAMD;
  GdkGLProc_glGetPerfMonitorCounterDataAMD glGetPerfMonitorCounterDataAMD;
};

GdkGL_GL_AMD_performance_monitor *gdk_gl_get_GL_AMD_performance_monitor (void);

/*
 * GL_AMD_vertex_shader_tesselator
 */

/* glTessellationFactorAMD */
typedef void (APIENTRYP GdkGLProc_glTessellationFactorAMD) (GLfloat factor);
GdkGLProc    gdk_gl_get_glTessellationFactorAMD (void);
#define      gdk_gl_glTessellationFactorAMD(proc, factor) \
  ( ((GdkGLProc_glTessellationFactorAMD) (proc)) (factor) )

/* glTessellationModeAMD */
typedef void (APIENTRYP GdkGLProc_glTessellationModeAMD) (GLenum mode);
GdkGLProc    gdk_gl_get_glTessellationModeAMD (void);
#define      gdk_gl_glTessellationModeAMD(proc, mode) \
  ( ((GdkGLProc_glTessellationModeAMD) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_AMD_vertex_shader_tesselator GdkGL_GL_AMD_vertex_shader_tesselator;

struct _GdkGL_GL_AMD_vertex_shader_tesselator
{
  GdkGLProc_glTessellationFactorAMD glTessellationFactorAMD;
  GdkGLProc_glTessellationModeAMD glTessellationModeAMD;
};

GdkGL_GL_AMD_vertex_shader_tesselator *gdk_gl_get_GL_AMD_vertex_shader_tesselator (void);

/*
 * GL_EXT_provoking_vertex
 */

/* glProvokingVertexEXT */
typedef void (APIENTRYP GdkGLProc_glProvokingVertexEXT) (GLenum mode);
GdkGLProc    gdk_gl_get_glProvokingVertexEXT (void);
#define      gdk_gl_glProvokingVertexEXT(proc, mode) \
  ( ((GdkGLProc_glProvokingVertexEXT) (proc)) (mode) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_provoking_vertex GdkGL_GL_EXT_provoking_vertex;

struct _GdkGL_GL_EXT_provoking_vertex
{
  GdkGLProc_glProvokingVertexEXT glProvokingVertexEXT;
};

GdkGL_GL_EXT_provoking_vertex *gdk_gl_get_GL_EXT_provoking_vertex (void);

/*
 * GL_AMD_draw_buffers_blend
 */

/* glBlendFuncIndexedAMD */
typedef void (APIENTRYP GdkGLProc_glBlendFuncIndexedAMD) (GLuint buf, GLenum src, GLenum dst);
GdkGLProc    gdk_gl_get_glBlendFuncIndexedAMD (void);
#define      gdk_gl_glBlendFuncIndexedAMD(proc, buf, src, dst) \
  ( ((GdkGLProc_glBlendFuncIndexedAMD) (proc)) (buf, src, dst) )

/* glBlendFuncSeparateIndexedAMD */
typedef void (APIENTRYP GdkGLProc_glBlendFuncSeparateIndexedAMD) (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
GdkGLProc    gdk_gl_get_glBlendFuncSeparateIndexedAMD (void);
#define      gdk_gl_glBlendFuncSeparateIndexedAMD(proc, buf, srcRGB, dstRGB, srcAlpha, dstAlpha) \
  ( ((GdkGLProc_glBlendFuncSeparateIndexedAMD) (proc)) (buf, srcRGB, dstRGB, srcAlpha, dstAlpha) )

/* glBlendEquationIndexedAMD */
typedef void (APIENTRYP GdkGLProc_glBlendEquationIndexedAMD) (GLuint buf, GLenum mode);
GdkGLProc    gdk_gl_get_glBlendEquationIndexedAMD (void);
#define      gdk_gl_glBlendEquationIndexedAMD(proc, buf, mode) \
  ( ((GdkGLProc_glBlendEquationIndexedAMD) (proc)) (buf, mode) )

/* glBlendEquationSeparateIndexedAMD */
typedef void (APIENTRYP GdkGLProc_glBlendEquationSeparateIndexedAMD) (GLuint buf, GLenum modeRGB, GLenum modeAlpha);
GdkGLProc    gdk_gl_get_glBlendEquationSeparateIndexedAMD (void);
#define      gdk_gl_glBlendEquationSeparateIndexedAMD(proc, buf, modeRGB, modeAlpha) \
  ( ((GdkGLProc_glBlendEquationSeparateIndexedAMD) (proc)) (buf, modeRGB, modeAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_AMD_draw_buffers_blend GdkGL_GL_AMD_draw_buffers_blend;

struct _GdkGL_GL_AMD_draw_buffers_blend
{
  GdkGLProc_glBlendFuncIndexedAMD glBlendFuncIndexedAMD;
  GdkGLProc_glBlendFuncSeparateIndexedAMD glBlendFuncSeparateIndexedAMD;
  GdkGLProc_glBlendEquationIndexedAMD glBlendEquationIndexedAMD;
  GdkGLProc_glBlendEquationSeparateIndexedAMD glBlendEquationSeparateIndexedAMD;
};

GdkGL_GL_AMD_draw_buffers_blend *gdk_gl_get_GL_AMD_draw_buffers_blend (void);

/*
 * GL_APPLE_texture_range
 */

/* glTextureRangeAPPLE */
typedef void (APIENTRYP GdkGLProc_glTextureRangeAPPLE) (GLenum target, GLsizei length, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glTextureRangeAPPLE (void);
#define      gdk_gl_glTextureRangeAPPLE(proc, target, length, pointer) \
  ( ((GdkGLProc_glTextureRangeAPPLE) (proc)) (target, length, pointer) )

/* glGetTexParameterPointervAPPLE */
typedef void (APIENTRYP GdkGLProc_glGetTexParameterPointervAPPLE) (GLenum target, GLenum pname, GLvoid* *params);
GdkGLProc    gdk_gl_get_glGetTexParameterPointervAPPLE (void);
#define      gdk_gl_glGetTexParameterPointervAPPLE(proc, target, pname, params) \
  ( ((GdkGLProc_glGetTexParameterPointervAPPLE) (proc)) (target, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_texture_range GdkGL_GL_APPLE_texture_range;

struct _GdkGL_GL_APPLE_texture_range
{
  GdkGLProc_glTextureRangeAPPLE glTextureRangeAPPLE;
  GdkGLProc_glGetTexParameterPointervAPPLE glGetTexParameterPointervAPPLE;
};

GdkGL_GL_APPLE_texture_range *gdk_gl_get_GL_APPLE_texture_range (void);

/*
 * GL_APPLE_vertex_program_evaluators
 */

/* glEnableVertexAttribAPPLE */
typedef void (APIENTRYP GdkGLProc_glEnableVertexAttribAPPLE) (GLuint index, GLenum pname);
GdkGLProc    gdk_gl_get_glEnableVertexAttribAPPLE (void);
#define      gdk_gl_glEnableVertexAttribAPPLE(proc, index, pname) \
  ( ((GdkGLProc_glEnableVertexAttribAPPLE) (proc)) (index, pname) )

/* glDisableVertexAttribAPPLE */
typedef void (APIENTRYP GdkGLProc_glDisableVertexAttribAPPLE) (GLuint index, GLenum pname);
GdkGLProc    gdk_gl_get_glDisableVertexAttribAPPLE (void);
#define      gdk_gl_glDisableVertexAttribAPPLE(proc, index, pname) \
  ( ((GdkGLProc_glDisableVertexAttribAPPLE) (proc)) (index, pname) )

/* glIsVertexAttribEnabledAPPLE */
typedef GLboolean (APIENTRYP GdkGLProc_glIsVertexAttribEnabledAPPLE) (GLuint index, GLenum pname);
GdkGLProc    gdk_gl_get_glIsVertexAttribEnabledAPPLE (void);
#define      gdk_gl_glIsVertexAttribEnabledAPPLE(proc, index, pname) \
  ( ((GdkGLProc_glIsVertexAttribEnabledAPPLE) (proc)) (index, pname) )

/* glMapVertexAttrib1dAPPLE */
typedef void (APIENTRYP GdkGLProc_glMapVertexAttrib1dAPPLE) (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
GdkGLProc    gdk_gl_get_glMapVertexAttrib1dAPPLE (void);
#define      gdk_gl_glMapVertexAttrib1dAPPLE(proc, index, size, u1, u2, stride, order, points) \
  ( ((GdkGLProc_glMapVertexAttrib1dAPPLE) (proc)) (index, size, u1, u2, stride, order, points) )

/* glMapVertexAttrib1fAPPLE */
typedef void (APIENTRYP GdkGLProc_glMapVertexAttrib1fAPPLE) (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
GdkGLProc    gdk_gl_get_glMapVertexAttrib1fAPPLE (void);
#define      gdk_gl_glMapVertexAttrib1fAPPLE(proc, index, size, u1, u2, stride, order, points) \
  ( ((GdkGLProc_glMapVertexAttrib1fAPPLE) (proc)) (index, size, u1, u2, stride, order, points) )

/* glMapVertexAttrib2dAPPLE */
typedef void (APIENTRYP GdkGLProc_glMapVertexAttrib2dAPPLE) (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
GdkGLProc    gdk_gl_get_glMapVertexAttrib2dAPPLE (void);
#define      gdk_gl_glMapVertexAttrib2dAPPLE(proc, index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) \
  ( ((GdkGLProc_glMapVertexAttrib2dAPPLE) (proc)) (index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) )

/* glMapVertexAttrib2fAPPLE */
typedef void (APIENTRYP GdkGLProc_glMapVertexAttrib2fAPPLE) (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
GdkGLProc    gdk_gl_get_glMapVertexAttrib2fAPPLE (void);
#define      gdk_gl_glMapVertexAttrib2fAPPLE(proc, index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) \
  ( ((GdkGLProc_glMapVertexAttrib2fAPPLE) (proc)) (index, size, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_vertex_program_evaluators GdkGL_GL_APPLE_vertex_program_evaluators;

struct _GdkGL_GL_APPLE_vertex_program_evaluators
{
  GdkGLProc_glEnableVertexAttribAPPLE glEnableVertexAttribAPPLE;
  GdkGLProc_glDisableVertexAttribAPPLE glDisableVertexAttribAPPLE;
  GdkGLProc_glIsVertexAttribEnabledAPPLE glIsVertexAttribEnabledAPPLE;
  GdkGLProc_glMapVertexAttrib1dAPPLE glMapVertexAttrib1dAPPLE;
  GdkGLProc_glMapVertexAttrib1fAPPLE glMapVertexAttrib1fAPPLE;
  GdkGLProc_glMapVertexAttrib2dAPPLE glMapVertexAttrib2dAPPLE;
  GdkGLProc_glMapVertexAttrib2fAPPLE glMapVertexAttrib2fAPPLE;
};

GdkGL_GL_APPLE_vertex_program_evaluators *gdk_gl_get_GL_APPLE_vertex_program_evaluators (void);

/*
 * GL_APPLE_object_purgeable
 */

/* glObjectPurgeableAPPLE */
typedef GLenum (APIENTRYP GdkGLProc_glObjectPurgeableAPPLE) (GLenum objectType, GLuint name, GLenum option);
GdkGLProc    gdk_gl_get_glObjectPurgeableAPPLE (void);
#define      gdk_gl_glObjectPurgeableAPPLE(proc, objectType, name, option) \
  ( ((GdkGLProc_glObjectPurgeableAPPLE) (proc)) (objectType, name, option) )

/* glObjectUnpurgeableAPPLE */
typedef GLenum (APIENTRYP GdkGLProc_glObjectUnpurgeableAPPLE) (GLenum objectType, GLuint name, GLenum option);
GdkGLProc    gdk_gl_get_glObjectUnpurgeableAPPLE (void);
#define      gdk_gl_glObjectUnpurgeableAPPLE(proc, objectType, name, option) \
  ( ((GdkGLProc_glObjectUnpurgeableAPPLE) (proc)) (objectType, name, option) )

/* glGetObjectParameterivAPPLE */
typedef void (APIENTRYP GdkGLProc_glGetObjectParameterivAPPLE) (GLenum objectType, GLuint name, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetObjectParameterivAPPLE (void);
#define      gdk_gl_glGetObjectParameterivAPPLE(proc, objectType, name, pname, params) \
  ( ((GdkGLProc_glGetObjectParameterivAPPLE) (proc)) (objectType, name, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_APPLE_object_purgeable GdkGL_GL_APPLE_object_purgeable;

struct _GdkGL_GL_APPLE_object_purgeable
{
  GdkGLProc_glObjectPurgeableAPPLE glObjectPurgeableAPPLE;
  GdkGLProc_glObjectUnpurgeableAPPLE glObjectUnpurgeableAPPLE;
  GdkGLProc_glGetObjectParameterivAPPLE glGetObjectParameterivAPPLE;
};

GdkGL_GL_APPLE_object_purgeable *gdk_gl_get_GL_APPLE_object_purgeable (void);

/*
 * GL_NV_video_capture
 */

/* glBeginVideoCaptureNV */
typedef void (APIENTRYP GdkGLProc_glBeginVideoCaptureNV) (GLuint video_capture_slot);
GdkGLProc    gdk_gl_get_glBeginVideoCaptureNV (void);
#define      gdk_gl_glBeginVideoCaptureNV(proc, video_capture_slot) \
  ( ((GdkGLProc_glBeginVideoCaptureNV) (proc)) (video_capture_slot) )

/* glBindVideoCaptureStreamBufferNV */
typedef void (APIENTRYP GdkGLProc_glBindVideoCaptureStreamBufferNV) (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset);
GdkGLProc    gdk_gl_get_glBindVideoCaptureStreamBufferNV (void);
#define      gdk_gl_glBindVideoCaptureStreamBufferNV(proc, video_capture_slot, stream, frame_region, offset) \
  ( ((GdkGLProc_glBindVideoCaptureStreamBufferNV) (proc)) (video_capture_slot, stream, frame_region, offset) )

/* glBindVideoCaptureStreamTextureNV */
typedef void (APIENTRYP GdkGLProc_glBindVideoCaptureStreamTextureNV) (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture);
GdkGLProc    gdk_gl_get_glBindVideoCaptureStreamTextureNV (void);
#define      gdk_gl_glBindVideoCaptureStreamTextureNV(proc, video_capture_slot, stream, frame_region, target, texture) \
  ( ((GdkGLProc_glBindVideoCaptureStreamTextureNV) (proc)) (video_capture_slot, stream, frame_region, target, texture) )

/* glEndVideoCaptureNV */
typedef void (APIENTRYP GdkGLProc_glEndVideoCaptureNV) (GLuint video_capture_slot);
GdkGLProc    gdk_gl_get_glEndVideoCaptureNV (void);
#define      gdk_gl_glEndVideoCaptureNV(proc, video_capture_slot) \
  ( ((GdkGLProc_glEndVideoCaptureNV) (proc)) (video_capture_slot) )

/* glGetVideoCaptureivNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoCaptureivNV) (GLuint video_capture_slot, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVideoCaptureivNV (void);
#define      gdk_gl_glGetVideoCaptureivNV(proc, video_capture_slot, pname, params) \
  ( ((GdkGLProc_glGetVideoCaptureivNV) (proc)) (video_capture_slot, pname, params) )

/* glGetVideoCaptureStreamivNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoCaptureStreamivNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetVideoCaptureStreamivNV (void);
#define      gdk_gl_glGetVideoCaptureStreamivNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glGetVideoCaptureStreamivNV) (proc)) (video_capture_slot, stream, pname, params) )

/* glGetVideoCaptureStreamfvNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoCaptureStreamfvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetVideoCaptureStreamfvNV (void);
#define      gdk_gl_glGetVideoCaptureStreamfvNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glGetVideoCaptureStreamfvNV) (proc)) (video_capture_slot, stream, pname, params) )

/* glGetVideoCaptureStreamdvNV */
typedef void (APIENTRYP GdkGLProc_glGetVideoCaptureStreamdvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble *params);
GdkGLProc    gdk_gl_get_glGetVideoCaptureStreamdvNV (void);
#define      gdk_gl_glGetVideoCaptureStreamdvNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glGetVideoCaptureStreamdvNV) (proc)) (video_capture_slot, stream, pname, params) )

/* glVideoCaptureNV */
typedef GLenum (APIENTRYP GdkGLProc_glVideoCaptureNV) (GLuint video_capture_slot, GLuint *sequence_num, GLuint64EXT *capture_time);
GdkGLProc    gdk_gl_get_glVideoCaptureNV (void);
#define      gdk_gl_glVideoCaptureNV(proc, video_capture_slot, sequence_num, capture_time) \
  ( ((GdkGLProc_glVideoCaptureNV) (proc)) (video_capture_slot, sequence_num, capture_time) )

/* glVideoCaptureStreamParameterivNV */
typedef void (APIENTRYP GdkGLProc_glVideoCaptureStreamParameterivNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glVideoCaptureStreamParameterivNV (void);
#define      gdk_gl_glVideoCaptureStreamParameterivNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glVideoCaptureStreamParameterivNV) (proc)) (video_capture_slot, stream, pname, params) )

/* glVideoCaptureStreamParameterfvNV */
typedef void (APIENTRYP GdkGLProc_glVideoCaptureStreamParameterfvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glVideoCaptureStreamParameterfvNV (void);
#define      gdk_gl_glVideoCaptureStreamParameterfvNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glVideoCaptureStreamParameterfvNV) (proc)) (video_capture_slot, stream, pname, params) )

/* glVideoCaptureStreamParameterdvNV */
typedef void (APIENTRYP GdkGLProc_glVideoCaptureStreamParameterdvNV) (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble *params);
GdkGLProc    gdk_gl_get_glVideoCaptureStreamParameterdvNV (void);
#define      gdk_gl_glVideoCaptureStreamParameterdvNV(proc, video_capture_slot, stream, pname, params) \
  ( ((GdkGLProc_glVideoCaptureStreamParameterdvNV) (proc)) (video_capture_slot, stream, pname, params) )

/* proc struct */

typedef struct _GdkGL_GL_NV_video_capture GdkGL_GL_NV_video_capture;

struct _GdkGL_GL_NV_video_capture
{
  GdkGLProc_glBeginVideoCaptureNV glBeginVideoCaptureNV;
  GdkGLProc_glBindVideoCaptureStreamBufferNV glBindVideoCaptureStreamBufferNV;
  GdkGLProc_glBindVideoCaptureStreamTextureNV glBindVideoCaptureStreamTextureNV;
  GdkGLProc_glEndVideoCaptureNV glEndVideoCaptureNV;
  GdkGLProc_glGetVideoCaptureivNV glGetVideoCaptureivNV;
  GdkGLProc_glGetVideoCaptureStreamivNV glGetVideoCaptureStreamivNV;
  GdkGLProc_glGetVideoCaptureStreamfvNV glGetVideoCaptureStreamfvNV;
  GdkGLProc_glGetVideoCaptureStreamdvNV glGetVideoCaptureStreamdvNV;
  GdkGLProc_glVideoCaptureNV glVideoCaptureNV;
  GdkGLProc_glVideoCaptureStreamParameterivNV glVideoCaptureStreamParameterivNV;
  GdkGLProc_glVideoCaptureStreamParameterfvNV glVideoCaptureStreamParameterfvNV;
  GdkGLProc_glVideoCaptureStreamParameterdvNV glVideoCaptureStreamParameterdvNV;
};

GdkGL_GL_NV_video_capture *gdk_gl_get_GL_NV_video_capture (void);

/*
 * GL_NV_copy_image
 */

/* glCopyImageSubDataNV */
typedef void (APIENTRYP GdkGLProc_glCopyImageSubDataNV) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth);
GdkGLProc    gdk_gl_get_glCopyImageSubDataNV (void);
#define      gdk_gl_glCopyImageSubDataNV(proc, srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, width, height, depth) \
  ( ((GdkGLProc_glCopyImageSubDataNV) (proc)) (srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, width, height, depth) )

/* proc struct */

typedef struct _GdkGL_GL_NV_copy_image GdkGL_GL_NV_copy_image;

struct _GdkGL_GL_NV_copy_image
{
  GdkGLProc_glCopyImageSubDataNV glCopyImageSubDataNV;
};

GdkGL_GL_NV_copy_image *gdk_gl_get_GL_NV_copy_image (void);

/*
 * GL_EXT_separate_shader_objects
 */

/* glUseShaderProgramEXT */
typedef void (APIENTRYP GdkGLProc_glUseShaderProgramEXT) (GLenum type, GLuint program);
GdkGLProc    gdk_gl_get_glUseShaderProgramEXT (void);
#define      gdk_gl_glUseShaderProgramEXT(proc, type, program) \
  ( ((GdkGLProc_glUseShaderProgramEXT) (proc)) (type, program) )

/* glActiveProgramEXT */
typedef void (APIENTRYP GdkGLProc_glActiveProgramEXT) (GLuint program);
GdkGLProc    gdk_gl_get_glActiveProgramEXT (void);
#define      gdk_gl_glActiveProgramEXT(proc, program) \
  ( ((GdkGLProc_glActiveProgramEXT) (proc)) (program) )

/* glCreateShaderProgramEXT */
typedef GLuint (APIENTRYP GdkGLProc_glCreateShaderProgramEXT) (GLenum type, const GLchar *string);
GdkGLProc    gdk_gl_get_glCreateShaderProgramEXT (void);
#define      gdk_gl_glCreateShaderProgramEXT(proc, type, string) \
  ( ((GdkGLProc_glCreateShaderProgramEXT) (proc)) (type, string) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_separate_shader_objects GdkGL_GL_EXT_separate_shader_objects;

struct _GdkGL_GL_EXT_separate_shader_objects
{
  GdkGLProc_glUseShaderProgramEXT glUseShaderProgramEXT;
  GdkGLProc_glActiveProgramEXT glActiveProgramEXT;
  GdkGLProc_glCreateShaderProgramEXT glCreateShaderProgramEXT;
};

GdkGL_GL_EXT_separate_shader_objects *gdk_gl_get_GL_EXT_separate_shader_objects (void);

/*
 * GL_NV_shader_buffer_load
 */

/* glMakeBufferResidentNV */
typedef void (APIENTRYP GdkGLProc_glMakeBufferResidentNV) (GLenum target, GLenum access);
GdkGLProc    gdk_gl_get_glMakeBufferResidentNV (void);
#define      gdk_gl_glMakeBufferResidentNV(proc, target, access) \
  ( ((GdkGLProc_glMakeBufferResidentNV) (proc)) (target, access) )

/* glMakeBufferNonResidentNV */
typedef void (APIENTRYP GdkGLProc_glMakeBufferNonResidentNV) (GLenum target);
GdkGLProc    gdk_gl_get_glMakeBufferNonResidentNV (void);
#define      gdk_gl_glMakeBufferNonResidentNV(proc, target) \
  ( ((GdkGLProc_glMakeBufferNonResidentNV) (proc)) (target) )

/* glIsBufferResidentNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsBufferResidentNV) (GLenum target);
GdkGLProc    gdk_gl_get_glIsBufferResidentNV (void);
#define      gdk_gl_glIsBufferResidentNV(proc, target) \
  ( ((GdkGLProc_glIsBufferResidentNV) (proc)) (target) )

/* glNamedMakeBufferResidentNV */
typedef void (APIENTRYP GdkGLProc_glNamedMakeBufferResidentNV) (GLuint buffer, GLenum access);
GdkGLProc    gdk_gl_get_glNamedMakeBufferResidentNV (void);
#define      gdk_gl_glNamedMakeBufferResidentNV(proc, buffer, access) \
  ( ((GdkGLProc_glNamedMakeBufferResidentNV) (proc)) (buffer, access) )

/* glNamedMakeBufferNonResidentNV */
typedef void (APIENTRYP GdkGLProc_glNamedMakeBufferNonResidentNV) (GLuint buffer);
GdkGLProc    gdk_gl_get_glNamedMakeBufferNonResidentNV (void);
#define      gdk_gl_glNamedMakeBufferNonResidentNV(proc, buffer) \
  ( ((GdkGLProc_glNamedMakeBufferNonResidentNV) (proc)) (buffer) )

/* glIsNamedBufferResidentNV */
typedef GLboolean (APIENTRYP GdkGLProc_glIsNamedBufferResidentNV) (GLuint buffer);
GdkGLProc    gdk_gl_get_glIsNamedBufferResidentNV (void);
#define      gdk_gl_glIsNamedBufferResidentNV(proc, buffer) \
  ( ((GdkGLProc_glIsNamedBufferResidentNV) (proc)) (buffer) )

/* glGetBufferParameterui64vNV */
typedef void (APIENTRYP GdkGLProc_glGetBufferParameterui64vNV) (GLenum target, GLenum pname, GLuint64EXT *params);
GdkGLProc    gdk_gl_get_glGetBufferParameterui64vNV (void);
#define      gdk_gl_glGetBufferParameterui64vNV(proc, target, pname, params) \
  ( ((GdkGLProc_glGetBufferParameterui64vNV) (proc)) (target, pname, params) )

/* glGetNamedBufferParameterui64vNV */
typedef void (APIENTRYP GdkGLProc_glGetNamedBufferParameterui64vNV) (GLuint buffer, GLenum pname, GLuint64EXT *params);
GdkGLProc    gdk_gl_get_glGetNamedBufferParameterui64vNV (void);
#define      gdk_gl_glGetNamedBufferParameterui64vNV(proc, buffer, pname, params) \
  ( ((GdkGLProc_glGetNamedBufferParameterui64vNV) (proc)) (buffer, pname, params) )

/* glGetIntegerui64vNV */
typedef void (APIENTRYP GdkGLProc_glGetIntegerui64vNV) (GLenum value, GLuint64EXT *result);
GdkGLProc    gdk_gl_get_glGetIntegerui64vNV (void);
#define      gdk_gl_glGetIntegerui64vNV(proc, value, result) \
  ( ((GdkGLProc_glGetIntegerui64vNV) (proc)) (value, result) )

/* glUniformui64NV */
typedef void (APIENTRYP GdkGLProc_glUniformui64NV) (GLint location, GLuint64EXT value);
GdkGLProc    gdk_gl_get_glUniformui64NV (void);
#define      gdk_gl_glUniformui64NV(proc, location, value) \
  ( ((GdkGLProc_glUniformui64NV) (proc)) (location, value) )

/* glUniformui64vNV */
typedef void (APIENTRYP GdkGLProc_glUniformui64vNV) (GLint location, GLsizei count, const GLuint64EXT *value);
GdkGLProc    gdk_gl_get_glUniformui64vNV (void);
#define      gdk_gl_glUniformui64vNV(proc, location, count, value) \
  ( ((GdkGLProc_glUniformui64vNV) (proc)) (location, count, value) )

/* glGetUniformui64vNV */
typedef void (APIENTRYP GdkGLProc_glGetUniformui64vNV) (GLuint program, GLint location, GLuint64EXT *params);
GdkGLProc    gdk_gl_get_glGetUniformui64vNV (void);
#define      gdk_gl_glGetUniformui64vNV(proc, program, location, params) \
  ( ((GdkGLProc_glGetUniformui64vNV) (proc)) (program, location, params) )

/* glProgramUniformui64NV */
typedef void (APIENTRYP GdkGLProc_glProgramUniformui64NV) (GLuint program, GLint location, GLuint64EXT value);
GdkGLProc    gdk_gl_get_glProgramUniformui64NV (void);
#define      gdk_gl_glProgramUniformui64NV(proc, program, location, value) \
  ( ((GdkGLProc_glProgramUniformui64NV) (proc)) (program, location, value) )

/* glProgramUniformui64vNV */
typedef void (APIENTRYP GdkGLProc_glProgramUniformui64vNV) (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value);
GdkGLProc    gdk_gl_get_glProgramUniformui64vNV (void);
#define      gdk_gl_glProgramUniformui64vNV(proc, program, location, count, value) \
  ( ((GdkGLProc_glProgramUniformui64vNV) (proc)) (program, location, count, value) )

/* proc struct */

typedef struct _GdkGL_GL_NV_shader_buffer_load GdkGL_GL_NV_shader_buffer_load;

struct _GdkGL_GL_NV_shader_buffer_load
{
  GdkGLProc_glMakeBufferResidentNV glMakeBufferResidentNV;
  GdkGLProc_glMakeBufferNonResidentNV glMakeBufferNonResidentNV;
  GdkGLProc_glIsBufferResidentNV glIsBufferResidentNV;
  GdkGLProc_glNamedMakeBufferResidentNV glNamedMakeBufferResidentNV;
  GdkGLProc_glNamedMakeBufferNonResidentNV glNamedMakeBufferNonResidentNV;
  GdkGLProc_glIsNamedBufferResidentNV glIsNamedBufferResidentNV;
  GdkGLProc_glGetBufferParameterui64vNV glGetBufferParameterui64vNV;
  GdkGLProc_glGetNamedBufferParameterui64vNV glGetNamedBufferParameterui64vNV;
  GdkGLProc_glGetIntegerui64vNV glGetIntegerui64vNV;
  GdkGLProc_glUniformui64NV glUniformui64NV;
  GdkGLProc_glUniformui64vNV glUniformui64vNV;
  GdkGLProc_glGetUniformui64vNV glGetUniformui64vNV;
  GdkGLProc_glProgramUniformui64NV glProgramUniformui64NV;
  GdkGLProc_glProgramUniformui64vNV glProgramUniformui64vNV;
};

GdkGL_GL_NV_shader_buffer_load *gdk_gl_get_GL_NV_shader_buffer_load (void);

/*
 * GL_NV_vertex_buffer_unified_memory
 */

/* glBufferAddressRangeNV */
typedef void (APIENTRYP GdkGLProc_glBufferAddressRangeNV) (GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length);
GdkGLProc    gdk_gl_get_glBufferAddressRangeNV (void);
#define      gdk_gl_glBufferAddressRangeNV(proc, pname, index, address, length) \
  ( ((GdkGLProc_glBufferAddressRangeNV) (proc)) (pname, index, address, length) )

/* glVertexFormatNV */
typedef void (APIENTRYP GdkGLProc_glVertexFormatNV) (GLint size, GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glVertexFormatNV (void);
#define      gdk_gl_glVertexFormatNV(proc, size, type, stride) \
  ( ((GdkGLProc_glVertexFormatNV) (proc)) (size, type, stride) )

/* glNormalFormatNV */
typedef void (APIENTRYP GdkGLProc_glNormalFormatNV) (GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glNormalFormatNV (void);
#define      gdk_gl_glNormalFormatNV(proc, type, stride) \
  ( ((GdkGLProc_glNormalFormatNV) (proc)) (type, stride) )

/* glColorFormatNV */
typedef void (APIENTRYP GdkGLProc_glColorFormatNV) (GLint size, GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glColorFormatNV (void);
#define      gdk_gl_glColorFormatNV(proc, size, type, stride) \
  ( ((GdkGLProc_glColorFormatNV) (proc)) (size, type, stride) )

/* glIndexFormatNV */
typedef void (APIENTRYP GdkGLProc_glIndexFormatNV) (GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glIndexFormatNV (void);
#define      gdk_gl_glIndexFormatNV(proc, type, stride) \
  ( ((GdkGLProc_glIndexFormatNV) (proc)) (type, stride) )

/* glTexCoordFormatNV */
typedef void (APIENTRYP GdkGLProc_glTexCoordFormatNV) (GLint size, GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glTexCoordFormatNV (void);
#define      gdk_gl_glTexCoordFormatNV(proc, size, type, stride) \
  ( ((GdkGLProc_glTexCoordFormatNV) (proc)) (size, type, stride) )

/* glEdgeFlagFormatNV */
typedef void (APIENTRYP GdkGLProc_glEdgeFlagFormatNV) (GLsizei stride);
GdkGLProc    gdk_gl_get_glEdgeFlagFormatNV (void);
#define      gdk_gl_glEdgeFlagFormatNV(proc, stride) \
  ( ((GdkGLProc_glEdgeFlagFormatNV) (proc)) (stride) )

/* glSecondaryColorFormatNV */
typedef void (APIENTRYP GdkGLProc_glSecondaryColorFormatNV) (GLint size, GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glSecondaryColorFormatNV (void);
#define      gdk_gl_glSecondaryColorFormatNV(proc, size, type, stride) \
  ( ((GdkGLProc_glSecondaryColorFormatNV) (proc)) (size, type, stride) )

/* glFogCoordFormatNV */
typedef void (APIENTRYP GdkGLProc_glFogCoordFormatNV) (GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glFogCoordFormatNV (void);
#define      gdk_gl_glFogCoordFormatNV(proc, type, stride) \
  ( ((GdkGLProc_glFogCoordFormatNV) (proc)) (type, stride) )

/* glVertexAttribFormatNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribFormatNV) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride);
GdkGLProc    gdk_gl_get_glVertexAttribFormatNV (void);
#define      gdk_gl_glVertexAttribFormatNV(proc, index, size, type, normalized, stride) \
  ( ((GdkGLProc_glVertexAttribFormatNV) (proc)) (index, size, type, normalized, stride) )

/* glVertexAttribIFormatNV */
typedef void (APIENTRYP GdkGLProc_glVertexAttribIFormatNV) (GLuint index, GLint size, GLenum type, GLsizei stride);
GdkGLProc    gdk_gl_get_glVertexAttribIFormatNV (void);
#define      gdk_gl_glVertexAttribIFormatNV(proc, index, size, type, stride) \
  ( ((GdkGLProc_glVertexAttribIFormatNV) (proc)) (index, size, type, stride) )

/* glGetIntegerui64i_vNV */
typedef void (APIENTRYP GdkGLProc_glGetIntegerui64i_vNV) (GLenum value, GLuint index, GLuint64EXT *result);
GdkGLProc    gdk_gl_get_glGetIntegerui64i_vNV (void);
#define      gdk_gl_glGetIntegerui64i_vNV(proc, value, index, result) \
  ( ((GdkGLProc_glGetIntegerui64i_vNV) (proc)) (value, index, result) )

/* proc struct */

typedef struct _GdkGL_GL_NV_vertex_buffer_unified_memory GdkGL_GL_NV_vertex_buffer_unified_memory;

struct _GdkGL_GL_NV_vertex_buffer_unified_memory
{
  GdkGLProc_glBufferAddressRangeNV glBufferAddressRangeNV;
  GdkGLProc_glVertexFormatNV glVertexFormatNV;
  GdkGLProc_glNormalFormatNV glNormalFormatNV;
  GdkGLProc_glColorFormatNV glColorFormatNV;
  GdkGLProc_glIndexFormatNV glIndexFormatNV;
  GdkGLProc_glTexCoordFormatNV glTexCoordFormatNV;
  GdkGLProc_glEdgeFlagFormatNV glEdgeFlagFormatNV;
  GdkGLProc_glSecondaryColorFormatNV glSecondaryColorFormatNV;
  GdkGLProc_glFogCoordFormatNV glFogCoordFormatNV;
  GdkGLProc_glVertexAttribFormatNV glVertexAttribFormatNV;
  GdkGLProc_glVertexAttribIFormatNV glVertexAttribIFormatNV;
  GdkGLProc_glGetIntegerui64i_vNV glGetIntegerui64i_vNV;
};

GdkGL_GL_NV_vertex_buffer_unified_memory *gdk_gl_get_GL_NV_vertex_buffer_unified_memory (void);

/*
 * GL_NV_texture_barrier
 */

/* glTextureBarrierNV */
typedef void (APIENTRYP GdkGLProc_glTextureBarrierNV) (void);
GdkGLProc    gdk_gl_get_glTextureBarrierNV (void);
#define      gdk_gl_glTextureBarrierNV(proc) \
  ( ((GdkGLProc_glTextureBarrierNV) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_NV_texture_barrier GdkGL_GL_NV_texture_barrier;

struct _GdkGL_GL_NV_texture_barrier
{
  GdkGLProc_glTextureBarrierNV glTextureBarrierNV;
};

GdkGL_GL_NV_texture_barrier *gdk_gl_get_GL_NV_texture_barrier (void);

/*
 * GL_ATI_blend_equation_separate
 */

/* glBlendEquationSeparateATI */
typedef void (APIENTRY * GdkGLProc_glBlendEquationSeparateATI) (GLenum equationRGB, GLenum equationAlpha);
GdkGLProc    gdk_gl_get_glBlendEquationSeparateATI (void);
#define      gdk_gl_glBlendEquationSeparateATI(proc, equationRGB, equationAlpha) \
  ( ((GdkGLProc_glBlendEquationSeparateATI) (proc)) (equationRGB, equationAlpha) )

/* proc struct */

typedef struct _GdkGL_GL_ATI_blend_equation_separate GdkGL_GL_ATI_blend_equation_separate;

struct _GdkGL_GL_ATI_blend_equation_separate
{
  GdkGLProc_glBlendEquationSeparateATI glBlendEquationSeparateATI;
};

GdkGL_GL_ATI_blend_equation_separate *gdk_gl_get_GL_ATI_blend_equation_separate (void);

/*
 * GL_ATIX_pn_triangles
 */

/* glPNTrianglesiATIX */
typedef void (APIENTRY * GdkGLProc_glPNTrianglesiATIX) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glPNTrianglesiATIX (void);
#define      gdk_gl_glPNTrianglesiATIX(proc, pname, param) \
  ( ((GdkGLProc_glPNTrianglesiATIX) (proc)) (pname, param) )

/* glPNTrianglesfATIX */
typedef void (APIENTRY * GdkGLProc_glPNTrianglesfATIX) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glPNTrianglesfATIX (void);
#define      gdk_gl_glPNTrianglesfATIX(proc, pname, param) \
  ( ((GdkGLProc_glPNTrianglesfATIX) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_ATIX_pn_triangles GdkGL_GL_ATIX_pn_triangles;

struct _GdkGL_GL_ATIX_pn_triangles
{
  GdkGLProc_glPNTrianglesiATIX glPNTrianglesiATIX;
  GdkGLProc_glPNTrianglesfATIX glPNTrianglesfATIX;
};

GdkGL_GL_ATIX_pn_triangles *gdk_gl_get_GL_ATIX_pn_triangles (void);

/*
 * GL_Autodesk_facet_normal
 */

/* glFacetNormal3b */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3b) (GLbyte nx, GLbyte ny, GLbyte nz);
GdkGLProc    gdk_gl_get_glFacetNormal3b (void);
#define      gdk_gl_glFacetNormal3b(proc, nx, ny, nz) \
  ( ((GdkGLProc_glFacetNormal3b) (proc)) (nx, ny, nz) )

/* glFacetNormal3d */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3d) (GLdouble nx, GLdouble ny, GLdouble nz);
GdkGLProc    gdk_gl_get_glFacetNormal3d (void);
#define      gdk_gl_glFacetNormal3d(proc, nx, ny, nz) \
  ( ((GdkGLProc_glFacetNormal3d) (proc)) (nx, ny, nz) )

/* glFacetNormal3f */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
GdkGLProc    gdk_gl_get_glFacetNormal3f (void);
#define      gdk_gl_glFacetNormal3f(proc, nx, ny, nz) \
  ( ((GdkGLProc_glFacetNormal3f) (proc)) (nx, ny, nz) )

/* glFacetNormal3i */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3i) (GLint nx, GLint ny, GLint nz);
GdkGLProc    gdk_gl_get_glFacetNormal3i (void);
#define      gdk_gl_glFacetNormal3i(proc, nx, ny, nz) \
  ( ((GdkGLProc_glFacetNormal3i) (proc)) (nx, ny, nz) )

/* glFacetNormal3s */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3s) (GLshort nx, GLshort ny, GLshort nz);
GdkGLProc    gdk_gl_get_glFacetNormal3s (void);
#define      gdk_gl_glFacetNormal3s(proc, nx, ny, nz) \
  ( ((GdkGLProc_glFacetNormal3s) (proc)) (nx, ny, nz) )

/* glFacetNormal3bv */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3bv) (const GLbyte *v);
GdkGLProc    gdk_gl_get_glFacetNormal3bv (void);
#define      gdk_gl_glFacetNormal3bv(proc, v) \
  ( ((GdkGLProc_glFacetNormal3bv) (proc)) (v) )

/* glFacetNormal3dv */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3dv) (const GLdouble *v);
GdkGLProc    gdk_gl_get_glFacetNormal3dv (void);
#define      gdk_gl_glFacetNormal3dv(proc, v) \
  ( ((GdkGLProc_glFacetNormal3dv) (proc)) (v) )

/* glFacetNormal3fv */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3fv) (const GLfloat *v);
GdkGLProc    gdk_gl_get_glFacetNormal3fv (void);
#define      gdk_gl_glFacetNormal3fv(proc, v) \
  ( ((GdkGLProc_glFacetNormal3fv) (proc)) (v) )

/* glFacetNormal3iv */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3iv) (const GLint *v);
GdkGLProc    gdk_gl_get_glFacetNormal3iv (void);
#define      gdk_gl_glFacetNormal3iv(proc, v) \
  ( ((GdkGLProc_glFacetNormal3iv) (proc)) (v) )

/* glFacetNormal3sv */
typedef void (APIENTRY * GdkGLProc_glFacetNormal3sv) (const GLshort *v);
GdkGLProc    gdk_gl_get_glFacetNormal3sv (void);
#define      gdk_gl_glFacetNormal3sv(proc, v) \
  ( ((GdkGLProc_glFacetNormal3sv) (proc)) (v) )

/* proc struct */

typedef struct _GdkGL_GL_Autodesk_facet_normal GdkGL_GL_Autodesk_facet_normal;

struct _GdkGL_GL_Autodesk_facet_normal
{
  GdkGLProc_glFacetNormal3b glFacetNormal3b;
  GdkGLProc_glFacetNormal3d glFacetNormal3d;
  GdkGLProc_glFacetNormal3f glFacetNormal3f;
  GdkGLProc_glFacetNormal3i glFacetNormal3i;
  GdkGLProc_glFacetNormal3s glFacetNormal3s;
  GdkGLProc_glFacetNormal3bv glFacetNormal3bv;
  GdkGLProc_glFacetNormal3dv glFacetNormal3dv;
  GdkGLProc_glFacetNormal3fv glFacetNormal3fv;
  GdkGLProc_glFacetNormal3iv glFacetNormal3iv;
  GdkGLProc_glFacetNormal3sv glFacetNormal3sv;
};

GdkGL_GL_Autodesk_facet_normal *gdk_gl_get_GL_Autodesk_facet_normal (void);

/*
 * GL_Autodesk_valid_back_buffer_hint
 */

/* glWindowBackBufferHint */
typedef void (APIENTRY * GdkGLProc_glWindowBackBufferHint) (void);
GdkGLProc    gdk_gl_get_glWindowBackBufferHint (void);
#define      gdk_gl_glWindowBackBufferHint(proc) \
  ( ((GdkGLProc_glWindowBackBufferHint) (proc)) () )

/* glValidBackBufferHint */
typedef GLboolean (APIENTRY * GdkGLProc_glValidBackBufferHint) (GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glValidBackBufferHint (void);
#define      gdk_gl_glValidBackBufferHint(proc, x, y, width, height) \
  ( ((GdkGLProc_glValidBackBufferHint) (proc)) (x, y, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_Autodesk_valid_back_buffer_hint GdkGL_GL_Autodesk_valid_back_buffer_hint;

struct _GdkGL_GL_Autodesk_valid_back_buffer_hint
{
  GdkGLProc_glWindowBackBufferHint glWindowBackBufferHint;
  GdkGLProc_glValidBackBufferHint glValidBackBufferHint;
};

GdkGL_GL_Autodesk_valid_back_buffer_hint *gdk_gl_get_GL_Autodesk_valid_back_buffer_hint (void);

/*
 * GL_EXT_fragment_lighting
 */

/* glFragmentLightModelfEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightModelfEXT) (GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentLightModelfEXT (void);
#define      gdk_gl_glFragmentLightModelfEXT(proc, pname, param) \
  ( ((GdkGLProc_glFragmentLightModelfEXT) (proc)) (pname, param) )

/* glFragmentLightModelfvEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightModelfvEXT) (GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentLightModelfvEXT (void);
#define      gdk_gl_glFragmentLightModelfvEXT(proc, pname, params) \
  ( ((GdkGLProc_glFragmentLightModelfvEXT) (proc)) (pname, params) )

/* glFragmentLightModeliEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightModeliEXT) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glFragmentLightModeliEXT (void);
#define      gdk_gl_glFragmentLightModeliEXT(proc, pname, param) \
  ( ((GdkGLProc_glFragmentLightModeliEXT) (proc)) (pname, param) )

/* glFragmentLightModelivEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightModelivEXT) (GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glFragmentLightModelivEXT (void);
#define      gdk_gl_glFragmentLightModelivEXT(proc, pname, params) \
  ( ((GdkGLProc_glFragmentLightModelivEXT) (proc)) (pname, params) )

/* glFragmentLightfEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightfEXT) (GLenum light, GLenum pname, GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentLightfEXT (void);
#define      gdk_gl_glFragmentLightfEXT(proc, light, pname, param) \
  ( ((GdkGLProc_glFragmentLightfEXT) (proc)) (light, pname, param) )

/* glFragmentLightfvEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightfvEXT) (GLenum light, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentLightfvEXT (void);
#define      gdk_gl_glFragmentLightfvEXT(proc, light, pname, params) \
  ( ((GdkGLProc_glFragmentLightfvEXT) (proc)) (light, pname, params) )

/* glFragmentLightiEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightiEXT) (GLenum light, GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glFragmentLightiEXT (void);
#define      gdk_gl_glFragmentLightiEXT(proc, light, pname, param) \
  ( ((GdkGLProc_glFragmentLightiEXT) (proc)) (light, pname, param) )

/* glFragmentLightivEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentLightivEXT) (GLenum light, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glFragmentLightivEXT (void);
#define      gdk_gl_glFragmentLightivEXT(proc, light, pname, params) \
  ( ((GdkGLProc_glFragmentLightivEXT) (proc)) (light, pname, params) )

/* glGetFragmentLightfvEXT */
typedef void (APIENTRY * GdkGLProc_glGetFragmentLightfvEXT) (GLenum light, GLenum pname, GLfloat *params);
GdkGLProc    gdk_gl_get_glGetFragmentLightfvEXT (void);
#define      gdk_gl_glGetFragmentLightfvEXT(proc, light, pname, params) \
  ( ((GdkGLProc_glGetFragmentLightfvEXT) (proc)) (light, pname, params) )

/* glGetFragmentLightivEXT */
typedef void (APIENTRY * GdkGLProc_glGetFragmentLightivEXT) (GLenum light, GLenum pname, GLint *params);
GdkGLProc    gdk_gl_get_glGetFragmentLightivEXT (void);
#define      gdk_gl_glGetFragmentLightivEXT(proc, light, pname, params) \
  ( ((GdkGLProc_glGetFragmentLightivEXT) (proc)) (light, pname, params) )

/* glFragmentMaterialfEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentMaterialfEXT) (GLenum face, GLenum pname, const GLfloat param);
GdkGLProc    gdk_gl_get_glFragmentMaterialfEXT (void);
#define      gdk_gl_glFragmentMaterialfEXT(proc, face, pname, param) \
  ( ((GdkGLProc_glFragmentMaterialfEXT) (proc)) (face, pname, param) )

/* glFragmentMaterialfvEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentMaterialfvEXT) (GLenum face, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glFragmentMaterialfvEXT (void);
#define      gdk_gl_glFragmentMaterialfvEXT(proc, face, pname, params) \
  ( ((GdkGLProc_glFragmentMaterialfvEXT) (proc)) (face, pname, params) )

/* glFragmentMaterialiEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentMaterialiEXT) (GLenum face, GLenum pname, const GLint param);
GdkGLProc    gdk_gl_get_glFragmentMaterialiEXT (void);
#define      gdk_gl_glFragmentMaterialiEXT(proc, face, pname, param) \
  ( ((GdkGLProc_glFragmentMaterialiEXT) (proc)) (face, pname, param) )

/* glFragmentMaterialivEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentMaterialivEXT) (GLenum face, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glFragmentMaterialivEXT (void);
#define      gdk_gl_glFragmentMaterialivEXT(proc, face, pname, params) \
  ( ((GdkGLProc_glFragmentMaterialivEXT) (proc)) (face, pname, params) )

/* glFragmentColorMaterialEXT */
typedef void (APIENTRY * GdkGLProc_glFragmentColorMaterialEXT) (GLenum face, GLenum mode);
GdkGLProc    gdk_gl_get_glFragmentColorMaterialEXT (void);
#define      gdk_gl_glFragmentColorMaterialEXT(proc, face, mode) \
  ( ((GdkGLProc_glFragmentColorMaterialEXT) (proc)) (face, mode) )

/* glGetFragmentMaterialfvEXT */
typedef void (APIENTRY * GdkGLProc_glGetFragmentMaterialfvEXT) (GLenum face, GLenum pname, const GLfloat *params);
GdkGLProc    gdk_gl_get_glGetFragmentMaterialfvEXT (void);
#define      gdk_gl_glGetFragmentMaterialfvEXT(proc, face, pname, params) \
  ( ((GdkGLProc_glGetFragmentMaterialfvEXT) (proc)) (face, pname, params) )

/* glGetFragmentMaterialivEXT */
typedef void (APIENTRY * GdkGLProc_glGetFragmentMaterialivEXT) (GLenum face, GLenum pname, const GLint *params);
GdkGLProc    gdk_gl_get_glGetFragmentMaterialivEXT (void);
#define      gdk_gl_glGetFragmentMaterialivEXT(proc, face, pname, params) \
  ( ((GdkGLProc_glGetFragmentMaterialivEXT) (proc)) (face, pname, params) )

/* glLightEnviEXT */
typedef void (APIENTRY * GdkGLProc_glLightEnviEXT) (GLenum pname, GLint param);
GdkGLProc    gdk_gl_get_glLightEnviEXT (void);
#define      gdk_gl_glLightEnviEXT(proc, pname, param) \
  ( ((GdkGLProc_glLightEnviEXT) (proc)) (pname, param) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_fragment_lighting GdkGL_GL_EXT_fragment_lighting;

struct _GdkGL_GL_EXT_fragment_lighting
{
  GdkGLProc_glFragmentLightModelfEXT glFragmentLightModelfEXT;
  GdkGLProc_glFragmentLightModelfvEXT glFragmentLightModelfvEXT;
  GdkGLProc_glFragmentLightModeliEXT glFragmentLightModeliEXT;
  GdkGLProc_glFragmentLightModelivEXT glFragmentLightModelivEXT;
  GdkGLProc_glFragmentLightfEXT glFragmentLightfEXT;
  GdkGLProc_glFragmentLightfvEXT glFragmentLightfvEXT;
  GdkGLProc_glFragmentLightiEXT glFragmentLightiEXT;
  GdkGLProc_glFragmentLightivEXT glFragmentLightivEXT;
  GdkGLProc_glGetFragmentLightfvEXT glGetFragmentLightfvEXT;
  GdkGLProc_glGetFragmentLightivEXT glGetFragmentLightivEXT;
  GdkGLProc_glFragmentMaterialfEXT glFragmentMaterialfEXT;
  GdkGLProc_glFragmentMaterialfvEXT glFragmentMaterialfvEXT;
  GdkGLProc_glFragmentMaterialiEXT glFragmentMaterialiEXT;
  GdkGLProc_glFragmentMaterialivEXT glFragmentMaterialivEXT;
  GdkGLProc_glFragmentColorMaterialEXT glFragmentColorMaterialEXT;
  GdkGLProc_glGetFragmentMaterialfvEXT glGetFragmentMaterialfvEXT;
  GdkGLProc_glGetFragmentMaterialivEXT glGetFragmentMaterialivEXT;
  GdkGLProc_glLightEnviEXT glLightEnviEXT;
};

GdkGL_GL_EXT_fragment_lighting *gdk_gl_get_GL_EXT_fragment_lighting (void);

/*
 * GL_EXT_multitexture
 */

/* glMultiTexCoord1dEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1dEXT) (GLenum target, GLdouble s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dEXT (void);
#define      gdk_gl_glMultiTexCoord1dEXT(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1dEXT) (proc)) (target, s) )

/* glMultiTexCoord1dvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1dvEXT) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dvEXT (void);
#define      gdk_gl_glMultiTexCoord1dvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1dvEXT) (proc)) (target, v) )

/* glMultiTexCoord1fEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1fEXT) (GLenum target, GLfloat s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fEXT (void);
#define      gdk_gl_glMultiTexCoord1fEXT(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1fEXT) (proc)) (target, s) )

/* glMultiTexCoord1fvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1fvEXT) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fvEXT (void);
#define      gdk_gl_glMultiTexCoord1fvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1fvEXT) (proc)) (target, v) )

/* glMultiTexCoord1iEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1iEXT) (GLenum target, GLint s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1iEXT (void);
#define      gdk_gl_glMultiTexCoord1iEXT(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1iEXT) (proc)) (target, s) )

/* glMultiTexCoord1ivEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1ivEXT) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1ivEXT (void);
#define      gdk_gl_glMultiTexCoord1ivEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1ivEXT) (proc)) (target, v) )

/* glMultiTexCoord1sEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1sEXT) (GLenum target, GLshort s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1sEXT (void);
#define      gdk_gl_glMultiTexCoord1sEXT(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1sEXT) (proc)) (target, s) )

/* glMultiTexCoord1svEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1svEXT) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1svEXT (void);
#define      gdk_gl_glMultiTexCoord1svEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1svEXT) (proc)) (target, v) )

/* glMultiTexCoord2dEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2dEXT) (GLenum target, GLdouble s, GLdouble t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dEXT (void);
#define      gdk_gl_glMultiTexCoord2dEXT(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2dEXT) (proc)) (target, s, t) )

/* glMultiTexCoord2dvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2dvEXT) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dvEXT (void);
#define      gdk_gl_glMultiTexCoord2dvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2dvEXT) (proc)) (target, v) )

/* glMultiTexCoord2fEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2fEXT) (GLenum target, GLfloat s, GLfloat t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fEXT (void);
#define      gdk_gl_glMultiTexCoord2fEXT(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2fEXT) (proc)) (target, s, t) )

/* glMultiTexCoord2fvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2fvEXT) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fvEXT (void);
#define      gdk_gl_glMultiTexCoord2fvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2fvEXT) (proc)) (target, v) )

/* glMultiTexCoord2iEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2iEXT) (GLenum target, GLint s, GLint t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2iEXT (void);
#define      gdk_gl_glMultiTexCoord2iEXT(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2iEXT) (proc)) (target, s, t) )

/* glMultiTexCoord2ivEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2ivEXT) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2ivEXT (void);
#define      gdk_gl_glMultiTexCoord2ivEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2ivEXT) (proc)) (target, v) )

/* glMultiTexCoord2sEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2sEXT) (GLenum target, GLshort s, GLshort t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2sEXT (void);
#define      gdk_gl_glMultiTexCoord2sEXT(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2sEXT) (proc)) (target, s, t) )

/* glMultiTexCoord2svEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2svEXT) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2svEXT (void);
#define      gdk_gl_glMultiTexCoord2svEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2svEXT) (proc)) (target, v) )

/* glMultiTexCoord3dEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3dEXT) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dEXT (void);
#define      gdk_gl_glMultiTexCoord3dEXT(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3dEXT) (proc)) (target, s, t, r) )

/* glMultiTexCoord3dvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3dvEXT) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dvEXT (void);
#define      gdk_gl_glMultiTexCoord3dvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3dvEXT) (proc)) (target, v) )

/* glMultiTexCoord3fEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3fEXT) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fEXT (void);
#define      gdk_gl_glMultiTexCoord3fEXT(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3fEXT) (proc)) (target, s, t, r) )

/* glMultiTexCoord3fvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3fvEXT) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fvEXT (void);
#define      gdk_gl_glMultiTexCoord3fvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3fvEXT) (proc)) (target, v) )

/* glMultiTexCoord3iEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3iEXT) (GLenum target, GLint s, GLint t, GLint r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3iEXT (void);
#define      gdk_gl_glMultiTexCoord3iEXT(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3iEXT) (proc)) (target, s, t, r) )

/* glMultiTexCoord3ivEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3ivEXT) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3ivEXT (void);
#define      gdk_gl_glMultiTexCoord3ivEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3ivEXT) (proc)) (target, v) )

/* glMultiTexCoord3sEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3sEXT) (GLenum target, GLshort s, GLshort t, GLshort r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3sEXT (void);
#define      gdk_gl_glMultiTexCoord3sEXT(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3sEXT) (proc)) (target, s, t, r) )

/* glMultiTexCoord3svEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3svEXT) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3svEXT (void);
#define      gdk_gl_glMultiTexCoord3svEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3svEXT) (proc)) (target, v) )

/* glMultiTexCoord4dEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4dEXT) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dEXT (void);
#define      gdk_gl_glMultiTexCoord4dEXT(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4dEXT) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4dvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4dvEXT) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dvEXT (void);
#define      gdk_gl_glMultiTexCoord4dvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4dvEXT) (proc)) (target, v) )

/* glMultiTexCoord4fEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4fEXT) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fEXT (void);
#define      gdk_gl_glMultiTexCoord4fEXT(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4fEXT) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4fvEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4fvEXT) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fvEXT (void);
#define      gdk_gl_glMultiTexCoord4fvEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4fvEXT) (proc)) (target, v) )

/* glMultiTexCoord4iEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4iEXT) (GLenum target, GLint s, GLint t, GLint r, GLint q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4iEXT (void);
#define      gdk_gl_glMultiTexCoord4iEXT(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4iEXT) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4ivEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4ivEXT) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4ivEXT (void);
#define      gdk_gl_glMultiTexCoord4ivEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4ivEXT) (proc)) (target, v) )

/* glMultiTexCoord4sEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4sEXT) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4sEXT (void);
#define      gdk_gl_glMultiTexCoord4sEXT(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4sEXT) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4svEXT */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4svEXT) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4svEXT (void);
#define      gdk_gl_glMultiTexCoord4svEXT(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4svEXT) (proc)) (target, v) )

/* glInterleavedTextureCoordSetsEXT */
typedef void (APIENTRY * GdkGLProc_glInterleavedTextureCoordSetsEXT) (GLint factor);
GdkGLProc    gdk_gl_get_glInterleavedTextureCoordSetsEXT (void);
#define      gdk_gl_glInterleavedTextureCoordSetsEXT(proc, factor) \
  ( ((GdkGLProc_glInterleavedTextureCoordSetsEXT) (proc)) (factor) )

/* glSelectTextureEXT */
typedef void (APIENTRY * GdkGLProc_glSelectTextureEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glSelectTextureEXT (void);
#define      gdk_gl_glSelectTextureEXT(proc, target) \
  ( ((GdkGLProc_glSelectTextureEXT) (proc)) (target) )

/* glSelectTextureCoordSetEXT */
typedef void (APIENTRY * GdkGLProc_glSelectTextureCoordSetEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glSelectTextureCoordSetEXT (void);
#define      gdk_gl_glSelectTextureCoordSetEXT(proc, target) \
  ( ((GdkGLProc_glSelectTextureCoordSetEXT) (proc)) (target) )

/* glSelectTextureTransformEXT */
typedef void (APIENTRY * GdkGLProc_glSelectTextureTransformEXT) (GLenum target);
GdkGLProc    gdk_gl_get_glSelectTextureTransformEXT (void);
#define      gdk_gl_glSelectTextureTransformEXT(proc, target) \
  ( ((GdkGLProc_glSelectTextureTransformEXT) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_EXT_multitexture GdkGL_GL_EXT_multitexture;

struct _GdkGL_GL_EXT_multitexture
{
  GdkGLProc_glMultiTexCoord1dEXT glMultiTexCoord1dEXT;
  GdkGLProc_glMultiTexCoord1dvEXT glMultiTexCoord1dvEXT;
  GdkGLProc_glMultiTexCoord1fEXT glMultiTexCoord1fEXT;
  GdkGLProc_glMultiTexCoord1fvEXT glMultiTexCoord1fvEXT;
  GdkGLProc_glMultiTexCoord1iEXT glMultiTexCoord1iEXT;
  GdkGLProc_glMultiTexCoord1ivEXT glMultiTexCoord1ivEXT;
  GdkGLProc_glMultiTexCoord1sEXT glMultiTexCoord1sEXT;
  GdkGLProc_glMultiTexCoord1svEXT glMultiTexCoord1svEXT;
  GdkGLProc_glMultiTexCoord2dEXT glMultiTexCoord2dEXT;
  GdkGLProc_glMultiTexCoord2dvEXT glMultiTexCoord2dvEXT;
  GdkGLProc_glMultiTexCoord2fEXT glMultiTexCoord2fEXT;
  GdkGLProc_glMultiTexCoord2fvEXT glMultiTexCoord2fvEXT;
  GdkGLProc_glMultiTexCoord2iEXT glMultiTexCoord2iEXT;
  GdkGLProc_glMultiTexCoord2ivEXT glMultiTexCoord2ivEXT;
  GdkGLProc_glMultiTexCoord2sEXT glMultiTexCoord2sEXT;
  GdkGLProc_glMultiTexCoord2svEXT glMultiTexCoord2svEXT;
  GdkGLProc_glMultiTexCoord3dEXT glMultiTexCoord3dEXT;
  GdkGLProc_glMultiTexCoord3dvEXT glMultiTexCoord3dvEXT;
  GdkGLProc_glMultiTexCoord3fEXT glMultiTexCoord3fEXT;
  GdkGLProc_glMultiTexCoord3fvEXT glMultiTexCoord3fvEXT;
  GdkGLProc_glMultiTexCoord3iEXT glMultiTexCoord3iEXT;
  GdkGLProc_glMultiTexCoord3ivEXT glMultiTexCoord3ivEXT;
  GdkGLProc_glMultiTexCoord3sEXT glMultiTexCoord3sEXT;
  GdkGLProc_glMultiTexCoord3svEXT glMultiTexCoord3svEXT;
  GdkGLProc_glMultiTexCoord4dEXT glMultiTexCoord4dEXT;
  GdkGLProc_glMultiTexCoord4dvEXT glMultiTexCoord4dvEXT;
  GdkGLProc_glMultiTexCoord4fEXT glMultiTexCoord4fEXT;
  GdkGLProc_glMultiTexCoord4fvEXT glMultiTexCoord4fvEXT;
  GdkGLProc_glMultiTexCoord4iEXT glMultiTexCoord4iEXT;
  GdkGLProc_glMultiTexCoord4ivEXT glMultiTexCoord4ivEXT;
  GdkGLProc_glMultiTexCoord4sEXT glMultiTexCoord4sEXT;
  GdkGLProc_glMultiTexCoord4svEXT glMultiTexCoord4svEXT;
  GdkGLProc_glInterleavedTextureCoordSetsEXT glInterleavedTextureCoordSetsEXT;
  GdkGLProc_glSelectTextureEXT glSelectTextureEXT;
  GdkGLProc_glSelectTextureCoordSetEXT glSelectTextureCoordSetEXT;
  GdkGLProc_glSelectTextureTransformEXT glSelectTextureTransformEXT;
};

GdkGL_GL_EXT_multitexture *gdk_gl_get_GL_EXT_multitexture (void);

/*
 * GL_EXT_scene_marker
 */

/* glBeginSceneEXT */
typedef void (APIENTRY * GdkGLProc_glBeginSceneEXT) (void);
GdkGLProc    gdk_gl_get_glBeginSceneEXT (void);
#define      gdk_gl_glBeginSceneEXT(proc) \
  ( ((GdkGLProc_glBeginSceneEXT) (proc)) () )

/* glEndSceneEXT */
typedef void (APIENTRY * GdkGLProc_glEndSceneEXT) (void);
GdkGLProc    gdk_gl_get_glEndSceneEXT (void);
#define      gdk_gl_glEndSceneEXT(proc) \
  ( ((GdkGLProc_glEndSceneEXT) (proc)) () )

/* proc struct */

typedef struct _GdkGL_GL_EXT_scene_marker GdkGL_GL_EXT_scene_marker;

struct _GdkGL_GL_EXT_scene_marker
{
  GdkGLProc_glBeginSceneEXT glBeginSceneEXT;
  GdkGLProc_glEndSceneEXT glEndSceneEXT;
};

GdkGL_GL_EXT_scene_marker *gdk_gl_get_GL_EXT_scene_marker (void);

/*
 * GL_IBM_static_data
 */

/* glFlushStaticDataIBM */
typedef void (APIENTRY * GdkGLProc_glFlushStaticDataIBM) (GLenum target);
GdkGLProc    gdk_gl_get_glFlushStaticDataIBM (void);
#define      gdk_gl_glFlushStaticDataIBM(proc, target) \
  ( ((GdkGLProc_glFlushStaticDataIBM) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_IBM_static_data GdkGL_GL_IBM_static_data;

struct _GdkGL_GL_IBM_static_data
{
  GdkGLProc_glFlushStaticDataIBM glFlushStaticDataIBM;
};

GdkGL_GL_IBM_static_data *gdk_gl_get_GL_IBM_static_data (void);

/*
 * GL_KTX_buffer_region
 */

/* glBufferRegionEnabled */
typedef GLuint (APIENTRY * GdkGLProc_glBufferRegionEnabled) (void);
GdkGLProc    gdk_gl_get_glBufferRegionEnabled (void);
#define      gdk_gl_glBufferRegionEnabled(proc) \
  ( ((GdkGLProc_glBufferRegionEnabled) (proc)) () )

/* glNewBufferRegion */
typedef GLuint (APIENTRY * GdkGLProc_glNewBufferRegion) (GLenum region);
GdkGLProc    gdk_gl_get_glNewBufferRegion (void);
#define      gdk_gl_glNewBufferRegion(proc, region) \
  ( ((GdkGLProc_glNewBufferRegion) (proc)) (region) )

/* glDeleteBufferRegion */
typedef void (APIENTRY * GdkGLProc_glDeleteBufferRegion) (GLenum region);
GdkGLProc    gdk_gl_get_glDeleteBufferRegion (void);
#define      gdk_gl_glDeleteBufferRegion(proc, region) \
  ( ((GdkGLProc_glDeleteBufferRegion) (proc)) (region) )

/* glReadBufferRegion */
typedef void (APIENTRY * GdkGLProc_glReadBufferRegion) (GLuint region, GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glReadBufferRegion (void);
#define      gdk_gl_glReadBufferRegion(proc, region, x, y, width, height) \
  ( ((GdkGLProc_glReadBufferRegion) (proc)) (region, x, y, width, height) )

/* glDrawBufferRegion */
typedef void (APIENTRY * GdkGLProc_glDrawBufferRegion) (GLuint region, GLint x, GLint y, GLsizei width, GLsizei height, GLint xDest, GLint yDest);
GdkGLProc    gdk_gl_get_glDrawBufferRegion (void);
#define      gdk_gl_glDrawBufferRegion(proc, region, x, y, width, height, xDest, yDest) \
  ( ((GdkGLProc_glDrawBufferRegion) (proc)) (region, x, y, width, height, xDest, yDest) )

/* proc struct */

typedef struct _GdkGL_GL_KTX_buffer_region GdkGL_GL_KTX_buffer_region;

struct _GdkGL_GL_KTX_buffer_region
{
  GdkGLProc_glBufferRegionEnabled glBufferRegionEnabled;
  GdkGLProc_glNewBufferRegion glNewBufferRegion;
  GdkGLProc_glDeleteBufferRegion glDeleteBufferRegion;
  GdkGLProc_glReadBufferRegion glReadBufferRegion;
  GdkGLProc_glDrawBufferRegion glDrawBufferRegion;
};

GdkGL_GL_KTX_buffer_region *gdk_gl_get_GL_KTX_buffer_region (void);

/*
 * GL_NV_element_array
 */

/* glElementPointerNV */
typedef void (APIENTRY * GdkGLProc_glElementPointerNV) (GLenum type, const GLvoid *pointer);
GdkGLProc    gdk_gl_get_glElementPointerNV (void);
#define      gdk_gl_glElementPointerNV(proc, type, pointer) \
  ( ((GdkGLProc_glElementPointerNV) (proc)) (type, pointer) )

/* glDrawElementArrayNV */
typedef void (APIENTRY * GdkGLProc_glDrawElementArrayNV) (GLenum mode, GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawElementArrayNV (void);
#define      gdk_gl_glDrawElementArrayNV(proc, mode, first, count) \
  ( ((GdkGLProc_glDrawElementArrayNV) (proc)) (mode, first, count) )

/* glDrawRangeElementArrayNV */
typedef void (APIENTRY * GdkGLProc_glDrawRangeElementArrayNV) (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count);
GdkGLProc    gdk_gl_get_glDrawRangeElementArrayNV (void);
#define      gdk_gl_glDrawRangeElementArrayNV(proc, mode, start, end, first, count) \
  ( ((GdkGLProc_glDrawRangeElementArrayNV) (proc)) (mode, start, end, first, count) )

/* glMultiDrawElementArrayNV */
typedef void (APIENTRY * GdkGLProc_glMultiDrawElementArrayNV) (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawElementArrayNV (void);
#define      gdk_gl_glMultiDrawElementArrayNV(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawElementArrayNV) (proc)) (mode, first, count, primcount) )

/* glMultiDrawRangeElementArrayNV */
typedef void (APIENTRY * GdkGLProc_glMultiDrawRangeElementArrayNV) (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawRangeElementArrayNV (void);
#define      gdk_gl_glMultiDrawRangeElementArrayNV(proc, mode, start, end, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawRangeElementArrayNV) (proc)) (mode, start, end, first, count, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_NV_element_array GdkGL_GL_NV_element_array;

struct _GdkGL_GL_NV_element_array
{
  GdkGLProc_glElementPointerNV glElementPointerNV;
  GdkGLProc_glDrawElementArrayNV glDrawElementArrayNV;
  GdkGLProc_glDrawRangeElementArrayNV glDrawRangeElementArrayNV;
  GdkGLProc_glMultiDrawElementArrayNV glMultiDrawElementArrayNV;
  GdkGLProc_glMultiDrawRangeElementArrayNV glMultiDrawRangeElementArrayNV;
};

GdkGL_GL_NV_element_array *gdk_gl_get_GL_NV_element_array (void);

/*
 * GL_NV_stencil_two_side
 */

/* glActiveStencilFaceNV */
typedef void (APIENTRY * GdkGLProc_glActiveStencilFaceNV) (GLenum face);
GdkGLProc    gdk_gl_get_glActiveStencilFaceNV (void);
#define      gdk_gl_glActiveStencilFaceNV(proc, face) \
  ( ((GdkGLProc_glActiveStencilFaceNV) (proc)) (face) )

/* proc struct */

typedef struct _GdkGL_GL_NV_stencil_two_side GdkGL_GL_NV_stencil_two_side;

struct _GdkGL_GL_NV_stencil_two_side
{
  GdkGLProc_glActiveStencilFaceNV glActiveStencilFaceNV;
};

GdkGL_GL_NV_stencil_two_side *gdk_gl_get_GL_NV_stencil_two_side (void);

/*
 * GL_SGIS_multitexture
 */

/* glMultiTexCoord1dSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1dSGIS) (GLenum target, GLdouble s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dSGIS (void);
#define      gdk_gl_glMultiTexCoord1dSGIS(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1dSGIS) (proc)) (target, s) )

/* glMultiTexCoord1dvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1dvSGIS) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1dvSGIS (void);
#define      gdk_gl_glMultiTexCoord1dvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1dvSGIS) (proc)) (target, v) )

/* glMultiTexCoord1fSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1fSGIS) (GLenum target, GLfloat s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fSGIS (void);
#define      gdk_gl_glMultiTexCoord1fSGIS(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1fSGIS) (proc)) (target, s) )

/* glMultiTexCoord1fvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1fvSGIS) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1fvSGIS (void);
#define      gdk_gl_glMultiTexCoord1fvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1fvSGIS) (proc)) (target, v) )

/* glMultiTexCoord1iSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1iSGIS) (GLenum target, GLint s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1iSGIS (void);
#define      gdk_gl_glMultiTexCoord1iSGIS(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1iSGIS) (proc)) (target, s) )

/* glMultiTexCoord1ivSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1ivSGIS) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1ivSGIS (void);
#define      gdk_gl_glMultiTexCoord1ivSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1ivSGIS) (proc)) (target, v) )

/* glMultiTexCoord1sSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1sSGIS) (GLenum target, GLshort s);
GdkGLProc    gdk_gl_get_glMultiTexCoord1sSGIS (void);
#define      gdk_gl_glMultiTexCoord1sSGIS(proc, target, s) \
  ( ((GdkGLProc_glMultiTexCoord1sSGIS) (proc)) (target, s) )

/* glMultiTexCoord1svSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord1svSGIS) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord1svSGIS (void);
#define      gdk_gl_glMultiTexCoord1svSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord1svSGIS) (proc)) (target, v) )

/* glMultiTexCoord2dSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2dSGIS) (GLenum target, GLdouble s, GLdouble t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dSGIS (void);
#define      gdk_gl_glMultiTexCoord2dSGIS(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2dSGIS) (proc)) (target, s, t) )

/* glMultiTexCoord2dvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2dvSGIS) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2dvSGIS (void);
#define      gdk_gl_glMultiTexCoord2dvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2dvSGIS) (proc)) (target, v) )

/* glMultiTexCoord2fSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2fSGIS) (GLenum target, GLfloat s, GLfloat t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fSGIS (void);
#define      gdk_gl_glMultiTexCoord2fSGIS(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2fSGIS) (proc)) (target, s, t) )

/* glMultiTexCoord2fvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2fvSGIS) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2fvSGIS (void);
#define      gdk_gl_glMultiTexCoord2fvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2fvSGIS) (proc)) (target, v) )

/* glMultiTexCoord2iSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2iSGIS) (GLenum target, GLint s, GLint t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2iSGIS (void);
#define      gdk_gl_glMultiTexCoord2iSGIS(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2iSGIS) (proc)) (target, s, t) )

/* glMultiTexCoord2ivSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2ivSGIS) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2ivSGIS (void);
#define      gdk_gl_glMultiTexCoord2ivSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2ivSGIS) (proc)) (target, v) )

/* glMultiTexCoord2sSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2sSGIS) (GLenum target, GLshort s, GLshort t);
GdkGLProc    gdk_gl_get_glMultiTexCoord2sSGIS (void);
#define      gdk_gl_glMultiTexCoord2sSGIS(proc, target, s, t) \
  ( ((GdkGLProc_glMultiTexCoord2sSGIS) (proc)) (target, s, t) )

/* glMultiTexCoord2svSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord2svSGIS) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord2svSGIS (void);
#define      gdk_gl_glMultiTexCoord2svSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord2svSGIS) (proc)) (target, v) )

/* glMultiTexCoord3dSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3dSGIS) (GLenum target, GLdouble s, GLdouble t, GLdouble r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dSGIS (void);
#define      gdk_gl_glMultiTexCoord3dSGIS(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3dSGIS) (proc)) (target, s, t, r) )

/* glMultiTexCoord3dvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3dvSGIS) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3dvSGIS (void);
#define      gdk_gl_glMultiTexCoord3dvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3dvSGIS) (proc)) (target, v) )

/* glMultiTexCoord3fSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3fSGIS) (GLenum target, GLfloat s, GLfloat t, GLfloat r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fSGIS (void);
#define      gdk_gl_glMultiTexCoord3fSGIS(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3fSGIS) (proc)) (target, s, t, r) )

/* glMultiTexCoord3fvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3fvSGIS) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3fvSGIS (void);
#define      gdk_gl_glMultiTexCoord3fvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3fvSGIS) (proc)) (target, v) )

/* glMultiTexCoord3iSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3iSGIS) (GLenum target, GLint s, GLint t, GLint r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3iSGIS (void);
#define      gdk_gl_glMultiTexCoord3iSGIS(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3iSGIS) (proc)) (target, s, t, r) )

/* glMultiTexCoord3ivSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3ivSGIS) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3ivSGIS (void);
#define      gdk_gl_glMultiTexCoord3ivSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3ivSGIS) (proc)) (target, v) )

/* glMultiTexCoord3sSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3sSGIS) (GLenum target, GLshort s, GLshort t, GLshort r);
GdkGLProc    gdk_gl_get_glMultiTexCoord3sSGIS (void);
#define      gdk_gl_glMultiTexCoord3sSGIS(proc, target, s, t, r) \
  ( ((GdkGLProc_glMultiTexCoord3sSGIS) (proc)) (target, s, t, r) )

/* glMultiTexCoord3svSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord3svSGIS) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord3svSGIS (void);
#define      gdk_gl_glMultiTexCoord3svSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord3svSGIS) (proc)) (target, v) )

/* glMultiTexCoord4dSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4dSGIS) (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dSGIS (void);
#define      gdk_gl_glMultiTexCoord4dSGIS(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4dSGIS) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4dvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4dvSGIS) (GLenum target, const GLdouble *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4dvSGIS (void);
#define      gdk_gl_glMultiTexCoord4dvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4dvSGIS) (proc)) (target, v) )

/* glMultiTexCoord4fSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4fSGIS) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fSGIS (void);
#define      gdk_gl_glMultiTexCoord4fSGIS(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4fSGIS) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4fvSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4fvSGIS) (GLenum target, const GLfloat *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4fvSGIS (void);
#define      gdk_gl_glMultiTexCoord4fvSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4fvSGIS) (proc)) (target, v) )

/* glMultiTexCoord4iSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4iSGIS) (GLenum target, GLint s, GLint t, GLint r, GLint q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4iSGIS (void);
#define      gdk_gl_glMultiTexCoord4iSGIS(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4iSGIS) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4ivSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4ivSGIS) (GLenum target, const GLint *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4ivSGIS (void);
#define      gdk_gl_glMultiTexCoord4ivSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4ivSGIS) (proc)) (target, v) )

/* glMultiTexCoord4sSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4sSGIS) (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
GdkGLProc    gdk_gl_get_glMultiTexCoord4sSGIS (void);
#define      gdk_gl_glMultiTexCoord4sSGIS(proc, target, s, t, r, q) \
  ( ((GdkGLProc_glMultiTexCoord4sSGIS) (proc)) (target, s, t, r, q) )

/* glMultiTexCoord4svSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoord4svSGIS) (GLenum target, const GLshort *v);
GdkGLProc    gdk_gl_get_glMultiTexCoord4svSGIS (void);
#define      gdk_gl_glMultiTexCoord4svSGIS(proc, target, v) \
  ( ((GdkGLProc_glMultiTexCoord4svSGIS) (proc)) (target, v) )

/* glMultiTexCoordPointerSGIS */
typedef void (APIENTRY * GdkGLProc_glMultiTexCoordPointerSGIS) (GLenum target, GLint size, GLenum type, GLsizei stride, const void *pointer);
GdkGLProc    gdk_gl_get_glMultiTexCoordPointerSGIS (void);
#define      gdk_gl_glMultiTexCoordPointerSGIS(proc, target, size, type, stride, pointer) \
  ( ((GdkGLProc_glMultiTexCoordPointerSGIS) (proc)) (target, size, type, stride, pointer) )

/* glSelectTextureSGIS */
typedef void (APIENTRY * GdkGLProc_glSelectTextureSGIS) (GLenum target);
GdkGLProc    gdk_gl_get_glSelectTextureSGIS (void);
#define      gdk_gl_glSelectTextureSGIS(proc, target) \
  ( ((GdkGLProc_glSelectTextureSGIS) (proc)) (target) )

/* glSelectTextureCoordSetSGIS */
typedef void (APIENTRY * GdkGLProc_glSelectTextureCoordSetSGIS) (GLenum target);
GdkGLProc    gdk_gl_get_glSelectTextureCoordSetSGIS (void);
#define      gdk_gl_glSelectTextureCoordSetSGIS(proc, target) \
  ( ((GdkGLProc_glSelectTextureCoordSetSGIS) (proc)) (target) )

/* proc struct */

typedef struct _GdkGL_GL_SGIS_multitexture GdkGL_GL_SGIS_multitexture;

struct _GdkGL_GL_SGIS_multitexture
{
  GdkGLProc_glMultiTexCoord1dSGIS glMultiTexCoord1dSGIS;
  GdkGLProc_glMultiTexCoord1dvSGIS glMultiTexCoord1dvSGIS;
  GdkGLProc_glMultiTexCoord1fSGIS glMultiTexCoord1fSGIS;
  GdkGLProc_glMultiTexCoord1fvSGIS glMultiTexCoord1fvSGIS;
  GdkGLProc_glMultiTexCoord1iSGIS glMultiTexCoord1iSGIS;
  GdkGLProc_glMultiTexCoord1ivSGIS glMultiTexCoord1ivSGIS;
  GdkGLProc_glMultiTexCoord1sSGIS glMultiTexCoord1sSGIS;
  GdkGLProc_glMultiTexCoord1svSGIS glMultiTexCoord1svSGIS;
  GdkGLProc_glMultiTexCoord2dSGIS glMultiTexCoord2dSGIS;
  GdkGLProc_glMultiTexCoord2dvSGIS glMultiTexCoord2dvSGIS;
  GdkGLProc_glMultiTexCoord2fSGIS glMultiTexCoord2fSGIS;
  GdkGLProc_glMultiTexCoord2fvSGIS glMultiTexCoord2fvSGIS;
  GdkGLProc_glMultiTexCoord2iSGIS glMultiTexCoord2iSGIS;
  GdkGLProc_glMultiTexCoord2ivSGIS glMultiTexCoord2ivSGIS;
  GdkGLProc_glMultiTexCoord2sSGIS glMultiTexCoord2sSGIS;
  GdkGLProc_glMultiTexCoord2svSGIS glMultiTexCoord2svSGIS;
  GdkGLProc_glMultiTexCoord3dSGIS glMultiTexCoord3dSGIS;
  GdkGLProc_glMultiTexCoord3dvSGIS glMultiTexCoord3dvSGIS;
  GdkGLProc_glMultiTexCoord3fSGIS glMultiTexCoord3fSGIS;
  GdkGLProc_glMultiTexCoord3fvSGIS glMultiTexCoord3fvSGIS;
  GdkGLProc_glMultiTexCoord3iSGIS glMultiTexCoord3iSGIS;
  GdkGLProc_glMultiTexCoord3ivSGIS glMultiTexCoord3ivSGIS;
  GdkGLProc_glMultiTexCoord3sSGIS glMultiTexCoord3sSGIS;
  GdkGLProc_glMultiTexCoord3svSGIS glMultiTexCoord3svSGIS;
  GdkGLProc_glMultiTexCoord4dSGIS glMultiTexCoord4dSGIS;
  GdkGLProc_glMultiTexCoord4dvSGIS glMultiTexCoord4dvSGIS;
  GdkGLProc_glMultiTexCoord4fSGIS glMultiTexCoord4fSGIS;
  GdkGLProc_glMultiTexCoord4fvSGIS glMultiTexCoord4fvSGIS;
  GdkGLProc_glMultiTexCoord4iSGIS glMultiTexCoord4iSGIS;
  GdkGLProc_glMultiTexCoord4ivSGIS glMultiTexCoord4ivSGIS;
  GdkGLProc_glMultiTexCoord4sSGIS glMultiTexCoord4sSGIS;
  GdkGLProc_glMultiTexCoord4svSGIS glMultiTexCoord4svSGIS;
  GdkGLProc_glMultiTexCoordPointerSGIS glMultiTexCoordPointerSGIS;
  GdkGLProc_glSelectTextureSGIS glSelectTextureSGIS;
  GdkGLProc_glSelectTextureCoordSetSGIS glSelectTextureCoordSetSGIS;
};

GdkGL_GL_SGIS_multitexture *gdk_gl_get_GL_SGIS_multitexture (void);

/*
 * GL_SGIX_fog_texture
 */

/* glTextureFogSGIX */
typedef void (APIENTRY * GdkGLProc_glTextureFogSGIX) (GLenum pname);
GdkGLProc    gdk_gl_get_glTextureFogSGIX (void);
#define      gdk_gl_glTextureFogSGIX(proc, pname) \
  ( ((GdkGLProc_glTextureFogSGIX) (proc)) (pname) )

/* proc struct */

typedef struct _GdkGL_GL_SGIX_fog_texture GdkGL_GL_SGIX_fog_texture;

struct _GdkGL_GL_SGIX_fog_texture
{
  GdkGLProc_glTextureFogSGIX glTextureFogSGIX;
};

GdkGL_GL_SGIX_fog_texture *gdk_gl_get_GL_SGIX_fog_texture (void);

/*
 * GL_SUN_multi_draw_arrays
 */

/* glMultiDrawArraysSUN */
typedef void (APIENTRY * GdkGLProc_glMultiDrawArraysSUN) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawArraysSUN (void);
#define      gdk_gl_glMultiDrawArraysSUN(proc, mode, first, count, primcount) \
  ( ((GdkGLProc_glMultiDrawArraysSUN) (proc)) (mode, first, count, primcount) )

/* glMultiDrawElementsSUN */
typedef void (APIENTRY * GdkGLProc_glMultiDrawElementsSUN) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
GdkGLProc    gdk_gl_get_glMultiDrawElementsSUN (void);
#define      gdk_gl_glMultiDrawElementsSUN(proc, mode, count, type, indices, primcount) \
  ( ((GdkGLProc_glMultiDrawElementsSUN) (proc)) (mode, count, type, indices, primcount) )

/* proc struct */

typedef struct _GdkGL_GL_SUN_multi_draw_arrays GdkGL_GL_SUN_multi_draw_arrays;

struct _GdkGL_GL_SUN_multi_draw_arrays
{
  GdkGLProc_glMultiDrawArraysSUN glMultiDrawArraysSUN;
  GdkGLProc_glMultiDrawElementsSUN glMultiDrawElementsSUN;
};

GdkGL_GL_SUN_multi_draw_arrays *gdk_gl_get_GL_SUN_multi_draw_arrays (void);

/*
 * GL_WIN_swap_hint
 */

/* glAddSwapHintRectWIN */
typedef void (APIENTRY * GdkGLProc_glAddSwapHintRectWIN) (GLint x, GLint y, GLsizei width, GLsizei height);
GdkGLProc    gdk_gl_get_glAddSwapHintRectWIN (void);
#define      gdk_gl_glAddSwapHintRectWIN(proc, x, y, width, height) \
  ( ((GdkGLProc_glAddSwapHintRectWIN) (proc)) (x, y, width, height) )

/* proc struct */

typedef struct _GdkGL_GL_WIN_swap_hint GdkGL_GL_WIN_swap_hint;

struct _GdkGL_GL_WIN_swap_hint
{
  GdkGLProc_glAddSwapHintRectWIN glAddSwapHintRectWIN;
};

GdkGL_GL_WIN_swap_hint *gdk_gl_get_GL_WIN_swap_hint (void);

G_END_DECLS

#endif /* __GDK_GL_GLEXT_H__ */
