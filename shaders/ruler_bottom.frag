/*
  copyright (C) 2012-2025 Tim Orford <tim@orford.org>

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

uniform float samples_per_pixel;
uniform float beats_per_pixel;
uniform float viewport_left;
uniform vec4 fg_colour;
uniform int markers[10];

varying vec2 MCposition;
varying vec2 ecPosition;

/*
 * This ruler version is intended to be positioned at the bottom of the window.
 * Actor height is expected to be 20px.
 * It works with frames, not beats and bars.
 * The uniforms are compatible with the other ruler shaders.
 * For coordinate calculations to work the quad must be positioned using glTranslatef(), with x1=0, y1=0.
 */
void main(void)
{
	vec4 marker_colours[10];
	marker_colours[0] = vec4(0.5, 0.8, 0.5, 1.0);
	marker_colours[1] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[2] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[3] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[4] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[5] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[6] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[7] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[8] = vec4(0.5, 0.6, 0.8, 1.0);
	marker_colours[9] = vec4(0.8, 0.6, 0.5, 1.0);

	vec4 c = vec4(1.0) * fg_colour;
	float alpha = c[3];
	c[3] = 0.0;

	float pixels_per_beat = 1.0 / beats_per_pixel;
	float pixels_per_sec = 2.0 / beats_per_pixel;
	float pixels_per_bar = pixels_per_beat * 4.0;

	if (MCposition.y > 20.0) {
		for (int i=0;i<10;i++) {
			float locator_x = float(markers[i]) / samples_per_pixel - viewport_left;

			if(MCposition.x > locator_x - 0.05 && MCposition.x < locator_x + 8.0){
				gl_FragColor = marker_colours[i];
				return;
			}
		}
	}

	float interval = 0.0;
	if (MCposition.y > 17.0) {
		// smallest
		interval = pixels_per_sec * 9.6 / float(int(1.9 * pixels_per_beat) + 1);
	} else if (MCposition.y > 14.0) {
		interval = pixels_per_sec * 48.0 / float(int(1.9 * pixels_per_beat) + 1);
	} else if (MCposition.y > 10.0) {
		interval = pixels_per_sec * 240.0 / float(int(1.9 * pixels_per_beat) + 1);
	} else {
		// full size lines
		interval = pixels_per_sec * 480.0 / float(int(1.9 * pixels_per_beat) + 1);
	}

	float m = floor(mod((MCposition.x + viewport_left), interval));
	//m is now an integer value between: 0...interval
	//-quantising the value means that the lines are of consistent width and brightness, at the expense of even spacing.
	//float val = smoothstep(0.0, 1.0, m);
	//val = 1.0 - val * smoothstep(interval, interval - 1.0, m);
	float val = m < 1.0 ? 1.0 : 0.0;
	c[3] = alpha * val;

	gl_FragColor = c;
}

