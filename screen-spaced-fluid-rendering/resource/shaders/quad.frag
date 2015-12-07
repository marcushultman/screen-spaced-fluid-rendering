#version 330

uniform sampler2D source;
uniform vec2 screenSize;

void main()
{
	vec2 screenCoord = gl_FragCoord.xy / screenSize;
	gl_FragDepth = texture(source, screenCoord).x;
}