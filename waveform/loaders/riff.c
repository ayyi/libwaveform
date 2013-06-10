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
#define ENABLE_CHECKS
#include "config.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <sys/time.h>
#include <glib.h>
#include <sndfile.h>
#include "waveform/waveform.h"

#define peak_byte_depth 2 // value are stored in the peak file as int16.


int
wf_load_riff_peak(Waveform* wv, const char* peak_file, size_t size)
{
	g_return_val_if_fail(wv, 0);
	PF2;

	SNDFILE* sndfile;
	SF_INFO sfinfo;
	sfinfo.format = 0;
	if(!(sndfile = sf_open(peak_file, SFM_READ, &sfinfo))){
		if(!g_file_test(peak_file, G_FILE_TEST_EXISTS)){
			gwarn("file open failure. file doesnt exist.");
		}else{
			gwarn("file open failure.");
		}
		goto out;
	}
	if(!(sfinfo.format & SF_FORMAT_PCM_16)){
		gwarn("TESTME not 16 bit");
	}
	if(!(sfinfo.format & SF_FORMAT_WAV)){
		gwarn("not wav format");
	}
	g_return_val_if_fail(sfinfo.channels <= 2, 0);

	sf_count_t n_frames = sfinfo.frames / WF_PEAK_VALUES_PER_SAMPLE;
	dbg(2, "n_channels=%i n_frames=%Li n_bytes=%Li n_blocks=%i", sfinfo.channels, n_frames, sfinfo.frames * peak_byte_depth * sfinfo.channels, (int)(ceil((float)n_frames / WF_PEAK_TEXTURE_SIZE)));
	dbg(2, "secs=%.3f %.3f", ((float)(n_frames)) / 44100, ((float)(n_frames * WF_PEAK_RATIO)) / 44100);

	uint32_t bytes = sfinfo.frames * peak_byte_depth * WF_PEAK_VALUES_PER_SAMPLE;

	short* read_buf = (sfinfo.channels == 1)
		? waveform_peak_malloc(wv, bytes) //no deinterleaving required, so can read directly into the peak buffer.
		: g_malloc(bytes);

	//read the whole peak file into memory:
	int readcount_frames;
	if((readcount_frames = sf_readf_short(sndfile, read_buf, sfinfo.frames)) < sfinfo.frames){
		gwarn("unexpected EOF: %s - read %i of %Li items", peak_file, readcount_frames, n_frames);
		//gerr ("read error. couldnt read %i bytes from %s", bytes, peak_file);
	}
	sf_close(sndfile);

#if 0
	int i; for (i=0;i<20;i++) printf("  %i %i\n", buf[2 * i], buf[2 * i + 1]);
#endif

	int ch_num = 0; //TODO
	if(sfinfo.channels == 1){
		wv->priv->peak.buf[ch_num] = read_buf;
	}else if(sfinfo.channels == 2){
		short* buf[WF_MAX_CH] = {
			waveform_peak_malloc(wv, bytes / sfinfo.channels),
			waveform_peak_malloc(wv, bytes / sfinfo.channels)
		};
		int i; for(i=0;i<readcount_frames/(2);i++){
			int c; for(c=0;c<sfinfo.channels;c++){
				int src = 2 * (i * sfinfo.channels + c);
				buf[c][2 * i    ] = read_buf[src    ]; // +
				buf[c][2 * i + 1] = read_buf[src + 1]; // -
			}
			//if(i < 10) dbg(0, " %i", buf[0][i]);
		}
		wv->priv->peak.buf[ch_num    ] = buf[WF_LEFT ];
		wv->priv->peak.buf[ch_num + 1] = buf[WF_RIGHT];
		/*
		for(i=0;i<10;i+=2){
			printf("^ %i %i %i %i\n", wv->priv->peak.buf[WF_LEFT][i], wv->priv->peak.buf[WF_RIGHT][i], wv->priv->peak.buf[WF_LEFT][i+1], wv->priv->peak.buf[WF_RIGHT][i+1]);
		}
		*/
		g_free(read_buf);
	}
	wv->priv->peak.size = n_frames * WF_PEAK_VALUES_PER_SAMPLE;
	dbg(2, "peak.size=%i", wv->priv->peak.size);
#ifdef ENABLE_CHECKS
	int k; for(k=0;k<10;k++){
		if(wv->priv->peak.buf[0][2*k + 0] < 0.0){ gwarn("positive peak not positive"); break; }
		if(wv->priv->peak.buf[0][2*k + 1] > 0.0){ gwarn("negative peak not negative"); break; }
	}
#endif
  out:
	return sfinfo.channels;
}

