/*
  Generates test wav

  contains code generated with Faust (http://faust.grame.fr)

  "Constant-Peak-Gain Resonator Synth" by Julius Smith

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

void compute_sinewave (double* buf, int n_frames);
void compute_noise    (int count, double** input, double** output);
void compute_kps      (int count, double** input, double** output);


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
	double fslider0; // resonator attenuation
	double fslider1; // excitation (samples) ?
	double fslider3; // duration (samples) ?
	double gain;     // output level

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
		fslider0 = 0.1;   // resonator attenuation
		fslider1 = 128.0; // excitation (samples) ?
		fslider3 = 128.0; // duration (samples) ?
		gain     = 1.0;   // output level

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
		float fSlow0 = (0.5f * (1.0f - fslider0));
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


template <int N> inline float  faustpower(float x)   { return powf(x,N); } 
template <int N> inline double faustpower(double x)  { return pow(x,N); }
template <int N> inline int    faustpower(int x)     { return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <>      inline int    faustpower<0>(int x)  { return 1; }
template <>      inline int    faustpower<1>(int x)  { return x; }

class CPGRS
{
  public:
	float     gain;          // gain
	float     attack;        // interface->addHorizontalSlider("attack",  &fslider4, 0.01f, 0.0f, 1.0f, 0.001f);
	float     sustain;       // interface->addHorizontalSlider("sustain", &fslider1, 0.5f, 0.0f, 1.0f, 0.01f);
	float     release;       // interface->addHorizontalSlider("release", &fslider2, 0.2f, 0.0f, 1.0f, 0.001f);
	float     decay;         // interface->addHorizontalSlider("decay",   &fslider3, 0.3f, 0.0f, 1.0f, 0.001f);
	float     fentry0;       // "freq", 4.4e+02f, 2e+01f, 2e+04f, 1.0f

  private:
	float     fSamplingFreq;
	float     fslider0;      // 2-filter: addHorizontalSlider("bandwidth (Hz)", &fslider0, 1e+02f, 2e+01f, 2e+04f, 1e+01f);
	float     fConst0;
	float     fConst1;
	float     on_off_button; // gate (on/off)
	int       iRec1[2];
	float     fRec2[2];
	int       iRec3[2];
	float     fVec0[3];
	float     fRec0[3];

	int       t;

  public:

	void init()
	{
		t = 0;

		gain = 8.0;
		sustain = 0.5f;
		release = 0.2f;
		decay = 0.3f;
		attack = 0.01f;

		fSamplingFreq = 44100;
		fslider0 = 1e+02f;
		fConst0 = (3.141592653589793f / fSamplingFreq);
		fentry0 = 4.4e+02f;
		fConst1 = (6.283185307179586f / fSamplingFreq);
		on_off_button = 0.0;
		for (int i=0; i<2; i++) iRec1[i] = 0;
		for (int i=0; i<2; i++) fRec2[i] = 0;
		for (int i=0; i<2; i++) iRec3[i] = 0;
		for (int i=0; i<3; i++) fVec0[i] = 0;
		for (int i=0; i<3; i++) fRec0[i] = 0;
	}
#if 0
	virtual int getNumInputs()  { return 0; }
	virtual int getNumOutputs() { return 1; }
#endif
	void compute (int count, float** input, float** output)
	{
		if(!(t % 4096)) on_off_button = !on_off_button;

		float   fSlow0 = expf((0 - (fConst0 * fslider0)));
		float   fSlow1 = (2 * cosf((fConst1 * fentry0)));
		float   fSlow2 = on_off_button;
		int     iSlow3 = (fSlow2 > 0);
		int     iSlow4 = (fSlow2 <= 0);
		float   fSlow5 = sustain;
		float   fSlow6 = (sustain + (0.001f * (fSlow5 == 0.0f)));
		float   fSlow7 = release;
		float   fSlow8 = (1 - (1.0f / powf((1e+03f * fSlow6),(1.0f / ((fSlow7 == 0.0f) + (fSamplingFreq * fSlow7))))));
		float   fSlow9 = decay;
		float   fSlow10 = (1 - powf(fSlow6,(1.0f / ((fSlow9 == 0.0f) + (fSamplingFreq * fSlow9)))));
		float   fSlow11 = attack;
		float   fSlow12 = (1.0f / ((fSlow11 == 0.0f) + (fSamplingFreq * fSlow11)));
		float   fSlow13 = (4.656612875245797e-10f * gain);
		float   fSlow14 = (0.5f * (1 - faustpower<2>(fSlow0)));
		float*  output0 = output[0];
		for (int i=0; i<count; i++) {
			iRec1[0] = (iSlow3 & (iRec1[1] | (fRec2[1] >= 1)));
			int iTemp0 = (iSlow4 & (fRec2[1] > 0));
			fRec2[0] = (((iTemp0 == 0) | (fRec2[1] >= 1e-06f)) * ((fSlow12 * (((iRec1[1] == 0) & iSlow3) & (fRec2[1] < 1))) + (fRec2[1] * ((1 - (fSlow10 * (iRec1[1] & (fRec2[1] > fSlow5)))) - (fSlow8 * iTemp0)))));
			iRec3[0] = (12345 + (1103515245 * iRec3[1]));
			float fTemp1 = (fSlow13 * (iRec3[0] * fRec2[0]));
			fVec0[0] = fTemp1;
			fRec0[0] = ((fSlow14 * (fVec0[0] - fVec0[2])) + (fSlow0 * ((fSlow1 * fRec0[1]) - (fSlow0 * fRec0[2]))));
			output0[i] = (float)fRec0[0];

			// post processing
			fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
			fVec0[2] = fVec0[1]; fVec0[1] = fVec0[0];
			iRec3[1] = iRec3[0];
			fRec2[1] = fRec2[0];
			iRec1[1] = iRec1[0];
		}

		t++;
	}
};
CPGRS cpgrs;


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
			output[0][c] = !(a % 10);
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
			//printf("n_channels=%i\n", n_channels);
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
	printf("Generating 10s test tone\n");

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

		int fade_in = 16384;
		if(f<fade_in) val *= ((float)f)/fade_in;

		int fade_out = 16384;
		if(n_frames - f < fade_out) val *= ((float)n_frames -f)/fade_out;

		for(c=0;c<n_channels;c++){
			buffer[f * n_channels + c] = val;
		}
	}
	compute_sinewave(buffer, 16384);
	ff += 16384;

	double* input[2] = {NULL, NULL};
	double* output[n_channels];// = {buffer + 16384 * n_channels, buffer + 16384 * n_channels};

	//---

	float* output_f[n_channels];
	cpgrs.init();
	for(int a=0;a<16384;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += 16384;
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
	ff += 16384;
	cpgrs.release = 0.01; // <----
	cpgrs.fentry0 = 2e+01f; // low freq
	for(int a=0;a<16384;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		cpgrs.compute(n_channels, (float**)input, (float**)output_f);
		for(c=0;c<n_channels;c++){
			output[0][c] = output_f[0][c];
		}
	}
	ff += 16384;

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

	//---

	ff += 8192;
	for(a=0;a<16384;a++){
		int c; for(c=0;c<n_channels;c++){
			output[c] = buffer + ff * n_channels + a * n_channels + c;
		}
		pink.compute(n_channels, (double**)input, (double**)output);
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


