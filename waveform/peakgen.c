/*
  copyright (C) 2012-2019 Tim Orford <tim@orford.org>

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
  - peak files are expired after 90 days
  - there is no size limit to the cache directory

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
#ifdef USE_SNDFILE
#include <sndfile.h>
#else
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/frame.h>
#include <libavutil/samplefmt.h>
#endif
#include "decoder/ad.h"
#ifdef USE_OPENGL
#include "agl/utils.h"
#endif
#include "waveform/waveform.h"
#include "waveform/audio.h"
#include "waveform/worker.h"
#include "waveform/loaders/ardour.h"
#include "waveform/peakgen.h"

#define BUFFER_LEN 256 // length of the buffer to hold audio during processing. currently must be same as WF_PEAK_RATIO
#define MAX_CHANNELS 2

#define DEFAULT_USER_CACHE_DIR ".cache/peak"

static int           peak_mem_size = 0;
static bool          need_file_cache_check = true;

static inline void   process_data        (short* data, int count, int channels, short max[], short min[]);
#ifdef UNUSED
static unsigned long sample2time         (SF_INFO, long samplenum);
#endif
static bool          wf_file_is_newer    (const char*, const char*);
static bool          wf_create_cache_dir ();
static char*         get_cache_dir       ();
static void          maintain_file_cache ();

static WfWorker peakgen = {0,};


static char*
waveform_get_peak_filename(const char* filename)
{
	// filename should be absolute.
	// caller must g_free the returned value.

	if(wf->load_peak == wf_load_ardour_peak){
		gwarn("cannot automatically determine path of Ardour peakfile");
		return NULL;
	}

	GError* error = NULL;
	gchar* uri = g_filename_to_uri(filename, NULL, &error);
	if(error){
		gwarn("%s", error->message);
		return NULL;
	}
	dbg(1, "uri=%s", uri);

	gchar* md5 = g_compute_checksum_for_string(G_CHECKSUM_MD5, uri, -1);
	g_free(uri);
	gchar* peak_basename = g_strdup_printf("%s.peak", md5);
	g_free(md5);
	char* cache_dir = get_cache_dir();
	gchar* peak_filename = g_build_filename(cache_dir, peak_basename, NULL);
	g_free(cache_dir);
	dbg(1, "peak_filename=%s", peak_filename);
	g_free(peak_basename);

	return peak_filename;
}


static bool
peakfile_is_current(const char* audio_file, const char* peak_file)
{
	/*
	note that this test will fail to detect a modified file, if an older file is now stored at this location.

	The freedesktop thumbnailer spec identifies modifications by comparing with both url and mtime stored in the thumbnail.
	This will mostly work, but strictly speaking still won't identify a changed file in all cases.

	One relatively simple improvement would be to match the sizes.
	*/

	if(!g_file_test(peak_file, G_FILE_TEST_EXISTS)){
		dbg(1, "peakfile does not exist: %s", peak_file);
		return false;
	}

	if(wf_file_is_newer(audio_file, peak_file)){
		dbg(1, "peakfile is too old");
		return false;
	}

	return true;
}


	typedef struct {
		Waveform*          waveform;
		WfPeakfileCallback callback;
		char*              filename;
		gpointer           user_data;
	} C;

	static void waveform_ensure_peakfile_done(Waveform* w, GError* error, gpointer user_data)
	{
		C* c = (C*)user_data;

		if(error && w->priv->peaks){
			w->priv->peaks->error = error;
		}

		if(c->callback) c->callback(c->waveform, c->filename, c->user_data);
		else g_free(c->filename);
		g_object_unref(c->waveform);
		g_free(c);
	}

/*
 *  Asynchronously ensure that a peakfile exists for the given Waveform.
 *  If a callback fn is supplied, the caller must g_free the returned filename.
 */
void
waveform_ensure_peakfile (Waveform* w, WfPeakfileCallback callback, gpointer user_data)
{
	if(!wf_create_cache_dir()) return;

	char* filename = g_path_is_absolute(w->filename) ? g_strdup(w->filename) : g_build_filename(g_get_current_dir(), w->filename, NULL);

	gchar* peak_filename = waveform_get_peak_filename(filename);
	if(!peak_filename){
		callback(w, NULL, user_data);
		goto out;
	}

	if(w->offline || peakfile_is_current(filename, peak_filename)){
		callback(w, peak_filename, user_data);
		goto out;
	}

	waveform_peakgen(w, peak_filename, waveform_ensure_peakfile_done, WF_NEW(C,
		.waveform = g_object_ref(w),
		.callback = callback,
		.filename = peak_filename,
		.user_data = user_data
	));

  out:
	g_free(filename);
}


/*
 *  Caller must g_free the returned filename.
 */
char*
waveform_ensure_peakfile__sync (Waveform* w)
{
	if(!wf_create_cache_dir()) return NULL;

	char* filename = g_path_is_absolute(w->filename) ? g_strdup(w->filename) : g_build_filename(g_get_current_dir(), w->filename, NULL);

	gchar* peak_filename = waveform_get_peak_filename(filename);
	if(!peak_filename) goto out;

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

	if(!wf_peakgen__sync(filename, peak_filename, NULL)){ g_free0(peak_filename); goto out; }

  out:
	g_free(filename);

	return peak_filename;
}


#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavresample/avresample.h"

typedef struct
{
	AVCodecContext* encoder;

	int64_t next_pts; // pts of the next frame that will be generated

	AVFrame* frame;

	struct SwsContext* sws_ctx;
	AVAudioResampleContext* avr;
} OutputStream;


static AVFrame*
alloc_audio_frame (enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
	AVFrame* frame = av_frame_alloc();

	if (!frame) {
		fprintf(stderr, "Error allocating an audio frame\n");
		return NULL;
	}

	frame->format = sample_fmt;
	frame->channel_layout = channel_layout;
	frame->sample_rate = sample_rate;
	frame->nb_samples = nb_samples;

	if (nb_samples) {
		int ret = av_frame_get_buffer(frame, 0);
		if (ret < 0) {
			gwarn("error allocating audio buffer");
			return NULL;
		}
	}

	return frame;
}


static void
open_audio2 (AVCodecContext* c, AVStream* stream, OutputStream *ost)
{
	int nb_samples;

	if (avcodec_open2(c, NULL, NULL) < 0) {
		gwarn("could not open codec");
		return;
	}

	if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
		nb_samples = 10000;
	else
		nb_samples = c->frame_size;

	ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);

	// Copy the stream parameters to the muxer
	int ret = avcodec_parameters_from_context(stream->codecpar, c);
	if (ret < 0) {
		gwarn("Could not copy the stream parameters");
		return;
	}
}


#define FAIL(A, ...) { \
	fprintf(stderr, A, ##__VA_ARGS__); \
	avio_close(format_context->pb); \
	g_free(tmp_path); \
	return false; \
	}


static bool
wf_ff_peakgen (const char* infilename, const char* peak_filename)
{
	WfDecoder f = {{0,}};

	if(!ad_open(&f, infilename)) return false;

	gchar* basename = g_path_get_basename(peak_filename);
	gchar* tmp_path = g_build_filename("/tmp", basename, NULL);
	g_free(basename);

	OutputStream output_stream = {0,};

	AVFormatContext* format_context;
	avformat_alloc_output_context2(&format_context, av_guess_format("wav", NULL, "audio/x-wav"), NULL, NULL);

	avio_open(&format_context->pb, tmp_path, AVIO_FLAG_READ_WRITE);
	if(!format_context->pb) {
		FAIL("could not open for writing");
	}

	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
	if(!codec){
		gwarn("codec not found");
		return false;
	}

	AVStream* stream = avformat_new_stream(format_context, NULL);
	if(!stream){
		gwarn("could not alloc stream")
		return false;
	}

	//av_dump_format(format_context, 0, tmp_file, 1);

	AVCodecContext* c = output_stream.encoder = avcodec_alloc_context3(codec);
	if(!c){
		gwarn("Could not alloc an encoding context");
		return false;
	}

	c->sample_fmt     = codec->sample_fmts ? codec->sample_fmts[0] : AV_SAMPLE_FMT_S16;
	c->sample_rate    = f.info.sample_rate;
	c->channel_layout = codec->channel_layouts ? codec->channel_layouts[0] : AV_CH_LAYOUT_STEREO;
	c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
	c->bit_rate       = f.info.sample_rate * 16 * f.info.channels; // TODO has any effect?

	stream->time_base = (AVRational){ 1, c->sample_rate };

	// some formats want stream headers to be separate
	if (format_context->oformat->flags & AVFMT_GLOBALHEADER)
		c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	open_audio2(c, stream, &output_stream);

	AVDictionary** options = NULL;
	if(avformat_write_header(format_context, options)){
		gwarn("could not write header");
		return false;
	}

	int total_frames_written = 0;
	WfPeakSample total[WF_STEREO] = {0,};

	#define n_blocks 8
	int read_len = WF_PEAK_RATIO * n_blocks;

	int16_t data[f.info.channels][read_len];
	WfBuf16 buf = {
		.buf = {
			data[0], data[1]
		},
		.size = n_blocks * WF_PEAK_RATIO
	};

	int readcount;
	int total_readcount = 0;
	while((readcount = ad_read_short(&f, &buf))){
		total_readcount += readcount;
		int remaining = readcount;

		WfPeakSample peak[f.info.channels];

		int n = MIN(n_blocks, readcount / WF_PEAK_RATIO + (readcount % WF_PEAK_RATIO ? 1 : 0));
		int j = 0; for(;j<n;j++){
			WfPeakSample w[f.info.channels];

			memset(peak, 0, sizeof(WfPeakSample) * f.info.channels);

			int k; for (k = 0; k < MIN(remaining, WF_PEAK_RATIO); k += f.info.channels){
				int c; for(c=0;c<f.info.channels;c++){
					int16_t val = buf.buf[c][WF_PEAK_RATIO * j + k];
					peak[c] = (WfPeakSample){
						MAX(peak[c].positive, val),
						MIN(peak[c].negative, MAX(val, -32767)), // TODO value of SHRT_MAX messes up the rendering - why?
					};
				}
			};
			remaining -= WF_PEAK_RATIO;
			int c; for(c=0;c<f.info.channels;c++){
				w[c] = peak[c];
				total[c] = (WfPeakSample){
					MAX(total[c].positive, w[c].positive),
					MIN(total[c].negative, w[c].negative),
				};
			}
			//-------------
			/*
			if(av_frame_make_writable(frame) < 0){
				fprintf(stderr, "Could not make frame writable\n");
				return false;
			}
			short* data = (short*)frame->data[0];
			for(int f=0; f<frame->nb_samples; f++){
				;
			}
			c = 0;
			data[0] = w[c].positive; //TODO
			if(!wf_ff_encode(cx, frame, pkt, fp))
				break;
			// flush the encoder
			wf_ff_encode(cx, NULL, pkt, fp);
			*/

			avio_write(format_context->pb, (unsigned char*)w, WF_PEAK_VALUES_PER_SAMPLE * f.info.channels * sizeof(short));

			total_frames_written += 2 * f.info.channels; // TODO get real value from writer
		}
	}

#if 0
	if(f.info.channels > 1) dbg(0, "max=%i,%i min=%i,%i", total[0].positive, total[1].positive, total[0].negative, total[1].negative);
	else dbg(0, "max=%i min=%i", total[0].positive, total[0].negative);
#endif

#ifdef DEBUG
	if(g_str_has_suffix(infilename, ".mp3")){
		dbg(1, "mp3");
		f.info.frames = total_readcount; // update the estimate with the real frame count.
	}
#else
	if(total_frames_written / WF_PEAK_VALUES_PER_SAMPLE != f.info.frames / WF_PEAK_RATIO){
		gwarn("unexpected number of frames written: %i != %"PRIu64, total_frames_written / WF_PEAK_VALUES_PER_SAMPLE, f.info.frames / WF_PEAK_RATIO);
	}
#endif

	//avio_flush(format_context->pb); // probably not needed

	av_write_trailer(format_context);

	avcodec_free_context(&output_stream.encoder);
	av_frame_free(&output_stream.frame);
	avio_close(format_context->pb);
	avformat_free_context(format_context);

	int renamed = !rename(tmp_path, peak_filename);
	g_free(tmp_path);
	if(!renamed) return false;

	return true;
}


typedef struct {
	char*         infilename;
	const char*   peak_filename;
	WfCallback3   callback;
	void*         user_data;
} PeakJob;

	static void peakgen_execute_job(Waveform* w, gpointer _job)
	{
		// runs in worker thread

		PeakJob* job = _job;

		GError* error = NULL;
		if(!wf_peakgen__sync(job->infilename, job->peak_filename, &error)){
#ifdef DEBUG
			if(wf_debug) gwarn("peakgen failed");
#endif
			w->priv->peaks->error = error;
		}
	}

	static void peakgen_free(gpointer item)
	{
		PeakJob* job = item;
		g_free0(job->infilename);
		g_free(job);
	}

	static void peakgen_post(Waveform* waveform, GError* error, gpointer item)
	{
		// runs in the main thread
		// will not be called if the waveform is destroyed.
		// there is only a single callback which will be called on success and failure

		PeakJob* job = item;
		// it is possible for the 'peaks' promise to be removed.
		// this indicates that we are no longer interested in this peak data.
		GError* _error = waveform->priv->peaks ? waveform->priv->peaks->error : NULL;
		job->callback(waveform, _error, job->user_data);
	}

void
waveform_peakgen(Waveform* w, const char* peak_filename, WfCallback3 callback, gpointer user_data)
{
	if(!peakgen.msg_queue) wf_worker_init(&peakgen);

	wf_worker_push_job(&peakgen, w, peakgen_execute_job, peakgen_post, peakgen_free,
		WF_NEW(PeakJob,
			.infilename = g_path_is_absolute(w->filename) ? g_strdup(w->filename) : g_build_filename(g_get_current_dir(), w->filename, NULL),
			.peak_filename = peak_filename,
			.callback = callback,
			.user_data = user_data
		)
	);
}


void
waveform_peakgen_cancel(Waveform* w)
{
	wf_worker_cancel_jobs(&peakgen, w);
}


/*
 *  Generate a peak file on disk for the given audio file.
 *  Returns true on success.
 *  If error arg is not NULL, the caller must free any GError created.
 */
bool
wf_peakgen__sync(const char* infilename, const char* peak_filename, GError** error)
{
	g_return_val_if_fail(infilename, false);
	PF;

	if(!wf_ff_peakgen(infilename, peak_filename)){
		if(wf_debug){
#ifdef USE_SNDFILE
			printf("peakgen: not able to open input file %s: %s\n", infilename, sf_strerror(NULL));
#endif
			if(!g_file_test(infilename, G_FILE_TEST_EXISTS)){
				printf("peakgen: no such input file: '%s'\n", infilename);
			}
		}
		if(error){
#ifdef USE_SNDFILE
			char* text = g_strdup_printf("Failed to create peak: not able to open input file: %s: %s", infilename, sf_strerror(NULL));
			*error = g_error_new_literal(g_quark_from_static_string(wf->domain), 1, text);
			g_free(text);
#endif
		}
		return false;
	}

	if(need_file_cache_check){
		maintain_file_cache();
		need_file_cache_check = false;
	}

	return true;
}


static inline void
process_data (short* data, int data_size_frames, int n_channels, short max[], short min[])
{
	//produce peak data from audio data.
	//
	//      data - interleaved audio.
	//      data_size_frames - the number of frames in data to process.
	//      pos  - is the *sample* count. (ie not frame count, and not byte count.)
	//      max  - (output) positive peaks. must be allocated with size n_channels
	//      min  - (output) negative peaks. must be allocated with size n_channels

	memset(max, 0, sizeof(short) * n_channels);
	memset(min, 0, sizeof(short) * n_channels);

	int k; for(k=0;k<data_size_frames;k+=n_channels){
		int c; for(c=0;c<n_channels;c++){
			max[c] = (data[k + c] > max[c]) ? data[k + c] : max[c];
			min[c] = (data[k + c] < min[c]) ? data[k + c] : min[c];
		}
	}
}


/*
 * Returns time in milliseconds, given the sample number.
 */
#ifdef UNUSED
static unsigned long
sample2time(SF_INFO sfinfo, long samplenum)
{
	// long is good up to 4.2GB
	
	return (10 * samplenum) / ((sfinfo.samplerate/100) * sfinfo.channels);
}
#endif


static gboolean
wf_create_cache_dir()
{
	gchar* path = get_cache_dir();
	gboolean ret  = !g_mkdir_with_parents(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP);
	if(!ret) gwarn("cannot access cache dir: %s", path);
	g_free(path);
	return ret;
}


static void*
peakbuf_allocate(Peakbuf* peakbuf, int c)
{
	g_return_val_if_fail(c < WF_STEREO, NULL);
	if(peakbuf->buf[c]){ gwarn("buffer already allocated. c=%i", c); return NULL; }

	peakbuf->buf[c] = g_malloc0(sizeof(short) * peakbuf->size);
	peak_mem_size += (peakbuf->size * sizeof(short));

	dbg(2, "c=%i b=%i: size=%i tot_peak_mem=%ikB", c, peakbuf->block_num, peakbuf->size, peak_mem_size / 1024);
	return peakbuf->buf[c];
}


void
waveform_peakbuf_free(Peakbuf* p)
{
	if(p){
		int c; for(c=0;c<WF_STEREO;c++) if(p->buf[c]) g_free0(p->buf[c]);
		g_free(p);
	}
}


static void
peakbuf_set_n_tiers(Peakbuf* peakbuf, int n_tiers, int resolution)
{
	if(n_tiers < 1 || n_tiers > MAX_TIERS){ gwarn("n_tiers out of range: %i", n_tiers); n_tiers = MAX_TIERS; }
	peakbuf->resolution = resolution;
	dbg(2, "n_tiers=%i", n_tiers);
}


/*
 *  Generate peak data for the given block number.
 *  The audio is supplied in @audiobuf and output in @peakbuf.
 *  @min_tiers specifies the _minimum_ resolution. The peakbuf produced may be of higher resolution than this.
 *  For thread-safety, the Waveform is not modified.
 */
void
waveform_peakbuf_regen(Waveform* waveform, WfBuf16* audiobuf, Peakbuf* peakbuf, int block_num, int min_output_tiers)
{
	// TODO caching: consider saving to disk, and using kernel caching. clear cache on exit.

	dbg(2, "%i", block_num);

	WF* wf = wf_get_instance();
	g_return_if_fail(audiobuf);
	g_return_if_fail(peakbuf);

	int input_resolution = TIERS_TO_RESOLUTION(MAX_TIERS);

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

	short* buf = peakbuf->buf[WF_LEFT];
	dbg(3, "peakbuf=%p buf0=%p", peakbuf, peakbuf->buf);
	if(!buf){
		//peakbuf->size = peakbuf_get_max_size_by_resolution(output_resolution);
		//peakbuf->size = wf_peakbuf_get_max_size(output_tiers);
		peakbuf->size = audiobuf->size * WF_PEAK_VALUES_PER_SAMPLE / io_ratio;
		dbg(2, "buf->size=%i blocksize=%i", peakbuf->size, WF_PEAK_BLOCK_SIZE * WF_PEAK_VALUES_PER_SAMPLE / io_ratio);
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
		WfBuf16* audio_buf = audiobuf;
									g_return_if_fail(peakbuf->size >= WF_PEAK_BLOCK_SIZE * WF_PEAK_VALUES_PER_SAMPLE / io_ratio);
		audio_buf->stamp = ++wf->audio.access_counter;
		int i, p; for(i=0, p=0; p<WF_PEAK_BLOCK_SIZE; i++, p+= io_ratio){

			process_data(&audio_buf->buf[c][p], io_ratio, 1, (short*)&maxplus, (short*)&maxmin);

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
#if 0 // valgrind warning
	dbg(2, "maxlevel=%i,%i (%.3fdB)", totplus, totmin, wf_int2db(MAX(totplus, -totmin)));
#endif

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


#define CACHE_EXPIRY_DAYS 90

static bool _maintain_file_cache()
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

	return G_SOURCE_REMOVE;
}

static void
maintain_file_cache()
{
	// http://people.freedesktop.org/~vuntz/thumbnail-spec-cache/delete.html

	g_idle_add(_maintain_file_cache, NULL);
}


