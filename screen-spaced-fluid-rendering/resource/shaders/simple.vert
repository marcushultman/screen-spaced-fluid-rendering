#version 330

uniform mat4 view;
uniform mat4 projection;

in vec3	aPosition;
in vec2	aTexCoord;
out vec2 texCoord;

void main() 
{
	gl_Position = projection * view * vec4(aPosition, 1);
	texCoord = aTexCoord;
}