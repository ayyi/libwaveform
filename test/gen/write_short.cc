/*
  Generates a short test wav to trigger drawing in hi-res mode.

  contains code generated with Faust (http://faust.grame.fr)

  ---------------------------------------------------------------

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

*/
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <getopt.h>
#include <sndfile.h>
#include "generator.h"
#include "cpgrs.h"

#define MONO 1

int
print_help ()
{
	printf("Usage: " PACKAGE_STRING " [OPTIONS] <output_filename>\n"
	     "  -c, --channels                   number of audio channels. 1 or 2 expected. default 1\n"
	     "  -l, --length                     length in milliseconds. default 150\n"
	     "  -v, --version                    Show version information\n"
	     "  -h, --help                       Print this message\n"
	     "  -d, --debug     level            Output debug info to stdout\n"
		);
	return EXIT_FAILURE;
}


int main(int argc, char* argv[])
{
	int n_channels = MONO;
	double duration = 150; // milliseconds

	const char* optstring = "c:l:hvd:";

	const struct option longopts[] = {
		{ "channels", 1, 0, 'c' },
		{ "length", 1, 0, 'l' },
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

		case 'l':
			duration = atof(optarg);
			break;

		case 'v':
			printf("version " PACKAGE_VERSION "\n");
			exit (0);
			break;

		case 'h':
			print_help ();
			exit (0);
			break;
		/*
		case 'd':
			debug = atoi(optarg);
			break;
		*/

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
	printf("Generating %.0fms test wav\n", duration);

	int    sample_rate = 44100; // frames / second

	long n_frames = (duration * sample_rate) / 1000;

	double* buffer = (double*)malloc(n_frames * sizeof(double) * n_channels);
	if (!buffer) {
		fprintf(stderr, "Could not allocate wav buffer\n");
		return 1;
	}

	//----------------- generate ------------------

	double* input[2] = {NULL, NULL};
	double* output[n_channels];// = {buffer + 16384 * n_channels, buffer + 16384 * n_channels};
	int ff = 0;

	CPGRS cpgrs;
	Note note(&cpgrs, 4096);
	cpgrs.release = 0.1f;
	for(int a=0;a<n_frames;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (double**)input, (double**)output);
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
	if(!sndfile) {
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

