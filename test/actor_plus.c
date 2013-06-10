/*
  Demonstration of the libwaveform WaveformActor interface

  Similar to actor.c but with additional features, eg background, ruler.

  ---------------------------------------------------------------

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
#define __ayyi_private__
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
#include "test/ayyi_utils.h"

#define WAV "test/data/mono_1.wav"

struct _app
{
	int timeout;
} app;

#define GL_WIDTH 300.0
#define GL_HEIGHT 256.0
#define HBORDER (GL_WIDTH / 32.0)
#define VBORDER 8
//#define HBORDER 0
//#define VBORDER 0
#define bool gboolean

GdkGLConfig*    glconfig       = NULL;
GdkGLDrawable*  gl_drawable    = NULL;
GdkGLContext*   gl_context     = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformCanvas* wfc            = NULL;
Waveform*       w1             = NULL;
WaveformActor*  a[]            = {NULL};//{NULL, NULL, NULL};
float           zoom           = 1.0;
GLuint          bg_textures[2] = {0, 0};

static void set_log_handlers   ();
static void setup_projection   (GtkWidget*);
static void draw               (GtkWidget*);
static bool on_expose          (GtkWidget*, GdkEventExpose*, gpointer);
static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
static void toggle_animate     ();
static void create_background  ();
static void background_paint   (GtkWidget*);
static void ruler_paint        (GtkWidget*);
uint64_t    get_time           ();
static char*find_wav           ();


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

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_can_focus(canvas, true);
	gtk_widget_set_size_request(GTK_WIDGET(canvas), GL_WIDTH + 2 * HBORDER, 128);
	gtk_widget_set_gl_capability(canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);
	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose_event",  G_CALLBACK(on_expose), NULL);

	gtk_widget_show_all(window);

	gboolean key_press(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		switch(event->keyval){
			case 61:
				start_zoom(zoom * 1.5);
				break;
			case 45:
				start_zoom(zoom / 1.5);
				break;
			case GDK_KEY_Left:
			case GDK_KEY_KP_Left:
				dbg(0, "left");
				//waveform_view_set_start(waveform, waveform->start_frame - 8192 / waveform->zoom);
				break;
			case GDK_KEY_Right:
			case GDK_KEY_KP_Right:
				dbg(0, "right");
				//waveform_view_set_start(waveform, waveform->start_frame + 8192 / waveform->zoom);
				break;
			case (char)'a':
				toggle_animate();
				break;
			case GDK_KP_Enter:
				break;
			case 113:
				exit(EXIT_SUCCESS);
				break;
			case GDK_Delete:
				break;
			default:
				dbg(0, "%i", event->keyval);
				break;
		}
		return TRUE;
	}

	g_signal_connect(window, "key-press-event", G_CALLBACK(key_press), NULL);

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

	//double hborder = GL_WIDTH / 32;

	double left = -((int)HBORDER);
	double right = vw + left;            //now tracks the allocation so we can get consistent ruler markings.
	double bottom = GL_HEIGHT + VBORDER;
	double top = -VBORDER;
	//dbg(0, "ortho: width=%.2f", right - left);
	glOrtho (left, right, bottom, top, 10.0, -100.0);
}


static void
draw(GtkWidget* widget)
{
	background_paint(widget);
	ruler_paint(widget);

	//TODO why does GL_DEPTH_TEST mess with the background painting?
	//glEnable(GL_DEPTH_TEST);

	//glPushMatrix(); /* modelview matrix */
		int i; for(i=0;i<G_N_ELEMENTS(a);i++) if(a[i]) wf_actor_paint(a[i]);
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


static gboolean
on_expose(GtkWidget* widget, GdkEventExpose* event, gpointer user_data)
{
	if(!GTK_WIDGET_REALIZED(widget)) return TRUE;
	if(!gl_initialised) return TRUE;

	START_DRAW {
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw(widget);

		gdk_gl_drawable_swap_buffers(gl_drawable);
	} END_DRAW
	return TRUE;
}


static gboolean canvas_init_done = false;
static void
on_canvas_realise(GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	gl_drawable = gtk_widget_get_gl_drawable(canvas);
	gl_context  = gtk_widget_get_gl_context(canvas);

	gl_init();

	wfc = wf_canvas_new(gl_context, gl_drawable);
	//wf_canvas_set_use_shaders(wfc, false);

	canvas_init_done = true;

	char* filename = find_wav();
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
		wf_actor_set_colour(a[i], colours[i][0], colours[i][1]);
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


float
_easing(int step, float start, float end)
{
	return (end - start) / step;
}


static void
start_zoom(float target_zoom)
{
	//when zooming in, the Region is preserved so the box gets bigger. Drawing is clipped by the Viewport.

	PF0;
	zoom = MAX(0.1, target_zoom);

	int i; for(i=0;i<G_N_ELEMENTS(a);i++)
		if(a[i]) wf_actor_allocate(a[i], &(WfRectangle){
			0.0,
			i * GL_HEIGHT / 2,
			(canvas->allocation.width - ((int)HBORDER) * 2) * target_zoom,
			GL_HEIGHT / 2 * 0.95
		});
}


static void
toggle_animate()
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


static void
create_background()
{
	//create an alpha-map gradient texture for use as background

	if(bg_textures[0]) return;

	glEnable(GL_TEXTURE_2D);

	int width = 256;
	int height = 256;
	char* pbuf = g_new0(char, width * height);
	int y; for(y=0;y<height;y++){
		int x; for(x=0;x<width;x++){
			*(pbuf + y * width + x) = ((x+y) * 0xff) / (width * 2);
		}
	}

	glGenTextures(1, bg_textures);
	if(glGetError() != GL_NO_ERROR){ gerr ("couldnt create bg_texture."); return; }
	dbg(0, "bg_texture=%i", bg_textures[0]);

	int pixel_format = GL_ALPHA;
	glBindTexture  (GL_TEXTURE_2D, bg_textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, pbuf);
	if(glGetError() != GL_NO_ERROR) gwarn("gl error binding bg texture!");

	g_free(pbuf);
}


static void
background_paint(GtkWidget* widget)
{
	AGl* agl = agl_get_instance();
	if(agl->use_shaders){
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		if(!glIsTexture(bg_textures[0])) gwarn("not texture");
		glBindTexture(GL_TEXTURE_2D, bg_textures[0]);

		wfc->priv->shaders.tex2d->uniform.fg_colour = 0x0000ffff;
		agl_use_program((AGlShader*)wfc->priv->shaders.tex2d);

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


void
set_log_handlers()
{
	void log_handler(const gchar* log_domain, GLogLevelFlags log_level, const gchar* message, gpointer user_data)
	{
	  switch(log_level){
		case G_LOG_LEVEL_CRITICAL:
		  printf("%s %s\n", ayyi_err, message);
		  break;
		case G_LOG_LEVEL_WARNING:
		  printf("%s %s\n", ayyi_warn, message);
		  break;
		default:
		  printf("log_handler(): level=%i %s\n", log_level, message);
		  break;
	  }
	}

	g_log_set_handler (NULL, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);

	char* domain[] = {NULL, "Waveform", "GLib-GObject", "GLib", "Gdk", "Gtk"};
	int i; for(i=0;i<G_N_ELEMENTS(domain);i++){
		g_log_set_handler (domain[i], G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, log_handler, NULL);
	}
}


uint64_t
get_time()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


static char*
find_wav()
{
	char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
	if(g_file_test(filename, G_FILE_TEST_EXISTS)){
		return filename;
	}
	g_free(filename);
	filename = g_build_filename(g_get_current_dir(), "../", WAV, NULL);
	return filename;
}


