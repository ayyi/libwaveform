/*
  copyright (C) 2012-2014 Tim Orford <tim@orford.org>

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
#define __waveform_peak_c__
#define __wf_private__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/worker.h"

static WF* wf = NULL;


gpointer
file_load_thread(gpointer data)
{
	// As a blocking call is used on the async queue (g_async_queue_pop) a main loop is no longer used.

	dbg(2, "new file load thread.");
	wf = wf_get_instance();

	g_return_val_if_fail(wf->msg_queue, NULL);

	g_async_queue_ref(wf->msg_queue);

	gboolean post(gpointer _item)
	{
		// do clean-up and notifications in the main thread

		QueueItem* job = _item;

		if(!job->cancelled) call(job->done, job->user_data);

		wf->jobs = g_list_remove(wf->jobs, job);

		g_free(job);

		return IDLE_STOP;
	}

	//check for new work
	while(true){
		QueueItem* job = g_async_queue_pop(wf->msg_queue); // blocks
		dbg(2, "new message! %p", job);

		// note that the job is run directly so that it runs in the worker thread.
		if(!job->cancelled){
			call(job->work, job->user_data);
			// no need to free item->user_data, it is done by the caller in the callback.
		}else{
			dbg(0, "job cancelled. not calling work fn");
			g_free0(job->user_data);
		}

		g_idle_add(post, job);

		g_usleep(100); // TODO optimise this value
	}

	return NULL;
}


