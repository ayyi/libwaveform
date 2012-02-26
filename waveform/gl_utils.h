#ifndef __waveform_gl_utils_h__
#define __waveform_gl_utils_h__

gboolean __drawing = FALSE;
#define START_DRAW \
	if(__drawing) gwarn("START_DRAW: already drawing"); \
	__draw_depth++; \
	__drawing = TRUE; \
	if ((__draw_depth > 1) || gdk_gl_drawable_gl_begin (view->priv->canvas->gl_drawable, view->priv->canvas->gl_context)) {
#define END_DRAW \
	__draw_depth--; \
	if(!__draw_depth) gdk_gl_drawable_gl_end(view->priv->canvas->gl_drawable); \
	} else gwarn("!! gl_begin fail")\
	(__drawing = FALSE);
#define ASSERT_DRAWING g_return_if_fail(__drawing);

void use_texture(int texture);

#endif //__waveform_gl_utils_h__
