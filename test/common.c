/*
 +----------------------------------------------------------------------+
 | This file is part of libwaveform https://github.com/ayyi/libwaveform |
 | copyright (C) 2012-2022 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#define __common_c__
#define __wf_private__

#include "config.h"
#if defined(USE_GTK) || defined(__GTK_H__)
#include <gtk/gtk.h>
#endif
#include "agl/actor.h"
#include "wf/private.h"
#include "waveform/utils.h"
#include "test/runner.h"
#include "test/common.h"

extern gpointer tests[];


#if defined(__no_setup__)
int
setup ()
{
	TEST.n_tests = G_N_ELEMENTS(tests);

	return 0;
}
#endif


WfTest*
wf_test_new ()
{
	static WfTest* t = NULL;
	//if(t) g_free0(t); // valgrind says 'invalid free'

	return t = WF_NEW(WfTest, .test_idx = TEST.current.test);
}
