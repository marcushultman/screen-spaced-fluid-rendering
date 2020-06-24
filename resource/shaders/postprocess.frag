#version 330

uniform sampler2D source;
uniform sampler2D depth;

out vec4 fragColor;

void main()
{
	vec2 screenCoord = gl_FragCoord.xy / textureSize(source, 0);
	fragColor = texture(source, screenCoord);
	gl_FragDepth = texture(depth, screenCoord).x;
}