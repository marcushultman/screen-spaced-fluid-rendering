#version 330

uniform mat4 view;
uniform mat4 projection;
uniform float znear;
uniform float zfar;

uniform float sphereRadius;

uniform sampler2D depthBuffer;
uniform sampler2D thicknessBuffer;

in vec2 texCoord;
in vec3 eyeSpacePos;
in vec4 color;

layout (location=0) out vec4 fragColor;

// Y =[0..1] range (near plane to far plane)
float convertDepth(float Y)
{
	//gl_FragDepth = (2 * znear) / (zfar + znear - gl_FragDepth * (zfar - znear));
	
	//return znear + Y * (zfar - znear);

	return Y;

	//return Y * 2 - 1;

	//return - (-1 * (Y - 2) + 1 * Y) / (Y * (-1 - 1));
	//return - (-1 * (Y - 2) + 1 * Y) / (Y * (-1 - 1));
	
	//return Y * 2 - 1;
	//return znear + Y * (zfar - znear);
	//return - (znear * (Y - 2) + zfar * Y) / (Y * (znear - zfar));
	return -((2 * znear) / Y - (zfar + znear)) / (zfar - znear);
}

vec4 uvToEye(vec2 uv, float depth)
{
	return vec4(uv.x, uv.y, convertDepth(depth), 1);
}

vec4 getEyePos(vec2 uv)
{
	return uvToEye(uv, texture(depthBuffer, uv).x); 
}

void main() 
{
	vec2 texelUnit = 1.0 / textureSize(depthBuffer, 0);

	vec2 screenCoord = gl_FragCoord.xy * texelUnit;
	float depth = texture(depthBuffer, screenCoord).x;

	//float z = znear * (depth + 1.0) / (zfar + znear - depth * (zfar - znear));
	//fragColor = vec4(vec3(z), 1);
	//return;

	//fragColor = vec4(vec3(convertDepth(depth)), 1);
	//fragColor = vec4(vec3(depth), 1);
	//return;
	
	// ------- Construct normal ---------------------------

	float maxDepth = 1;
	vec2 texelUnitX = vec2(texelUnit.x, 0);
	vec2 texelUnitY = vec2(0, texelUnit.y);

	// read eye-space depth from texture
	if (depth >= maxDepth)  discard;

	// calculate eye-space position from depth
	vec4 posEye = uvToEye(screenCoord, depth);

	// calculate differences
	vec4 ddx = getEyePos(screenCoord + texelUnitX) - posEye;
	vec4 ddx2 = posEye - getEyePos(screenCoord - texelUnitX);
	if (abs(ddx.z) > abs(ddx2.z)) {
		ddx = ddx2;
	}

	vec4 ddy = getEyePos(screenCoord + texelUnitY) - posEye;
	vec4 ddy2 = posEye - getEyePos(screenCoord - texelUnitY);
	if (abs(ddy2.z) < abs(ddy.z)) {
		ddy = ddy2;
	}

	// calculate normal
	vec3 n = (inverse(view) * vec4(cross(ddx.xyz, ddy.xyz), 0)).xyz;
	n = normalize(n);

	n = (n + vec3(1)) / 2.0;
	fragColor = vec4(n, 1);
	return;
	
	vec3 lightDir = (vec4(-1, -1, -1, 1) * inverse(view)).xyz;

	float diffuse = max(0.0, dot(n, lightDir));
	fragColor = diffuse * color;

	return;

	// -------------------------------------------------------
	
	//vec3 lightDir = (vec4(-1, -1, -1, 1) * inverse(view)).xyz;

	// calculate eye-space sphere normal from texture coordinates
	vec3 N;
	N.xy = texCoord*2.0-1.0;
	
	float r2 = dot(N.xy, N.xy);
	if (r2 > 1) discard; // kill pixels outside circle
	N.z = sqrt(1.0 - r2 * r2);
	
	N = (N + vec3(1)) / 2.0;
	
	//fragColor = vec4(N, 1);

	//float diffuse = max(0.0, dot(N, lightDir));
	//diffuse = max(0.0, dot(n, lightDir));
	//fragColor = diffuse * color;
}