#version 330

uniform mat4 view;
uniform mat4 projection;

in vec3	position;
in vec2	texCoord;

out vec2 TexCoord;

void main() 
{
	gl_Position = projection * view * vec4(position, 1);
	TexCoord = texCoord;
}