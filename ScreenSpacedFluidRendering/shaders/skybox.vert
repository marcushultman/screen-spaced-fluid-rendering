#version 330

uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraPosition;

in vec3	position;

out vec3 TexCoord;

void main() 
{
	TexCoord = vec3(position.x, -position.yz);
	gl_Position = projection * view * vec4(cameraPosition + position, 1);
}