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

extern WF* wf;


static void
waveform_view_plus_gl_init(WaveformViewPlus* view)
{
	PF;
	if(gl_initialised) return;

	WF_START_DRAW {

		agl_set_font_string("Roboto 10");
		if(agl_get_instance()->use_shaders){
			wf_shaders.ass->uniform.colour1 = view->title_colour1;
			wf_shaders.ass->uniform.colour2 = view->title_colour2;
		}

	} WF_END_DRAW

	gl_initialised = true;
}


static void
draw(WaveformViewPlus* view)
{
	WaveformViewPlusPrivate* v = view->priv;
	Waveform* w = view->waveform;
	WaveformActor* actor = view->priv->actor;
	AGl* agl = agl_get_instance();

#if 0 //white border
	glPushMatrix(); /* modelview matrix */
		glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor3f(1.0, 1.0, 1.0);

		int wid = waveform_view_plus_get_width(view);;
		int h   = waveform_view_plus_get_height(view);
		glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 1); glVertex3f(wid, 0.0, 1);
		glVertex3f(wid, h,   1); glVertex3f(0.0,   h, 1);
		glEnd();
	glPopMatrix();
#endif

	if(!w) return;

	if(actor) wf_actor_paint(actor);

	if(v->show_grid){
		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);

		WfSampleRegion region = {view->start_frame, w->n_frames};
		wf_grid_paint(view->priv->canvas, &region, &viewport);
	}

	agl_print(2, waveform_view_plus_get_height(view) - 16, 0, view->text_colour, view->text);

	if(!v->title_is_rendered) waveform_view_plus_render_text(view);

	// title text:
	if(agl->use_shaders){
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);

		agl_use_program((AGlShader*)wf_shaders.ass);

		//texture size
		float th = agl_power_of_two(view->title_height + view->title_y_offset);

#undef ALIGN_TOP
#ifdef ALIGN_TOP
		float y1 = -((int)th - view->title_height - view->title_y_offset);
		agl_textured_rect(v->ass_textures[0], waveform_view_plus_get_width(view) - view->title_width - 4.0f, y, view->title_width, th, &(AGlRect){0.0, 0.0, ((float)view->title_width) / v->title_texture_width, 1.0});
#else
		float y = waveform_view_plus_get_height(view) - th;
		agl_textured_rect(v->ass_textures[0],
			waveform_view_plus_get_width(view) - view->title_width - 4.0f,
			y,
			view->title_width,
			th,
			&(AGlQuad){0.0, 0.0, ((float)view->title_width) / v->title_texture_width, 1.0}
		);
#endif
	}
}


static void
waveform_view_plus_gl_on_allocate(WaveformViewPlus* view)
{
	if(!view->priv->actor) return;

	WfRectangle rect = {0, 0, waveform_view_plus_get_width(view), waveform_view_plus_get_height(view)};
	wf_actor_allocate(view->priv->actor, &rect);

	wf_canvas_set_viewport(view->priv->canvas, NULL);
}


