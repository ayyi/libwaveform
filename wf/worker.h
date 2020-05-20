/**
* +----------------------------------------------------------------------+
* | This file is part of libwaveform                                     |
* | https://github.com/ayyi/libwaveform                                  |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#ifndef __wf_worker_h__
#define __wf_worker_h__

#include "wf/typedefs.h"

#ifdef __wf_private__

typedef struct _QueueItem QueueItem;

#ifdef __wf_worker_private__
struct _QueueItem
{
	GWeakRef         ref;
	WfCallback2      work;
	WfCallback3      done;
	WfCallback       free;
	void*            user_data;
	gboolean         cancelled;
};
#endif

void     wf_worker_init        (WfWorker*);
void     wf_worker_push_job    (WfWorker*, Waveform*, WfCallback2 work, WfCallback3 done, WfCallback free, gpointer);
void     wf_worker_cancel_jobs (WfWorker*, Waveform*);

#endif
#endif
