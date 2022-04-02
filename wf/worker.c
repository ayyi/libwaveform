/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. http://ayyi.org               |
* | copyright (C) 2012-2020 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/
#define __wf_private__
#define __wf_worker_private__
#include "config.h"
#include <sys/types.h>
#include "wf/debug.h"
#include "wf/waveform.h"
#include "wf/worker.h"

#define ENABLE_THREADS

static gpointer worker_thread (gpointer);


void
wf_worker_init (WfWorker* worker)
{
	worker->msg_queue = g_async_queue_new();

#ifdef ENABLE_THREADS
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
#else
	worker_thread(worker);
#endif
}


// note that without an idle fn, unreffing in the worker can cause a finalize in the worker thread
static gboolean
worker_unref_waveform (gpointer _w)
{
	Waveform* waveform = _w;
	g_object_unref(waveform); // remove the reference added by g_weak_ref_get()
	return G_SOURCE_REMOVE;
}


typedef struct {
	WfWorker*  worker;
	QueueItem* job;
} WorkerJob;


/*
 *   Do clean-up and notifications in the main thread
 */
static gboolean
worker_post (gpointer _wj)
{
	WorkerJob* wj = _wj;
	QueueItem* job = wj->job;
	WfWorker* w = wj->worker;

	if(!job->cancelled){
		Waveform* waveform = g_weak_ref_get(&job->ref);
		call(job->done, waveform, NULL, job->user_data);
		if(waveform) g_object_unref(waveform);
	}

	if (job->free) {
		g_clear_pointer(&job->user_data, job->free);
	}

	w->jobs = g_list_remove(w->jobs, job);

	g_weak_ref_set(&job->ref, NULL);
	g_free(job);
	g_free(wj);

	return G_SOURCE_REMOVE;
}


static inline void
process_new_job (WfWorker* w, QueueItem* job)
{
	dbg(2, "starting new job: %p", job);

	Waveform* waveform = g_weak_ref_get(&job->ref);
	if(waveform){
		if(!job->cancelled){
			// note that the job is run directly so that it runs in the worker thread.
			job->work(waveform, job->user_data);
		}
		g_idle_add(worker_unref_waveform, waveform); // release the ref added by g_weak_ref_get()
	}

	g_timeout_add(1, worker_post,
		WF_NEW(WorkerJob,
			.job = job,
			.worker = w
		)
	);
}


#ifdef ENABLE_THREADS
static gpointer
worker_thread (gpointer data)
{
	dbg(2, "new file load thread");
	WfWorker* w = data;

	g_return_val_if_fail(w->msg_queue, NULL);

	g_async_queue_ref(w->msg_queue);

	// check for new work
	while(true){
		QueueItem* job = g_async_queue_pop(w->msg_queue); // blocking
		process_new_job(w, job);

		g_usleep(100);
	}

	return NULL;
}
#endif


#ifndef ENABLE_THREADS
static gpointer
worker_thread (gpointer data)
{
	dbg(2, "new file load thread");
	WfWorker* w = data;

	g_return_val_if_fail(w->msg_queue, NULL);

	g_async_queue_ref(w->msg_queue);

	bool check_work (gpointer data)
	{
		WfWorker* w = data;

		QueueItem* job = g_async_queue_try_pop(w->msg_queue);
		if(job){
			process_new_job(w, job);
		}

		return G_SOURCE_CONTINUE;
	}

	g_timeout_add(500, check_work, data);

	return NULL;
}
#endif


void
wf_worker_push_job (WfWorker* w, Waveform* waveform, WfCallback2 work, WfCallback3 done, WfCallback free, gpointer user_data)
{
	// note that the ref count for the Waveform is not incremented as
	// this will prevent the cancellation from occurring.

	QueueItem* item = WF_NEW(QueueItem,
		.work = work,
		.done = done,
		.free = free,
		.user_data = user_data
	);

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

#ifdef DEBUG
	int n_jobs = g_list_length(w->jobs);
	dbg(n_jobs ? 1 : 2, "n_jobs=%i", n_jobs);
#endif
}


