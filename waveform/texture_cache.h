/*
  copyright (C) 2012-2018 Tim Orford <tim@orford.org>

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
#include "waveform/waveform.h"

#ifdef WF_USE_TEXTURE_CACHE
#ifdef __wf_private__

#define WF_TEXTURE_CACHE_LORES_MASK (1 << 23)
#define WF_TEXTURE_CACHE_HIRES_MASK (1 << 22)
#define WF_TEXTURE_CACHE_HIRES_NG_MASK (1 << 21)
#define WF_TEXTURE_CACHE_V_LORES_MASK (1 << 20)

typedef void  (*AGlOnSteal) (Texture*);

struct _texture_cache
{
	GArray*     t;             // type Texture
	AGlOnSteal  on_steal;
};

void  texture_cache_init            ();
void  texture_cache_set_on_steal    (AGlOnSteal);
int   texture_cache_lookup          (int tex_type, WaveformBlock);
guint texture_cache_assign_new      (int tex_type, WaveformBlock);
void  texture_cache_freshen         (int tex_type, WaveformBlock);
void  texture_cache_remove          (int tex_type, Waveform*, int);
void  texture_cache_remove_waveform (Waveform*);

#endif // __wf_private__
#endif // WF_USE_TEXTURE_CACHE
#endif // __waveform_texture_cache_h__
