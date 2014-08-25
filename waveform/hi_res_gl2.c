/*
  copyright (C) 2014 Tim Orford <tim@orford.org>

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

  ------------------------------------------------------------------------

  This HI_RES renderer is shader-only (GLSL 1.20) and passes peak data
  directly to the shader in a 2D texture.

  Texture format:
    - each texture can hold up to 32 blocks.
    - each 8bit value directly the stores the peak value (either max or min)
    - each block is stored in 4 rows per block per channel
    - row layout:
      - row 0: 4096 max values
      - row 1: max values repeated at progressively lower x resolution (mipmap-like)
      - row 2: 4096 min values
      - row 3: min values repeated at progressively lower x resolution

      - row 4...: for multi channel audio, the above is repeated for other channels.

  Texture size is 16k per block per channel

  A 10 hour 44100k mono audio file would use: 24000 blocks, 750 textures, 384MB.

  Memory usage:
    - textures are timestamped when used.
    - resources for each WfWaveform are removed when the waveform is free'd.
    - to handle cases such as in a DAW where there are very large numbers of
      audio files, a garbage collector is run after resources are added. The
      collector removes the textures that have been used least recently.

*/
#ifndef __actor_c__
#define __wf_private__
#define __wf_canvas_priv__
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <sndfile.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include "waveform/utils.h"
#include "waveform/canvas.h"
#include "waveform/actor.h"
#include "waveform/peak.h"

extern int wf_debug;
#endif // __actor_c__

void hi_gl2_cache_print ();

extern NGRenderer hi_renderer_gl2;


static void
hi_gl2_free_item(/*Waveform* waveform, */gpointer _data)
{
	// this is called by the hash_table when an item is removed from hi_res_ng_data.

	// ** the item has already been removed from the hash_table
	//    so we have no way of referencing the Waveform.

	HiResNGWaveform* data = _data;

#if 0
	struct C {
		HiResNGWaveform* data;
		Waveform*        waveform;
	} c = {data, NULL};

	bool find_val(gpointer key, gpointer value, gpointer user_data)
	{
		// reverse lookup - find the key given the data.
		struct C* c = user_data;
		return (value == c->data)
			? c->waveform = key, true
			: false;
	}

	if(g_hash_table_find(hi_res_ng_data, find_val, &c)){

		int s; for(s=0;s<data->size;s++){
			ng_gl2_free_section(c.waveform, &data->section[s], s);
		}
	}
	else gwarn("waveform not found");
#endif

	g_free(data);
}


#ifdef NOT_USED
static void hi_gl2_uninit()
{
	g_hash_table_destroy(hi_renderer_gl2.ng_data);
	hi_renderer_gl2.ng_data = NULL;
}
#endif


void
hi_gl2_on_steal(WaveformBlock* wb, guint tex)
{
	HiResNGWaveform* data = wb->waveform->priv->render_data[MODE_HI];
	if(data){
		int _s = wb->block & (~WF_TEXTURE_CACHE_HIRES_NG_MASK);
		int s = _s / MAX_BLOCKS_PER_TEXTURE;
#ifdef DEBUG
		g_return_if_fail(_s % MAX_BLOCKS_PER_TEXTURE == 0);
		g_return_if_fail(s < data->size);
#endif
		Section* section = &data->section[s];
		if(section){
			g_return_if_fail(tex == section->texture);
			section->texture = 0;
			section->completed = false;
			memset(section->ready, 0, sizeof(bool) * MAX_BLOCKS_PER_TEXTURE);
			dbg(0, "section %i cleared", s);
		}
	}
}


// temporary
void
hi_gl2_cache_print()
{
	static int n_textures; n_textures = 0;

	void _hi_ng_print(gpointer key, gpointer value, gpointer _)
	{
		HiResNGWaveform* data = (HiResNGWaveform*)value;
		int s, n=0; for(s=0;s<data->size;s++){
			Section* section = &data->section[s];
			if(section->buffer){
				//dbg(0, "  %2i: t=%u %i", s, section->texture, section->time_stamp);
				n++;
			}
			if(section->texture) n_textures++;
		}
		if(!n) dbg(0, "all sections EMPTY");
	}

	if(g_hash_table_size(hi_renderer_gl2.ng_data)){
		g_hash_table_foreach(hi_renderer_gl2.ng_data, _hi_ng_print, NULL);
		dbg(0, "n_textures=%i", n_textures);
	}else
		dbg(0, "EMPTY");
}


