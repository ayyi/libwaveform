#ifndef __ayyi_utils_h__
#define __ayyi_utils_h__

#include "glib.h"

#define UNDERLINE printf("-----------------------------------------------------\n")
#define TIMER_STOP FALSE
#define call(FN, A, ...) if(FN) (FN)(A, ##__VA_ARGS__)

#ifdef USE_SDL
#  define is_sdl(WFC) (WFC && WFC->root->type == CONTEXT_TYPE_SDL)
#else
#  define is_sdl(WFC) false
#endif

#ifndef __common_c__
extern gboolean __wf_drawing;
#endif
#define START_DRAW \
	if(__wf_drawing){ gwarn("START_DRAW: already drawing"); } \
	__draw_depth++; \
	__wf_drawing = TRUE; \
	if (is_sdl(wfc) || (__draw_depth > 1) || gdk_gl_drawable_gl_begin (gl_drawable, gl_context)) {
#define END_DRAW \
	__draw_depth--; \
	if(!is_sdl(wfc)){ \
		if(!__draw_depth) gdk_gl_drawable_gl_end(gl_drawable); \
		} else { gwarn("!! gl_begin fail"); } \
	} \
	(__wf_drawing = FALSE);

void        errprintf                (char* fmt, ...);
void        errprintf2               (const char* func, char* format, ...);
void        warn_gerror              (const char* msg, GError**);
void        info_gerror              (const char* msg, GError**);

gchar*      path_from_utf8           (const gchar* utf8);

GList*      get_dirlist              (const char*);

void        string_increment_suffix  (char* newstr, const char* orig, int new_max);

int         get_terminal_width       ();

gboolean    audio_path_get_leaf      (const char* path, char* leaf);
gchar*      audio_path_get_base      (const char*);
gboolean    audio_path_get_wav_leaf  (char* leaf, const char* path, int len);
char*       audio_path_truncate      (char*, char);

#ifdef __ayyi_utils_c__
char grey     [16] = "\x1b[2;39m"; // 2 = dim
char yellow   [16] = "\x1b[1;33m";
char yellow_r [16] = "\x1b[30;43m";
char white__r [16] = "\x1b[30;47m";
char cyan___r [16] = "\x1b[30;46m";
char magent_r [16] = "\x1b[30;45m";
char blue_r   [16] = "\x1b[30;44m";
char red      [16] = "\x1b[1;31m";
char red_r    [16] = "\x1b[30;41m";
char green    [16] = "\x1b[1;32m";
char green_r  [16] = "\x1b[30;42m";
char ayyi_warn[32] = "\x1b[1;33mwarning:\x1b[0;39m";
char ayyi_err [32] = "\x1b[1;31merror!\x1b[0;39m";
char go_rhs   [32] = "\x1b[A\x1b[50C"; //go up one line, then goto column 60
char ok       [32] = " [ \x1b[1;32mok\x1b[0;39m ]";
char fail     [32] = " [\x1b[1;31mFAIL\x1b[0;39m]";
#else
extern char grey     [16];
extern char yellow   [16];
extern char yellow_r [16];
extern char white__r [16];
extern char cyan___r [16];
extern char magent_r [16];
extern char blue_r   [16];
extern char red      [16];
extern char red_r    [16];
extern char green    [16];
extern char green_r  [16];
extern char ayyi_warn[32];
extern char ayyi_err [32];
extern char go_rhs   [32];
extern char ok       [];
extern char fail     [];
#endif

#endif //__ayyi_utils_h__
