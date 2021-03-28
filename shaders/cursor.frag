/**
* +----------------------------------------------------------------------+
* | This file is part of the Ayyi project. https://www.ayyi.org          |
* | copyright (C) 2012-2021 Tim Orford <tim@orford.org>                  |
* +----------------------------------------------------------------------+
* | This program is free software; you can redistribute it and/or modify |
* | it under the terms of the GNU General Public License version 3       |
* | as published by the Free Software Foundation.                        |
* +----------------------------------------------------------------------+
*
*/

uniform vec4 colour;
uniform float width;

varying vec2 position;
 
void main ()
{
	float a1 = 0.5 + 0.5 * max(position.x + 0.5 - (width - 1.0), 0.0);

	gl_FragColor = vec4(
		vec3(colour),
		colour.a * a1 * position.x / width
	);
}
