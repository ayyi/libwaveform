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

  ---------------------------------------------------------

  peakgen
  -------

  description:
  - generates a peakfile from an audio file.
  - output is 16bit, alternating positive and negative peaks
  - output has riff header so we know what type of peak file it is. Could possibly revert to headerless file.
  - peak files are cached in ~/.cache/

  todo:
  - what is maximum file size?

*/
#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <sndfile.h>
#include "waveform/utils.h"
#include "waveform/peak.h"
#include "waveform/audio.h"
#include "waveform/peakgen.h"

#define BUFFER_LEN 256 // length of the buffer to hold audio during processing. currently must be same as WF_PEAK_RATIO
#define MAX_CHANNELS 2

#define DEFAULT_USER_CACHE_DIR ".cache/peak"

static int           peak_mem_size = 0;
static bool          need_file_cache_check = true;

static inline void   process_data        (short* data, int count, int channels, long long pos, short max[], short min[]);
static unsigned long sample2time         (SF_INFO, long samplenum);
static bool          wf_file_is_newer    (const char*, const char*);
static bool          wf_create_cache_dir ();
static char*         get_cache_dir       ();
static void          maintain_file_cache ();


char*
waveform_ensure_peakfile (Waveform* w)
{
	gchar* peak_filename = NULL;

	if(!wf_create_cache_dir()) return NULL;

	char* filename = g_path_is_absolute(w->filename) ? g_strdup(w->filename) : g_build_filename(g_get_current_dir(), w->filename, NULL);

	GError* error = NULL;
	gchar* uri = g_filename_to_uri(filename, NULL, &error);
	if(error){
		gwarn("%s", error->message);
		goto out;
	}
	dbg(1, "uri=%s", uri);

	gchar* md5 = g_compute_checksum_for_string(G_CHECKSUM_MD5, uri, -1);
	g_free(uri);
	gchar* peak_basename = g_strdup_printf("%s.peak", md5);
	g_free(md5);
	char* cache_dir = get_cache_dir();
	peak_filename = g_build_filename(cache_dir, peak_basename, NULL);
	g_free(cache_dir);
	dbg(1, "peak_filename=%s", peak_filename);
	g_free(peak_basename);

	if(g_file_test(peak_filename, G_FILE_TEST_EXISTS)){ 
		dbg (1, "peak file exists. (%s)", peak_filename);

		/*
		note that this test will fail to detect a modified file, if an older file is now stored at this location.

		The freedesktop thumbnailer spec identifies modifications by comparing with both url and mtime stored in the thumbnail.
		This will mostly work, but strictly speaking still won't identify a changed file in all cases.
		*/
		if(w->offline || wf_file_is_newer(peak_filename, filename)) return peak_filename;

		dbg(1, "peakfile is too old");
	}

	if(!wf_peakgen(filename, peak_filename)){ g_free0(peak_filename); goto out; }

  out:
	g_free(filename);

	return peak_filename;
}


#define FAIL_ \
	sf_close (infile); \
	return false;

gboolean
wf_peakgen(const char* infilename, const char* peak_filename)
{
	//return true on success

	g_return_val_if_fail(infilename, false);
	PF;

	SNDFILE *infile, *outfile;

	SF_INFO sfinfo;
	if(!(infile = sf_open(infilename, SFM_READ, &sfinfo))){
		if(!g_file_test(infilename, G_FILE_TEST_EXISTS)){
			printf("peakgen: no such input file: '%s'\n", infilename);
		}else{
			printf("peakgen: not able to open input file %s.\n", infilename);
			puts(sf_strerror (NULL));
		}
		return false;
	}
	dbg(1, "n_frames=%Lu %i", sfinfo.frames, ((int)sfinfo.frames/256));

	if (sfinfo.channels > MAX_CHANNELS){
		printf ("Not able to process more than %d channels\n", MAX_CHANNELS) ;
		return false;
	};
	if(wf_debug) printf("samplerate=%i channels=%i frames=%i\n", sfinfo.samplerate, sfinfo.channels, (int)sfinfo.frames);
	if(sfinfo.channels > 16){ printf("format not supported. unexpected number of channels: %i\n", sfinfo.channels); FAIL_; }

	short* data = g_malloc0(sizeof(short) * BUFFER_LEN * sfinfo.channels);

	//copy the sfinfo for the output file:
	SF_INFO sfinfo_w;
	sfinfo_w.channels   = sfinfo.channels;
	sfinfo_w.format     = sfinfo.format;
	sfinfo_w.samplerate = sfinfo.samplerate;
	sfinfo_w.seekable   = sfinfo.seekable;

	int bytes_per_frame = sfinfo.channels * sizeof(short);

	if(!(outfile = sf_open(peak_filename, SFM_WRITE, &sfinfo_w))){
		printf ("Not able to open output file %s.\n", peak_filename);
		puts(sf_strerror(NULL));
		FAIL_;
	}

	#define EIGHT_HOURS (60 * 60 * 8)
	#define MAX_READ_ITER (44100 * EIGHT_HOURS / BUFFER_LEN)

	//while there are frames in the input file, read them and process them:
	short total_max[sfinfo.channels];
	int readcount, i = 0;
	long long samples_read = 0;
	gint32 total_bytes_written = 0;
	gint32 total_frames_written = 0;
	while((readcount = sf_readf_short(infile, data, BUFFER_LEN))){
		if(wf_debug && (readcount < BUFFER_LEN)){
			dbg(1, "EOF i=%i readcount=%i total_frames_written=%i", i, readcount, total_frames_written);
		}

		short max[sfinfo.channels];
		short min[sfinfo.channels];
		memset(max, 0, sizeof(short) * sfinfo.channels);
		memset(min, 0, sizeof(short) * sfinfo.channels);
		process_data(data, readcount, sfinfo.channels, samples_read, max, min);
		samples_read += readcount;
		memcpy(total_max, max, sizeof(short) * sfinfo.channels);
		short w[sfinfo.channels][2];
		int c; for(c=0;c<sfinfo.channels;c++){
			w[c][0] = max[c];
			w[c][1] = min[c];
		}
		total_frames_written += sf_write_short (outfile, (short*)w, WF_PEAK_VALUES_PER_SAMPLE * sfinfo.channels);
		total_bytes_written += sizeof(short);
#if 0
		if(sfinfo.channels == 2){
			short* z = w;
			if(i<10) printf("  %i %i %i %i\n", z[0], z[1], z[2], z[3]);
		}else{
			if(i<10) printf("  %i %i\n", max[0], min[0]);
		}
#endif
		if(++i > MAX_READ_ITER){ printf("warning: stopped before EOF.\n"); break; }
	}

	if(wf_debug){
		long secs = sample2time(sfinfo, samples_read) / 1000;
		long ms   = sample2time(sfinfo, samples_read) - 1000 * secs;
		long mins = secs / 60;
		secs = secs - mins * 60;
		printf("size: %'Li bytes %li:%02li:%03li. maxlevel=%i(%fdB)\n", samples_read * sizeof(short), mins, secs, ms, total_max[0], wf_int2db(total_max[0]));
	}

	dbg(1, "total_items_written=%i items_per_channel=%i peaks_per_channel=%i", total_frames_written, total_frames_written/sfinfo.channels, total_frames_written/(sfinfo.channels * WF_PEAK_VALUES_PER_SAMPLE));
	dbg(2, "total bytes written: %i (of%Li)", total_bytes_written, (long long)sfinfo.frames * bytes_per_frame);

	sf_close (outfile);
	sf_close (infile);
	g_free(data);

	if(need_file_cache_check){
		maintain_file_cache();
		need_file_cache_check = false;
	}

	return true;
#if 0    //goto wont compile
failed_:
	sf_close (infile);
	return false;
#endif
}


static inline void
process_data (short* data, int data_size_frames, int n_channels, long long Xpos, short max[], short min[])
{
	//produce peak data from audio data.
	//
	//      data - interleaved audio.
	//      data_size_frames - the number of frames in data to process.
	//      pos  - is the *sample* count. (ie not frame count, and not byte count.)
	//      max  - (output) positive peaks. must be allocated with size n_channels
	//      min  - (output) negative peaks. must be allocated with size n_channels

	/*
	for (k = chan; k < count; k+= channels){
		for (chan=0; chan < channels; chan++)
			//data [k] *= channel_gain [chan] ;
			printf("data: (%i) %i\n", chan, data[k]);
	}*/
	int c; for(c=0;c<n_channels;c++){
		max[c] = 0;
		min[c] = 0;
	}
	int k; for(k=0;k<data_size_frames;k+=n_channels){
		//for(chan=0; chan<channels; chan++) printf("data: (%i) %i\n", chan, data[k+chan]);

		for(c=0;c<n_channels;c++){
			max[c] = (data[k + c] > max[c]) ? data[k + c] : max[c];
			min[c] = (data[k + c] < min[c]) ? data[k + c] : min[c];
		}
	}
}


//long is good up to 4.2GB
unsigned long
sample2time(SF_INFO sfinfo, long samplenum)
{
	//returns time in milliseconds, given the sample number.
	
	//int64_t milliseconds;
	unsigned long milliseconds;
	milliseconds = (10 * samplenum) / ((sfinfo.samplerate/100) * sfinfo.channels);
	//printf("                                    %9li 10=%li  milliseconds=%li\n", samplenum, (10 * samplenum), milliseconds);
	
	return milliseconds;
}


static gboolean
wf_create_cache_dir()
{
	gchar* path = get_cache_dir();
	gboolean ret  = !g_mkdir_with_parents(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP);
	if(!ret) gwarn("cannot access cache dir: %s", path);
	g_free(path);
	return ret;
}


static gboolean
is_last_block(Waveform* waveform, int block_num)
{
	//dont use hires_peaks->len here - thats the number allocated.

	int n_blocks = waveform_get_n_audio_blocks(waveform);

	dbg(2, "n_blocks=%i block_num=%i", n_blocks, block_num);
	return (block_num == (n_blocks - 1));
}


static void*
peakbuf_allocate(Peakbuf* peakbuf, int c)
{
	g_return_val_if_fail(c < WF_STEREO, NULL);
	if(peakbuf->buf[c]){ gwarn("buffer already allocated. c=%i", c); return NULL; }

	peakbuf->buf[c] = g_malloc0(sizeof(short) * peakbuf->size);
	peak_mem_size += (peakbuf->size * sizeof(short));

	//dbg(0, "short=%i", sizeof(short));
	dbg(1, "b=%i: size=%i tot_peak_mem=%ikB", peakbuf->block_num, peakbuf->size, peak_mem_size / 1024);
	return peakbuf->buf[c];
}


static void
peakbuf_set_n_tiers(Peakbuf* peakbuf, int n_tiers, int resolution)
{
	if(n_tiers < 1 || n_tiers > MAX_TIERS){ gwarn("n_tiers out of range: %i", n_tiers); n_tiers = MAX_TIERS; }
	peakbuf->n_tiers = n_tiers;
	peakbuf->resolution = resolution;
	dbg(2, "n_tiers=%i", n_tiers);
}


void
waveform_peakbuf_regen(Waveform* waveform, int block_num, int min_tiers)
{
	// make a ram peakbuf for a single block.
	//  -the needed audio file data is assumed to be already available.
	// @min_tiers -specifies the _minimum_ resolution. The peakbuf produced may be of higher resolution than this.

	// TODO caching: consider saving to disk, and using kernel caching. clear cache on exit.

	PF;

	int min_output_tiers = min_tiers;

	WF* wf = wf_get_instance();
	WfAudioData* audio = waveform->priv->audio_data;
	g_return_if_fail(audio);
	g_return_if_fail(audio->buf16 && (block_num < audio->n_blocks) && audio->buf16[block_num]);

	int input_resolution = 256 / (1 << audio->n_tiers_present); // i think this is the same as "spacing"

	//decide the size of the peakbuf:
	//-------------------------------
	// -here we jump in peakbuf sizes of x16 - could change to using 4 stages of x4 if beneficial.
	int output_resolution = WF_PEAK_RATIO; // output_resolution goes from 1 meaning full resolution, to 256 for normal lo-res peakbuf
	int output_tiers      = 0;
	if(min_output_tiers >= 0){ output_resolution = WF_PEAK_RATIO >> 4; output_tiers = 4; } //=16
	if(min_output_tiers >  3){ output_resolution = 1;                  output_tiers = 8; } //use the whole file!
	int io_ratio = output_resolution / input_resolution; //number of input bytes for 1 byte of output
	dbg(2, "%i: min_output_tiers=%i input_resolution=%i output_resolution=%i io_ratio=%i", block_num, min_output_tiers, input_resolution, output_resolution, io_ratio);

	/*

	input_resolution:  the amount of audio data that is available. 1 corresponds to the full audio file. 256 would correspond to the cached peakfile.
	output_resolution: the resolution of the produced Peakbuf.
	io_ratio:          the number of input bytes for 1 byte of output
	peakbuf_size:      =blocksize*?

	the most common case is for input_resolution=1 and output_resolution=16

	n_tiers_available  input_resolution  output_resolution  io_ratio  output_n_tiers peakbuf_size
	-----------------  ----------------  -----------------  --------  -------------- ------------
	8                  1                   1                1
	7                  2                   2                1
	6                                      4
	5                                      8
	4                                     16
	3                                     32
	2                                     64
	1                  128               128                1
	0                                    256 (the lo-res case)

	the case we want is where the whole file is available, but peakbuf size is 1/16 size
	8                                      1                 1                     8          1MB
	8                                      4                 4                     6        256kB
	8                  1                  16                16                     4         64kB
	8                  1                  64                64                     2         16kB
	8                                    128               128                     1          8kB

	for lo-res peaks, one peak block contains 256 * 8 frames.                    =4kB
	for hi-res peaks, one peak block contains 256 * 8 * resolution frames ?      =8k --> 1MB

	buffer sizes in bytes:

		if WF_PEAK_BLOCK_SIZE == 256 * 32:
			10s file has 54 blocks
			full res audio:      16k  <-- this is too small, can increase.
			with io=16, peakbuf:  2k

		if WF_PEAK_BLOCK_SIZE == 256 * 256: (equiv to a regular med res peak file)
			10s file has ~7 blocks
			full res audio:      128k <-- seems reasonable but still not large.
			with io=16, peakbuf:  16k

	*/

	Peakbuf* peakbuf = waveform_get_peakbuf_n(waveform, block_num);
	g_return_if_fail(peakbuf);
	short* buf = peakbuf->buf[WF_LEFT];
	dbg(3, "peakbuf=%p buf0=%p", peakbuf, peakbuf->buf);
	if(!buf){
		//peakbuf->size = peakbuf_get_max_size_by_resolution(output_resolution);
		//peakbuf->size = wf_peakbuf_get_max_size(output_tiers);
		dbg(2, "buf->size=%i blocksize=%i", peakbuf->size, WF_PEAK_BLOCK_SIZE * WF_PEAK_VALUES_PER_SAMPLE / io_ratio);
		peakbuf->size = audio->buf16[block_num]->size * WF_PEAK_VALUES_PER_SAMPLE / io_ratio;
		if(is_last_block(waveform, block_num)){
			dbg(2, "is_last_block. (%i)", block_num);
			//if(block_num == 0) peakbuf->size = pool_item->samplecount / PEAK_BLOCK_TO_GRAPHICS_BLOCK * PEAK_TILE_SIZE;
		}
		//dbg(0, "block_num=%i of %i. allocating buffer... %i %i %i", block_num, pool_item->hires_peaks->len, PEAK_BLOCK_TO_GRAPHICS_BLOCK, PEAK_TILE_SIZE, (256 >> (output_resolution-1)));
		int c; for(c=0;c<waveform_get_n_channels(waveform);c++){
			buf = peakbuf_allocate(peakbuf, c);
		}
	}

	short maxplus[2] = {0,0};      // highest positive audio value encountered (per block). One for each channel
	short maxmin [2] = {0,0};      // highest negative audio value encountered.
	short totplus = 0;
	short totmin  = 0;

	int n_chans = waveform_get_n_channels(waveform);
	int c; for(c=0;c<n_chans;c++){
		short* buf = peakbuf->buf[c];
		WfBuf16* audio_buf = audio->buf16[block_num];
									g_return_if_fail(peakbuf->size >= WF_PEAK_BLOCK_SIZE * WF_PEAK_VALUES_PER_SAMPLE / io_ratio);
		audio_buf->stamp = ++wf->audio.access_counter;
		int i, p; for(i=0, p=0; p<WF_PEAK_BLOCK_SIZE; i++, p+= io_ratio){

//#endif
			process_data/*_real*/(&audio_buf->buf[c][p], io_ratio, 1, 0, (short*)&maxplus, (short*)&maxmin);

#if 0
			short* dd = &audio_buf->buf[c][p];
			if(i < 20) printf("    %i %i %i %i\n", audio_buf->buf[WF_LEFT][p], dd[0], maxplus[0], maxmin [0]);
#endif

			buf[2 * i    ] = maxplus[0];
			buf[2 * i + 1] = maxmin [0];

			totplus = MAX(totplus, maxplus[0]);
			totmin  = MIN(totmin,  maxmin [0]);
		}

		peakbuf->maxlevel = MAX(peakbuf->maxlevel, MAX(totplus, -totmin));

		/*
		for(i=0;i<10;i++){
			printf("      %i %i\n", buf[2 * i], buf[2 * i + 1]);
		}
		*/
	}
	dbg(1, "maxlevel=%i,%i (%.3fdB)", totplus, totmin, wf_int2db(MAX(totplus, -totmin)));

	peakbuf_set_n_tiers(peakbuf, output_tiers, output_resolution);
}


static bool
wf_file_is_newer(const char* file1, const char* file2)
{
	//return TRUE if file1 date is newer than file2 date.

	struct stat info1;
	struct stat info2;
	if(stat(file1, &info1)) return false;
	if(stat(file2, &info2)) return false;
	if(info1.st_mtime < info2.st_mtime) dbg(2, "%i %i %i", info1.st_mtime, info2.st_mtime, sizeof(time_t));
	return (info1.st_mtime > info2.st_mtime);
}


static char*
get_cache_dir()
{
	const gchar* env = g_getenv("XDG_CACHE_HOME");
	if(env) dbg(0, "cache_dir=%s", env);
	if(env) return g_strdup(env);

	gchar* dir_name = g_build_filename(g_get_home_dir(), DEFAULT_USER_CACHE_DIR, NULL);
	return dir_name;
}


static void
maintain_file_cache()
{
	// http://people.freedesktop.org/~vuntz/thumbnail-spec-cache/delete.html

	#define CACHE_EXPIRY_DAYS 30

	bool _maintain_file_cache()
	{
		char* dir_name = get_cache_dir();
		dbg(2, "dir=%s", dir_name);
		GError* error = NULL;
		GDir* d = g_dir_open(dir_name, 0, &error);

		struct timeval time;
		gettimeofday(&time, NULL);
		time_t now = time.tv_sec;

		int n_deleted = 0;
		struct stat info;
		const char* leaf;
		while ((leaf = g_dir_read_name(d))) {
			if (g_str_has_suffix(leaf, ".peak")) {
				gchar* filename = g_build_filename(dir_name, leaf, NULL);
				if(!stat(filename, &info)){
					time_t days_old = (now - info.st_mtime) / (60 * 60 * 24);
					//dbg(0, "%i days_old=%i", info.st_mtime, days_old);
					if(days_old > CACHE_EXPIRY_DAYS){
						dbg(2, "deleting: %s", filename);
						g_unlink(filename);
						n_deleted++;
					}
				}
				g_free(filename);
			}
		}
		dbg(1, "peak files deleted: %i", n_deleted);

		g_dir_close(d);
		g_free(dir_name);

		return IDLE_STOP;
	}

	g_idle_add(_maintain_file_cache, NULL);
}


