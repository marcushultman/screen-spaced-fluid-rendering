#version 330

uniform mat4 view;
uniform mat4 projection;

uniform float znear;
uniform float zfar;
uniform float sphereRadius;

uniform sampler2D backgroundTexture;
uniform sampler2D depthTexture;
uniform sampler2D thicknessTexture;
uniform samplerCube reflectionTexture;

in vec2 texCoord;
in vec3 eyeSpacePos;
in vec4 color;

layout (location=0) out vec4 fragColor;

vec4 uvToEye(vec2 uv, float depth)
{
	vec4 clipSpacePos;
	clipSpacePos.xy = uv.xy * 2.0f - 1.0f;
	clipSpacePos.z = depth;
	clipSpacePos.w = 1;
	vec4 hPos = inverse(projection) * clipSpacePos;
	return vec4(hPos.xyz / hPos.w, 1);
}

vec4 getEyePos(vec2 uv)
{
	return uvToEye(uv, texture(depthTexture, uv).x);
}

void main()
{
	// ------- Retrive depth -------
	
	vec2 screenSize = textureSize(depthTexture, 0);
	vec2 texelUnit = 1.0 / (screenSize);
	vec2 screenCoord = gl_FragCoord.xy * texelUnit;
	float depth = texture(depthTexture, screenCoord).x;
	
	// Kill pixels outside circle
	vec2 uv = texCoord * 2.0 - 1.0;
	if (dot(uv, uv) > 1) discard;

	// DEBUG: Linearized depth
	//depth = (2 * znear) / (zfar + znear - depth * (zfar - znear));
	//fragColor = vec4(vec3(depth), 1);
	//return;
	
	// DEBUG: Thickness
	//fragColor = texture(thicknessTexture, screenCoord);
	//return;

	// DEBUG: Background texture
	//fragColor = texture(backgroundTexture, screenCoord) * vec4(1.8, .2, .2, 1);
	//return;

	// ------- Reconstruct normal -------
	
	vec2 texelUnitX = vec2(texelUnit.x, 0);
	vec2 texelUnitY = vec2(0, texelUnit.y);
	
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
	
	// Calculate world space normal
	mat4 invView = inverse(view);
	vec3 N = vec3(invView * vec4(normalize(cross(ddx.xyz, ddy.xyz)), 0));
	
	// DEBUG: Render normal
	//N = (N + vec3(1)) / 2.0;
	//fragColor = vec4(N, 1);
	//return;

  
	// ------- Render -------

	vec4 viewPos;

	// Light direction
	vec4 lightDir = vec4(-.2, 1, 1, 0);
	vec3 viewDirection = normalize(vec3(invView * vec4(0.0, 0.0, 0.0, 1.0) - posEye));
	
	float fresnelPwr = .8;
	float fresnel = (1 - fresnelPwr) + fresnelPwr
		* pow(1 - dot(N, viewDirection), 5);

	float ambientPwr = .15;
	float attenuation = .8;
	
	// AMBIENT
	vec3 ambient = vec3(ambientPwr);

	// DIFFUSE
	float diffPwr = attenuation * max(0.0, dot(N, lightDir.xyz));
	vec3 diffuse = diffPwr * (invView * posEye).y * vec3(.01, .02, .03);
	
	// CUBEMAP REFLECTION
	vec3 refDir = reflect(viewDirection, N);
	if (refDir.y > .5) refDir.y = -refDir.y;
	diffuse = diffuse + fresnel * texture(reflectionTexture, refDir).xyz;

	// BACKGROUND DISTORTION
	float thickness = texture(thicknessTexture, screenCoord).x;
	float distPwr = 1 - thickness;
	diffuse = (1 - distPwr) * diffuse + distPwr * texture(backgroundTexture,
		screenCoord + N.xy * .025 * thickness).xyz;

	// DEBUG: Color due to absorbation
	//fragColor = vec4(mix(vec3(1), diffuse, thickness), 1);
	//return;

	// SPECULAR
	vec3 specular = vec3(0);
	vec3 lightSpec = vec3(.5);
	vec3 matSpec = vec3(.3);
	float matShininess = 10;
	if (dot(N, lightDir.xyz) >= 0.0) {
		specular = attenuation * lightSpec * matSpec
			* pow(max(0.0, dot(reflect(-lightDir.xyz, N), viewDirection)), matShininess);
	}
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}