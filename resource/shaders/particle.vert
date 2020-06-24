#version 330

uniform mat4 view;
uniform mat4 projection;
uniform float radius;

layout(location = 0) in vec2 _vertex_pos;
layout(location = 1) in vec2 _tex_coord;
layout(location = 2) in vec3 _particle_pos;

out vec2 tex_coord;
out vec3 eye_space_pos;

void main() {
  vec3 cam_right = vec3(view[0][0], view[1][0], view[2][0]);
  vec3 cam_up = vec3(view[0][1], view[1][1], view[2][1]);

  vec3 vertex_world =
      _particle_pos + cam_right * radius * _vertex_pos.x + cam_up * radius * _vertex_pos.y;
  vec4 view_pos = view * vec4(vertex_world, 1);

  gl_Position = projection * view_pos;

  tex_coord = _tex_coord;
  eye_space_pos = view_pos.xyz;
}
