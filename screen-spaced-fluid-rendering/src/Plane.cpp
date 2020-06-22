#include "Plane.h"

Plane::Plane(float size)
	: Plane(size, size)
{
}

Plane::Plane(float width, float height)
{
	load(width, height);
}


Plane::~Plane()
{
}

void Plane::load(float width, float height)
{
	const float positions [] = {
		// X	Y	Z
		width, 0, height,
		-width, 0, height,
		width, 0, -height,
		-width, 0, -height,
	};

	const float texCoords [] = {
		0, 0,
		1, 0,
		0, 1,
		1, 1,
	};

	const int indices [] = {
		0, 1, 2,
    2, 1, 3,
	};

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	GLuint buffer;

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Load shader
	
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	auto vv = textFileRead("screen-spaced-fluid-rendering/resource/shaders/simple.vert");
	auto ff = textFileRead("screen-spaced-fluid-rendering/resource/shaders/simple.frag");

	glShaderSource(vertexShader, 1, &vv, NULL);
	glShaderSource(fragmentShader, 1, &ff, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	int compileOK;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
  assert(compileOK);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
  assert(compileOK);

	m_shader = glCreateProgram();
	glAttachShader(m_shader, vertexShader);
	glAttachShader(m_shader, fragmentShader);
	glLinkProgram(m_shader);
	glValidateProgram(m_shader);

	// Load texture

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	ILuint image = ilGenImage();
	ilBindImage(image);

	ilLoadImage("screen-spaced-fluid-rendering/resource/ground.jpg");
	ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
      ilGetInteger(IL_IMAGE_WIDTH),
      ilGetInteger(IL_IMAGE_HEIGHT),
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      ilGetData());
	ilDeleteImage(image);

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Plane::draw(const glm::mat4 view, const glm::mat4 proj)
{
	glUseProgram(m_shader);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_shader, "projection"),
		1, GL_FALSE, glm::value_ptr(proj));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glUniform1i(glGetUniformLocation(m_shader, "uTexture"), 0);

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}