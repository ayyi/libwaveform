/*
 +---------------------------------------------------------------------
 | This file is part of the Ayyi project. https://www.ayyi.org
 | copyright (C) 2012-2021 Tim Orford <tim@orford.org>
 +---------------------------------------------------------------------
 | This program is free software; you can redistribute it and/or modify
 | it under the terms of the GNU General Public License version 3
 | as published by the Free Software Foundation.
 +----------------------------------------------
 |
 | Demonstration of overlaying text on a waveform window.
 |
 */

#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <getopt.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <gtk/gtk.h>
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#include <gdk/gdkkeysyms.h>
#include <ass/ass.h>
#include "agl/gtk.h"
#include "waveform/actor.h"
#define __wf_private__
#include "test/common2.h"

extern AssShader ass;

#define GL_WIDTH 256.0
#define GL_HEIGHT 128.0
#define TEXT_WIDTH "512"
#define TEXT_HEIGHT "64"
	const int frame_w = 512;
	const int frame_h =  64;
#define VBORDER 8
#define WAV "test/data/mono_0:10.wav"

GdkGLConfig*    glconfig       = NULL;
static bool     gl_initialised = false;
GtkWidget*      canvas         = NULL;
WaveformContext*wfc            = NULL;
Waveform*       w1             = NULL;
Waveform*       w2             = NULL;
WaveformActor*  actor          = NULL;
AGlRootActor*   scene          = NULL;
float           zoom           = 1.0;
float           dz             = 20.0;
GLuint          ass_textures[] = {0, 0};
gpointer        tests[]        = {};

char* script = 
	"[Script Info]\n"
	"ScriptType: v4.00+\n"
	"PlayResX: " TEXT_WIDTH "\n"
	"PlayResY: " TEXT_HEIGHT "\n"
	"ScaledBorderAndShadow: yes\n"
	"[V4+ Styles]\n"
	"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
	"Style: Default,Fontin Sans Rg,40,&H000000FF,&HFF0000FF,&H00FF0000,&H00000000,-1,0,0,0,100,100,0,0,1,2.5,0,1,2,2,2,1\n"
	"[Events]\n"
	"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
	"Dialogue: 0,0:00:00.00,0:00:15.00,Default,,0000,0000,0000,," WAV " \n";

typedef struct
{
    int width, height, stride;
    unsigned char* buf;      // 8 bit alphamap
} image_t;

static void on_canvas_realise  (GtkWidget*, gpointer);
static void on_allocate        (GtkWidget*, GtkAllocation*, gpointer);
static void start_zoom         (float target_zoom);
static void toggle_animate     ();
static void render_text        ();
static void blend_single       (image_t*, ASS_Image*);
uint64_t    get_time           ();

static ASS_Library* ass_library;
static ASS_Renderer* ass_renderer;

static const struct option long_options[] = {
	{ "non-interactive",  0, NULL, 'n' },
};

static const char* const short_options = "n";


static bool
ass_node_paint (AGlActor* actor)
{
	if(agl_get_instance()->use_shaders){
		if(!glIsTexture(ass_textures[0])) pwarn("not texture");

		ass.uniform.colour1 = 0xffffffff;
		ass.uniform.colour2 = 0xff0000ff;

		agl_textured_rect(ass_textures[0], 0., 0., GL_WIDTH, frame_h, NULL);
	}

	agl_print(0, 0, 0, 0x66ff66ff, "Regular text");

	return true;
}


AGlActor*
ass_node ()
{
	agl_get_instance()->programs[AGL_APPLICATION_SHADER_1] = &ass.shader;

	return agl_actor__new(AGlActor,
		.paint = ass_node_paint,
		.program = &ass.shader,
		.region = {0, GL_HEIGHT - frame_h, GL_WIDTH, GL_HEIGHT}
	);
}


void
msg_callback (int level, const char* fmt, va_list va, void* data)
{
    if (level > 6) return;
    printf("libass: ");
    vprintf(fmt, va);
    printf("\n");
}


static void
init (int frame_w, int frame_h)
{
    ass_library = ass_library_init();
    if (!ass_library) {
        printf("ass_library_init failed!\n");
        exit(EXIT_FAILURE);
    }

    ass_set_message_cb(ass_library, msg_callback, NULL);

    ass_renderer = ass_renderer_init(ass_library);
    if (!ass_renderer) {
        printf("ass_renderer_init failed!\n");
        exit(EXIT_FAILURE);
    }

    ass_set_frame_size(ass_renderer, frame_w, frame_h);
    ass_set_fonts(ass_renderer, NULL, "Sans", 1, NULL, 1);
}


int
main (int argc, char *argv[])
{
	set_log_handlers();

	wf_debug = 0;

	int opt;
	while((opt = getopt_long (argc, argv, short_options, long_options, NULL)) != -1) {
		switch(opt) {
			case 'n':
				g_timeout_add(3000, (gpointer)exit, NULL);
				break;
		}
	}

	gtk_init(&argc, &argv);
	if(!(glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE))){
		perr ("Cannot initialise gtkglext."); return EXIT_FAILURE;
	}

	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	canvas = gtk_drawing_area_new();
#ifdef HAVE_GTK_2_18
	gtk_widget_set_can_focus     (canvas, true);
#endif
	gtk_widget_set_size_request  (canvas, 480, 64);
	gtk_widget_set_gl_capability (canvas, glconfig, NULL, 1, GDK_GL_RGBA_TYPE);
	gtk_widget_add_events        (canvas, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
	gtk_container_add((GtkContainer*)window, (GtkWidget*)canvas);

	scene = (AGlRootActor*)agl_actor__new_root(canvas);
	wfc = wf_context_new((AGlActor*)scene);

	g_signal_connect((gpointer)canvas, "realize",       G_CALLBACK(on_canvas_realise), NULL);
	g_signal_connect((gpointer)canvas, "size-allocate", G_CALLBACK(on_allocate), NULL);
	g_signal_connect((gpointer)canvas, "expose_event",  G_CALLBACK(agl_actor__on_expose), scene);

	gtk_widget_show_all(window);

	gboolean key_press (GtkWidget* widget, GdkEventKey* event, gpointer user_data)
	{
		switch(event->keyval){
			case 61:
				start_zoom(zoom * 1.5);
				break;
			case 45:
				start_zoom(zoom / 1.5);
				break;
			case KEY_Left:
			case KEY_KP_Left:
				dbg(0, "left");
				break;
			case KEY_Right:
			case KEY_KP_Right:
				dbg(0, "right");
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
	g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);

	render_text();

	gtk_main();

	return EXIT_SUCCESS;
}


static void
render_text ()
{
	init(frame_w, frame_h);
	ASS_Track* track = ass_read_memory(ass_library, g_strdup(script), strlen(script), NULL);
	if (!track) {
		printf("track init failed!\n");
		exit(EXIT_FAILURE);
	}

	// ASS_Image is a list of alphamaps each with a uint32 RGBA colour/alpha
	ASS_Image* img = ass_render_frame(ass_renderer, track, 100, NULL);

	// ass output will be composited into this buffer.
	// 16bits per pixel for use with GL_LUMINANCE8_ALPHA8 mode.
	#define N_CHANNELS 2 // luminance + alpha
	image_t out = {frame_w, frame_h, frame_w * N_CHANNELS, NULL };
	out.buf = g_new0(guchar, frame_w * out.height * N_CHANNELS);

#if 0 //BACKGROUND
	int width  = out.width;
	int height = out.height;
	int y; for(y=0;y<height;y++){
		int x; for(x=0;x<width;x++){
			*(out.buf + y * width + x) = ((x+y) * 0xff) / (width * 2);
		}
	}
#else
	//clear the background to border colour for better antialiasing at edges
	int x, y; for(y=0;y<out.height;y++){
		for(x=0;x<out.width;x++){
			*(out.buf + y * out.stride + N_CHANNELS * x) = 0xff; //TODO should be border colour.
		}
	}
#endif

	ASS_Image* i = img;
	int cnt = 0;
	for(;i;i=i->next,cnt++){
		blend_single(&out, i); //is each one of these a single glyph?
	}
	printf("%d images blended\n", cnt);

	{
		glGenTextures(1, ass_textures);
		if(gl_error){ perr ("couldnt create ass_texture."); exit(EXIT_FAILURE); }

		int pixel_format = GL_LUMINANCE_ALPHA;
		agl_use_texture (ass_textures[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE8_ALPHA8, out.width, out.height, 0, pixel_format, GL_UNSIGNED_BYTE, out.buf);
		gl_warn("gl error binding ass texture");
	}

	ass_free_track(track);
	ass_renderer_done(ass_renderer);
	ass_library_done(ass_library);
	g_free(out.buf);
}


static gboolean canvas_init_done = false;
static void
on_canvas_realise (GtkWidget* _canvas, gpointer user_data)
{
	PF;
	if(canvas_init_done) return;
	if(!GTK_WIDGET_REALIZED (canvas)) return;

	agl_actor__add_child((AGlActor*)scene, ass_node());

	gl_initialised = true;
	canvas_init_done = true;

	char* filename = g_build_filename(g_get_current_dir(), WAV, NULL);
	w1 = waveform_load_new(filename);
	g_free(filename);

	int n_frames = waveform_get_n_frames(w1);

	WfSampleRegion region = {
		0, n_frames,
	};

	uint32_t colours[2] = {0x3399ffff, 0x0000ffff}; // blue

	actor = wf_context_add_new_actor(wfc, w1);

	wf_actor_set_region (actor, &region);
	wf_actor_set_colour (actor, colours[0]);

	on_allocate(canvas, &canvas->allocation, user_data);

	void _on_scene_requests_redraw(AGlScene* scene, gpointer _)
	{
		gdk_window_invalidate_rect(canvas->window, NULL, false);
	}
	scene->draw = _on_scene_requests_redraw;
}


static void
on_allocate (GtkWidget* widget, GtkAllocation* allocation, gpointer user_data)
{
	if(!gl_initialised) return;

	((AGlActor*)scene)->region = (AGlfRegion){0, 0, GL_WIDTH, GL_HEIGHT};

	//optimise drawing by telling the canvas which area is visible
	wf_context_set_viewport(wfc, &(WfViewPort){0, 0, GL_WIDTH, GL_HEIGHT});

	start_zoom(zoom);
}


#define _r(c)  ( (c)>>24)
#define _g(c)  (((c)>>16)&0xFF)
#define _b(c)  (((c)>> 8)&0xFF)
#define _a(c)  (((c)    )&0xFF)

static void
blend_single (image_t* frame, ASS_Image* img)
{
	// composite img onto frame

	int x, y;
	unsigned char opacity = 255 - _a(img->color);
	unsigned char b = _b(img->color);
	dbg(2, "  %ix%i stride=%i x=%i", img->w, img->h, img->stride, img->dst_x);

	#define LUMINANCE_CHANNEL (x * N_CHANNELS)
	#define ALPHA_CHANNEL (x * N_CHANNELS + 1)
	unsigned char* src = img->bitmap;
	unsigned char* dst = frame->buf + img->dst_y * frame->stride + img->dst_x * N_CHANNELS;
	for (y = 0; y < img->h; ++y) {
		for (x = 0; x < img->w; ++x) {
			unsigned k = ((unsigned) src[x]) * opacity / 255;
			dst[LUMINANCE_CHANNEL] = (k * b + (255 - k) * dst[LUMINANCE_CHANNEL]) / 255;
			dst[ALPHA_CHANNEL] = (k * 255 + (255 - k) * dst[ALPHA_CHANNEL]) / 255;
		}
		src += img->stride;
		dst += frame->stride;
	}
}


static void
start_zoom (float target_zoom)
{
	PF;
	zoom = MAX(0.1, target_zoom);

	if(actor) wf_actor_set_rect(actor, &(WfRectangle){
		0.0f,
		0.0f,
		GL_WIDTH * zoom,
		GL_HEIGHT * 0.95
	});
}


static void
toggle_animate ()
{
	PF0;
	gboolean on_idle(gpointer _)
	{
		static uint64_t frame = 0;
		static uint64_t t0    = 0;
		if(!frame) t0 = get_time();
		else{
			uint64_t time = get_time();
			if(!(frame % 1000))
				dbg(0, "rate=%.2f fps", ((float)frame) / ((float)(time - t0)) / 1000.0);

			if(!(frame % 8)){
			}
		}
		frame++;
		return G_SOURCE_CONTINUE;
	}
	g_timeout_add(50, on_idle, NULL);
}


uint64_t
get_time ()
{
	struct timeval start;
	gettimeofday(&start, NULL);
	return start.tv_sec * 1000 + start.tv_usec / 1000;
}


