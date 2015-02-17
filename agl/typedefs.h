#ifndef __agl_typedefs_h__
#define __agl_typedefs_h__

typedef struct _agl_shader        AGlShader;
typedef struct _alphamap_shader   AlphaMapShader;
typedef struct _plain_shader      PlainShader;
typedef struct _texture_unit      AGlTextureUnit; 
typedef struct _AGlFBO            AGlFBO;
typedef struct _AGlActor          AGlActor;
typedef struct _AGlTextureActor   AGlTextureActor;

typedef struct {int x, y;}             AGliPt;
typedef struct {int x1, y1, x2, y2;}   AGliRegion;
typedef struct {float x, y, w, h;}     AGlRect;
typedef struct {float x0, y0, x1, y1;} AGlQuad;
typedef struct {float r, g, b;}        AGlColourFloat;


#endif //__agl_typedefs_h__
