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

uniform sampler1D tex1d;
uniform sampler1D tex1d_neg;
uniform sampler1D tex1d_3;
uniform sampler1D tex1d_4;
uniform float peaks_per_pixel;
//uniform float glheight;         //these are gl coordinates, not screen pixels.
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;
uniform int n_channels;

//varying vec2 MCposition;        //TODO get coords for current object, not window

void main(void)
{
	float object_height = (bottom - top) / 256.0; //transform to texture coords

	//float val = texture1D(tex1d, gl_TexCoord[0].s).a * MCposition.y / gl_height;

	float d = 1.0 / 256.0;

	float y = gl_TexCoord[0].t;
	float mid = 0.5; //if using texture coords, mid is always 0.5
	float mid_rhs = 0.75;
	float h = 1.0;
	if(n_channels == 2){
		mid = 0.25;
		h = 0.5;
	}
	//if we are not doing any scaling, oversampling is unnecessary? (as this is very common, perhaps should have a separate simplified shader.)
	float over_sample = (peaks_per_pixel == 1.0) ? 1.0 : 2.0;
	d = d / over_sample;
	float alpha_range = 0.5;
	//float y = MCposition.y / 256.0;
	//float y = (MCposition.y - top) / (bottom - top);
	float val = 0.0;

	float peak_pstv;
	float peak_negv;
	int i;
	float aa = alpha_range / (peaks_per_pixel * over_sample);
	int n = int(max(1, peaks_per_pixel * over_sample)) + 2;

	// warning
	// this value must be updated if 'over_sample' or max value of 'peaks_per_pixel' change.
	// value of 16 is based on oversample=2 and max peaks_per_pixel of 8
	const int N_WIN = 16;
	n = int(min(n, N_WIN));
	float window[N_WIN];
		// r300 driver cannot initialise an array in a loop!
		window[ 2] = 1.0;
		window[ 3] = 1.0;
		window[ 4] = 1.0;
		window[ 5] = 1.0;
		window[ 6] = 1.0;
		window[ 7] = 1.0;
		window[ 8] = 1.0;
		window[ 9] = 1.0;
		window[10] = 1.0;
		window[11] = 1.0;
		window[12] = 1.0;
		window[13] = 1.0;
	if(n > 3){
	window[  0] = 0.25;
	window[  1] = 0.75;
	window[n-2] = 0.75;
	window[n-1] = 0.25;
	}else{
		window[  0] = 1.00;
		window[  1] = 1.00;
		window[  2] = 0.00;
	}

	int c_max = int(min(n_channels, 2));
	int c;for(c=0;c<c_max;c++){
		float mid_ = (c == 0) ? mid : mid_rhs;
		if((c == 0 && y < h) || (c == 1 && y > h)){
			for(i=0;i<n;i++){
				float tx = min(0.9999, gl_TexCoord[0].x + max(0.0, float(i - n/2)) * d);
				float bb = aa * window[i];
				if (y > mid_) {
					peak_pstv = (c == 0) ? texture1D(tex1d, tx).a : texture1D(tex1d_3, tx).a;
					if(y - mid_ < peak_pstv * h) val += bb;
				} else {
					peak_negv = (c == 0) ? texture1D(tex1d_neg, tx).a : texture1D(tex1d_4, tx).a;
					if(-y + mid_ < peak_negv * h) val += bb;
				}
			}
		}
	}

	if(val > 0.0) val += (1.0 - alpha_range); //min value

	gl_FragColor = vec4(min(1.0, val)) * fg_colour;
	//gl_FragColor = vec4(val, val, val, 0.5)// * fg_colour;
	//	+ vec4(0.5, 0.25, 0.0, 1.0);

	//centre line:
	//if(gl_TexCoord[0].y > 0.495 && gl_TexCoord[0].y < 0.505) gl_FragColor = vec4(1.0, 0.0, 0.0, 0.75);

	//if(gl_TexCoord[0].x == 0.0) gl_FragColor = vec4(1.0, val, 0.0, 1.0);
}

