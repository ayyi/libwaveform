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
#ifndef __wf_private_h__
#define __wf_private_h__

enum
{
	WF_LEFT = 0,
	WF_RIGHT,
	WF_MAX_CH
};

struct _peakbuf1 {
	int        size;             // the number of shorts allocated.
	short*     buf[WF_MAX_CH];   // holds the complete peakfile. The second pointer is only used for stereo files.
};

struct _waveform_priv
{
	WfPeakBuf          peak;
	WfAudioData*       audio_data;// tiered hi-res data for peaks.
	gint              _property1; // just testing
};

struct _wf
{
	int           peak_mem_size;
	GHashTable*   peak_cache;
	PeakLoader    load_peak;
	TextureCache* texture_cache;

	struct
	{
		GHashTable* cache;
		int         mem_size;
		int         access_counter;
	} audio;

	gboolean      pref_use_shaders;
	GAsyncQueue*  msg_queue;
};

struct __gl_blocks
{
	int                size;
	struct {
		unsigned*      main;
		unsigned*      neg;               // only used in shader mode.
	}                  peak_texture[WF_MAX_CH];
#ifdef WF_SHOW_RMS
	unsigned*          rms_texture;
#endif
	double             last_fraction;     // the fraction of the last block that is actually used.
};

typedef struct _texture
{
	guint         id;
	WaveformBlock wb;
	int           time_stamp;
} Texture;

WF*            wf_get_instance         ();
#if 0
WfWavCache*    wf_wav_cache_new        (int n_channels);
#endif
uint32_t       wf_peakbuf_get_max_size (int n_tiers);

short*         wf_peak_malloc          (Waveform*, uint32_t bytes);
Peakbuf*       wf_get_peakbuf_n        (Waveform*, int);
void           wf_peakbuf_regen        (Waveform*, int block_num, int min_output_resolution);
void           wf_print_blocks         (Waveform*);

void           wf_audio_free           (Waveform*);
gboolean       wf_load_audio_block     (Waveform*, int block_num);

void           wf_actor_init           ();
WaveformActor* wf_actor_new            (Waveform*);
void           wf_actor_load_texture1d (Waveform*, int blocknum);

float          wf_canvas_gl_to_px      (WaveformCanvas*, float x);

#endif //__wf_private_h__
