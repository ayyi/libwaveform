/*
  Generates a test wav of length exactly equal to 1 WaveformActor display block.
  The wav is empty except for the first and last sample.

  ---------------------------------------------------------------

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
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <getopt.h>
#include <sndfile.h>

#define MONO 1

int
print_help ()
{
	printf("Usage: " PACKAGE_STRING " [OPTIONS] <output_filename>\n"
	     "  -c, --channels                   number of audio channels. 1 or 2 expected. default 1\n"
	     "  -n, --numblocks                  number of blocks. >= 1 expected. default 1\n"
	     "  -v, --version                    Show version information\n"
	     "  -h, --help                       Print this message\n"
	     "  -d, --debug     level            Output debug info to stdout\n"
		);
	return EXIT_FAILURE;
}


int main(int argc, char* argv[])
{
	int n_channels = MONO;
	int n_blocks = 1;

	const char* optstring = "c:hvd:";

	const struct option longopts[] = {
		{ "channels", 1, 0, 'c' },
		{ "numblocks", 1, 0, 'n' },
		{ "version", 0, 0, 'v' },
		{ "help", 0, 0, 'h' },
		{ "debug", 1, 0, 'd' },
		{ 0, 0, 0, 0 }
	};

	int option_index = 0;
	int c = 0;

	while (1) {
		c = getopt_long (argc, argv, optstring, longopts, &option_index);

		if (c == -1) {
			break;
		}

		switch (c) {
		case 0:
			break;

		case 'c':
			n_channels = atoi(optarg);
			break;

		case 'n':
			n_blocks = atoi(optarg);
			if(n_blocks < 1 || n_blocks > 10){ printf("n_blocks out of range: %i\n", n_blocks); return EXIT_FAILURE; }
			break;

		case 'v':
			printf("version " PACKAGE_VERSION "\n");
			exit (0);
			break;

		case 'h':
			print_help ();
			exit (0);
			break;

		default:
			return print_help();
		}
	}

	char* output_filename = NULL;
	if (optind < argc) {
		output_filename = argv[optind++];
	}else{
		fprintf(stderr, "No output filename specified\n");
		return print_help();
	}
	printf("Generating 1 block test wav\n");

	#define TEX_BORDER 4                               // ------------- duplicate of private libwaveform setting
	long n_frames_per_block = 256 * (256 - 2 * TEX_BORDER);
	long n_frames = n_frames_per_block * n_blocks;

	int size = n_frames * n_channels;
	double* buffer = (double*)malloc(size * sizeof(double));
	if (!buffer) {
		fprintf(stderr, "Could not allocate wav buffer\n");
		return 1;
	}
	memset(buffer, size * sizeof(double), 0);

	//----------------- generate ------------------

	// wav is empty except for first and last sample

	for(int i=0;i<n_blocks;i++){
		buffer[(i    ) * n_frames_per_block * n_channels    ] =  1.0; // start of block
		buffer[(i + 1) * n_frames_per_block * n_channels - 1] = -1.0; // end of block
	}

	//------------------- save --------------------

	SF_INFO info = {
		0,
		44100,
		n_channels,
		SF_FORMAT_WAV | SF_FORMAT_PCM_16
	};

	printf("filename=%s\n", output_filename);
	SNDFILE* sndfile = sf_open(output_filename, SFM_WRITE, &info);
	if(!sndfile){
		fprintf(stderr, "Sndfile open failed '%s': %s\n", argv[1], sf_strerror(sndfile));
		goto fail;
	}

	if(sf_writef_double(sndfile, buffer, n_frames) != n_frames){
		fprintf(stderr, "Write failed\n");
		sf_close(sndfile);
		goto fail;
	}

	sf_write_sync(sndfile);
	sf_close(sndfile);
	free(buffer);

	return EXIT_SUCCESS;
fail:
	free(buffer);
	return -1;
}

