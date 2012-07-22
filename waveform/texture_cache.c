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
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <GL/gl.h>
#include <gtkglext-1.0/gdk/gdkgl.h>
#include <gtkglext-1.0/gtk/gtkgl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/texture_cache.h"

#define WF_TEXTURE_ALLOCATION_INCREMENT 20
#define WF_TEXTURE_MAX                  1024 //never allocate more than this (1024 textures equiv to ~6mins audio at medium res)

#ifdef WF_USE_TEXTURE_CACHE
static int time_stamp = 0;
static TextureCache* c;

static int  texture_cache_get_new          ();
static void texture_cache_assign           (TextureCache*, int, WaveformBlock);
static int  texture_cache_steal            ();
static void texture_cache_print            ();
static int  texture_cache_lookup_idx       (WaveformBlock);
static int  texture_cache_lookup_idx_by_id (guint);
static void texture_cache_unassign         (WaveformBlock);
static void texture_cache_shrink           (int);
static int  texture_cache_count_used       ();


void
texture_cache_init()
{
	if(c) return;

	WF* wf = wf_get_instance();
	c = wf->texture_cache = g_new0(TextureCache, 1);

	c->t = g_array_new(FALSE, TRUE, sizeof(Texture));
	c->t = g_array_set_size(c->t, 0);
}


void
texture_cache_gen()
{
	//create an additional set of available textures.

	WF* wf = wf_get_instance();
	if(!wf->texture_cache) texture_cache_init();

							//check all textures
#if 0
							{
								int i; for(i=0;i<c->t->len;i++){
									Texture* tx = &g_array_index(c->t, Texture, i);
									if(!glIsTexture(tx->id)) gwarn("not texture! %i: %i", i, tx->id);
								}
							}
#endif

	int size = c->t->len + WF_TEXTURE_ALLOCATION_INCREMENT;
	if(size > WF_TEXTURE_MAX){ gwarn("texture allocation full"); texture_cache_print(); return; }
	c->t = g_array_set_size(c->t, size);

#if 1
	guint textures[WF_TEXTURE_ALLOCATION_INCREMENT];
#else
	guint* textures = g_malloc0(WF_TEXTURE_ALLOCATION_INCREMENT * sizeof(guint)); //just testing but makes no difference
#endif
	glGenTextures(WF_TEXTURE_ALLOCATION_INCREMENT, textures);
	dbg(2, "size=%i-->%i textures=%u...%u", size-WF_TEXTURE_ALLOCATION_INCREMENT, size, textures[0], textures[WF_TEXTURE_ALLOCATION_INCREMENT-1]);
	if(glGetError() != GL_NO_ERROR) gwarn("failed to generate %i textures. cache_size=%i", WF_TEXTURE_ALLOCATION_INCREMENT, c->t->len);

	//check the new textures are not already in the cache
	int t; for(t=0;t< WF_TEXTURE_ALLOCATION_INCREMENT;t++){
		int idx = texture_cache_lookup_idx_by_id (textures[t]);
		if(idx > -1){
			Texture* tx = &g_array_index(c->t, Texture, t);
			gwarn("given duplicate texture id: %i wf=%p b=%i", textures[t], tx->wb.waveform, tx->wb.block);
		}
	}

	int i = 0;
	for(t=c->t->len-WF_TEXTURE_ALLOCATION_INCREMENT;t<c->t->len;t++, i++){
		Texture* tx = &g_array_index(wf->texture_cache->t, Texture, t);
		tx->id = textures[i];
	}
}


static void
texture_cache_shrink(int idx)
{
	dbg(1, "*** %i-->%i", c->t->len, idx);
	g_return_if_fail(!(idx % WF_TEXTURE_ALLOCATION_INCREMENT));

	guint textures[WF_TEXTURE_ALLOCATION_INCREMENT];

	int i = 0;
	int t; for(t=idx;t<idx+WF_TEXTURE_ALLOCATION_INCREMENT;t++, i++){
		Texture* tx = &g_array_index(c->t, Texture, t);
		textures[i] = tx->id;
	}
	glDeleteTextures(WF_TEXTURE_ALLOCATION_INCREMENT, textures);
	c->t = g_array_set_size(c->t, c->t->len - WF_TEXTURE_ALLOCATION_INCREMENT);
}


guint
texture_cache_assign_new (TextureCache* cache, WaveformBlock wfb)
{
	int t = texture_cache_get_new();
	int texture_id = texture_cache_get(t);
	texture_cache_assign(cache, t, wfb);
	return texture_id;
}


static void
texture_cache_assign(TextureCache* cache, int t, WaveformBlock wb)
{
	g_return_if_fail(t >= 0);
	g_return_if_fail(t < c->t->len);

	Texture* tx = &g_array_index(cache->t, Texture, t);
	tx->wb = wb;
	tx->time_stamp = time_stamp++;
	dbg(2, "t=%i b=%i time=%i", t, wb.block, time_stamp);

	static guint timeout = 0;
	if(wf_debug > 1){
		if(timeout) g_source_remove(timeout);
		gboolean _texture_cache_print(gpointer data)
		{
			texture_cache_print();
			return TIMER_STOP;
		}
		timeout = g_timeout_add(1000, _texture_cache_print, NULL);
	}
}


static void
texture_cache_queue_clean()
{
	static guint idle_id = 0;

	gboolean texture_cache_clean(gpointer user_data)
	{
		gboolean last_block_is_empty()
		{
			int m = c->t->len - 1;
			if(m == -1) return false;
			gboolean empty = true;
			int i; for(i=0;i<WF_TEXTURE_ALLOCATION_INCREMENT;i++){
				Texture* tx = &g_array_index(c->t, Texture, m);
				if(tx->wb.waveform){
					empty = false;
					break;
				}
				m--;
			}
			return empty;
		}
		int i = 0;
		while(
			true//(c->t->len > WF_TEXTURE_ALLOCATION_INCREMENT) //dont delete last block
			&& last_block_is_empty()
			&& (i++ < 10)
		) texture_cache_shrink(c->t->len - WF_TEXTURE_ALLOCATION_INCREMENT);

		idle_id = 0;
		return IDLE_STOP;
	}

	if(!idle_id) idle_id = g_idle_add(texture_cache_clean, NULL);
}


static void
texture_cache_unassign(WaveformBlock wb)
{
	g_return_if_fail(wb.waveform);

	dbg(2, "block=%i", wb.block);
	int i = 0;
	int t;
	while((t = texture_cache_lookup_idx(wb)) > -1){
		g_return_if_fail(t < c->t->len);

		Texture* tx = &g_array_index(c->t, Texture, t);
		g_return_if_fail(tx);
		tx->wb = (WaveformBlock){NULL, 0};
		tx->time_stamp = 0;
		dbg(2, "t=%i removed", t);
		i++;
		g_return_if_fail(i <= 4);
	}

	texture_cache_queue_clean();

	//texture_cache_print();
}


guint
texture_cache_get(int t)
{
	//WF* wf = wf_get_instance();
	Texture* tx = &g_array_index(c->t, Texture, t);
	return tx ? tx->id : 0;
}


int
texture_cache_lookup(WaveformBlock wb)
{
	dbg(2, "%p %i", wb.waveform, wb.block);
	int i; for(i=0;i<c->t->len;i++){
		Texture* t = &g_array_index(c->t, Texture, i);
		if(t->wb.waveform == wb.waveform && t->wb.block == wb.block){
			dbg(3, "found %i at %i", wb.block, i);
			return t->id;
		}
	}
	dbg(2, "not found: b=%i", wb.block);
	return -1;
}


static int
texture_cache_lookup_idx(WaveformBlock wb)
{
	int i; for(i=0;i<c->t->len;i++){
		Texture* t = &g_array_index(c->t, Texture, i);
		if(t->wb.waveform == wb.waveform && t->wb.block == wb.block){
			dbg(3, "found %i at %i", wb.block, i);
			return i;
		}
	}
	dbg(2, "not found: b=%i", wb.block);
	return -1;
}


static int
texture_cache_lookup_idx_by_id(guint id)
{
	int i; for(i=0;i<c->t->len;i++){
		Texture* t = &g_array_index(c->t, Texture, i);
		if(t->id == id){
			return i;
		}
	}
	return -1;
}


static int
texture_cache_get_new()
{
	int t = texture_cache_find_empty();
	if(t < 0){
		texture_cache_gen();
		t = texture_cache_find_empty();
		if(t < 0){
			t = texture_cache_steal();
		}
	}
	return t;
}


int
texture_cache_find_empty()
{
	int t; for(t=0;t<c->t->len;t++){
		Texture* tx = &g_array_index(c->t, Texture, t);
		if(!tx->wb.waveform){
			dbg(3, "%i", t);
			return t;
		}
	}
	return -1;
}


static int
texture_cache_steal()
{
	int oldest = -1;
	int n = -1;
	int t; for(t=0;t<c->t->len;t++){
		Texture* tx = &g_array_index(c->t, Texture, t);
		if(tx->wb.waveform){
			if(oldest == -1 || tx->time_stamp < oldest){
				n = t;
				oldest = tx->time_stamp;
			}
		}
	}
	if(n > -1){
		dbg(2, "%i time=%i", oldest, ((Texture*)&g_array_index(c->t, Texture, n))->time_stamp);
		//TODO clear all references to this texture
		WaveformBlock* wb = &((Texture*)&g_array_index(c->t, Texture, n))->wb;

		int find_texture_in_block(int n, WaveformBlock* wb)
		{
			WfGlBlock* blocks = wb->waveform->textures;
			guint* peak_texture[4] = {
				&blocks->peak_texture[0].main[wb->block],
				&blocks->peak_texture[0].neg[wb->block],
				&blocks->peak_texture[1].main[wb->block],
				&blocks->peak_texture[1].neg[wb->block]
			};
			int i; for(i=0;i<4;i++){
				if(peak_texture[i] && *peak_texture[i] == ((Texture*)&g_array_index(c->t, Texture, n))->id) return i;
			}
			return -1;
		}

		int p;
		if((p = find_texture_in_block(n, wb)) < 0){
			gwarn("!!");
		}else{
			dbg(1, "clearing texture for block=%i %i ...", wb->block, p);
			WfGlBlock* blocks = wb->waveform->textures;
			guint* peak_texture[4] = {
				&blocks->peak_texture[0].main[wb->block],
				&blocks->peak_texture[0].neg[wb->block],
				&blocks->peak_texture[1].main[wb->block],
				&blocks->peak_texture[1].neg[wb->block]
			};
			*peak_texture[p] = 0;
		}
	}
	return n;
}


void
texture_cache_remove(Waveform* w, int b)
{
	texture_cache_unassign((WaveformBlock){w, b});
}


void
texture_cache_remove_waveform(Waveform* waveform) //tmp? should probably only be called by wf_unref()
{
	int size0 = texture_cache_count_used();

	//TODO this first loop can be very long. dont do this when just using low res textures.
	int b; for(b=0;b<=waveform->textures->size;b++){
		texture_cache_unassign((WaveformBlock){waveform, b});
	}
	if(waveform->textures_lo){
		for(b=0;b<=waveform->textures_lo->size;b++){
			texture_cache_unassign((WaveformBlock){waveform, b | WF_TEXTURE_CACHE_LORES_MASK});
		}
	}

	dbg(2, "size=%i n_removed=%i", c->t->len, size0 - texture_cache_count_used());
	if(wf_debug) texture_cache_print();
}


static int
texture_cache_count_used()
{
	int n_used = 0;
	if(c->t->len){
		int i; for(i=0;i<c->t->len;i++){
			Texture* t = &g_array_index(c->t, Texture, i);
			if(t->wb.waveform) n_used++;
		}
	}
	return n_used;
}


static void
texture_cache_print()
{
	int n_used = 0;
	if(c->t->len){
		printf("         t  b  w\n");
		int i; for(i=0;i<c->t->len;i++){
			Texture* t = &g_array_index(c->t, Texture, i);
			printf("    %2i: %2i %i %p\n", i, t->time_stamp, t->wb.block, t->wb.waveform);
			if(t->wb.waveform) n_used++;
		}
	}
	dbg(0, "array_size=%i n_used=%i", c->t->len, n_used);
}


#endif //WF_USE_TEXTURE_CACHE

