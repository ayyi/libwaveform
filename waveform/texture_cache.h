#ifndef __waveform_texture_cache_h__
#define __waveform_texture_cache_h__
#include "waveform/peak.h"

#ifdef WF_USE_TEXTURE_CACHE

struct _texture_cache
{
	//guint       textures[WF_TEXTURE_MAX];
	GArray*     t;                        // type Texture
};

void  texture_cache_init       ();
void  texture_cache_gen        ();
guint texture_cache_get        (int);
int   texture_cache_lookup     (WaveformBlock);
int   texture_cache_get_new    ();
int   texture_cache_find_empty ();
void  texture_cache_assign     (int, WaveformBlock);
guint texture_cache_assign_new (WaveformBlock);
void  texture_cache_remove     (Waveform*); //tmp

#endif // WF_USE_TEXTURE_CACHE
#endif //__waveform_texture_cache_h__
