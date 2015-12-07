#version 330

uniform sampler2D source;

uniform float offset[3] = float[]( 0.0, 1.3846153846, 3.2307692308 );
uniform float weight[3] = float[]( 0.2270270270, 0.3162162162, 0.0702702703 );
uniform bool vertical;

vec2 dCoord(int i){
	if (vertical)
		return vec2(0.0, offset[i]);
	return vec2(offset[i], 0.0);
}

void main()
{
	vec2 screenSize = textureSize(source, 0);
	vec2 screenCoord = gl_FragCoord.xy / screenSize;
	float depth = texture(source, screenCoord).x;
	
	float maxDepth = 1;
	if (depth >= maxDepth) discard;

	//gl_FragDepth = depth; return; // uncommment to disable blur
	
	depth *= weight[0];
	for (int i = 1; i < 3; i++) {
        depth += texture(source, 
			(gl_FragCoord.xy + dCoord(i)) / screenSize).x * weight[i];
        depth += texture(source,
			(gl_FragCoord.xy - dCoord(i)) / screenSize).x * weight[i];
    }

	gl_FragDepth = depth;
}