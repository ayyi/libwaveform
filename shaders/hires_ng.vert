/*
  copyright (C) 2021 Tim Orford <tim@orford.org>

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

attribute vec4 vertex;

uniform vec2 modelview;
uniform vec2 translate;

varying vec2 tex_coords;
varying vec2 position;

void main ()
{
	tex_coords = vertex.zw;
	position = vertex.xy;
	gl_Position = vec4(vec2(1., -1.) * (vertex.xy + translate) / modelview - vec2(1.0, -1.0), 1.0, 1.0);
}
