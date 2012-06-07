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
//uniform float glheight;         //these are gl coordinates, not screen pixels.
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;

varying vec2 MCposition;        //TODO get coords for current object, not window
varying vec2 ecPosition;

void main(void)
{
	//float object_height = (bottom - top) / 256.0; //transform to texture coords
	//float height = 1.0;
	//float mid = object_height / 2.0;
	/*
	vec2 peak = peaks[int(MCposition.x)];
	float colour = 0.0;
	gl_FragColor = vec4(colour);
	*/

	//float y = MCposition.y / 256.0;
	//float y = (MCposition.y - top) / (bottom - top);

	vec4 c = vec4(1.0) * fg_colour;

	float pixels_per_beat = 1.0 / beats_per_pixel;
	/*
	   x= 0 0.0 0.0
	   x= 1 0.1 0.1
	   x= 2 0.2 0.2
	   x=11 1.1 0.1
	*/
	//note: frag coords are offset 0.5 pixels from the centre of the pixel.
	//      gl_FragCoord has lower-left origin.
	//          layout(origin_upper_left) in vec4 gl_FragCoord;
	//          layout(pixel_center_integer) in vec4 gl_FragCoord;
	if(fract((ecPosition.x -0.5) / pixels_per_beat) > 0.05) c[3] = 0.0;
	//gl_FragCoord works better when model projection is not the same the view.
	//-if they the projectsions are the same, gl_FragCoord will give same results as ecPosition ?
	//if(fract((gl_FragCoord.x -0.5 ) / pixels_per_beat) > 0.05) c[3] = 0.0;

	gl_FragColor = c;

	//if(gl_TexCoord[0].x == 0.0) gl_FragColor = vec4(1.0, val, 0.0, 1.0);
}

