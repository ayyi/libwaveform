/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

// note that the same header can be provided by any Ayyi library
#ifndef __ayyi_debug_h__
#define __ayyi_debug_h__

#include "agl/utils.h"

extern int wf_debug;

#ifdef DEBUG
#define AGL_DEBUG if(agl->debug)
#else
#define AGL_DEBUG if(false)
#endif

#define PF {if(wf_debug) printf("%s()...\n", __func__);}
#define PF0 printf("%s...\n", __func__)
#define PF2 {if(wf_debug > 1) printf("%s...\n", __func__);}
#ifdef DEBUG
#define dbg(A, STR, ...) ({if(A <= wf_debug){ fputs(__func__, stdout); printf(": "STR"\n", ##__VA_ARGS__);}})
#else
#define dbg(A, STR, ...) ({})
#endif
#define dbg2(FLAG, A, STR, ...) ({if(A <= wf_debug && (AGL_DEBUG_ ## FLAG & agl_get_instance()->debug_flags)){ fputs(__func__, stdout); printf(": "STR"\n", ##__VA_ARGS__);}})
#define pwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)
#define gwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__);
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)

typedef enum
{
  AGL_DEBUG_GLYPH_CACHE = 1,
  AGL_DEBUG_SHADERS = 1 << 1,
  AGL_DEBUG_ALL     = 1 << 2,

} AGlDebugFlags;

#define AGL_DEBUG_CHECK(T) (agl_get_instance()->debug_flags & AGL_DEBUG_ ## T)

#ifdef __agl_utils_c__
char ayyi_yellow [12] = "\x1b[1;33m";
char ayyi_white  [12] = "\x1b[0;39m";
char ayyi_bold   [12] = "\x1b[1;39m";
char ayyi_grey   [16] = "\x1b[38;5;240m";
#else
extern char ayyi_yellow[12];
extern char ayyi_white [12];
extern char ayyi_bold  [12];
extern char ayyi_grey  [16];
#endif


#endif
