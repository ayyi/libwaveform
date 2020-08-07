/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://www.ayyi.org           |
* | copyright (C) 2018-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __agl_x11_h__
#define __agl_x11_h__

#include <X11/Xlib.h>
#include "agl/actor.h"

extern Display* dpy;

typedef struct {
    Window    window;
    AGlScene* scene;
} AGlWindow;

AGlWindow* agl_window            (const char* name, int x, int y, int width, int height, bool fullscreen);
void       agl_window_destroy    (AGlWindow**);

bool       agl_is_fullscreen     (Window);
void       agl_toggle_fullscreen (Window);

GMainLoop* agl_main_loop_new     ();

#endif
