uniform sampler2D tex2d;
uniform vec4 fg_colour;
 
const float blurSize = 1.0/256.0; // this size results in every step being one pixel wide if the tex2d texture is of size 256x256
 
void main(void)
{
	vec4 sum = vec4(0.0);

	//float alpha = texture2D(tex2d, gl_TexCoord[0].xy).a;
	//if(alpha > 0.5){
	//	gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
	//}else{

	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 4.0*blurSize)) * 0.05;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 3.0*blurSize)) * 0.09;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 2.0*blurSize)) * 0.12;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - 1.0*blurSize)) * 0.15;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y               )) * 0.16;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 1.0*blurSize)) * 0.15;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 2.0*blurSize)) * 0.12;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 3.0*blurSize)) * 0.09;
	sum += texture2D(tex2d, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + 4.0*blurSize)) * 0.05;

	//sum.g = 0.0;
	gl_FragColor = sum * fg_colour;

	//vec3 t1 = texture2D(tex2d, gl_TexCoord[0].xy);
	//gl_FragColor = vec4(t1, 1.0);
	//gl_FragColor = vec4(1.0, 0.5, 0.25, 1.0);
	//}
}
