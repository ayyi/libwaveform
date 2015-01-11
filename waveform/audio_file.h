/*
  copyright (C) 2012-2015 Tim Orford <tim@orford.org>
  copyright (C) 2011 Robin Gareus <robin@gareus.org>

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
#ifndef __waveform_audiofile_h__
#define __waveform_audiofile_h__

#ifdef USE_FFMPEG
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

typedef struct
{
   unsigned int sample_rate;
   unsigned int channels;
   int64_t      length;       // milliseconds
   int64_t      frames;       // total number of frames (eg a frame for 16bit stereo is 4 bytes).
   int          bit_rate;
   int          bit_depth;
} AudioInfo;


typedef struct
{
   AVFormatContext* format_context;
   AVCodecContext*  codec_context;
   AVCodec*         codec;
   int              audio_stream;
   AVPacket         packet;
   int              pkt_len;
   uint8_t*         pkt_ptr;

   AudioInfo        info;

   int16_t          m_tmpBuffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
   int16_t*         m_tmpBufferStart;
   unsigned long    m_tmpBufferLen;

   int64_t          decoder_clock;
   int64_t          output_clock;
   int64_t          seek_frame;
} FF;


bool    wf_ff_open     (FF*, const char* filename);
void    wf_ff_close    (FF*);
ssize_t wf_ff_read     (FF*, float* d, size_t len);

#endif
#endif
