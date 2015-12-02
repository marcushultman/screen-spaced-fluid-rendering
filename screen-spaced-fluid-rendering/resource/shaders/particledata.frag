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
	// Eye-space normal from texCoords
	vec3 N = vec3(texCoord * 2.0 - 1.0, 0);
	float r2 = dot(N.xy, N.xy);
	if (r2 > 1) discard; // kill pixels outside circle
	//N.z = sqrt(1.0 - r2 * r2);
	N.z = -sqrt(1.0 - r2);

	// Calculate depth
	vec4 viewPos = vec4(eyeSpacePos + N * sphereRadius, 1.0);
	vec4 clipSpacePos = projection * viewPos;
	gl_FragDepth = clipSpacePos.z / clipSpacePos.w;

	// Linearized depth
	float z = gl_FragDepth;
	//gl_FragDepth	= (2 * znear) / (zfar + znear - z * (zfar - znear));
	//z				= (2 * znear) / (zfar + znear - z * (zfar - znear));

	fragColor = vec4(vec3(z), 1);
	return;
	
	// Light direction in world space
	vec4 lightDir = inverse(view) * vec4(-.2, 1, -1, 0);

	// Water depth
	float waterDepth = .8 + .05 * (inverse(view) * viewPos).y;

	float ambient = .2;
	float diff = ambient + (1 - ambient) * max(0.0, dot(N, lightDir.xyz));
	vec3 color = waterDepth * vec3(.2, .4, .8);
	fragColor = vec4(diff * color, 1.0);
}