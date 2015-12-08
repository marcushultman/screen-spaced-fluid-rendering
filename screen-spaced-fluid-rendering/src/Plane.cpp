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
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vv = textFileRead("resource/shaders/simple.vert");
	const char * ff = textFileRead("resource/shaders/simple.frag");

	glShaderSource(vertexShader, 1, &vv, NULL);
	glShaderSource(fragmentShader, 1, &ff, NULL);

	int compileOK;

	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		GLint infoLogLength;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compilation error in vertexShader: %s\n", strInfoLog);
		delete [] strInfoLog;

		throw;
	}

	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK);
	if (!compileOK) {
		GLint infoLogLength;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, strInfoLog);

		fprintf(stderr, "Compilation error in fragmentShader: %s\n", strInfoLog);
		delete [] strInfoLog;

		throw;
	}

	m_shader = glCreateProgram();
	glAttachShader(m_shader, vertexShader);
	glAttachShader(m_shader, fragmentShader);

	glBindFragDataLocation(m_shader, 0, "fragmentColor");

	glBindAttribLocation(m_shader, 0, "position");
	glBindAttribLocation(m_shader, 1, "texCoord");

	glLinkProgram(m_shader);
	glValidateProgram(m_shader);

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

	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}