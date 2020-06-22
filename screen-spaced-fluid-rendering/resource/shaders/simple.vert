#version 330

uniform mat4 view;
uniform mat4 projection;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
out vec2 texCoord;

void main() 
{
  gl_Position = projection * view * vec4(aPosition, 1);
  texCoord = aTexCoord;
}