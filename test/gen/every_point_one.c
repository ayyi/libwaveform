/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2012-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |                                                                      |
 | Generates a wav with short markers every 0.1 s                       |
 |                                                                      |
 +----------------------------------------------------------------------+
 |
 */

#define __wf_private__
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <getopt.h>
#include <sndfile.h>

#define MONO 1

double* new_buffer (long size);

int
print_help ()
{
	printf("Usage: " PACKAGE_STRING " [OPTIONS] <output_filename>\n"
	     "  -c, --channels                   number of audio channels. 1 or 2 expected. default 1\n"
	     "  -n, --numblocks                  number of blocks >= 1. default 1\n"
	     "  -v, --version                    Show version information\n"
	     "  -h, --help                       Print this message\n"
	     "  -d, --debug     level            Output debug info to stdout\n"
		);
	return EXIT_FAILURE;
}


int main (int argc, char* argv[])
{
	int n_channels = MONO;
	int n_blocks = 1;

	const char* optstring = "c:n:hvd:";

	const struct option longopts[] = {
		{ "channels", 1, 0, 'c' },
		{ "numblocks", 1, 0, 'n' },
		{ "version", 0, 0, 'v' },
		{ "help", 0, 0, 'h' },
		{ "debug", 1, 0, 'd' },
		{ 0, }
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
			if(n_blocks < 1 || n_blocks > 64){ printf("n_blocks out of range: %i\n", n_blocks); return EXIT_FAILURE; }
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
	} else {
		fprintf(stderr, "No output filename specified\n");
		return print_help();
	}
	printf("Generating wav ...\n");

	#define BURST 20
	int empty_nframes = 441 - BURST;
	int empty_size = empty_nframes * n_channels;
	double* empty_buffer = new_buffer (empty_size);

	long n_frames1 = BURST;
	int size1 = n_frames1 * n_channels;
	double* buffer1 = new_buffer (empty_size);
	for (int i=0;i<size1;i++) {
		buffer1[i] = sin(((double)i) / M_PI);
	}

	long n_frames = BURST;
	int size = n_frames * n_channels;
	double* buffer2 = new_buffer (size);
	for (int i=0;i<size;i++) {
		buffer2[i] = sin(((double)i) / M_PI) / 3.;
	}

	double* buffer3 = new_buffer (size);
	for (int i=0;i<size;i++) {
		buffer3[i] = sin(((double)i) / M_PI) / 6.;
	}

	SF_INFO info = {
		0,
		44100,
		n_channels,
		SF_FORMAT_WAV | SF_FORMAT_PCM_16
	};

	printf("filename=%s\n", output_filename);
	SNDFILE* sndfile = sf_open(output_filename, SFM_WRITE, &info);
	if (!sndfile) {
		fprintf(stderr, "Sndfile open failed '%s': %s\n", argv[1], sf_strerror(sndfile));
		goto fail;
	}

	for (int b = 0; b < 200; b++) {
		if (!(b % 10)) {
			if (sf_writef_double(sndfile, buffer1, n_frames1) != n_frames1) {
				fprintf(stderr, "Write failed\n");
				sf_close(sndfile);
				goto fail;
			}
		} else if (!(b % 5)) {
			if (sf_writef_double(sndfile, buffer2, n_frames) != n_frames) {
				fprintf(stderr, "Write failed\n");
				sf_close(sndfile);
				goto fail;
			}
		} else {
			if (sf_writef_double(sndfile, buffer3, n_frames) != n_frames) {
				fprintf(stderr, "Write failed\n");
				sf_close(sndfile);
				goto fail;
			}
		}

		if (sf_writef_double(sndfile, empty_buffer, empty_nframes) != empty_nframes) {
			fprintf(stderr, "Write failed\n");
			sf_close(sndfile);
			goto fail;
		}
	}

	sf_write_sync(sndfile);
	sf_close(sndfile);
	free(buffer1);
	free(buffer2);
	free(buffer3);

	return EXIT_SUCCESS;
fail:
	free(buffer1);
	return -1;
}

double*
new_buffer (long size)
{
	double* buffer = (double*)malloc(size * sizeof(double));
	if (!buffer) {
		fprintf(stderr, "Could not allocate wav buffer\n");
		exit(1);
	}
	memset(buffer, 0, size * sizeof(double));

	return buffer;
}
