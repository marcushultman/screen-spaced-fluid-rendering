#version 330

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

uniform float sphereRadius;

layout (location = 0)	in	vec3		aVertex;
layout (location = 1)	in	vec2		aTexCoord;
layout (location = 2)	in	mat4		aTransform;

out vec2 texCoord;
out vec3 eyeSpacePos;
out vec4 color;

void main() 
{
	// TODO: Create world from view and aTransform
	vec3 cameraPosition = inverse(view)[3].xyz;
	vec3 position = aTransform[3].xyz;
	vec3 look = normalize(cameraPosition - position);
	vec3 right = cross(vec3(0,1,0), look);
	vec3 up2 = cross(look, right);
	mat4 transform;
	transform[0] = vec4(right, 0);
	transform[1] = vec4(up2, 0);
	transform[2] = vec4(look, 0);
	transform[3] = vec4(0, 0, 0, 1);

	gl_Position = projection * view * aTransform * transform *  vec4(aVertex, 1);

	texCoord = aTexCoord;
	eyeSpacePos = (view * aTransform * transform * vec4(aVertex, 1)).xyz;
	color = vec4(1, .5, .5, 1);
}