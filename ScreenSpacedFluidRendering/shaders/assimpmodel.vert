#version 330

uniform mat4 modelMatrix;
uniform mat4 view;
uniform mat4 projection;

in vec3	position;
in vec3	normal;
in vec3	color;
in vec2	texCoord;

out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;

void main() 
{
	gl_Position = projection * view * modelMatrix * vec4(position, 1);
	Normal = normal;
	Color = color;
	TexCoord = texCoord;
}