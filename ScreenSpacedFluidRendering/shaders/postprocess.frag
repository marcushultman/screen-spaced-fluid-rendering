#version 330

uniform sampler2D frameBufferTexture;

out vec4 fragmentColor;

void main() 
{
	vec4 color = texture(frameBufferTexture, gl_FragCoord.xy / textureSize(frameBufferTexture, 0));
	//float mean = (color.r + color.g + color.b) / 3.0;
	//fragmentColor = vec4(mean, mean, mean, 1);
	fragmentColor = color;

	if (textureSize(frameBufferTexture, 0).x == 1)
	{
		fragmentColor = vec4(1, 0, 0, 1);
		return;
	}
}