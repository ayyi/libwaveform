// VERTEX_SHADER:
uniform vec4 u_color;

varying vec4 final_color;

void main() {
  //gl_Position = /*u_projection * */u_modelview * vec4(aPosition, 0.0, 1.0);
  gl_Position = vec4(aPosition, 0.0, 1.0);

  final_color = u_color;
  // Pre-multiply alpha
  final_color.rgb *= final_color.a;
  final_color *= u_alpha;
}

// FRAGMENT_SHADER:
varying vec4 final_color;

void main() {
  setOutputColor(final_color);
}

