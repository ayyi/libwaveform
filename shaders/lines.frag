/*
  copyright (C) 2013 Tim Orford <tim@orford.org>

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

uniform vec4 colour;
uniform int n_channels;
uniform float texture_width;
uniform sampler2D tex;
varying vec2 MCposition;


float dist_to_line(vec2 pt1, vec2 pt2, vec2 testPt)
{
	// http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html

	// this seems to assume the line is infinitely long - not what we want.
				if(testPt.y > max(pt1.y, pt2.y)) return 4.0 * abs(testPt.y - max(pt1.y, pt2.y)); // approximation
				if(testPt.y < min(pt1.y, pt2.y)) return 4.0 * abs(testPt.y - min(pt1.y, pt2.y)); // approximation

	vec2 lineDir = pt2 - pt1;
	vec2 perpDir = vec2(lineDir.y, -lineDir.x);
	vec2 dirToPt1 = pt1 - testPt;
	return abs(dot(normalize(perpDir), dirToPt1));
}

float dist_to_line1(vec2 pt1, vec2 pt2, vec2 testPt)
{
	// an approximation based on the assumption that the line is solely in the previous column
	// Returned value is either between 0 and 1, or is very large
	// (so should be used with a falloff that goes to zero at distance=1.0)
	// All pts are aligned to pixel corners.

	if(testPt.y > max(pt1.y, pt2.y) + 0.5) return 1000.0; //  TODO this means horizontal lines are ignored.
	if(testPt.y < min(pt1.y, pt2.y) - 0.5) return 1000.0;

	if(pt2.y > pt1.y){
		// ascending - at bottom distance is 1, at top distance is 0

		/*
		if(pt2.y - pt1.y < 1.0){ // FIXME special case for horizontal lines
			return 0.0;
		}
		*/

		return (pt2.y - testPt.y) / (pt2.y - pt1.y);
	}else{
		// descending (at bottom distance is 0, at top distance is 1)    pt1.y > testPt.y > pt2.y

		/*
		if(pt1.y - pt2.y < 1.0){ // FIXME special case for horizontal lines
			return pt1.y - pt2.y;
		}
		*/

		return (testPt.y - pt2.y) / (pt1.y - pt2.y);
	}

	return 1000.0;
}

float dist_to_line2(vec2 pt1, vec2 pt2, vec2 testPt)
{
	// an approximation based on the assumption that the line is solely in the **following** column
	// Returned value is either between 0 and 1, or is very large
	// (so should be used with a falloff that goes to zero at distance=1.0)
	// All pts are aligned to pixel corners.

	if(testPt.y > max(pt1.y, pt2.y) + 0.5) return 1000.0;
	if(testPt.y < min(pt1.y, pt2.y) - 0.5) return 1000.0;

	if(abs(pt2.y - pt1.y) < 1.0){
		//return abs(pt2.y - pt1.y);
	}

	if(pt2.y > pt1.y){
		// ascending - at bottom distance is 0, at top distance is 1
		return (testPt.y - pt1.y) / (pt2.y - pt1.y);
	}else{
		// descending (at bottom distance is 1, at top distance is 0)
		return (pt1.y - testPt.y) / (pt1.y - pt2.y);
	}

	return 1000.0;
}

void main(void)
{
	float y = 256.0 - MCposition.y; // invert

	// for stereo, lhs covers range y=256-->128, rhs is y=128-->0
	int c = (n_channels > 1 && y > 128.0) ? 1 : 0;
	float ct = float(c) / float(n_channels);

	//if(MCposition.y < 128.0) discard;

	// left hand side border
	if(MCposition.x < 2.0){
		gl_FragColor = vec4(1.0, 0.2, 0.1, 0.5);
		return;
	}
	// bottom border
	if(y < 4.0){
		gl_FragColor = vec4(1.0, 0.8, 0.1, 0.5);
		return;
	}

	if(MCposition.x > 0.1){ // TODO first pixel
		float y0 = texture2D(tex, vec2(gl_TexCoord[0].x - 2.0/texture_width, ct)).a * 256.0; // TODO too far left
		float y1 = texture2D(tex, vec2(gl_TexCoord[0].x - 1.0/texture_width, ct)).a * 256.0;
		float y2 = texture2D(tex, vec2(gl_TexCoord[0].x,                     ct)).a * 256.0;
		//float y3 = texture2D(tex, vec2(gl_TexCoord[0].x + 1.0/texture_width, 0.0)).a * 256.0;
		//float y4 = texture2D(tex, vec2(gl_TexCoord[0].x + 2.0/texture_width, 0.0)).a * 256.0;

		if(true){
			vec2 pt0 = vec2(MCposition.x - 2.0, y0);                  //  ---- is this too far left?
			vec2 pt1 = vec2(MCposition.x - 1.0, y1);
			vec2 pt2 = vec2(MCposition.x,       y2);
			//vec2 pt3 = vec2(MCposition.x + 1.0, y3);
			//vec2 line3 = vec2(MCposition.x + 2.0, y4);

			//float x_offset = 0.5;
			float x_offset = 0.0;
			float dist0 =   1.0 * dist_to_line1(pt0, pt1, vec2(MCposition.x - x_offset, y));
			float dist1 =   1.0 * dist_to_line2(pt1, pt2, vec2(MCposition.x - x_offset, y));
			//float dist2 = 100.0 * dist_to_line(pt2, pt3, vec2(MCposition.x - x_offset, MCposition.y));
			//float dist3 = 100.0; //max(1.0, dist_to_line(pt3, line3, MCposition.xy));
			float dist = min(dist0, dist1);
			//dist = min(dist, dist3);
			//dist = min(dist, dist0);

			if(dist < 1.0){
				gl_FragColor = vec4(colour.r, colour.g, colour.b, 1.0 - dist);

				//primitive gamma curve - makes lines bolder - nicer in some ways, but needs to be improved. does not eliminate ropiness.
				/*
				if(dist < 0.5){
					gl_FragColor = vec4(colour.r, colour.g, colour.b, 1.0);
				}else if(dist < 1.0){
					gl_FragColor = vec4(colour.r, colour.g, colour.b, 1.0 - 2.0 * (dist - 0.5));
				*/

			}else if(gl_TexCoord[0].x > 0.495 && gl_TexCoord[0].x < 0.505){
				gl_FragColor = vec4(1.0, 1.0, 0.1, 0.5);
			}else{
				discard;
			}
		}
	}else{
		discard;
	}
}

