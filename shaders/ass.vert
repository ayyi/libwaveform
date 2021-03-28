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

attribute vec4 vertex;

uniform vec2 modelview;
uniform vec2 translate;

varying vec2 tex_coords;

void main () 
{
	tex_coords = vertex.zw;
	gl_Position = vec4(vec2(1., -1.) * (vertex.xy + translate) / modelview - vec2(1.0, -1.0), 1.0, 1.0);
}
