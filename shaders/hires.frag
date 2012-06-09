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
//uniform vec2 peaks[128];
uniform float peaks_per_pixel;
//uniform float glheight;         //these are gl coordinates, not screen pixels.
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;
uniform int n_channels;

void main(void)
{
	float object_height = (bottom - top) / 256.0; //transform to texture coords
	//float height = 1.0;
	//float mid = object_height / 2.0;
	/*
	vec2 peak = peaks[int(MCposition.x)];
	float colour = 0.0;
	if (MCposition.y > mid) {
		if (MCposition.y - mid < peak.x) colour = 1.0;
	} else {
		if (-MCposition.y + mid < peak.y) colour = 1.0;
	}
	gl_FragColor = vec4(colour);
	*/

	//float val = texture1D(tex1d, gl_TexCoord[0].s).a * MCposition.y / gl_height;

	float d = 1.0 / 4096.0; //TODO pass texture_size as uniform

	float y = gl_TexCoord[0].t;
	float mid = 0.5; //if using texture coords, mid is always 0.5
	float mid_rhs = 0.75;
	float h = 1.0;
	if(n_channels == 2){
		mid = 0.25;
		h = 0.5;
	}
	float over_sample = 2.0;
	d = d / over_sample;
	float alpha_range = 0.05; // only reduce the alpha range when ppp is high (for transitions to lower res)
	if(peaks_per_pixel < 6.0) alpha_range = 0.75;
	if(peaks_per_pixel < 3.0) alpha_range = 0.5;
	//float y = MCposition.y / 256.0;
	//float y = (MCposition.y - top) / (bottom - top);
	float val = 0.0;

	float peak_pstv;
	float peak_negv;
	int i;
	float aa = alpha_range / (peaks_per_pixel * over_sample);
	//TODO use window fn from peak.frag
	int n = int(max(1, peaks_per_pixel * over_sample)) + 2;
	for(i=-n/2;i<n/2;i++){
		float tx = min(0.99999, gl_TexCoord[0].x + float(i) * d);
		//sampler1D sss[2];
		if (y > mid) {
			peak_pstv = texture1D(tex1d, tx).a;
			if(y - mid < peak_pstv * h) val += aa;
		} else {
			peak_negv = texture1D(tex1d_neg, tx).a;
			if(-y + mid < peak_negv * h) val += aa;
		}

		if(n_channels == 2){
			if (y > mid_rhs) {
				peak_pstv = texture1D(tex1d_3, tx).a;
				if(y - mid_rhs < peak_pstv * h) val += aa;
			} else if (y > 0.5) {
				peak_negv = texture1D(tex1d_4, tx).a;
				if(-y + mid_rhs < peak_negv * h) val += aa;
			}
		}
	}

	if(val > 0.0) val += (1.0 - alpha_range); //min value

	gl_FragColor = vec4(min(1.0, val)) * fg_colour;

	//yellow edge:
	/*
	vec4 fc = vec4(min(1.0, val)) * fg_colour;
	if(gl_TexCoord[0].x > 0.995) fc[2] = 0.0;
	gl_FragColor = fc;
	*/

	//gl_FragColor = vec4(val, val, val, 0.5)// * fg_colour;
	//	+ vec4(0.5, 0.25, 0.0, 1.0);

	//centre line:
	//if(gl_TexCoord[0].y > 0.495 && gl_TexCoord[0].y < 0.505) gl_FragColor = vec4(1.0, 0.0, 0.0, 0.75);

	//if(gl_TexCoord[0].x == 0.0) gl_FragColor = vec4(1.0, val, 0.0, 1.0);

	/*
	if (y > mid) {
		float y_ = y - mid;   //range: 0.0 - 0.5
		float y__ = 2.0 * y_; //range: 0.0 - 1.0
		peak_pstv = texture1D(tex1d, gl_TexCoord[0].x -1.0 * d).a;
		if(y_ < peak_pstv * h){
			float peak_ = 2.0 * peak_pstv; // normalised to 0.0 - 1.0
			float di = 4.0 * (peak_ - y__) + 0.25;
			gl_FragColor = vec4(val) * fg_colour + vec4(clamp(di, 0.0, 1.0), 0.0, 0.0, 1.0);
		}
	} else {
		//gl_FragColor = vec4(val) * fg_colour;
	}
	*/
}

