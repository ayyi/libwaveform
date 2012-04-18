uniform sampler2D tex2d;
uniform vec4 fg_colour;
 
void main(void)
{
	gl_FragColor = vec4(fg_colour[0], fg_colour[1], fg_colour[2], fg_colour[3] * texture2D(tex2d, gl_TexCoord[0].xy).a);
}
