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

  ------------------------------------------------------------

  One half of a convolution blur
 
*/

uniform sampler2D tex2d;
uniform vec4 fg_colour;
uniform float peaks_per_pixel; //goes down as we zoom in

const float blurSize = 1.0/256.0; // this size results in every step being one pixel wide if the tex2d texture is of width 256
 
void main(void)
{
	float w1 = 0.75; // main level
	float w2 = 0.25; // bloom level
	float l[11];
	for(int i=0;i<11;i++) l[i] = 0.0;

	float blur_size = blurSize * peaks_per_pixel; // gets smaller as we zoom in
	vec4 sum = vec4(0.0);
	vec4 bloom = vec4(0.0);

	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)) * w1;

	float m[4];
	m[0] = 1.00;
	m[1] = 2.00;
	m[2] = 4.00;
	m[3] = 8.00;
	float mm = 1.0;
	if(true || sum.r < 1.0 || (sum.a < 1.0)){ // TODO why enabling this test gives wrong results?
	//if(false){
		//oversampling with 1 pixel linear window function
		if(peaks_per_pixel > m[0]){
			mm = 2.0;
			if(peaks_per_pixel > m[1]){
				mm = 3.0;
				if(peaks_per_pixel > m[2]){
					mm = 4.0;
					if(peaks_per_pixel > m[3]){
						l[-4 + 5] += (peaks_per_pixel - m[3]) * w1;
						l[ 4 + 5] += (peaks_per_pixel - m[3]) * w1;
					}
					l[-3 + 5] += (peaks_per_pixel - m[2]) * w1;
					l[ 3 + 5] += (peaks_per_pixel - m[2]) * w1;
				}
				l[-2 + 5] += (peaks_per_pixel - m[1]) * w1;
				l[ 2 + 5] += (peaks_per_pixel - m[1]) * w1;
			}
			l[-1 + 5] += (peaks_per_pixel - m[0]) * w1;
			l[ 1 + 5] += (peaks_per_pixel - m[0]) * w1;
		}
		float minv = 0.3;
		for(int i=0;i<11;i++) if(l[i] > 0.0) l[i] = min(minv + l[i] * (1.0 - minv), 1.0) / mm;

		// this is unacceptably slow
		/*
		// if ppp is 4, then we will only be sampling every fourth texture value here.
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x - 4.0*blur_size, gl_TexCoord[0].y)) * 0.05;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x - 3.0*blur_size, gl_TexCoord[0].y)) * 0.09;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x - 2.0*blur_size, gl_TexCoord[0].y)) * 0.12;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x - 1.0*blur_size, gl_TexCoord[0].y)) * 0.15;
		//sum += texture2D(tex2d, vec2(gl_TexCoord[0].x                , gl_TexCoord[0].y)) * 0.16;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x + 1.0*blur_size, gl_TexCoord[0].y)) * 0.15;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x + 2.0*blur_size, gl_TexCoord[0].y)) * 0.12;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x + 3.0*blur_size, gl_TexCoord[0].y)) * 0.09;
		bloom += texture2D(tex2d, vec2(gl_TexCoord[0].x + 4.0*blur_size, gl_TexCoord[0].y)) * 0.05;
		*/

		//need to use blur_size here so that bloom scales with zoom
		//to do this properly we need mipmapping
		if(peaks_per_pixel < 2.0){
			l[1] += 0.05 * w2;
			l[2] += 0.09 * w2;
			l[3] += 0.12 * w2;
			l[4] += 0.15 * w2;
			l[6] += 0.15 * w2;
			l[7] += 0.12 * w2;
			l[8] += 0.09 * w2;
			l[9] += 0.05 * w2;
		}else{
			//when zoomed out, we need to sample wider
			l[1] += 0.12 * w2;
			l[3] += 0.15 * w2;

			l[7] += 0.15 * w2;
			l[9] += 0.12 * w2;
		}

		for(int i=0;i<11;i++) if(l[i] > 0.0) sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + float(i - 5) * blurSize, gl_TexCoord[0].y)) * l[i];
	}
	sum = min(sum, 1.0);

	gl_FragColor = vec4(sum.r, sum.r, sum.r, sum.a) * fg_colour;
	/*
	gl_FragColor = texture2D(tex2d, gl_TexCoord[0].xy);
	*/
}
