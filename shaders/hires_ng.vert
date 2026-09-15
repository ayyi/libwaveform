/*
 +----------------------------------------------------------------------+
 | This file is part of the Ayyi project. https://www.ayyi.org          |
 | copyright (C) 2021-2026 Tim Orford <tim@orford.org>                  |
 +----------------------------------------------------------------------+
 | This program is free software; you can redistribute it and/or modify |
 | it under the terms of the GNU General Public License version 3       |
 | as published by the Free Software Foundation.                        |
 +----------------------------------------------------------------------+
 |
 */

#version 330 core
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 texcoord;

uniform vec2 modelview;
uniform vec2 translate;

out vec2 tex_coords;
out vec2 position;

void main ()
{
	tex_coords = texcoord;
	position = pos;
	gl_Position = vec4(vec2(1., -1.) * (pos.xy + translate) / modelview - vec2(1.0, -1.0), 1.0, 1.0);
}
