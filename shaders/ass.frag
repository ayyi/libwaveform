uniform sampler2D tex2d;
uniform vec4 fg_colour;
 
void main(void)
{
	float l0 = fg_colour[0] * texture2D(tex2d, gl_TexCoord[0].xy).r;
	gl_FragColor = vec4(l0, l0, l0, fg_colour[3] * texture2D(tex2d, gl_TexCoord[0].xy).a);
}
