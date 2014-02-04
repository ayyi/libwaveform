/*
  Generates test wav

  contains code generated with Faust (http://faust.grame.fr)

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
#include "cpgrs.h"

#define MONO 1

#define TEX_BORDER 0
#define WF_SAMPLES_PER_TEXTURE (256 * 256 - TEX_BORDER)   //dupe of private def in actor.c
#define LONG_SECTION  (WF_SAMPLES_PER_TEXTURE / 4)
#define SHORT_SECTION (WF_SAMPLES_PER_TEXTURE / 4)


void compute_sinewave (double* buf, int n_frames);
void compute_noise    (int count, double** input, double** output);
void compute_kps      (int count, double** input, double** output);

CPGRS cpgrs;


int
print_help ()
{
	printf("Usage: " PACKAGE_STRING " [OPTIONS] <output_filename>\n"
	     "  -c, --channels                   number of audio channels. 1 or 2 expected. default 1\n"
	     "  -v, --version                    Show version information\n"
	     "  -h, --help                       Print this message\n"
	     "  -d, --debug     level            Output debug info to stdout\n"
		);
	return EXIT_FAILURE;
}


class Pink
{
  private:
	int   iRec1[3];
	float fRec0[3];

  public:
	Pink()
	{
		for (int i=0; i<3; i++) iRec1[i] = 0;
		for (int i=0; i<3; i++) fRec0[i] = 0;
	}

	void compute (int count, double** input, double** output)
	{
		double* output0 = output[0];
		for (int i=0; i<count; i++) {
			iRec1[0] = (12345 + (1103515245 * iRec1[1]));
			fRec0[0] = ((((6.9067828423840845e-12f * iRec1[2]) + (2.308528039463576e-11f * iRec1[0])) + (1.80116083982126f * fRec0[1])) - ((2.9362651228132963e-11f * iRec1[1]) + (0.80257737639225f * fRec0[2])));
			output0[i] = (double)(1e+01f * fRec0[0]);
			// post processing
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			iRec1[2] = iRec1[1]; iRec1[1] = iRec1[0];
		}
	}
};
Pink pink;


class Noise
{
	int iRec0[2];

  public:
	double gain;

	void init()
	{
		gain = 1.0;

		iRec0[0] = 0;
		iRec0[1] = 0;
	}

	void compute (int count, double** input, double** output)
	{
		double* output0 = output[0];
		int i; for (i=0; i<count; i++) {
			iRec0[0] = (12345 + (1103515245 * iRec0[1]));
			output0[i] = (double)(4.656612875245797e-10f * iRec0[0]) * gain;
			// post processing
			iRec0[1] = iRec0[0];
		}
	}
};
Noise noise;


class KPS
{
  public:
	double resonator_attenuation; // resonator attenuation
	double fslider1;              // excitation (samples) ?
	double fslider3;              // duration (samples) ?
	double gain;                  // output level

  private:
	double fRec0[3];
	double fRec1[2];
	int    iRec2[2];
	float  fVec0[2];
	float  fVec1[512];
	int    IOTA;

  public:
	void init()
	{
		resonator_attenuation = 0.1; // resonator attenuation
		fslider1 = 128.0;            // excitation (samples) ?
		fslider3 = 128.0;            // duration (samples) ?
		gain     = 1.0;              // output level

		IOTA = 0;
		fRec0[1] = 0;
		fRec0[2] = 0;
		fRec0[3] = 0;
		fRec1[0] = 0;
		fRec1[1] = 0;
		for (int i=0; i<  2; i++) iRec2[i] = 0;
		for (int i=0; i<  2; i++) fVec0[i] = 0;
		for (int i=0; i<512; i++) fVec1[i] = 0;
	}

	void compute (int count, double** input, double** output)
	{
		float fSlow0 = (0.5f * (1.0f - resonator_attenuation));
		float fSlow1 = (1.0f / fslider1);
		float fSlow2 = 1.0;// on/off
		float fSlow3 = (4.656612875245797e-10f * gain);
		int   iSlow4 = (int)((int)(fslider3 - 1.5) & 4095);
		double* output0 = output[0];
		for (int i=0; i<count; i++) {
			fVec0[0] = fSlow2;
			fRec1[0] = ((((fSlow2 - fVec0[1]) > 0.0f) + fRec1[1]) - (fSlow1 * (fRec1[1] > 0.0f)));
			iRec2[0] = (12345 + (1103515245 * iRec2[1]));
			fVec1[IOTA&511] = ((fSlow3 * (iRec2[0] * (fRec1[0] > 0.0f))) + (fSlow0 * (fRec0[1] + fRec0[2])));
			fRec0[0] = fVec1[(IOTA-iSlow4)&511];
			output0[i] = fRec0[0];
			// post processing
			fRec0[2] = fRec0[1];
			fRec0[1] = fRec0[0];
			IOTA = IOTA+1;
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			fVec0[1] = fVec0[0];
		}
	}
};
KPS kps;


class Impulse
{
	int a;

  public:
	Impulse()
	{
	}

	void init()
	{
		a = 0;
	}

	void compute (int n_chans, double** input, double** output)
	{
		for (int c=0; c<n_chans; c++) {
			output[0][c] = !(a % 2048);
		}
		a++;
	}
};
Impulse impulse;


int main(int argc, char* argv[])
{
	int n_channels = MONO;

	const char* optstring = "c:hvd:";

	const struct option longopts[] = {
		{ "channels", 1, 0, 'c' },
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
		fprintf(stderr, "Output filename required\n");
		return print_help();
	}
	printf("Generating 10s test wav\n");

	double freq        = 440;   // hz
	double duration    = 10;    // seconds
	int    sample_rate = 44100; // frames / second

	long n_frames = duration * sample_rate;

	double* buffer = (double*) malloc(n_frames * sizeof(double) * n_channels);
	if (!buffer) {
		fprintf(stderr, "Could not allocate buffer for output\n");
		return 1;
	}

	int ff = 0;

	// create a single tone
	long f;
	for (f=0;f<n_frames;f++) {
		double time = f * duration / n_frames;
		double val = sin(2.0 * M_PI * time * freq);

		int fade_in = LONG_SECTION;
		if(f<fade_in) val *= ((float)f)/fade_in;

		int fade_out = LONG_SECTION;
		if(n_frames - f < fade_out) val *= ((float)n_frames -f)/fade_out;

		for(c=0;c<n_channels;c++){
			buffer[f * n_channels + c] = val;
		}
	}
	compute_sinewave(buffer, LONG_SECTION);
	ff += LONG_SECTION;

	double* input[2] = {NULL, NULL};
	double* output[n_channels];// = {buffer + 16384 * n_channels, buffer + 16384 * n_channels};

	//---

	float aa[2];
	float* output_f[] = {&aa[0], &aa[1]};

	cpgrs.init();
	for(int a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		float* output_f[] = {(float*)&output[0][0], (float*)&output[0][1]};
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
	}
	ff += LONG_SECTION;
	cpgrs.release = 0.01; // <----
	for(int a=0;a<16384;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += LONG_SECTION;
	cpgrs.release = 0.01;  // <----
	cpgrs.freq = 100.0;    // lower freq
	for(int a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += LONG_SECTION;

	impulse.init();
	for(int a=0;a<SHORT_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		impulse.compute(n_channels, (double**)input, (double**)output);
	}
	ff += SHORT_SECTION;

	cpgrs.release = 0.01; // <----
	cpgrs.freq = 2.0;     // low freq
	for(int a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += LONG_SECTION;
	cpgrs.freq = 440.0;
	cpgrs.attack = 1.0f; // <----
	cpgrs.release = 1.0f; // <----
	cpgrs.gain = 32.0;
	for(int a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += LONG_SECTION;

	cpgrs.freq = 2000.0;
	cpgrs.attack = 1.0f; // <----
	cpgrs.release = 1.0f; // <----
	cpgrs.gain = 16.0;
	for(int a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += LONG_SECTION;

	//---

	if(n_channels > 1){
		//mute left then right to check correct orientation
		for(f=0;f<8192;f++){
			buffer[(f + ff) * n_channels + 0] = 0.0;
		}
		ff += 8192;
		for(f=0;f<8192;f++){
			buffer[(f + ff) * n_channels + 1] = 0.0;
		}
		ff += 8192;
	}

	//---

	noise.init();
	noise.gain = 0.25;
	for(int a=0;a<4096;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		noise.compute(2, (double**)input, (double**)output);
	}
	ff += 4096;
	noise.init();
	noise.gain = 0.5;
	for(int a=0;a<4096;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		noise.compute(2, (double**)input, (double**)output);
	}
	ff += 4096;
	noise.init();
	noise.gain = 0.75;
	for(int a=0;a<4096;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		noise.compute(2, (double**)input, (double**)output);
	}
	ff += 4096;
	noise.init();
	noise.gain = 1.0;
	for(int a=0;a<4096;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		noise.compute(2, (double**)input, (double**)output);
	}
	ff += 8192;

	//---

	kps.init();
	int a; for(a=0;a<8192;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		kps.compute(n_channels, (double**)input, (double**)output);
	}
	ff += 8192;

	kps.init();
	kps.gain = 0.5;
	for(a=0;a<8192;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		kps.compute(n_channels, (double**)input, (double**)output);
	}
	ff += 8192;

	kps.init();
	for(a=0;a<8192;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		kps.compute(n_channels, (double**)input, (double**)output);
	}
	ff += 8192;

	kps.init();
	kps.gain = 0.5;
	for(a=0;a<8192;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		kps.compute(n_channels, (double**)input, (double**)output);
	}
	ff += 8192;

	//---

	for(a=0;a<LONG_SECTION;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		pink.compute(n_channels, (double**)input, (double**)output);
	}

	impulse.init();
	for(a=0;a<8192;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + (ff + a) * n_channels + c;
		}
		impulse.compute(n_channels, (double**)input, (double**)output);
	}
	ff += 8192;

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


void
compute_sinewave(double* buf, int n_frames)
{
}


#if 0
void
Xcompute_noise(int count, double** input, double** output)
{
	static float fcheckbox0 = 0.0;

	double fSlow0 = (4.656612875245797e-10f * fcheckbox0);
	double* input0 = input[0];
	double* input1 = input[1];
	//float* output0 = output[0];
	double* output0 = output;
	int i; for (i=0; i<count; i++) {
		//output0[i] = (float)(fSlow0 * ((12345 + (float)input0[i]) - (1103515245 * (float)input1[i])));
		output0[i] = (float)(fSlow0 * ((12345 + (float)1.0) - (1103515245 * (float)1.0)));
	}
}
#endif


