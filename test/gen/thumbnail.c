/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2013-2019 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#include "config.h"
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

int main(int argc, char* argv[])
{
	GdkPixbuf* pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 96, 96);
	gdk_pixbuf_save(pixbuf, "thumbnail.png", "png", NULL, NULL);

	return EXIT_SUCCESS;
}
