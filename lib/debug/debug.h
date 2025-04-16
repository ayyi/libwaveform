/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __ayyi_debug_h__
#define __ayyi_debug_h__

#include <stdio.h>
#include <inttypes.h>
#include <glib.h>

#ifdef DEBUG
// note that the level check is now outside the print fn in order to
// prevent functions that return print arguments from being run
#  define dbg(A, B, ...) do {if(A <= _debug_) debug_printf(__func__, A, B, ##__VA_ARGS__);} while(FALSE)

#  define PF {if(_debug_) printf("%s()...\n", __func__);}
#  define PF0 {printf("%s()...\n", __func__);}
#  define PF2 {if(_debug_ > 1) printf("%s...\n", __func__);}
#  define PF_DONE {if(_debug_) printf("%s(): done.\n", __func__);}
#else
#  define dbg(A, B, ...)
#  define PF
#  define PF0
#  define PF2
#  define PF_DONE
#endif

#define perr(A, ...) g_critical("%s(): "A, __func__, ##__VA_ARGS__)
#define pwarn(A, ...) g_warning("%s(): "A, __func__, ##__VA_ARGS__)

#ifndef g_object_unref0
#define g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#endif

#ifdef DEBUG
void     debug_printf       (const char* func, int level, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
#endif
void     warnprintf         (const char* format, ...) __attribute__ ((format (printf, 1, 2)));
void     warnprintf2        (const char* func, char* format, ...);
void     errprintf          (const char* format, ...);

void     set_log_handlers   ();

extern int _debug_;         // debug level. 0=off.

extern char ayyi_white [12];
extern char ayyi_grey  [16];
extern char ayyi_bold  [12];
extern char ayyi_warn  [32];
extern char ayyi_err   [32];

#endif
