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
#define __wf_private__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
//#include "waveform/gl_ext.h"
#include "waveform/utils.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/peak.h"

extern int wf_debug;


static void
_draw_line(int x1, int y1, int x2, int y2, float r, float g, float b, float a)
{
	//dbg(2, "%2i %2i", x1, y1);
	glLineWidth(1);
	glColor4f(r, g, b, a);

	glBegin(GL_LINES);
	glVertex2f(x1, y1); glVertex2f(x2, y2);
	glEnd();
}


#ifdef TMEP
static void
_set_pixel(int x, int y, guchar r, guchar g, guchar b, guchar aa)
{
	//dbg(2, "%i %i", x, y);

	glColor3f(((float)r)/0x100, ((float)g)/0x100, ((float)b)/0x100);
	glPushMatrix();
	glNormal3f(0, 0, 1);
	glDisable(GL_TEXTURE_2D);
	glPointSize(4.0);
	//glPointParameter(GL_POINT_DISTANCE_ATTENUATION, x, y, z); //xyz are 0.0 - 1.0

	//make pt rounded (doesnt work)
	/*
	glEnable(GL_POINT_SMOOTH); // opengl.org says not supported and not recommended.
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/

	glBegin(GL_POINTS);
	glVertex3f(x, y, 1);
	glEnd();
	glPopMatrix();
	glColor3f(1.0, 1.0, 1.0);
}
#endif


void
draw_wave_buffer_hi(Waveform* w, WfSampleRegion region, WfRectangle* rect, Peakbuf* peakbuf, int chan, float v_gain, uint32_t rgba)
{
	//for use with peak data of alternative plus and minus peaks.

	//TODO change this to do multiple channels

	float r = ((float)((rgba >> 24)       ))/0x100;
	float g = ((float)((rgba >> 16) & 0xff))/0x100;
	float b = ((float)((rgba >>  8) & 0xff))/0x100;
	float alpha = ((float)((rgba  ) & 0xff))/0x100;
	//dbg(0, "0x%x alpha=%.2f", rgba, alpha);

	int64_t region_end = region.start + (int64_t)region.len;

	float region_len = region.len;
	short* data = peakbuf->buf[chan];

	int io_ratio = (peakbuf->n_tiers == 4 || peakbuf->n_tiers == 3) ? 16 : 1; //TODO
	int x = 0;
	int p = 0;
	int p_ = region.start / io_ratio;
//dbg(0, "width=%.2f region=%Li-->%Li xgain=%.2f n_tiers=%i", rect->len, region.start, region.start + (int64_t)region.len, rect->len / region_len, peakbuf->n_tiers);
//dbg(0, "x: %.2f --> %.2f", (((double)0) / region_len) * rect->len, (((double)4095) / region_len) * rect->len);
	g_return_if_fail(region_end / io_ratio <= peakbuf->size / WF_PEAK_VALUES_PER_SAMPLE);
	while (p < region.len / io_ratio){
									if(2 * p_ >= peakbuf->size) gwarn("s_=%i size=%i", p_, peakbuf->size);
									g_return_if_fail(2 * p_ < peakbuf->size);
		x = rect->left + (((double)p) / region_len) * rect->len * io_ratio;
//if(!p) dbg(0, "x=%i", x);
		if (x - rect->left >= rect->len) break;
//if(p < 10) printf("    x=%i %2i %2i\n", x, data[2 * s], data[2 * p_ + 1]);

		double y1 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_    ]) * v_gain * (rect->height / 2.0) / (1 << 15);
		double y2 = ((double)data[WF_PEAK_VALUES_PER_SAMPLE * p_ + 1]) * v_gain * (rect->height / 2.0) / (1 << 15);

		_draw_line(x, rect->top + rect->height / 2, x, rect->top - y1 + rect->height / 2, r, g, b, alpha);
		_draw_line(x, rect->top + rect->height / 2, x, rect->top - y2 + rect->height / 2, r, g, b, alpha);
//if(p == 4095)
//		_draw_line(rect->left + x, 0, rect->left + x, rect->height, r, g, b, 1.0);

		p++;
		p_++;
	}
	dbg(2, "n_lines=%i x0=%.1f x=%i y=%.1f h=%.1f", p, rect->left, x, rect->top, rect->height);
}


void
draw_wave_buffer_v_hi(Waveform* w, WfSampleRegion region, WfRectangle* rect, WfViewPort* viewport, WfBuf16* buf, int chan, float v_gain, uint32_t rgba)
{
	//for use at resolution 1, operates on audio data, NOT peak data.

	// @region - sample range within the current block. The values are wrt the Waveform, not the block.
	// @rect   - the canvas area corresponding exactly to the WfSampleRegion.

	//TODO change this to do multiple channels

	g_return_if_fail(region.len <= buf->size);

	const float r = ((float)((rgba >> 24)       ))/0x100;
	const float g = ((float)((rgba >> 16) & 0xff))/0x100;
	const float b = ((float)((rgba >>  8) & 0xff))/0x100;

	const double zoom = rect->len / (double)region.len;
	const int s0 = region.start % buf->size;
	//const int x0 = MAX(0, floor(viewport->left - rect->left) - 3);
//TODO crop by viewport
	const int x0 = rect->left;
	dbg(2, "rect=%.2f-->%.2f region=%Lu-->%Lu viewport=%.1f-->%.1f zoom=%.3f", rect->left, rect->left + rect->len, region.start, region.start + ((uint64_t)region.len), viewport->left, viewport->right, zoom);
	int oldx = x0 - 1;
	int oldy = 0;
	//int s = MAX(0, x0 / zoom - 1); // s should not depend on x!!
	int s = 0;
//dbg(0, "x0=%i s0=%i", x0, s0);
	#define BIG_NUMBER 4096
	int i = 0;
	//dbg(0, "start=%.2f", MAX(0, floor(viewport->left - rect->left) - 3));
	int x; for(x = x0; x < x0 + BIG_NUMBER && /*rect->left +*/ x < viewport->right; x++){
		double s_ = ((double)x - rect->left) / zoom;
		double dist = s_ - s;
//if(i < 5) dbg(0, "x=%i s_=%.3f dist=%.2f", x, s_, dist);
		if (dist > 2.0) {
			//			if(dist > 5.0) gwarn("dist %.2f", dist);
			int ds = dist - 1;
			dist -= ds;
			s += ds;
		}
//if(i < 5) dbg(0, "  ss=%i", s0 + s);
		//else dbg(0, "unchanged"); //this occurs at hi zoom but appears to be harmless.
//if(x < viewport->left - rect->left) continue;
		if (s0 + s >= buf->size ) { gwarn("end of block reached: region.start=%i region.end=%Lu %i", s0, region.start + ((uint64_t)region.len), buf->size); break; }
		if (s  + 3 >= region.len) { gwarn("end of region reached: region.len=%i x=%i s0=%i s=%i", region.len, x, s0, s); break; }

		short* d = buf->buf[chan];
		double y1 = (s0 + s   < buf->size) ? d[s0 + s  ] : 0;
		double y2 = (s0 + s+1 < buf->size) ? d[s0 + s+1] : 0;
		double y3 = (s0 + s+2 < buf->size) ? d[s0 + s+2] : 0;
		double y4 = (s0 + s+3 < buf->size) ? d[s0 + s+3] : 0;

		double d0 = dist;
		double d1 = dist - 1.0;
		double d2 = dist - 2.0;
		double d3 = dist - 3.0;

		int y = (int)(
			(
				- (d1 * d2 * d3 * y1) / 6
				+ (d0 * d2 * d3 * y2) / 2
				- (d0 * d1 * d3 * y3) / 2
				+ (d0 * d1 * d2 * y4) / 6
			)
			* v_gain * (rect->height / 2.0) / (1 << 15)
		);
//if(i < 10) printf("  x=%i=%.2f %i y=%i dist=%.2f sidx=%i %.2f\n", x, rect->left + x, d[s0 + sidx], y, dist, sidx, floor((x / zoom) - 1));
		// draw straight line from old pos to new pos
		_draw_line(
			oldx,   rect->top - oldy + rect->height / 2,
			x,      rect->top -    y + rect->height / 2,
			g, b, r, 1.0);

		oldx = x;
		oldy = y;
		i++;
	}
	if(x >= x0 + BIG_NUMBER){ gwarn("too many lines! rect.width=%.2f", rect->len); }
	else dbg(2, "n_lines=%i x=%i-->%i", i, x0, x);
}


