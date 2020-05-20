/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_utils_c__
#define __wf_private__

#include <glib.h>
#include "wf/debug.h"
#include "wf/utils.h"

int wf_debug = 0;


/*
 *  Return the filename of the other half of a split stereo pair.
 */
bool
wf_get_filename_for_other_channel (const char* filename, char* other, int n_chars)
{
	g_strlcpy(other, filename, n_chars);

	gchar* p = g_strrstr(other, "%L.");
	if(p){
		*(p+1) = 'R';
		dbg (3, "pair=%s", other);
		return TRUE;
	}

	p = g_strrstr(other, "%R.");
	if(p){
		*(p+1) = 'L';
		return TRUE;
	}

	p = g_strrstr(other, "-L.");
	if(p){
		*(p+1) = 'R';
		return TRUE;
	}

	p = g_strrstr(other, "-R.");
	if(p){
		*(p+1) = 'L';
		return TRUE;
	}

    other[0] = '\0';
	return FALSE;
}


void
wf_debug_printf (const char* func, int level, const char* format, ...)
{
    va_list args;

    va_start(args, format);
    if (level <= wf_debug) {
#ifdef SHOW_TIME
		fprintf(stderr, "%Lu %s(): ", _get_time(), func);
#else
		fprintf(stderr, "%s(): ", func);
#endif
        vfprintf(stderr, format, args);
        fprintf(stderr, "\n");
    }
    va_end(args);
}


