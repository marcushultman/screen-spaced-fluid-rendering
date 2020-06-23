#version 330

uniform mat4 projection;
uniform float radius;

in vec2 tex_coord;
in vec3 eye_space_pos;

layout(location = 0) out vec4 thickness;

void main() {
  // Eye-space normal from texCoords
  vec3 N = vec3(tex_coord * 2.0 - 1.0, 0);
  float r2 = dot(N.xy, N.xy);
  if (r2 > 1) {
    discard;  // kill pixels outside circle
  }
  N.z = sqrt(1.0 - r2);

  // Depth
  vec4 viewPos = vec4(eye_space_pos + N * radius, 1.0);
  vec4 clipSpacePos = projection * viewPos;
  gl_FragDepth = clipSpacePos.z / clipSpacePos.w;

  // Thickness
  thickness = vec4(.2 * vec3(1 - r2), 1);
}