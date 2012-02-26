
#include "waveform/hi_res.h"

extern WF* wf;

static int __draw_depth = 0;


static void
gl_init(WaveformView* view)
{
	if(gl_initialised) return;

	START_DRAW {

		if(wf_get_instance()->pref_use_shaders && !shaders_supported()){
			printf("gl shaders not supported. expect reduced functionality.\n");
			//printf("Warning: this program expects OpenGL 2.0\n");
			wf_canvas_use_program(view->priv->canvas, 0);
			view->priv->canvas->use_shaders = false;
		}
		printf("GL_RENDERER = %s\n", (const char*)glGetString(GL_RENDERER));

		GLboolean npotTexturesAvailable = GL_FALSE;
		if(GL_ARB_texture_non_power_of_two){
			if(wf_debug) printf("non_power_of_two textures are available.\n");
			npotTexturesAvailable = GL_TRUE;
		}else{
			fprintf(stderr, "GL_ARB_texture_non_power_of_two extension is not available!\n" );
			fprintf(stderr, "Framebuffer effects will be lower resolution (lower quality).\n\n" );
		}

	} END_DRAW

	gl_initialised = true;
}


static void
waveform_view_gl_init(WaveformView* view)
{
	PF;

	gl_init(view);

#ifdef WF_USE_TEXTURE_CACHE
	texture_cache_gen();
#endif

	//if(view->priv->canvas->use_shaders) wf_canvas_init_shaders(view->priv->canvas);

#ifdef USE_FBO
	START_DRAW {
		if(!fbo0) create_fbo();
	} END_DRAW
#endif

}


static void
draw(WaveformView* view)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

	if(!w || !w->gl_blocks) return;

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
	if(false && view->priv->show_grid){
		WfViewPort viewport; wf_actor_get_viewport(actor, &viewport);
		wf_canvas_set_viewport(view->priv->canvas, &viewport); //TODO dont do this on every draw

		WfSampleRegion region = {view->start_frame, w->n_frames};
		wf_grid_paint(view->priv->canvas, &region);
	}
}


static void
waveform_view_gl_on_allocate(WaveformView* view)
{
				//extern void _wf_set_last_fraction(Waveform*);
				//_wf_set_last_fraction(view->waveform);

	int width = waveform_view_get_width(view);
	WfRectangle rect = {0, 0, width, GL_HEIGHT};
	wf_actor_allocate(view->priv->actor, &rect);
#if 0
	int start_block_num = 0;
	int end_block_num = view->waveform->gl_blocks->size - 1; //TODO check zoom etc

	int b; for(b=start_block_num;b<=end_block_num;b++){
		waveform_view_load_block(view, b);
	}
#endif

	wf_canvas_set_viewport(view->priv->canvas, NULL);
}


