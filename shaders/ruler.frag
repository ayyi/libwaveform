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

uniform float beats_per_pixel;
uniform float viewport_left;
//uniform float top;
uniform vec4 fg_colour;

varying vec2 MCposition;
varying vec2 ecPosition;

void main(void)
{
	//for coordinate calculations to work the quad must be positioned using glTranslatef(), with x1=0, y1=0.

	vec4 c = vec4(1.0) * fg_colour;
	float alpha = c[3];
	c[3] = 0.0;

	float pixels_per_beat = 1.0 / beats_per_pixel;
	float pixels_per_bar = pixels_per_beat * 4.0;

	//note: frag coords are offset 0.5 pixels from the centre of the pixel.
	//      gl_FragCoord has lower-left origin.
	//          layout(origin_upper_left) in vec4 gl_FragCoord;
	//          layout(pixel_center_integer) in vec4 gl_FragCoord;
//	if(fract((ecPosition.x -0.5) / pixels_per_beat) > 0.05) c[3] = 0.0;
	//gl_FragCoord works better when model projection is not the same the view.
	//-if the projections are the same, gl_FragCoord will give same results as ecPosition ?
	//if(fract((gl_FragCoord.x -0.5 ) / pixels_per_beat) > 0.05) c[3] = 0.0;


	float interval = 0.0;
	if(MCposition.y < 3.0){
		interval = (pixels_per_beat < 1.5)
			? pixels_per_beat * 4.0
			: (pixels_per_beat < 6.0)
				? pixels_per_beat
				: (pixels_per_beat < 24.0)
					? pixels_per_beat / 4.0   //semiquaver
					: pixels_per_beat / 16.0;

	}else if(MCposition.y < 10.0){
		interval = (pixels_per_beat < 1.5)
			? pixels_per_beat * 16.0
			: (pixels_per_beat < 6.0)
				? pixels_per_beat * 4.0 //every bar
				: pixels_per_beat;

	}else if(MCposition.y < 50.0){
		//full size lines are every bar, unless zoomed out then only show every 4 bars.
		interval  = (pixels_per_beat < 1.5)
			? pixels_per_bar * 16.0
			: (pixels_per_beat < 6.0)
				? pixels_per_bar * 4.0 //only show every 4 bars
				: pixels_per_bar;
	}

	float m = floor(mod((MCposition.x - 0.5 + viewport_left), interval));
	float val = smoothstep(0.0, 0.5, m);
	val = 1.0 - val * smoothstep(interval, interval - 0.5, m);
	c[3] = alpha * val;

	gl_FragColor = c;
}

