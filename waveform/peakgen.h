/*
  copyright (C) 2012-2018 Tim Orford <tim@orford.org>

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
#ifndef __wf_peakgen_h__
#define __wf_peakgen_h__

void     waveform_ensure_peakfile      (Waveform*, WfPeakfileCallback, gpointer);
char*    waveform_ensure_peakfile__sync(Waveform*);
void     waveform_peakgen              (Waveform*, const char* peakfile, WfCallback3, gpointer);
void     waveform_peakgen_cancel       (Waveform*);

gboolean wf_peakgen__sync              (const char* wav, const char* peakfile, GError**);

#endif
