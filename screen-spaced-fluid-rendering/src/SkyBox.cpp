#include "SkyBox.h"

SkyBox::SkyBox()
{
	load();
}


SkyBox::~SkyBox()
{
}


void SkyBox::load()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); 
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto vv = textFileRead("screen-spaced-fluid-rendering/resource/shaders/skybox.vert");
	auto ff = textFileRead("screen-spaced-fluid-rendering/resource/shaders/skybox.frag");

  auto p = vv.c_str();
	glShaderSource(vertexShader, 1, &p, NULL);
  p = ff.c_str();
	glShaderSource(fragmentShader, 1, &p, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	
	m_shader = glCreateProgram();
	glAttachShader(m_shader, vertexShader);
	glAttachShader(m_shader, fragmentShader);
	glLinkProgram(m_shader);
	glValidateProgram(m_shader);

	// Load texture cube

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

	GLenum face [] = {
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 
		GL_TEXTURE_CUBE_MAP_POSITIVE_X, 

		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	};
	std::string imageFilename [] = {
		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_bk.tga",
		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_ft.tga",

		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_dn.tga",
		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_up.tga",
		
		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_lf.tga",
		"screen-spaced-fluid-rendering/resource/envmap_miramar/miramar_rt.tga",
	};

	ILuint imageIds[6];
	ilGenImages(6, imageIds);
	for (unsigned int i = 0; i < 6; i++){
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

void SkyBox::draw(const glm::mat4 view, const glm::mat4 proj)
{
	glUseProgram(m_shader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"),
		1, GL_FALSE, glm::value_ptr(view));

	glUniformMatrix4fv(glGetUniformLocation(m_shader, "projection"),
		1, GL_FALSE, glm::value_ptr(proj));

	// Extract camera position
	glm::vec3 cameraPosition = glm::vec3(glm::inverse(view)[3]);

	glUniform3fv(glGetUniformLocation(m_shader, "cameraPosition"),
		1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
	glUniform1i(glGetUniformLocation(m_shader, "uTexture"), 0);
	
	m_box.draw();

	glUseProgram(0);
}