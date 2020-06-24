#version 330

uniform sampler2D uTexture;

in vec2 texCoord;
out vec4 fragColor;

void main() 
{
	fragColor = texture(uTexture, 4 * texCoord);
}