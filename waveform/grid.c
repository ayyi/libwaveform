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

  ---------------------------------------------------------------

  WaveformGrid draws timeline markers onto a shared opengl drawable.

*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/animator.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/grid.h"
#include "waveform/gl_ext.h"

struct _grid
{
	struct {
		WfAnimatable  start;
		WfAnimatable  len;
	}               animatable;
	WfAnimation     animation;
	guint           timer_id;
};


void
wf_grid_paint(WaveformCanvas* canvas, WfSampleRegion* viewport_samples)
{
	//draw a line every 1 second.

	g_return_if_fail(canvas);
	g_return_if_fail(viewport_samples);

	WfViewPort* viewport = canvas->viewport;
	g_return_if_fail(viewport);
	float width = viewport->right - viewport->left;
	uint64_t n_frames = viewport_samples->len - viewport_samples->start;
	float zoom = n_frames / width;
	dbg(0, "n_frames=%Lu width=%.2f zoom=%.2f", n_frames, width, zoom);

	int i = 0;
	uint64_t f; for(f = canvas->sample_rate; f < n_frames && i++ < 0xff; f += canvas->sample_rate){
		float x = /*width * */zoom * ((float)f - viewport_samples->start) / n_frames;
		dbg(0, "x=%.2f", x);

		glPushMatrix();
			wf_canvas_use_program(canvas, 0);
			glNormal3f(0, 0, 1);
			glDisable(GL_TEXTURE_1D);
			glDisable(GL_TEXTURE_2D);
			//glDisable(GL_BLEND);
			glLineWidth(1);
			glColor4f(0.5, 0.5, 1.0, 1.0);

			int h = viewport->bottom;
			glBegin(GL_LINES);
			glVertex3f(x, 0.0, 1); glVertex3f(x, h, 1);
			glEnd();
		glPopMatrix();
	}
}


