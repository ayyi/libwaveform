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
uniform sampler1D tex1d_0neg;
uniform sampler1D tex1d_3;
uniform sampler1D tex1d_4;
uniform int n_channels;

void main(void)
{
	float top = 0.0;
	float bottom = 256.0;
	float peaks_per_pixel = 1.0;
	float object_height = (bottom - top) / 256.0; //transform to texture coords

	float d = 1.0 / 256.0;

	float y = gl_TexCoord[0].t;
	float mid = 0.5; //if using texture coords, mid is always 0.5
	float mid_rhs = 0.75;
	float h = 1.0;
	if(n_channels == 2){
		mid = 0.25;
		h = 0.5;
	}
	float alpha_range = 0.5; //not needed - remove
	float val = 0.0;

	float peak_pstv;
	float peak_negv;
	int n = 1;//int(max(1, over_sample)) + 2;

	int c_max = int(min(n_channels, 2));
	int c;for(c=0;c<c_max;c++){
		float mid_ = (c == 0) ? mid : mid_rhs;
		if((c == 0 && y < h) || (c == 1 && y > h)){
			//float tx = min(0.9999, gl_TexCoord[0].x + max(0.0, float(-n/2)) * d);
			float tx = gl_TexCoord[0].x + max(0.0, float(-n/2)) * d;
			float bb = alpha_range;
			if (y > mid_) {
				peak_pstv = (c == 0) ? texture1D(tex1d, tx).a : texture1D(tex1d_3, tx).a;
				if(y - mid_ < peak_pstv * h) val += bb;
			} else {
				peak_negv = (c == 0) ? texture1D(tex1d_0neg, tx).a : texture1D(tex1d_4, tx).a;
				if(-y + mid_ < peak_negv * h) val += bb;
			}
		}
	}

	if(val > 0.0) val += (1.0 - alpha_range); //min value

	gl_FragColor = vec4(min(1.0, val));
}

