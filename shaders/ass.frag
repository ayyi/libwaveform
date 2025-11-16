uniform sampler2D tex2d;
uniform vec4 colour1;
uniform vec4 colour2;

in vec2 tex_coords;
out vec4 colour;
 
void main ()
{
	float rr = texture2D(tex2d, tex_coords.xy).r;

	float l0 = colour1[0] * rr + colour2[0] * (1.0 - rr);
	float l1 = colour1[1] * rr + colour2[1] * (1.0 - rr);
	float l2 = colour1[2] * rr + colour2[2] * (1.0 - rr);

	colour = vec4(l0, l1, l2, colour1[3] * texture2D(tex2d, tex_coords).a);
}
