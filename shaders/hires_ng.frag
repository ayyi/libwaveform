/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2014-2025 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

uniform sampler2D tex2d;
uniform float top;
uniform float bottom;
uniform vec4 fg_colour;
uniform int n_channels;
uniform float v_gain;
uniform float tex_height;
uniform float tex_width;
uniform int mm_level;

varying vec2 position;
varying vec2 tex_coords;

const vec4 x_gain = vec4(1.0, 2.0, 4.0, 8.0);
const vec4 mm2tx = vec4(0.00, 0.00, 0.50, 0.75);
const vec4 mm2ty = vec4(0.0, 1.0, 1.0, 1.0);
const vec4 tx_min = vec4(0.0, 0.0, 0.5, 0.75); // TODO add border between LOD sections to avoid overlapping


void main (void)
{
	float dx = 1.0 / tex_width;
	float y = bottom - position.y; // invert y

	float mid = (bottom - top) / 2.0;
	float mid3 = mid;
	vec2 t = vec2(mm2tx[mm_level], mm2ty[mm_level]);

	float yc = y;
	if(n_channels < 2){
		yc = (y - mid) / v_gain + mid;
	}else{
		if(y < mid - 1.0){
			// LHS
			mid3 = mid / 2.0;
			mid -= mid / 2.0;
			yc = (y - mid3) / v_gain + mid3;
		}else if(y > mid + 1.0){
			// RHS
			mid3 = mid / 2.0;
			yc = (y - mid - mid3) / v_gain + mid3;
			mid += mid / 2.0;
			t.y += 4.0;
		}else{
			// leave 2 pixel gap between channels       -- TODO there is now a texture lookup error of up to 1 pixel causing cropping
			discard;
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

		t.y = t.y / tex_height + tex_coords.y;

		y1 = (mid3 - (yc + smooth)) / mid3;
		y2 = (mid3 - (yc - smooth)) / mid3;

	}else{
		// min

		t.y = (t.y + 2.0) / tex_height + tex_coords.y;

		y1 = ((yc - smooth) - mid3) / mid3;
		y2 = ((yc + smooth) - mid3) / mid3;
	}

	//(texture2D(tex2d, vec2(tx, t.y)).a > y1) ? 1.0 : 0.0;

	t.x += tex_coords.x / x_gain[mm_level];

	gl_FragColor = vec4(
		fg_colour.rgb,
		fg_colour.a * min(1.0,
			smoothstep(y1, y2, texture2D(tex2d, vec2(t.x,                            t.y)).a) * 0.70 +
			smoothstep(y1, y2, texture2D(tex2d, vec2(max(mm2tx[mm_level], t.x - dx), t.y)).a) * 0.40 +
			smoothstep(y1, y2, texture2D(tex2d, vec2(t.x + dx,                       t.y)).a) * 0.40
		)
	);

	// alternative sampling that only uses 2 texture values
	// -it has some artefacts and also does not look very nice
	// -it has an inherent problem that it varies between sharp and blurry depending on the weight
	/*
	vec2 vTexelSize = 1.0 / vec2(tex_width);

	float color1 = smoothstep(y1, y2, texture2D(tex2d, t                ).a);
	//float color2 = smoothstep(y1, y2, texture2D(tex2d, t + vec2(dx, 0.0)).a);
	float color2 = smoothstep(y1, y2, texture2D(tex2d, vec2(min(0.9999, t.x + dx), t.y)).a);

	float texelCenter = floor(t.x / dx) * dx + dx * 0.5;
	float distanceToCenter = t.x - texelCenter;
	float weight = smoothstep(0.0, 0.5 * dx, distanceToCenter);
	weight = clamp(weight, 0.0, 1.0);

	gl_FragColor = vec4(fg_colour.rgb, fg_colour.a * mix(color1, color2, weight));
	*/
}

