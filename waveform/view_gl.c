/*
  copyright (C) 2012-2013 Tim Orford <tim@orford.org>

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

extern WF* wf;


static void
waveform_view_gl_init(WaveformView* view)
{
	PF;

	if(gl_initialised) return;

	//WF_START_DRAW { } WF_END_DRAW

	gl_initialised = true;

#ifdef WF_USE_TEXTURE_CACHE
	texture_cache_gen();
#endif
}


static void
draw(WaveformView* view)
{
	Waveform* w = view->waveform;
	WaveformActor* actor = view->priv->actor;

#if 0 //white border
	glPushMatrix(); /* modelview matrix */
		glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor3f(1.0, 1.0, 1.0);

		int wid = GL_WIDTH;
		int h   = GL_HEIGHT;
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 1); glVertex3f(wid, 0.0, 1);
		glVertex3f(wid, h,   1); glVertex3f(0.0,   h, 1);
		glEnd();
	glPopMatrix();
#endif

	if(!w || !w->textures) return;

	wf_actor_paint(actor);

					//TODO copy of private fn from WaveformActor - needs refactoring
					void wf_actor_get_viewport(WaveformActor* a, WfViewPort* viewport)
					{
						WaveformCanvas* canvas = a->canvas;

						if(canvas->viewport) *viewport = *canvas->viewport;
						else {
							viewport->left   = a->rect.left;
							viewport->top    = a->rect.top;
							viewport->right  = a->rect.left + a->rect.len;
							viewport->bottom = a->rect.top + a->rect.height;
						}
					}
	if(view->priv->show_grid){
		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);

		WfSampleRegion region = {view->start_frame, w->n_frames};
		wf_grid_paint(view->priv->canvas, &region, &viewport);
	}
}


static void
waveform_view_gl_on_allocate(WaveformView* view)
{
	if(!view->priv->actor) return;

	int width = waveform_view_get_width(view);
	WfRectangle rect = {0, 0, width, GL_HEIGHT};
	wf_actor_allocate(view->priv->actor, &rect);

	wf_canvas_set_viewport(view->priv->canvas, NULL);
}


