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
#include "waveform/actor.h"

int draw_wave_buffer(int mode, int startx,
	float swidth, float sheight,
	int dwidth, int dheight,
	int offx, int offy,
	int force, float* din,
	uint32_t rgba);

//void     draw_wave_buffer2 (Waveform*, WfSampleRegion, int startx, float swidth, float sheight, int dwidth, int dheight, int offx, int offy, float* din, uint32_t rgba);
void     draw_wave_buffer2     (Waveform*, WfSampleRegion, float v_gain, float dwidth, int dheight, int offx, int offy, float* d_in, uint32_t rgba);
void     draw_wave_buffer_hi   (Waveform*, WfSampleRegion, WfRectangle*, Peakbuf*, int chan, float v_gain, uint32_t rgba);
void     draw_wave_buffer_v_hi (Waveform*, WfSampleRegion, WfRectangle*, WfViewPort*, WfBuf16*, int chan, float v_gain, uint32_t rgba);
gboolean draw_waveform         (WaveformActor*, WfSampleRegion, int width, float height, int offx, int offy, int mode, uint32_t rgba);
