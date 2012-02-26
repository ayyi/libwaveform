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
uniform vec2 peaks[128];
uniform float peaks_per_pixel;
//uniform float glheight;         //these are gl coordinates, not screen pixels.
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;
uniform int n_channels;

varying vec2 MCposition;        //TODO get coords for current object, not window

void main(void)
{
	float object_height = (bottom - top) / 256.0; //transform to texture coords
	//float height = 1.0;
	float mid = object_height / 2.0;
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

	float d = 1.0 / 256.0;

	float y = gl_TexCoord[0].t;
	mid = 0.5; //if using texture coords, mid is always 0.5
	float mid_rhs = 0.75;
	float h = 1.0;
	if(n_channels == 2){
		mid = 0.25;
		h = 0.5;
	}
	//float y = MCposition.y / 256.0;
	//float y = (MCposition.y - top) / (bottom - top);
	float val = 0.0;

	float peak_pstv;
	float peak_negv;
	int i;
	for(i=0;i<int(max(1, peaks_per_pixel));i++){
		if (y > mid) {
			peak_pstv = texture1D(tex1d,     gl_TexCoord[0].x + float(i) * d).a;
			if(y - mid < peak_pstv * h) val += 0.5 / peaks_per_pixel;
		} else {
			peak_negv = texture1D(tex1d_neg, gl_TexCoord[1].x + float(i) * d).a;
			if(-y + mid < peak_negv * h) val += 0.5 / peaks_per_pixel;
			//val += peak_negv / (peaks_per_pixel);
		}

		if(n_channels == 2){
			peak_pstv = texture1D(tex1d_3, gl_TexCoord[2].x + 1.0 * float(i) * d).a;
			peak_negv = texture1D(tex1d_4, gl_TexCoord[3].x + 1.0 * float(i) * d).a;
			if (y > mid_rhs) {
				if(y - mid_rhs < peak_pstv * h) val += 0.5 / peaks_per_pixel;
			} else if (y > 0.5) {
				if(-y + mid_rhs < peak_negv * h) val += 0.5 / peaks_per_pixel;
				//val += 0.25 * peak_negv / (peaks_per_pixel);
			}
		}
	}
	if(val > 0.0) val += 0.5; //min value

	//antialiasing:
	if(peaks_per_pixel < 1.1){
		if (y > mid) {
			float y_ = y - mid;   //range: 0.0 - 0.5
			float y__ = 2.0 * y_; //range: 0.0 - 1.0
			float tex_offset = 0.5; //critical value! need better control. should depend on zoom
			peak_pstv = texture1D(tex1d, gl_TexCoord[0].x - tex_offset * d).a;
			if(y_ < peak_pstv * h){
				float peak_ = 2.0 * peak_pstv; // normalised to 0.0 - 1.0
				float di = 0.5 * (peak_ - y__) + 0.5; //TODO the level should depend on how many x pixels we are from start of texture value
				val += clamp(di, 0.0, 1.0);
			}
		} else {
			float y_ = -y + mid;
			float y__ = 2.0 * (-y + mid); // normalised 0.0 - 1.0
			peak_negv = texture1D(tex1d_neg, gl_TexCoord[1].x - 0.5 * d).a;
			if(y_ < peak_negv * h){
				float peak_ = 2.0 * peak_negv; // normalised to 0.0 - 1.0
				float di = 0.5 * (peak_ - y__) + 0.5;
				val += clamp(di, 0.0, 1.0);
			}
		}
	}

	gl_FragColor = vec4(min(1.0, val)) * fg_colour;
	//gl_FragColor = vec4(val, val, val, 0.5);// * fg_colour;
		//+ vec4(0.5, 0.25, 0.0, 1.0);

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

