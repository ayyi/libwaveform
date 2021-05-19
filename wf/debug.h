/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifdef __wf_private__

#ifndef __wf_utils_c__
extern int wf_debug;
#endif

#pragma once

#include <sys/time.h>

#ifdef DEBUG
#define IF_WF_DEBUG if(agl->debug)
#else
#define IF_WF_DEBUG if(false)
#endif

#include "debug/debug.h"

#endif
