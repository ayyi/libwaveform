/*
  copyright (C) 2012 Tim Orford <tim@orford.org>

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
#ifndef __wf_alphabuf_h__
#define __wf_alphabuf_h__

struct _alpha_buf {
	int        width;
	int        height;
	guchar*    buf;
	int        buf_size;
};

AlphaBuf*  wf_alphabuf_new             (Waveform*, int blocknum, int scale, gboolean is_rms, int border);
AlphaBuf*  wf_alphabuf_new_hi          (Waveform*, int blocknum, int scale, gboolean is_rms, int border);
void       wf_alphabuf_free            (AlphaBuf*);
GdkPixbuf* wf_alphabuf_to_pixbuf       (AlphaBuf*);

#endif //__wf_alphabuf_h__
