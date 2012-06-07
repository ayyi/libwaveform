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
#include "cpgrs.h"

template <int N> inline float  faustpower(float x)   { return powf(x,N); } 
template <int N> inline double faustpower(double x)  { return pow(x,N); }
template <int N> inline int    faustpower(int x)     { return faustpower<N/2>(x) * faustpower<N-N/2>(x); } 
template <>      inline int    faustpower<0>(int x)  { return 1; }
template <>      inline int    faustpower<1>(int x)  { return x; }


void
CPGRS::init()
{
	t = 0;

	gain    = 8.0;
	sustain = 0.5f;
	release = 0.2f;
	decay   = 0.3f;
	attack  = 0.01f;

	fSamplingFreq = 44100;
	filter_bandwidth = 1e+02f;
	fConst0 = (3.141592653589793f / fSamplingFreq);
	freq = 4.4e+02f;
	fConst1 = (6.283185307179586f / fSamplingFreq);
	on_off_button = 0.0;
	for (int i=0; i<2; i++) iRec1[i] = 0;
	for (int i=0; i<2; i++) fRec2[i] = 0;
	for (int i=0; i<2; i++) iRec3[i] = 0;
	for (int i=0; i<3; i++) fVec0[i] = 0;
	for (int i=0; i<3; i++) fRec0[i] = 0;
}

void
CPGRS::compute (int count, float** input, float** output)
{
	if(!(t % 4096)) on_off_button = !on_off_button;

	float   fSlow0 = expf((0 - (fConst0 * filter_bandwidth)));
	float   fSlow1 = (2 * cosf((fConst1 * freq)));
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


