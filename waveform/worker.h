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
#ifndef __wf_worker_h__
#define __wf_worker_h__

#ifdef __wf_private__

gpointer file_load_thread (gpointer);

typedef void   (*WfCallback)    (gpointer user_data);

typedef struct _queue_item
{
	WfCallback       work;
	WfCallback       done;
	void*            user_data;
	gboolean         cancelled;
} QueueItem;

#endif
#endif
