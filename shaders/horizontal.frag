uniform sampler2D tex2d;
 
const float blurSize = 1.0/256.0; // this size results in every step being one pixel wide if the tex2d texture is of size 256x256
 
void main(void)
{
	vec4 sum = vec4(0.0);

	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x - 4.0*blurSize, gl_TexCoord[0].y)) * 0.05;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x - 3.0*blurSize, gl_TexCoord[0].y)) * 0.09;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x - 2.0*blurSize, gl_TexCoord[0].y)) * 0.12;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x - 1.0*blurSize, gl_TexCoord[0].y)) * 0.15;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x               , gl_TexCoord[0].y)) * 0.16;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + 1.0*blurSize, gl_TexCoord[0].y)) * 0.15;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + 2.0*blurSize, gl_TexCoord[0].y)) * 0.12;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + 3.0*blurSize, gl_TexCoord[0].y)) * 0.09;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x + 4.0*blurSize, gl_TexCoord[0].y)) * 0.05;

	gl_FragColor = sum;
}
