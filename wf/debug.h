/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free s20tware; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifdef __wf_private__

#ifndef __wf_utils_c__
extern int wf_debug;
#endif

#ifndef __ayyi_debug_h__
#define __ayyi_debug_h__

#include <stdio.h>
#include <inttypes.h>
#include <sys/time.h>

void wf_debug_printf (const char* func, int level, const char* format, ...);

#define PF {if(wf_debug) printf("%s()...\n", __func__);}
#define PF0 printf("%s...\n", __func__)
#define PF2 {if(wf_debug > 1) printf("%s...\n", __func__);}
#define dbg(A, B, ...) wf_debug_printf(__func__, A, B, ##__VA_ARGS__)
#define pwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)
#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)

extern char ayyi_yellow[12];
extern char ayyi_white [12];
extern char ayyi_bold  [12];
extern char ayyi_grey  [16];

#endif
#endif
