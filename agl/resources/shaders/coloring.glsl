// VERTEX_SHADER:
uniform vec4 u_color;

varying vec4 final_color;

void main() {
	//gl_Position = u_projection * u_modelview * vec4(aPosition, 0.0, 1.0);
	gl_Position = u_modelview * ftransform();

	vUv = vec2(aUv.x, aUv.y);

	final_color = u_color;
	// pre-multiply
	final_color.rgb *= final_color.a;
	final_color *= u_alpha;
}

// FRAGMENT_SHADER:

_IN_ vec4 final_color;

void main() {
	vec4 diffuse = Texture(u_source, vUv);

	// u_clip_rect is a rect (x,y,w,h) followed by 2 corner sizes followed by 2 more corner sizes

	//setOutputColor(final_color * diffuse.a);

	// TODO fix projection so y origin is not the bottom of window
	float y = (u_viewport.y + u_viewport.w) - gl_FragCoord.y;

	// TODO if we are always clipping, what is the point of the viewport?
	if(gl_FragCoord.x < u_clip_rect[0][2] && y < u_clip_rect[0][3])
		gl_FragColor = final_color * diffuse.a;
	else
		gl_FragColor = vec4(0.0);
}
