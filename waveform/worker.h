/*
  copyright (C) 2012-2019 Tim Orford <tim@orford.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 3
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/
#ifndef __wf_worker_h__
#define __wf_worker_h__
#include "waveform/typedefs.h"

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
