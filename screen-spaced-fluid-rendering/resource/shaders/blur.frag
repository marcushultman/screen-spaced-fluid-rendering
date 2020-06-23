#version 330

uniform sampler2D depth_texture;
uniform sampler2D thickness_texture;
uniform vec2 direction;
uniform bool flip;
uniform vec2 screenSize;

float offset[3] = float[](0.0f, 1.3846153846f, 3.2307692308f);
float weight[3] = float[](0.2270270270f, 0.3162162162f, 0.0702702703f);

layout(location = 0) out vec3 thickness;

vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
  vec4 color = vec4(0);
  vec2 off1 = vec2(1.3846153846) * direction;
  vec2 off2 = vec2(3.2307692308) * direction;
  color += texture(image, uv) * 0.2270270270;
  color += texture(image, uv + (off1 / resolution)) * 0.3162162162;
  color += texture(image, uv - (off1 / resolution)) * 0.3162162162;
  color += texture(image, uv + (off2 / resolution)) * 0.0702702703;
  color += texture(image, uv - (off2 / resolution)) * 0.0702702703;
  return color;
}

void main() {
  vec2 uv = vec2(gl_FragCoord.xy / screenSize);
  if (flip) {
    uv.y = 1.0 - uv.y;
  }
  gl_FragDepth = blur9(depth_texture, uv, screenSize, direction).x;
  thickness = blur9(thickness_texture, uv, screenSize, direction).xyz;
}
