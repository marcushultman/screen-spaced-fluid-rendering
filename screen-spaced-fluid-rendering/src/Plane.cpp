#include "Plane.h"

Plane::Plane(float size)
	: Plane(size, size)
{
}

Plane::Plane(float width, float height)
{
	// Vertex positions
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

	GLuint buffer;

	// Create the vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create the buffer objects
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);
	
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, false/*normalized*/, 0/*stride*/, 0/*offset*/);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLuint vertexShader, fragmentShader;

	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vv = textFileRead(vertexShaderFile);
	const char * ff = textFileRead(fragmentShaderFile);

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

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");

	glBindAttribLocation(shaderProgram, 0, "position");
	glBindAttribLocation(shaderProgram, 1, "texCoord");

	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);
}


Plane::~Plane()
{
}

void Plane::Draw(mat4 view, mat4 projection)
{
	glUseProgram(shaderProgram);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, 
		"view"), 
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram,
		"projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glUseProgram(0);
}