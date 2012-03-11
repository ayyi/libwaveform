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
#ifndef __waveform_texture_cache_h__
#define __waveform_texture_cache_h__
#include "waveform/peak.h"

#ifdef WF_USE_TEXTURE_CACHE

#define WF_TEXTURE_CACHE_LORES_MASK (1 << 23)

struct _texture_cache
{
	GArray*     t;             // type Texture
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
