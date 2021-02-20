uniform vec4 colour;
uniform float width;

varying vec2 position;
 
void main(void)
{
	float a1 = 0.5 + 0.5 * max(position.x + 0.5 - (width - 1.0), 0.0);
	float a = colour[3] * a1 * position.x / width;
	gl_FragColor = vec4(vec3(colour), a);
}
