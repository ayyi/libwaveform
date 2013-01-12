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

extern WF* wf;


static void
waveform_view_plus_gl_init(WaveformViewPlus* view_plus)
{
	PF;
	WaveformView* view = (WaveformView*)view_plus;

	if(gl_initialised) return;

	WF_START_DRAW {

//TODO move to wf_canvas_init_gl()
		GLboolean npotTexturesAvailable = GL_FALSE;
		if(GL_ARB_texture_non_power_of_two){
			if(wf_debug) printf("non_power_of_two textures are available.\n");
			npotTexturesAvailable = GL_TRUE;
		}else{
			fprintf(stderr, "GL_ARB_texture_non_power_of_two extension is not available!\n" );
			fprintf(stderr, "Framebuffer effects will be lower resolution (lower quality).\n\n" );
		}

		agl_set_font("Roboto 10");
		waveform_view_plus_render_text(view_plus);
		wf_shaders.ass->uniform.colour1 = view_plus->title_colour1;
		wf_shaders.ass->uniform.colour2 = view_plus->title_colour2;

	} WF_END_DRAW

	gl_initialised = true;

#ifdef WF_USE_TEXTURE_CACHE
	texture_cache_gen();
#endif
}


static void
draw(WaveformViewPlus* view)
{
	Waveform* w = view->waveform;
	WaveformActor* actor = view->priv->actor;

#if 0 //white border
	glPushMatrix(); /* modelview matrix */
		glNormal3f(0, 0, 1); glDisable(GL_TEXTURE_2D);
		glLineWidth(1);
		glColor3f(1.0, 1.0, 1.0);

		int wid = GL_WIDTH;
		int h   = waveform_view_plus_get_height(view);
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

	//text:
	AGl* agl = agl_get_instance();
	if(agl->use_shaders){
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ass_textures[0]);
		if(!glIsTexture(ass_textures[0])) gwarn("not texture");
//glDisable(GL_BLEND);

		agl_use_program((AGlShader*)wf_shaders.ass);

		//texture size
		float tw = agl_power_of_two(view->title_width);
		float th = agl_power_of_two(view->title_height + view->title_y_offset);

		float x1 = 0.0f;
//dbg(0, "offset=%i", (int)th - view->title_height - view->title_y_offset);
		//float y2 = waveform_view_plus_get_height(view) + (th - view->title_height - view->title_y_offset);
		float y1 = -((int)th - view->title_height - view->title_y_offset);
//dbg(0, "y1=%.2f", y1);
		float y2 = y1 + th;// - view->title_height - view->title_y_offset);
		float x2 = x1 + tw;
//dbg(0, "h=%i th=%i diff=%i y1=%i y2=%i", view->title_height, (int)th, (int)th - view->title_height, (int)y1, (int)y2);
		glBegin(GL_QUADS);
		glTexCoord2d(0.0, 0.0); glVertex3d(x1, y1, -1);
		glTexCoord2d(1.0, 0.0); glVertex3d(x2, y1, -1);
		glTexCoord2d(1.0, 1.0); glVertex3d(x2, y2, -1);
		glTexCoord2d(0.0, 1.0); glVertex3d(x1, y2, -1);
		glEnd();
	}

	agl_print(2, waveform_view_plus_get_height(view) - 16, 0, view->text_colour, view->text);
}


static void
waveform_view_plus_gl_on_allocate(WaveformViewPlus* view)
{
	if(!view->priv->actor) return;

	WfRectangle rect = {0, 0, waveform_view_plus_get_width(view), waveform_view_plus_get_height(view)};
	wf_actor_allocate(view->priv->actor, &rect);

	wf_canvas_set_viewport(view->priv->canvas, NULL);
}


