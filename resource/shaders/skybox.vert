#version 330

uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraPosition;

in vec3	aPosition;
out vec3 texCoord;

void main() 
{
	texCoord = vec3(aPosition.x, -aPosition.yz);
	gl_Position = projection * view * vec4(cameraPosition + aPosition, 1);
}