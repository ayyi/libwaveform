
#define PANGO_ENABLE_BACKEND

/* we always want to disable cast checks */
#ifndef G_DISABLE_CAST_CHECKS
#define G_DISABLE_CAST_CHECKS
#endif

#include <glib-object.h>
#include <fontconfig/fontconfig.h>
#include "agl/fontmap.h"

G_BEGIN_DECLS

#define FLAG_INVERSE 1
#define FLAG_OUTLINE 2 // not yet implemented

typedef gint32 Fixed; // Fixed point number (16.16)

typedef struct _Colour32
{
  // reversed so can be cast from uint32 rrggbbaa.
  guint8 alpha;
  guint8 blue;
  guint8 green;
  guint8 red;
  
} Colour32;

typedef struct _PangoGlRenderer     PangoGlRenderer;

#define PANGO_GL_RENDERER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), PANGO_TYPE_GL_RENDERER, PangoGlRendererClass))
#define PANGO_IS_GL_RENDERER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), PANGO_TYPE_GL_RENDERER))
#define PANGO_GL_RENDERER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), PANGO_TYPE_GL_RENDERER, PangoGlRendererClass))

typedef struct {
  PangoRendererClass parent_class;
  PangoContext* context;
} PangoGlRendererClass;

struct _PangoGlRenderer
{
  PangoRenderer parent_instance;
  Colour32      color;
  int           flags;
  guint         cur_tex; /* current texture */
};

#define PANGO_TYPE_GL_RENDERER       (pango_gl_renderer_get_type())
#define PANGO_GL_RENDERER(object)    (G_TYPE_CHECK_INSTANCE_CAST ((object), PANGO_TYPE_GL_RENDERER, PangoGlRenderer))
#define PANGO_IS_GL_RENDERER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), PANGO_TYPE_GL_RENDERER))

GType          pango_gl_renderer_get_type (void);

void           pango_gl_render_layout_subpixel (PangoLayout *layout, int x, int y, double z, Colour32 *color, int flags);
void           pango_gl_render_layout (PangoLayout  *layout, int x, int y, double z, Colour32 *color, int flags);
void           pango_gl_render_layout_line (PangoLayoutLine *line, int x, int y, Colour32 *color);
void           pango_gl_render_clear_caches ();

Fixed          double_to_fixed (double val);

G_END_DECLS

