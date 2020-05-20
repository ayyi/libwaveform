/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) GTK+ Team and others                                   |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __shader_builder_h__
#define __shader_builder_h__

G_BEGIN_DECLS

typedef struct
{
  GBytes* preamble;
  GBytes* vs_preamble;
  GBytes* fs_preamble;

  int version;

  guint debugging: 1;
  guint gles: 1;
  guint gl3: 1;
  guint legacy: 1;

} GskGLShaderBuilder;


void shader_builder_init             (GskGLShaderBuilder*,
                                      const char* common_preamble_resource_path,
                                      const char* vs_preamble_resource_path,
                                      const char* fs_preamble_resource_path);
void shader_builder_finish           (GskGLShaderBuilder*);

void shader_builder_set_glsl_version (GskGLShaderBuilder*, int);

int  shader_builder_create_program   (GskGLShaderBuilder*, const char* resource_path, GError**);

G_END_DECLS

#endif
