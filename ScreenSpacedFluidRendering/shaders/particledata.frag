#version 330

uniform mat4 view;
uniform mat4 projection;
uniform float znear;
uniform float zfar;

uniform float sphereRadius;

in vec2 texCoord;
in vec3 eyeSpacePos;
in vec4 color;

layout (location=0) out vec4 fragColor;

void main() 
{
	//vec3 lightDir = (vec4(-1, -1, -1, 1) * inverse(view)).xyz;

	// calculate eye-space sphere normal from texture coordinates
	vec3 N;
	N.xy = texCoord*2.0-1.0;

	float r2 = dot(N.xy, N.xy);
	if (r2 > 1) discard; // kill pixels outside circle
	N.z = sqrt(1.0 - r2 * r2);

	// calculate depth
	vec4 pixelPos = vec4(eyeSpacePos + N * sphereRadius, 1.0);
	vec4 clipSpacePos = projection * pixelPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;
	
	//gl_FragDepth = (gl_FragDepth + 1.0) / (500 + 1 - gl_FragDepth * (500 - 1));
	//gl_FragDepth = znear * (gl_FragDepth + 1.0) / (zfar + znear - gl_FragDepth * (zfar - znear));




	//gl_FragDepth = (gl_FragDepth - znear) / (zfar - znear);
	//gl_FragDepth = (2 * znear) / (zfar + znear - gl_FragDepth * (zfar - znear));

	//float c0 = (1 - zfar / znear) / 2.0;
	//float c1 = (1 + zfar / znear) / 2.0;
	//gl_FragDepth = 1.0 / (c0 * gl_FragDepth + c1);
}