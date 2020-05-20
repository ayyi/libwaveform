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
#include "config.h"
#include <gdk/gdk.h>
#include "agl/debug.h"
#include "agl/utils.h"
#include "agl/ext.h"
#include "text/types.h"
#include "text/shaderbuilder.h"


#define AGL_GL_ERROR (gl_error_quark ())

/*
 *  Return an integer corresponding to the string "glerror"
 */
GQuark
gl_error_quark (void)
{
	return g_quark_from_string("glerror");
}


void
shader_builder_init (GskGLShaderBuilder* self, const char* common_preamble_resource_path, const char* vs_preamble_resource_path, const char* fs_preamble_resource_path)
{
	memset (self, 0, sizeof (*self));

#ifdef G_ENABLE_DEBUG
	GError* error = NULL;
	char** resources = g_resources_enumerate_children ("/org/ayyi/agl/resources/shaders", 0, &error);
	if(error){
		printf("%s\n", error->message);
		g_error_free(error);
	}
	if(resources){
		int i = 0;
		while(resources[i]){
			printf(" %s\n", resources[i]);
			i++;
		}
		g_strfreev(resources);
	}
#endif

	self->preamble = g_resources_lookup_data (common_preamble_resource_path, 0, NULL);
	self->vs_preamble = g_resources_lookup_data (vs_preamble_resource_path, 0, NULL);
	self->fs_preamble = g_resources_lookup_data (fs_preamble_resource_path, 0, NULL);

	g_assert (self->preamble);
	g_assert (self->vs_preamble);
	g_assert (self->fs_preamble);
}


void
shader_builder_finish (GskGLShaderBuilder *self)
{
	g_bytes_unref (self->preamble);
	g_bytes_unref (self->vs_preamble);
	g_bytes_unref (self->fs_preamble);
}


void
shader_builder_set_glsl_version (GskGLShaderBuilder* self, int version)
{
	self->version = version;
}


static gboolean
check_shader_error (int shader_id, GError** error)
{
	int status;
	glGetShaderiv (shader_id, GL_COMPILE_STATUS, &status);

	if (G_LIKELY (status == GL_TRUE))
		return true;

	int log_len;
	glGetShaderiv (shader_id, GL_INFO_LOG_LENGTH, &log_len);
	char* buffer = g_malloc0 (log_len + 1);
	glGetShaderInfoLog (shader_id, log_len, NULL, buffer);

	int code_len;
	glGetShaderiv (shader_id, GL_SHADER_SOURCE_LENGTH, &code_len);
	char* code = g_malloc0 (code_len + 1);
	glGetShaderSource (shader_id, code_len, NULL, code);

	g_set_error(
		error,
		AGL_GL_ERROR,
		AGL_GL_ERROR_COMPILATION_FAILED,
		"Compilation failure in shader.\nError message: %s\n\nSource code:\n%s\n\n",
		buffer,
		code
	);

	g_free (buffer);
	g_free (code);

	return FALSE;
}


int
shader_builder_create_program (GskGLShaderBuilder* self, const char* resource_path, GError** error)
{
	GBytes* source_bytes = g_resources_lookup_data (resource_path, 0, NULL);
	char version_buffer[64];
	int program_id = -1;

	g_assert (source_bytes);

	const char* source = g_bytes_get_data (source_bytes, NULL);
	const char* vertex_shader_start = strstr (source, "VERTEX_SHADER");
	const char* fragment_shader_start = strstr (source, "FRAGMENT_SHADER");

	g_assert (vertex_shader_start);
	g_assert (fragment_shader_start);

	/* They both start at the next newline */
	vertex_shader_start = strstr (vertex_shader_start, "\n");
	fragment_shader_start = strstr (fragment_shader_start, "\n");

	g_snprintf (version_buffer, sizeof(version_buffer), "#version %d\n", self->version);

	int vertex_id = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource(
		vertex_id,
		8,
		(const char* []){
			version_buffer,
			self->debugging ? "#define GSK_DEBUG 1\n" : "",
			self->legacy ? "#define GSK_LEGACY 1\n" : "",
			self->gl3 ? "#define GSK_GL3 1\n" : "",
			self->gles ? "#define GSK_GLES 1\n" : "",
			g_bytes_get_data (self->preamble, NULL),
			g_bytes_get_data (self->vs_preamble, NULL),
			vertex_shader_start
		},
		(int[]) {
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			fragment_shader_start - vertex_shader_start - 12
		}
	);
	glCompileShader (vertex_id);

	if (!check_shader_error (vertex_id, error)) {
		glDeleteShader (vertex_id);
		goto out;
	}

	int fragment_id = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource(
		fragment_id,
		8,
		(const char* []) {
			version_buffer,
			self->debugging ? "#define GSK_DEBUG 1\n" : "",
			self->legacy ? "#define GSK_LEGACY 1\n" : "",
			self->gl3 ? "#define GSK_GL3 1\n" : "",
			self->gles ? "#define GSK_GLES 1\n" : "",
			g_bytes_get_data (self->preamble, NULL),
			g_bytes_get_data (self->fs_preamble, NULL),
			fragment_shader_start
		},
		(int[]) {
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
			-1,
		}
	);
	glCompileShader (fragment_id);

	if (!check_shader_error (fragment_id, error)){
		glDeleteShader (fragment_id);
		goto out;
	}

	program_id = glCreateProgram ();
	glAttachShader (program_id, vertex_id);
	glAttachShader (program_id, fragment_id);
	glLinkProgram (program_id);

	dbg(1, "%i (%s)", program_id, resource_path);

	int status;
	glGetProgramiv (program_id, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		int log_len = 0;

		glGetProgramiv (program_id, GL_INFO_LOG_LENGTH, &log_len);

		char* buffer = g_malloc0 (log_len + 1);
		glGetProgramInfoLog (program_id, log_len, NULL, buffer);

		g_warning ("Linking failure in shader:\n%s", buffer);
		g_set_error (error, AGL_GL_ERROR, AGL_GL_ERROR_LINK_FAILED, "Linking failure in shader: %s", buffer);

		g_free (buffer);

		glDeleteProgram (program_id);

		goto out;
	}

	glDetachShader (program_id, vertex_id);
	glDeleteShader (vertex_id);

	glDetachShader (program_id, fragment_id);
	glDeleteShader (fragment_id);

  out:
	g_bytes_unref (source_bytes);

	return program_id;
}


