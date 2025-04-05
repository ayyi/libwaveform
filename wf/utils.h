/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2024 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __wf_private__

#ifndef g_free0
#define g_free0(var) ((var == NULL) ? NULL : (var = (g_free(var), NULL)))
#endif

#ifndef g_list_free0
#define g_list_free0(var) ((var == NULL) ? NULL : (var = (g_list_free (var), NULL)))
#endif

#ifndef g_error_free0
#define g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#endif

#ifndef call
#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)
#endif

#endif // __wf_private__

#define WF_NEW(T, ...) ({ T* obj = g_new0(T, 1); *obj = (T){__VA_ARGS__}; obj; })

#define set_str(P, NAME) ({ if (P) g_free(P); P = NAME; })
#define wf_set_gobject(P, VAL) ({ if (P) g_object_unref(P); P = g_object_ref(VAL); })

bool       wf_get_filename_for_other_channel (const char* filename, char* other, int n_chars);
