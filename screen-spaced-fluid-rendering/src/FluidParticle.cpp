#include "FluidParticle.h"

extern float nearPlane, farPlane;

// Contructor
FluidParticle::FluidParticle(float size)
{
	// Vertex positions
	const float positions [] = {
		// X	Y	Z
		size, size, 0,
		-size, size, 0,
		size, -size, 0,
		-size, -size, 0,
	};

	const float texCoords [] = {
		1, 1,
		0, 1,
		1, 0,
		0, 0,
	};

	const int indices [] = {
		0, 1, 2,
		2, 1, 3,
	};

	GLuint buffer;

	// Create the vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	// Create the buffer objects
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

	// Create buffer for instanced particle positions
	glGenBuffers(1, &transformBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(2, 1);

	// Index buffer
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	// Shaders
	GLuint vertexShader, fragmentShader, fragmentDataShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragmentDataShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vertexShaderSource = textFileRead(m_vertexShaderFile);
	const char * fragmentShaderSource = textFileRead(m_fragementShaderFile);
	const char * fragmentDataShaderSource = textFileRead(m_fragementDataShaderFile);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glShaderSource(fragmentDataShader, 1, &fragmentDataShaderSource, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	glCompileShader(fragmentDataShader);

	int compileOK[3] = { 0, 0, 0 };
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK[0]);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK[1]);
	glGetShaderiv(fragmentDataShader, GL_COMPILE_STATUS, &compileOK[2]);
	if (!(compileOK[0] + compileOK[1] + compileOK[2])) {
		fprintf(stderr, "Compilation error in %s shader\n",
			(!compileOK[0] ? "Vertex" : (!compileOK[1] ? "Fragement" : "Fragement Data")));
		//GLint infoLogLength;
		//glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
		//GLchar* strInfoLog = new GLchar[infoLogLength + 1];
		//glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);
		throw;
	}

	particleDataShaderProgram = glCreateProgram();
	glAttachShader(particleDataShaderProgram, vertexShader);
	glAttachShader(particleDataShaderProgram, fragmentDataShader);

	glLinkProgram(particleDataShaderProgram);
	glValidateProgram(particleDataShaderProgram);

	glUseProgram(particleDataShaderProgram);
	glUniform1f(glGetUniformLocation(particleDataShaderProgram, "sphereRadius"), size);
	glUniform1f(glGetUniformLocation(particleDataShaderProgram, "znear"), nearPlane);
	glUniform1f(glGetUniformLocation(particleDataShaderProgram, "zfar"), farPlane);
	
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);
	glValidateProgram(shaderProgram);

	glUseProgram(shaderProgram);
	glUniform1f(glGetUniformLocation(shaderProgram, "sphereRadius"), size);
	glUniform1f(glGetUniformLocation(shaderProgram, "znear"), nearPlane);
	glUniform1f(glGetUniformLocation(shaderProgram, "zfar"), farPlane);
	
	glUseProgram(0);
}

FluidParticle::~FluidParticle()
{
}

void FluidParticle::setupFBO(int width, int height){

	// Set up renderbuffer
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	// Color buffer
	glGenTextures(1, &m_colorTexture);
	glBindTexture(GL_TEXTURE_2D, m_colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Depth data
	glGenTextures(1, &m_dataTexture);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Thickess data
	/*GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	GL_TEXTURE_2D, depthTexture, 0);*/
	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_dataTexture, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("There is a problem with the FBO\n");
		throw;
	}	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void FluidParticle::DrawData(mat4 view, mat4 projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(1,1,1,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawShader(particleDataShaderProgram, view, projection);
}

void FluidParticle::Draw(mat4 view, mat4 projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture); //m_colorTexture);
	glUniform1i(glGetUniformLocation(shaderProgram, "depthTex"), 0);

	DrawShader(shaderProgram, view, projection);


	// DEBUG: blit texture
	int W = 1280, H = 960;
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, W, H,
		0, 0, W / 4, H / 4, GL_COLOR_BUFFER_BIT, GL_NEAREST);

}

void FluidParticle::DrawShader(GLuint program, const glm::mat4 view, const glm::mat4 projection){
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	glUseProgram(program);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(program, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"),
		1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vertexArrayObject);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_positions.size());

	glUseProgram(0);
}