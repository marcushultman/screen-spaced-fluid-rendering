#version 330

uniform mat4 projection;
uniform float radius;

in vec2 tex_coord;
in vec3 eye_space_pos;

layout(location = 0) out vec4 thickness;

void main() {
  // eye-space normal
  vec3 normal = vec3(tex_coord * 2.0 - 1.0, 0);
  float r2 = dot(normal.xy, normal.xy);
  if (r2 > 1) {
    discard;  // outside_circle
  }
  normal.z = sqrt(1.0 - r2);

  // depth
  vec4 view_pos = vec4(eye_space_pos + normal * radius, 1.0);
  vec4 clip_space_pos = projection * view_pos;
  gl_FragDepth = clip_space_pos.z / clip_space_pos.w;

  // thickness
  thickness = vec4(.2 * vec3(1 - r2), 1);
}
