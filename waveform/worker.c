/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>

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
#define __wf_private__
#define __wf_worker_private__
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
#include "waveform/waveform.h"
#include "waveform/worker.h"

static gpointer worker_thread         (gpointer);


void
wf_worker_init(WfWorker* worker)
{
	worker->msg_queue = g_async_queue_new();

#ifdef HAVE_GLIB_2_32
	if(!g_thread_new("file load thread", worker_thread, worker)){
		perr("error creating thread\n");
#else
	GError* error = NULL;
	if(!g_thread_create(worker_thread, worker, false, &error)){
		perr("error creating thread: %s\n", error->message);
		g_error_free(error);
#endif
	}
}


	typedef struct {
		WfWorker*  worker;
		QueueItem* job;
	} WorkerJob;

	// note that withoug an idle fn, unreffing in the worker can cause a finalize in the worker thread
	static bool worker_unref_waveform(gpointer _w)
	{
		Waveform* waveform = _w;
		g_object_unref(waveform); // remove the reference added by g_weak_ref_get()
		return G_SOURCE_REMOVE;
	}

	static bool worker_post(gpointer _wj)
	{
		// do clean-up and notifications in the main thread

		WorkerJob* wj = _wj;
		QueueItem* job = wj->job;
		WfWorker* w = wj->worker;

		if(!job->cancelled){
			Waveform* waveform = g_weak_ref_get(&job->ref);
			call(job->done, waveform, job->user_data);
			if(waveform) g_object_unref(waveform);
		}
		if(job->free && job->user_data){
			job->free(job->user_data);
			job->user_data = NULL;
		}

		w->jobs = g_list_remove(w->jobs, job);

		g_weak_ref_set(&job->ref, NULL);
		g_free(job);
		g_free(wj);

		return G_SOURCE_REMOVE;
	}

static gpointer
worker_thread(gpointer data)
{
	dbg(2, "new file load thread.");
	WfWorker* w = data;

	g_return_val_if_fail(w->msg_queue, NULL);

	g_async_queue_ref(w->msg_queue);

	//check for new work
	while(true){
		QueueItem* job = g_async_queue_pop(w->msg_queue); // blocking
		dbg(2, "starting new job: %p", job);

		Waveform* waveform = g_weak_ref_get(&job->ref);
		if(waveform){
			if(!job->cancelled){
				// note that the job is run directly so that it runs in the worker thread.
				job->work(waveform, job->user_data);
			}
			g_idle_add(worker_unref_waveform, waveform);
		}
#ifdef DEBUG
		if(!w || job->cancelled) dbg(1, "job cancelled. not calling work fn");
#endif

		WorkerJob* wj = g_new0(WorkerJob, 1);
		wj->job = job;
		wj->worker = w;
		g_idle_add(worker_post, wj);

		g_usleep(100); // TODO optimise this value
	}

	return NULL;
}


void
wf_worker_push_job(WfWorker* w, Waveform* waveform, WfCallback2 work, WfCallback2 done, WfCallback free, gpointer user_data)
{
	// note that the ref count for the Waveform is not incremented as
	// this will prevent the cancellation from occurring.

	QueueItem* item = g_new0(QueueItem, 1);
	*item = (QueueItem){
		.work = work,
		.done = done,
		.free = free,
		.user_data = user_data
	};

	g_weak_ref_set(&item->ref, waveform);

	w->jobs = g_list_append(w->jobs, item);
	g_async_queue_push(w->msg_queue, item);
}


void
wf_worker_cancel_jobs(WfWorker* w, Waveform* waveform)
{
	GList* l = w->jobs;
	for(;l;l=l->next){
		QueueItem* j = l->data;
		if(!j->cancelled){
			Waveform* wav = g_weak_ref_get(&j->ref);
			if(wav){
				if(wav == waveform) j->cancelled = true;
				g_object_unref(wav);
			}
		}
	}
	int n_jobs = g_list_length(w->jobs);
	dbg(n_jobs ? 1 : 2, "n_jobs=%i", n_jobs);
}


