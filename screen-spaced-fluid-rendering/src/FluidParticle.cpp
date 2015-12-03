#include "FluidParticle.h"

extern float nearPlane, farPlane;

#define DOWNSAMPLE_SCALAR 1.0f

// Contructor
FluidParticle::FluidParticle(float size, int width, int height)
{
	m_width = width;
	m_height = height;

	setupVAO(size);
	setupShaders(size);
}

FluidParticle::~FluidParticle()
{
}

void FluidParticle::setupVAO(float size)
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
}

void FluidParticle::setupFBO(){

	// Set up renderbuffer
	glGenFramebuffers(1, &m_dataFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);

	// Color buffer
	glGenTextures(1, &m_colorTexture);
	glBindTexture(GL_TEXTURE_2D, m_colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height,
		0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Depth data
	glGenTextures(1, &m_dataTexture);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Thickess data
	/*GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
	GL_TEXTURE_2D, depthTexture, 0);*/

	// Blur data
	/*glGenTextures(1, &m_blurTexture);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTexture, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_blurTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_dataTexture, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("There is a problem with the FBO\n");
		throw;
	}	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidParticle::setupBlurFBO()
{
	// Set up renderbuffer
	glGenFramebuffers(1, &m_blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);

	glGenTextures(1, &m_blurTexture);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width / DOWNSAMPLE_SCALAR, m_height / DOWNSAMPLE_SCALAR,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_blurTexture, 0);

	glDrawBuffer(GL_NONE);

	/*glGenTextures(1, &m_blurTexture);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
		m_width / DOWNSAMPLE_SCALAR, m_height / DOWNSAMPLE_SCALAR,
		0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_blurTexture, 0);*/

	// Depth data
	/*glGenTextures(1, &m_dataTexture);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0,
		GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);*/

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("There is a problem with the blur FBO\n");
		throw;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidParticle::setupShaders(float size)
{
	GLuint vertexShader, fragmentShader, fragmentDataShader,
		fragmentBlurVShader, fragmentBlurShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	fragmentDataShader = glCreateShader(GL_FRAGMENT_SHADER);

	fragmentBlurVShader = glCreateShader(GL_VERTEX_SHADER);
	fragmentBlurShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char * vertexShaderSource = textFileRead(m_vertexShaderFile);
	const char * fragmentShaderSource = textFileRead(m_fragementShaderFile);
	const char * fragmentDataShaderSource = textFileRead(m_fragementDataShaderFile);

	const char * fragmentBlurVShaderSource = textFileRead(m_fragementBlurVShaderFile);
	const char * fragmentBlurShaderSource = textFileRead(m_fragementBlurShaderFile);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glShaderSource(fragmentDataShader, 1, &fragmentDataShaderSource, NULL);

	glShaderSource(fragmentBlurVShader, 1, &fragmentBlurVShaderSource, NULL);
	glShaderSource(fragmentBlurShader, 1, &fragmentBlurShaderSource, NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	glCompileShader(fragmentDataShader);

	glCompileShader(fragmentBlurVShader);
	glCompileShader(fragmentBlurShader);

	int compileOK[5] = { 0, 0, 0, 0, 0 };
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileOK[0]);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileOK[1]);
	glGetShaderiv(fragmentDataShader, GL_COMPILE_STATUS, &compileOK[2]);

	glGetShaderiv(fragmentBlurVShader, GL_COMPILE_STATUS, &compileOK[3]);
	glGetShaderiv(fragmentBlurShader, GL_COMPILE_STATUS, &compileOK[4]);
	
	if (!(compileOK[0] + compileOK[1] + compileOK[2] + compileOK[3])) {
		fprintf(stderr, "Compilation error in %s shader\n",
			(!compileOK[0] ? "Vertex" :
			(!compileOK[1] ? "Fragement" :
			(!compileOK[2] ? "Data" : "Blur"))));
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
	glUniform1f(glGetUniformLocation(shaderProgram, "sampling"), DOWNSAMPLE_SCALAR);


	m_blurProgram = glCreateProgram();
	glAttachShader(m_blurProgram, fragmentBlurVShader);
	glAttachShader(m_blurProgram, fragmentBlurShader);

	glLinkProgram(m_blurProgram);
	glValidateProgram(m_blurProgram);

	glUseProgram(m_blurProgram);
	glUniform1f(glGetUniformLocation(m_blurProgram, "sampling"), DOWNSAMPLE_SCALAR);

	glUseProgram(0);
}



void FluidParticle::DrawData(mat4 view, mat4 projection)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);
	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	DrawShader(particleDataShaderProgram, view, projection);
}

void FluidParticle::Draw(mat4 view, mat4 projection, int renderDepth)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glUseProgram(shaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture); //m_dataTexture); <- m_dataTexture is the upscaled blured version
	glUniform1i(glGetUniformLocation(shaderProgram, "depthTex"), 0);

	glUniform1i(glGetUniformLocation(shaderProgram, "renderDepth"), renderDepth);

	DrawShader(shaderProgram, view, projection);


	// DEBUG: blit texture
	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, m_dataFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_width, m_height,
		0, 0, m_width / 4, m_height / 4, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_blurFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, m_width / DOWNSAMPLE_SCALAR, m_height / DOWNSAMPLE_SCALAR,
		m_width / 4, 0, m_width / 2, m_height / 4, GL_COLOR_BUFFER_BIT, GL_NEAREST);*/

	//glReadBuffer(GL_COLOR_ATTACHMENT0);
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


void FluidParticle::blur()
{
	// Pre-step: Set viewport, bind buffer, clear contents..
	glViewport(0, 0, m_width / DOWNSAMPLE_SCALAR, m_height / DOWNSAMPLE_SCALAR);
	glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// .. bind program, set texture
	glUseProgram(m_blurProgram);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(m_blurProgram, "source"), 0);

	// Step 1. Bind data texture, sample the full size texture
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glUniform1f(glGetUniformLocation(m_blurProgram, "sampling"), DOWNSAMPLE_SCALAR);
	glUniform1f(glGetUniformLocation(m_blurProgram, "vertical"), 0); drawQuad();
	glUniform1f(glGetUniformLocation(m_blurProgram, "vertical"), 1); drawQuad();

	// Step 2. Blur multiple times by rendering to the same texture - constant size
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glUniform1f(glGetUniformLocation(m_blurProgram, "sampling"), 1);
	for (unsigned int i = 0, o = 1; i < 16; i++) {
		glUniform1f(glGetUniformLocation(m_blurProgram, "vertical"), (o = 1 - o));
		drawQuad();
	}

	// Step 4. Reset viewport TODO: Upscale texture again
	glViewport(0, 0, m_width, m_height);
	/*glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUniform1f(glGetUniformLocation(m_blurProgram, "sampling"), 1 / DOWNSAMPLE_SCALAR);
	glUniform1f(glGetUniformLocation(m_blurProgram, "vertical"), 0); drawQuad();
	glUniform1f(glGetUniformLocation(m_blurProgram, "vertical"), 1); drawQuad();*/
	
	glUseProgram(0);

	//glDrawBuffer(GL_COLOR_ATTACHMENT0);

	//glViewport(0, 0, m_width, m_height);
	//glViewport(0, 0, m_width / DOWNSAMPLE_SCALAR, m_height / DOWNSAMPLE_SCALAR);
}

void FluidParticle::drawQuad()
{
	static GLuint s_quadVAO = 0;
	if (s_quadVAO == 0) {
		static const float positions [] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f,
		};

		glGenVertexArrays(1, &s_quadVAO);
		glBindVertexArray(s_quadVAO);

		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
	}

	glBindVertexArray(s_quadVAO);
	glDrawArrays(GL_QUADS, 0, 4);
}