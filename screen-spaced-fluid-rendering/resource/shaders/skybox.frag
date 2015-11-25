#version 330

uniform samplerCube s_texture;

in vec3 TexCoord;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(s_texture, TexCoord);
}