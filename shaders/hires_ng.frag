/*
  copyright (C) 2014 Tim Orford <tim@orford.org>

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

uniform sampler2D tex2d;
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;
uniform int n_channels;
uniform float tex_height;
uniform float tex_width;
uniform int mm_level;

varying vec2 MCposition;

const vec4 x_gain = vec4(1.0, 2.0, 4.0, 8.0);
const vec4 mm2tx = vec4(0.00, 0.00, 0.50, 0.75);
const vec4 mm2ty = vec4(0.0, 1.0, 1.0, 1.0);
const vec4 tx_min = vec4(0.0, 0.0, 0.5, 0.75); // TODO add border between LOD sections to avoid overlapping


void main(void)
{
	float dx = 1.0 / tex_width;
	float y = bottom - MCposition.y; // invert y

	float mid = (bottom -top) / 2.0;
	float mid3 = mid;
	vec2 t = vec2(mm2tx[mm_level], mm2ty[mm_level]);

	int c = 0;
	float yc = y;
	if(n_channels > 1){
		if(y < mid){
			// LHS
			mid3 = mid / 2.0;
			mid = mid - mid / 2.0;
			yc = y;
		}else{
			// RHS
			c = 1;
			mid3 = mid / 2.0;
			yc = y - mid;
			mid += mid / 2.0;
			t.y += 4.0;
		}
	}

	float smooth = (abs(y - mid) > 2.0) ? 2.0 : 1.0;

	/*
	 *   y1 and y2 are used for comparison with the texture value.
	 *
	 *   if y1,y2 are 1.0, output is always black
	 *   if y1,y2 are 0.0, output is always white
	 */
	float y1, y2;

	if(y < mid){
		// max

		t.y = t.y / tex_height + gl_TexCoord[0].y;

		y1 = (mid3 - (yc + smooth)) / mid3;
		y2 = (mid3 - (yc - smooth)) / mid3;

	}else{
		// min

		t.y = (t.y + 2.0) / tex_height + gl_TexCoord[0].y;

		y1 = ((yc - smooth) - mid3) / mid3;
		y2 = ((yc + smooth) - mid3) / mid3;
	}

	//(texture2D(tex2d, vec2(tx, t.y)).a > y1) ? 1.0 : 0.0;

	t.x += gl_TexCoord[0].x / x_gain[mm_level];

	// TODO try and use just 2 samples but calculate the correct ratio depending on how close.
	vec4 colour = fg_colour;
	colour.a *= min(1.0,
			smoothstep(y1, y2, texture2D(tex2d, vec2(t.x,                            t.y)).a) * 0.70 +
			smoothstep(y1, y2, texture2D(tex2d, vec2(max(mm2tx[mm_level], t.x - dx), t.y)).a) * 0.40 +
			smoothstep(y1, y2, texture2D(tex2d, vec2(t.x + dx,                       t.y)).a) * 0.40
	);
	gl_FragColor = colour;
}

