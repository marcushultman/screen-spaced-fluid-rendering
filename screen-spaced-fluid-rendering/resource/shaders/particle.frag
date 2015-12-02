#version 330

uniform mat4 view;
uniform mat4 projection;

uniform float znear;
uniform float zfar;
uniform float sphereRadius;

uniform sampler2D depthTex;
//uniform sampler2D thicknessTex;

in vec2 texCoord;
in vec3 eyeSpacePos;
in vec4 color;

layout (location=0) out vec4 fragColor;

vec3 uvToEye(vec2 uv, float depth)
{
	vec4 clipSpacePos;
	clipSpacePos.xy = (vec2(1) - uv) * 2.0f - 1.0f;
	clipSpacePos.z = depth;
	clipSpacePos.w = 1;
	vec4 hPos = inverse(projection) * clipSpacePos;
	return hPos.xyz / hPos.w;
}

vec3 getEyePos(vec2 uv)
{
	return uvToEye(uv, texture(depthTex, uv).x);
}

void main() 
{
	vec2 texelUnit = 1.0 / textureSize(depthTex, 0);
	vec2 screenCoord = gl_FragCoord.xy * texelUnit;
	float depth = texture(depthTex, screenCoord).x;
	
	float maxDepth = 1;
	if (depth >= maxDepth) discard;
	
	//fragColor = vec4(vec3(depth), 1); return;
	
	// ------- Construct normal ---------------------------
	
	vec2 texelUnitX = vec2(texelUnit.x, 0);
	vec2 texelUnitY = vec2(0, texelUnit.y);
	
	// calculate eye-space position from depth
	vec4 posEye = vec4(uvToEye(screenCoord, depth), 1.0);
	
	// calculate differences
	vec3 ddx = getEyePos(screenCoord + texelUnitX) - posEye.xyz;
	vec3 ddx2 = posEye.xyz - getEyePos(screenCoord - texelUnitX);
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}
	
	vec3 ddy = getEyePos(screenCoord + texelUnitY) - posEye.xyz;
	vec3 ddy2 = posEye.xyz - getEyePos(screenCoord - texelUnitY);
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}
	
	// calculate normal
	vec3 N = normalize(cross(ddx, ddy));
	
	// World space normals
	N = vec3(inverse(view) * vec4(N, 0));
	
	// Debug: render normal
	N = (N + vec3(1)) / 2.0;
	fragColor = vec4(N, 1);
	return;

  
	// --- Render ---

	vec4 viewPos;

	// Light direction in world space
	vec4 lightDir = inverse(view) * vec4(-.2, 1, -1, 0);
	
	// Water depth
	float waterDepth = .8 + .05 * (inverse(view) * viewPos).y;
	
	float ambient = .2;
	float diff = ambient + (1 - ambient) * max(0.0, dot(N, lightDir.xyz));
	vec3 color = waterDepth * vec3(.2, .4, .8);
	fragColor = vec4(diff * color, 1.0);
}