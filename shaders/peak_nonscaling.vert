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

//varying vec2 MCposition;

void main() 
{
   gl_TexCoord[0] = gl_MultiTexCoord1;
   gl_TexCoord[1] = gl_MultiTexCoord2;
   gl_TexCoord[2] = gl_MultiTexCoord3;
   gl_TexCoord[3] = gl_MultiTexCoord4;
   //MCposition = gl_Vertex.xy;
   //vec3 ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex); //TODO
   gl_Position = ftransform();
}
