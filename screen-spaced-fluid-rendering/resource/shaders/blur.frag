#version 330

uniform sampler2D source;
uniform float sampling;
	
uniform float offset[5] = float[]( 0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216,
                                   0.0540540541, 0.0162162162 );
uniform bool vertical;

vec2 dCoord(int i){
	if (vertical)
		return vec2(0.0, offset[i]);
	return vec2(offset[i], 0.0);
}

void main()
{
	vec2 screenSize = textureSize(source, 0) / sampling;
	vec2 texelUnit = 1.0f / screenSize;
	vec2 screenCoord = gl_FragCoord.xy * texelUnit;
	float depth = texture(source, screenCoord).x;
	
	float maxDepth = 1;
	if (depth >= maxDepth) discard;

	//gl_FragDepth = depth; return; // uncommment to disable blur
	
	depth *= weight[0];
	for (int i = 1; i < 5; i++) {
        depth += texture(source, 
			(gl_FragCoord.xy + dCoord(i)) / screenSize).x * weight[i];
        depth += texture(source,
			(gl_FragCoord.xy - dCoord(i)) / screenSize).x * weight[i];
    }

	gl_FragDepth = depth;
}