#version 330

uniform mat4 inv_view;
uniform mat4 inv_projection;

uniform float znear;
uniform float zfar;

uniform sampler2D background_texture;
uniform sampler2D depth_texture;
uniform sampler2D thickness_texture;
uniform samplerCube reflection_texture;

in vec2 tex_coord;
in vec3 eye_space_pos;
in vec4 color;

layout(location = 0) out vec3 frag_color;

vec3 uvToEye(vec2 uv, float depth) {
  vec4 clipSpacePos;
  clipSpacePos.xy = uv.xy * 2.0f - 1.0f;
  clipSpacePos.z = depth;
  clipSpacePos.w = 1;
  clipSpacePos = inv_projection * clipSpacePos;
  return clipSpacePos.xyz / clipSpacePos.w;
}

vec3 getEyePos(vec2 uv) {
  return uvToEye(uv, texture(depth_texture, uv).x);
}

void main() {
  // ------- Retrive depth -------

  vec2 screenSize = textureSize(depth_texture, 0);
  vec2 texelUnit = 1.0 / (screenSize);
  vec2 screenCoord = gl_FragCoord.xy * texelUnit;
  float depth = texture(depth_texture, screenCoord).x;

  vec2 uv = tex_coord * 2.0 - 1.0;
  if (dot(uv, uv) > 1) {
    discard;  // outside circle
  }

  // DEBUG: Linearized depth
  // depth = (2 * znear) / (zfar + znear - depth * (zfar - znear));
  // frag_color = vec3(depth);
  // return;

  // DEBUG: Thickness
  // frag_color = texture(thickness_texture, screenCoord).xyz;
  // return;

  // DEBUG: Background texture
  // frag_color = texture(background_texture, screenCoord).xyz * vec3(1.8, .2, .2);
  // return;

  // ------- Reconstruct normal -------

  vec2 texelUnitX = vec2(texelUnit.x, 0);
  vec2 texelUnitY = vec2(0, texelUnit.y);

  // calculate eye-space position from depth
  vec3 posEye = uvToEye(screenCoord, depth);

  // calculate differences
  vec3 ddx = getEyePos(screenCoord + texelUnitX) - posEye;
  vec3 ddx2 = posEye - getEyePos(screenCoord - texelUnitX);
  ddx = abs(ddx.z) > abs(ddx2.z) ? ddx2 : ddx;

  vec3 ddy = getEyePos(screenCoord + texelUnitY) - posEye;
  vec3 ddy2 = posEye - getEyePos(screenCoord - texelUnitY);
  ddy = abs(ddy.z) > abs(ddy2.z) ? ddy2 : ddy;

  // Calculate world space normal
  vec3 N = vec3(inv_view * vec4(normalize(cross(ddx, ddy)), 0));

  // DEBUG: Render normal
  // N = (N + vec3(1)) / 2.0;
  // frag_color = N;
  // return;

  // ------- Render -------

  // Light direction
  vec4 light_dir = vec4(-.2, 1, 1, 0);
  vec3 view_direction = normalize(vec3(inv_view * vec4(vec3(0), 1) - vec4(posEye, 1)));

  float fresnel_power = .6;
  float fresnel = (1 - fresnel_power) + fresnel_power * pow(1 - dot(N, view_direction), 1.5);

  float ambient_power = .15;
  float attenuation = .8;

  // AMBIENT
  vec3 ambient = vec3(ambient_power);

  // DIFFUSE
  float diff_power = attenuation * max(0.0, dot(N, light_dir.xyz));
  vec3 diffuse = diff_power * (inv_view * vec4(posEye, 1)).y * vec3(.01, .02, .03);

  // CUBEMAP REFLECTION
  vec3 ref_dir = reflect(view_direction, N);
  ref_dir.y *= ref_dir.y > .5 ? -1 : 1;
  diffuse += fresnel * texture(reflection_texture, ref_dir).xyz;

  // BACKGROUND DISTORTION
  float thickness = texture(thickness_texture, screenCoord).x;
  float dist_power = 1 - thickness;
  diffuse = (1 - dist_power) * diffuse +
            dist_power * texture(background_texture, screenCoord + N.xy * .025 * thickness).xyz;

  // DEBUG: Color due to absorbation
  // frag_color = mix(vec3(1), diffuse, thickness);
  // return;

  // SPECULAR
  vec3 specular = vec3(0);
  vec3 light_spec = vec3(.5);
  vec3 mat_spec = vec3(.3);
  float mat_shininess = 10;
  if (dot(N, light_dir.xyz) >= 0.0) {
    specular = attenuation * light_spec * mat_spec *
               pow(max(0.0, dot(reflect(-light_dir.xyz, N), view_direction)), mat_shininess);
  }
  frag_color = ambient + diffuse + specular;
}