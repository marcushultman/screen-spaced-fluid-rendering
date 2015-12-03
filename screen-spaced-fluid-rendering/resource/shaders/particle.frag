#version 330

uniform mat4 view;
uniform mat4 projection;

uniform float znear;
uniform float zfar;
uniform float sphereRadius;

uniform sampler2D depthTex;
uniform float sampling;

uniform int renderDepth;

//uniform sampler2D thicknessTex;

in vec2 texCoord;
in vec3 eyeSpacePos;
in vec4 color;

layout (location=0) out vec4 fragColor;

vec3 uvToEye(vec2 uv, float depth)
{
	vec4 clipSpacePos;
	clipSpacePos.xy = uv.xy * 2.0f - 1.0f;
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
	vec2 screenSize = textureSize(depthTex, 0);
	vec2 texelUnit = 1.0 / (screenSize);
	vec2 screenCoord = gl_FragCoord.xy * texelUnit / sampling;
	float depth = texture(depthTex, screenCoord).x;
	
	float maxDepth = 1;
	if (depth >= maxDepth) discard;

	// Linearize and render
	if (renderDepth == 1){
		depth = (2 * znear) / (zfar + znear - depth * (zfar - znear));
		fragColor = vec4(vec3(depth), 1);
		return;
	}

	
	// ------- Construct normal ---------------------------
	
	vec2 texelUnitX = vec2(texelUnit.x, 0);
	vec2 texelUnitY = vec2(0, texelUnit.y);
	
	// calculate eye-space position from depth
	vec3 posEye = uvToEye(screenCoord, depth);
	
	// calculate differences
	vec3 ddx = getEyePos(screenCoord + texelUnitX) - posEye;
	vec3 ddx2 = posEye - getEyePos(screenCoord - texelUnitX);
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}
	
	vec3 ddy = getEyePos(screenCoord + texelUnitY) - posEye;
	vec3 ddy2 = posEye - getEyePos(screenCoord - texelUnitY);
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}
	
	// calculate normal
	vec3 N = normalize(cross(ddx, ddy) / sampling);
	
	// World space normals
	N = vec3(inverse(view) * vec4(N, 0));
	
	// Debug: render normal
	//N = (N + vec3(1)) / 2.0;
	//fragColor = vec4(N, 1);
	//return;

  
	// --- Render ---

	vec4 viewPos;

	// Light direction in world space
	//vec4 lightDir = inverse(view) * vec4(-.2, 1, -1, 0);
	vec4 lightDir = vec4(-.2, 1, 1, 0);
	
	// Water depth
	float waterDepth = 1; // .8 + .05 * (inverse(view) * viewPos).y;
	
	float ambient = .6;
	float diff = ambient + (1 - ambient) * max(0.0, dot(N, lightDir.xyz));
	vec3 color = waterDepth * vec3(.4, .6, .9);
	fragColor = vec4(diff * color, 1.0);
}