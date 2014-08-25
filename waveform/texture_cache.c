/*
  copyright (C) 2012-2014 Tim Orford <tim@orford.org>

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
static TextureCache* c1 = NULL; // 1d textures
static TextureCache* c2 = NULL; // 2d textures
#define cache_by_type(T) (T == GL_TEXTURE_2D ? c2 : c1)
static WF* wf = NULL;

static void texture_cache_gen              (TextureCache*);
static guint texture_cache_get             (TextureCache*, int);
static int  texture_cache_get_new          (TextureCache*);
static void texture_cache_assign           (TextureCache*, int, WaveformBlock);
static int  texture_cache_find_empty       (TextureCache*);
static int  texture_cache_steal            (TextureCache*);
       void texture_cache_print            ();
static int  texture_cache_lookup_idx       (TextureCache*, WaveformBlock);
#ifdef WF_DEBUG
static int  texture_cache_lookup_idx_by_id (TextureCache*, guint);
#endif
static void texture_cache_unassign         (TextureCache*, WaveformBlock);
static void texture_cache_shrink           (TextureCache*, int);
static int  texture_cache_count_used       (TextureCache*);


void
texture_cache_init()
{
	if(c1) return;

	wf = wf_get_instance();

	c1 = g_new0(TextureCache, 1);
	c1->t = g_array_new(FALSE, TRUE, sizeof(Texture));
	c1->t = g_array_set_size(c1->t, 0);

	c2 = g_new0(TextureCache, 1);
	c2->t = g_array_new(FALSE, TRUE, sizeof(Texture));
	c2->t = g_array_set_size(c2->t, 0);
}


static void
texture_cache_gen(TextureCache* c)
{
	//create an additional set of available textures.

	if(!c1) texture_cache_init();

	static bool error_shown = false;

#if 0
	//check all textures
	{
		int i; for(i=0;i<c->t->len;i++){
			Texture* tx = &g_array_index(c->t, Texture, i);
			if(!glIsTexture(tx->id)) gwarn("not texture! %i: %i", i, tx->id);
		}
	}
#endif

	int size = c->t->len + WF_TEXTURE_ALLOCATION_INCREMENT;
	if(size > WF_TEXTURE_MAX){ if(wf_debug){ gwarn("texture allocation full"); if(!error_shown) texture_cache_print(); error_shown = true;} return; }
	c->t = g_array_set_size(c->t, size);

	guint textures[WF_TEXTURE_ALLOCATION_INCREMENT];
	glGenTextures(WF_TEXTURE_ALLOCATION_INCREMENT, textures);
	dbg(2, "size=%i-->%i textures=%u...%u", size-WF_TEXTURE_ALLOCATION_INCREMENT, size, textures[0], textures[WF_TEXTURE_ALLOCATION_INCREMENT-1]);
	if(glGetError() != GL_NO_ERROR) gwarn("failed to generate %i textures. cache_size=%i", WF_TEXTURE_ALLOCATION_INCREMENT, c->t->len);

	int t;
#ifdef WF_DEBUG
	//check the new textures are not already in the cache
	for(t=0;t< WF_TEXTURE_ALLOCATION_INCREMENT;t++){
		int idx = texture_cache_lookup_idx_by_id (c, textures[t]);
		if(idx > -1){
			Texture* tx = &g_array_index(c->t, Texture, t);
			gwarn("given duplicate texture id: %i wf=%p b=%i", textures[t], tx->wb.waveform, tx->wb.block);
		}
	}
#endif

	int i = 0;
	for(t=c->t->len-WF_TEXTURE_ALLOCATION_INCREMENT;t<c->t->len;t++, i++){
		Texture* tx = &g_array_index(c->t, Texture, t);
		tx->id = textures[i];
	}
}


static void
texture_cache_shrink(TextureCache* c, int idx)
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
texture_cache_assign_new (int tex_type, WaveformBlock wfb)
{
	TextureCache* cache = cache_by_type(tex_type);

	if(wfb.block & WF_TEXTURE_CACHE_HIRES_MASK){
		dbg(0, "HI RES");
	}

	int t = texture_cache_get_new(cache);
	int texture_id = texture_cache_get(cache, t);
	texture_cache_assign(cache, t, wfb);
	return texture_id;
}


static void
texture_cache_assign(TextureCache* c, int t, WaveformBlock wb)
{
	g_return_if_fail(t >= 0);
	g_return_if_fail(t < c->t->len);

	Texture* tx = &g_array_index(c->t, Texture, t);
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


void
texture_cache_freshen(int tex_type, WaveformBlock wb)
{
	TextureCache* c = cache_by_type(tex_type);

	int i = texture_cache_lookup_idx(c, wb);
	if(i > -1){
		Texture* tx = &g_array_index(c->t, Texture, i);
		tx->time_stamp = time_stamp++;
	}
}


static void
texture_cache_queue_clean()
{
	static guint idle_id = 0;

	gboolean texture_cache_clean(gpointer user_data)
	{
		gboolean last_block_is_empty(TextureCache* c)
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
		int j; for(j=0;j<2;j++){
			TextureCache* c = j ? c2 : c1;
			while(
				(c->t->len > WF_TEXTURE_ALLOCATION_INCREMENT) //dont delete last block
				&& last_block_is_empty(c)
				&& (i++ < 10)
			) texture_cache_shrink(c, c->t->len - WF_TEXTURE_ALLOCATION_INCREMENT);
		}

		idle_id = 0;
		return IDLE_STOP;
	}

	if(!idle_id) idle_id = g_idle_add(texture_cache_clean, NULL);
}


static void
texture_cache_unassign(TextureCache* c, WaveformBlock wb)
{
	g_return_if_fail(wb.waveform);

	dbg(2, "block=%i", wb.block);
	int i = 0;
	int t;
	while((t = texture_cache_lookup_idx(c, wb)) > -1){
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


static guint
texture_cache_get(TextureCache* c, int t)
{
	Texture* tx = &g_array_index(c->t, Texture, t);
	return tx ? tx->id : 0;
}


int
texture_cache_lookup(int tex_type, WaveformBlock wb)
{
	TextureCache* c = cache_by_type(tex_type);

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
texture_cache_lookup_idx(TextureCache* c, WaveformBlock wb)
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


#ifdef WF_DEBUG
static int
texture_cache_lookup_idx_by_id(TextureCache* c, guint id)
{
	int i; for(i=0;i<c->t->len;i++){
		Texture* t = &g_array_index(c->t, Texture, i);
		if(t->id == id){
			return i;
		}
	}
	return -1;
}
#endif


static int
texture_cache_get_new(TextureCache* c)
{
	int t = texture_cache_find_empty(c);
	if(t < 0){
		texture_cache_gen(c);
		t = texture_cache_find_empty(c);
		if(t < 0){
			t = texture_cache_steal(c);
		}
	}
	return t;
}


static int
texture_cache_find_empty(TextureCache* c)
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
texture_cache_steal(TextureCache* c)
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
		// clear all references to this texture

		dbg(2, "%i time=%i", oldest, ((Texture*)&g_array_index(c->t, Texture, n))->time_stamp);
		Texture* tex = (Texture*)&g_array_index(c->t, Texture, n);
		WaveformBlock* wb = &tex->wb;

		if(wb->block & WF_TEXTURE_CACHE_HIRES_NG_MASK){
			extern void hi_gl2_on_steal(WaveformBlock*, guint);
			hi_gl2_on_steal(wb, tex->id);
		}else{
			extern void med_lo_on_steal(WaveformBlock*, guint);
			med_lo_on_steal(wb, tex->id);
		}
	}
	return n;
}


void
texture_cache_remove(int tex_type, Waveform* w, int b)
{
	texture_cache_unassign(cache_by_type(tex_type), (WaveformBlock){w, b});
}


void
texture_cache_remove_waveform(Waveform* waveform) //tmp? should probably only be called by wf_unref()
{
	int j; for(j=0;j<2;j++){
		TextureCache* c = j ? c2 : c1;

		int size0 = texture_cache_count_used(c);

		//TODO this first loop can be very long. dont do this when just using low res textures.
		int b; for(b=0;b<=((WfGlBlock*)waveform->priv->render_data[MODE_MED])->size;b++){
			texture_cache_unassign(c, (WaveformBlock){waveform, b});
		}
		if(waveform->priv->render_data[MODE_LOW]){
			for(b=0;b<=((WfGlBlock*)waveform->priv->render_data[MODE_LOW])->size;b++){
				texture_cache_unassign(c, (WaveformBlock){waveform, b | WF_TEXTURE_CACHE_LORES_MASK});
			}
		}

		dbg(2, "size=%i n_removed=%i", c->t->len, size0 - texture_cache_count_used(c));
	}
	if(wf_debug) texture_cache_print();
}


static int
texture_cache_count_used(TextureCache* c)
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


void
texture_cache_print()
{
	int j; for(j=0;j<2;j++){
		TextureCache* c = j ? c2 : c1;
		dbg(0, "%s", j ? "2D:" : "1D:");
		int n_used = 0;
		GList* waveforms = NULL;
		if(c->t->len){
			printf("         %2s %3s  %3s   %-4s\n", "id", "ts", "b", "wvfm");
			int i; for(i=0;i<c->t->len;i++){
				Texture* t = &g_array_index(c->t, Texture, i);
				if(t->wb.waveform){
					n_used++;
					if(!g_list_find(waveforms, t->wb.waveform)) waveforms = g_list_append(waveforms, t->wb.waveform);
				}
				char* mode = (!t->wb.waveform)
					? " "
					: (t->wb.block & WF_TEXTURE_CACHE_LORES_MASK)
						? "L"
						: (t->wb.block & WF_TEXTURE_CACHE_HIRES_MASK)
							? "h"
							: (t->wb.block & WF_TEXTURE_CACHE_HIRES_NG_MASK)
								? "H"
								: "M";
				printf("    %3i: %2u %3i %4i %s %4i\n", i, t->id, t->time_stamp, t->wb.block & (~(WF_TEXTURE_CACHE_LORES_MASK | WF_TEXTURE_CACHE_HIRES_NG_MASK)), mode, g_list_index(waveforms, t->wb.waveform) + 1);
			}
		}
		dbg(0, "array_size=%i n_used=%i n_waveforms=%i", c->t->len, n_used, g_list_length(waveforms));
		g_list_free(waveforms);
	}
}


#endif //WF_USE_TEXTURE_CACHE

