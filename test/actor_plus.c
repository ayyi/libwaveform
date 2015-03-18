/*
  Demonstration of the libwaveform WaveformActor interface

  Similar to actor.c but with additional features, eg background, ruler.

  ---------------------------------------------------------------

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
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "waveform/waveform.h"
#include "waveform/actor.h"
#include "waveform/fbo.h"
#include "waveform/gl_utils.h"
#include "test/common.h"
#include "test/ayyi_utils.h"

#define WAV "test/data/mono_1.wav"

#define GL_WIDTH 300.0
#define GL_HEIGHT 256.0
#define HBORDER (GL_WIDTH / 32.0)
#define VBORDER 8
//#define HBORDER 0
//#define VBORDER 0
#define bool gboolean

AGl*            agl            = NULL;
GdkGLConfig*    glconfig       = NULL;
GdkGLDrawable*  gl_drawable    = NULL;
GdkGLContext*   gl_context     = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w1             = NULL;
WaveformActor*  a[]            = {NULL};
float           zoom           = 1.0;
GLuint          bg_textures[2] = {0, 0};
gpointer        tests[]        = {};

KeyHandler
	zoom_in,
	zoom_out,
	scroll_left,
	scroll_right,
	toggle_animate,
	toggle_shaders,
	quit;

Key keys[] = {
	{KEY_Left,      scroll_left},
	{KEY_KP_Left,   scroll_left},
	{KEY_Right,     scroll_right},
	{KEY_KP_Right,  scroll_right},
	{61,            zoom_in},
	{45,            zoom_out},
	{GDK_KP_Enter,  NULL},
	{(char)'<',     NULL},
	{(char)'>',     NULL},
	{(char)'a',     toggle_animate},
	{(char)'s',     toggle_shaders},
	{GDK_Delete,    NULL},
	{113,           quit},
	{0},
};

static void setup_projection   (GtkWidget*);
static void draw               (GtkWidget*);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
static void create_background  ();
static void background_paint   (GtkWidget*);
static void ruler_paint        (GtkWidget*);
uint64_t    get_time           ();


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 1;

	memset(&app, 0, sizeof(struct _app));

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		gerr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	agl = agl_get_instance();

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_can_focus(canvas, true);
	gtk_widget_set_size_request(GTK_WIDGET(canvas), GL_WIDTH + 2 * HBORDER, 128);
	gtk_widget_set_gl_capability(canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events(canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose_event",  G_CALLBACK(on_expose), NULL);

	gtk_widget_show_all(window);

	add_key_handlers((GtkWindow*)window, NULL, (Key*)&keys);

	gboolean window_on_delete(GtkWidget* widget, GdkEvent* event, gpointer user_data){
		gtk_main_quit();
		return false;
	}
	g_signal_connect(window, "delete-event", G_CALLBACK(window_on_delete), NULL);

	gtk_main();

	return EXIT_SUCCESS;
}


static void
gl_init()
{
	if(gl_initialised) return;

	START_DRAW {

		if(!agl_shaders_supported()){
			gwarn("shaders not supported");
		}
		printf("GL_RENDERER = %s\n", (const char*)glGetString(GL_RENDERER));

		create_background();

	} END_DRAW

	gl_initialised = true;
}


static void
setup_projection(GtkWidget* widget)
{
	int vx = 0;
	int vy = 0;
	int vw = widget->allocation.width;
	int vh = widget->allocation.height;
	glViewport(vx, vy, vw, vh);
	dbg (0, "viewport: %i %i %i %i", vx, vy, vw, vh);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double left = -((int)HBORDER);
	double right = vw + left;            //now tracks the allocation so we can get consistent ruler markings.
	double bottom = GL_HEIGHT + VBORDER;
	double top = -VBORDER;
	glOrtho (left, right, bottom, top, 10.0, -100.0);
}


static void
draw(GtkWidget* widget)
{
	background_paint(widget);
	ruler_paint(widget);

	//glPushMatrix(); /* modelview matrix */
		int i; for(i=0;i<G_N_ELEMENTS(a);i++) if(a[i]) ((AGlActor*)a[i])->paint((AGlActor*)a[i]);
	//glPopMatrix();

#undef SHOW_BOUNDING_BOX
#ifdef SHOW_BOUNDING_BOX
	glPushMatrix(); /* modelview matrix */
		glTranslatef(0.0, 0.0, 0.0);
		glNormal3f(0, 0, 1);
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1);

		int w = GL_WIDTH;
		int h = GL_HEIGHT/2;
		glBegin(GL_QUADS);
		glVertex3f(-0.2, -0.2, 1); glVertex3f(w, -0.2, 1);
		glVertex3f(w, h, 1);       glVertex3f(-0.2, h, 1);
		glEnd();
		glEnable(GL_TEXTURE_2D);
	glPopMatrix();
#endif

	//fbo_print(a[0], 0, GL_HEIGHT * 0.5, 0.5, 0xffffffff, 0xff);
}


static bool
on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED(widget)) return true;
	if(!gl_initialised) return true;

	START_DRAW {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		draw(widget);

		gdk_gl_drawable_swap_buffers(gl_drawable);
	} END_DRAW
	return true;
}


static gboolean canvas_init_done = false;
static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	gl_init();

	wfc = wf_canvas_new((AGlRootActor*)agl_actor__new_root(canvas));

	gl_drawable = wfc->root->gl.gdk.drawable;
	gl_context = wfc->root->gl.gdk.context;

	canvas_init_done = true;

	char* filename = find_wav(WAV);
	w1 = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region[] = {
		{0,            n_frames    },
		{0,            n_frames / 2},
		{n_frames / 4, n_frames / 4},
		{n_frames / 2, n_frames / 2},
	};

	uint32_t colours[4][2] = {
		{0xffffff77, 0x0000ffff},
		{0x66eeffff, 0x0000ffff},
		{0xffdd66ff, 0x0000ffff},
		{0x66ff66ff, 0x0000ffff},
	};

	int i; for(i=0;i<G_N_ELEMENTS(a);i++){
		a[i] = wf_canvas_add_new_actor(wfc, w1);

		wf_actor_set_region(a[i], &region[i]);
		wf_actor_set_colour(a[i], colours[i][0]);
	}

	on_allocate(canvas, &canvas->allocation, user_data);

	//allow the WaveformCanvas to initiate redraws
	void _on_wf_canvas_requests_redraw(WaveformCanvas* wfc, gpointer _)
	{
		gdk_window_invalidate_rect(canvas->window, NULL, false);
	}
	wfc->draw = _on_wf_canvas_requests_redraw;
}


static void
on_allocate(GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	setup_projection(widget);

	//optimise drawing by telling the canvas which area is visible
	wf_canvas_set_viewport(wfc, &(WfViewPort){0, 0, allocation->width - ((int)HBORDER), GL_HEIGHT});

	start_zoom(zoom);
}


static void
start_zoom(float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_set_rect(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / 2,
			(canvas->allocation.width - ((int)HBORDER) * 2) * target_zoom,
			GL_HEIGHT / 2 * 0.95
		});
}


void
toggle_animate(WaveformView* _)                   // FIXME arg type doesnt match
{
	PF0;
	gboolean on_idle(gpointer _)
	{
		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame)
			t0 = get_time();
		else{
			uint64_t time = get_time();
			if(!(frame % 1000))
				dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);

			if(!(frame % 8)){
				float v = (frame % 16) ? 2.0 : 1.0/2.0;
				if(v > 16.0) v = 1.0;
				start_zoom(v);
			}
		}
		frame++;
		return IDLE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


void
toggle_shaders(WaveformView* _)
{
	PF0;
	wf_canvas_set_use_shaders(wfc, !agl->use_shaders);
}


void
zoom_in(WaveformView* waveform)
{
	start_zoom(zoom * 1.5);
}


void
zoom_out(WaveformView* waveform)
{
	start_zoom(zoom / 1.5);
}


void
scroll_left(WaveformView* waveform)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame - n_visible_frames / 10);
}


void
scroll_right(WaveformView* waveform)
{
	//int n_visible_frames = ((float)waveform->waveform->n_frames) / waveform->zoom;
	//waveform_view_set_start(waveform, waveform->start_frame + n_visible_frames / 10);
}


void
quit(WaveformView* waveform)
{
	exit(EXIT_SUCCESS);
}


static void
create_background()
{
	//create an alpha-map gradient texture for use as background

	if(bg_textures[0]) return;

	int width = 256;
	int height = 256;
	char* pbuf = g_new0(char, width * height);
	int y; for(y=0;y<height;y++){
		int x; for(x=0;x<width;x++){
			*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
		}
	}

	glEnable(GL_TEXTURE_2D);

	glGenTextures(1, bg_textures);
	if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); goto out; }
	dbg(0, "bg_texture=%i", bg_textures[0]);

	int pixel_format = GL_ALPHA;
	glBindTexture  (GL_TEXTURE_2D, bg_textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
	gl_warn("binding bg texture");

  out:
	g_free(pbuf);
}


static void
background_paint(GtkWidget* widget)
{
	if(agl->use_shaders){
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		if(!glIsTexture(bg_textures[0])) gwarn("not texture");
		glBindTexture(GL_TEXTURE_2D, bg_textures[0]);

		agl->shaders.alphamap->uniform.fg_colour = 0x0000ffff;
		agl_use_program((AGlShader*)agl->shaders.alphamap);

	}else{
		glColor4f(1.0, 0.7, 0.0, 1.0);

		glEnable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
				if(!glIsTexture(bg_textures[0])) gwarn("not texture");
		glBindTexture(GL_TEXTURE_2D, bg_textures[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		agl_use_program(0);
	}

	double top = -VBORDER;
	double bot = GL_HEIGHT + VBORDER;
	double x1 = -HBORDER;
	double x2 = widget->allocation.width + HBORDER;
	glBegin(GL_QUADS);
	glTexCoord2d(1.0, 1.0); glVertex2d(x1, top);
	glTexCoord2d(0.0, 1.0); glVertex2d(x2, top);
	glTexCoord2d(0.0, 0.0); glVertex2d(x2, bot);
	glTexCoord2d(1.0, 0.0); glVertex2d(x1, bot);
	glEnd();
}


static void
ruler_paint(GtkWidget* widget)
{
	if(!agl_get_instance()->use_shaders) return;

	double width = canvas->allocation.width - 2 * ((int)HBORDER);

	wfc->priv->shaders.ruler->uniform.fg_colour = 0xffffff7f;
	wfc->priv->shaders.ruler->uniform.beats_per_pixel = 0.1 * (GL_WIDTH / width) / zoom;
	agl_use_program((AGlShader*)wfc->priv->shaders.ruler);

#if 0 //shader debugging
	{
		float smoothstep(float edge0, float edge1, float x)
		{
			float t = CLAMP((x - edge0) / (edge1 - edge0), 0.0, 1.0);
			return t * t * (3.0 - 2.0 * t);
		}

		float pixels_per_beat = 1.0 / wfc->priv->shaders.ruler->uniform.beats_per_pixel;
		dbg(0, "ppb=%.2f", pixels_per_beat);
		int x; for(x=0;x<30;x++){
			float m = (x * 100) % ((int)pixels_per_beat * 100);
			float m_ = x - pixels_per_beat * floor(x / pixels_per_beat);
			printf("  %.2f %.2f %.2f\n", m / 100, m_, smoothstep(0.0, 0.5, m_));
		}
	}
#endif

	double top = 0;
	double bot = GL_HEIGHT * 0.25;
	double x1 = 0;
	double x2 = width;

	glPushMatrix();
	glScalef(1.0, -1.0, 1.0);           // inverted vertically to make alignment of marks to bottom easier in the shader
	glTranslatef(0.0, -GL_HEIGHT, 0.0); // making more negative moves downward
	glBegin(GL_QUADS);
	glVertex2d(x1, top);
	glVertex2d(x2, top);
	glVertex2d(x2, bot);
	glVertex2d(x1, bot);
	glEnd();
	glPopMatrix();
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


