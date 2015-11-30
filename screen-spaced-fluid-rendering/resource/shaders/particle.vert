#version 330

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform float sphereRadius;

layout (location = 0)	in	vec3		aVertex;
layout (location = 1)	in	vec2		aTexCoord;
layout (location = 2)	in	vec3		aPosition;

out vec2 texCoord;
out vec3 eyeSpacePos;
out vec4 color;

void main() 
{
	// Create world from view and aPosition
	vec3 cameraPosition = inverse(view)[3].xyz;
	vec3 look = normalize(cameraPosition - aPosition);
	vec3 right = cross(vec3(0,1,0), look);
	vec3 up2 = cross(look, right);
	mat4 world;
	world[0] = vec4(right, 0);
	world[1] = vec4(up2, 0);
	world[2] = vec4(look, 0);
	world[3] = vec4(aPosition, 1);

	vec4 viewPos = view * world * vec4(aVertex, 1);
	gl_Position = projection * viewPos;

	texCoord = aTexCoord;
	eyeSpacePos = viewPos.xyz;
	color = vec4(1, .5, .5, 1);
}