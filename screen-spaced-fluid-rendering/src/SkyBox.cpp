#include "SkyBox.h"

SkyBox::SkyBox()
{
}


SkyBox::~SkyBox()
{
}


void SkyBox::Load()
{
	box = new Box(200);

	GLuint vS, fS;

	vS = glCreateShader(GL_VERTEX_SHADER);
	fS = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vv = textFileRead("resource/shaders/skybox.vert");
	const char * ff = textFileRead("resource/shaders/skybox.frag");

	glShaderSource(vS, 1, &vv, NULL);
	glShaderSource(fS, 1, &ff, NULL);

	glCompileShader(vS);
	glCompileShader(fS);
	
	shader = glCreateProgram();
	glAttachShader(shader, vS);
	glAttachShader(shader, fS);

	glBindFragDataLocation(shader, 0, "fragmentColor");

	glBindAttribLocation(shader, 0, "position");
	glBindAttribLocation(shader, 1, "texCoord");

	glLinkProgram(shader);
	glValidateProgram(shader);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	GLenum face [] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, 

		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	};
	std::wstring imageFilename [] = {
		L"resource/envmap_miramar/miramar_bk.tga",
		L"resource/envmap_miramar/miramar_ft.tga",

		L"resource/envmap_miramar/miramar_dn.tga",
		L"resource/envmap_miramar/miramar_up.tga",
		
		L"resource/envmap_miramar/miramar_lf.tga",
		L"resource/envmap_miramar/miramar_rt.tga",
	};

	ILuint imageIds[6];
	ilGenImages(6, imageIds);

	for (unsigned int i = 0; i < 6; i++)
	{
		ilBindImage(imageIds[i]);

		ilLoadImage(imageFilename[i].c_str());
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

		glTexImage2D(face[i], 0, GL_RGBA,
			ilGetInteger(IL_IMAGE_WIDTH),
			ilGetInteger(IL_IMAGE_HEIGHT),
			0, GL_RGBA, GL_UNSIGNED_BYTE,
			ilGetData());
	}
	ilDeleteImages(6, imageIds);
	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

}

void SkyBox::Draw(mat4 view, mat4 projection)
{
	glUseProgram(shader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glProgramUniformMatrix4fv(shader,
		glGetUniformLocation(shader, "view"),
		1, GL_FALSE, glm::value_ptr(view));

	glProgramUniformMatrix4fv(shader,
		glGetUniformLocation(shader, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	// Extract camera position
	vec3 cameraPosition = vec3(glm::inverse(view)[3]);

	glProgramUniform3fv(shader,
		glGetUniformLocation(shader, "cameraPosition"),
		1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glUniform1i(glGetUniformLocation(shader, "s_texture"), 0);
	
	box->Draw();

	glUseProgram(0);
}