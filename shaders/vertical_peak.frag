uniform sampler2D tex2d;
uniform vec4 fg_colour;
uniform float peaks_per_pixel;

const float h_over_sample = 1.0;
const float v_over_sample = 2.0;                       // oversampling only needed when rendering tall images. value should adjust to quad size.
const float blur_size = 1.0 / (256.0 * v_over_sample); // this size results in every step being one pixel wide if the tex2d texture is of size 256x256
 
void main(void)
{
	float d = 1.0 / 256.0;
	vec4 sum = vec4(0.0);

	int i;
	for(i=0;i<int(max(1, peaks_per_pixel * h_over_sample));i++){
		float dx = float(i) * d;

		// 9 point
		/*
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 4.0 * blur_size)) * 0.05;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 3.0 * blur_size)) * 0.09;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 2.0 * blur_size)) * 0.12;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 1.0 * blur_size)) * 0.15;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y                  )) * 0.16;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 1.0 * blur_size)) * 0.15;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 2.0 * blur_size)) * 0.12;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 3.0 * blur_size)) * 0.09;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 4.0 * blur_size)) * 0.05;
		*/

		// 5 point
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 2.0 * blur_size)) * 0.052;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y - 1.0 * blur_size)) * 0.244;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y                  )) * 0.407;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 1.0 * blur_size)) * 0.244;
		sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + dx, gl_TexCoord[0].y + 2.0 * blur_size)) * 0.052;
	}

	gl_FragColor = sum * fg_colour / ceil(peaks_per_pixel);
}
