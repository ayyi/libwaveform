uniform sampler2D tex2d;
uniform vec4 fg_colour;
 
void main(void)
{
   gl_FragColor = fg_colour * texture2D(tex2d, gl_TexCoord[0].xy);
}
