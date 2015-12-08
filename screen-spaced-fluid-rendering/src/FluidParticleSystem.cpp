#include "FluidParticleSystem.h"

#define SAMPLING_SCALAR (2.0f/3.0f)

FluidParticleSystem::FluidParticleSystem(float particleSize,
	unsigned int screenWidth, unsigned int screenHeight,
	float nearPlane, float farPlane)
{
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_nearPlane = nearPlane;
	m_farPlane = farPlane;

	setupVAO(particleSize);

	setupDataFBO();
	setupBlurFBO();

	setupShaders(particleSize);
}

FluidParticleSystem::~FluidParticleSystem()
{
}


void FluidParticleSystem::setupVAO(float particleSize)
{
	// Vertex positions
	const float positions [] = {
		// X	Y	Z
		particleSize, particleSize, 0,
		-particleSize, particleSize, 0,
		particleSize, -particleSize, 0,
		-particleSize, -particleSize, 0,
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
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

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
	glGenBuffers(1, &m_positionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(2, 1);

	// Index buffer
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void FluidParticleSystem::setupDataFBO(){

	// Set up renderbuffer
	glGenFramebuffers(1, &m_dataFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);

	// Color buffer
	glGenTextures(1, &m_colorTexture);
	glBindTexture(GL_TEXTURE_2D, m_colorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_screenWidth, m_screenHeight,
		0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Depth data
	glGenTextures(1, &m_dataTexture);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_screenWidth, m_screenHeight,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Thickess data
	glGenTextures(1, &m_thicknessTexture);
	glBindTexture(GL_TEXTURE_2D, m_thicknessTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screenWidth, m_screenHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//glEnablei(GL_BLEND, 1);
	//glDisablei(GL_DEPTH_TEST, 1);
	//glDisablei(GL_CULL_FACE, 1);
	//glBlendFunci(1, GL_SRC_ALPHA, GL_ONE);
	

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_colorTexture, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_dataTexture, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_thicknessTexture, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("There is a problem with the FBO\n");
		throw;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidParticleSystem::setupBlurFBO()
{
	// Set up renderbuffer
	glGenFramebuffers(1, &m_blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);

	glGenTextures(1, &m_blurTexture);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SAMPLING_SCALAR * m_screenWidth,
		SAMPLING_SCALAR * m_screenHeight,
		0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_blurTexture, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		printf("There is a problem with the blur FBO\n");
		throw;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FluidParticleSystem::setupShaders(float particleSize)
{
	GLuint particleVertexShader, particleFragmentShader, dataFragmentShader,
		quadVertexShader, quadFragmentShader, blurFragmentShader;
	particleVertexShader = glCreateShader(GL_VERTEX_SHADER);
	particleFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	dataFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	quadVertexShader = glCreateShader(GL_VERTEX_SHADER);
	quadFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	blurFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char * particleVertexShaderSource		= textFileRead(m_particleVertexShaderFile);
	const char * particleFragmentShaderSource	= textFileRead(m_particleFragmentShaderFile);
	const char * dataFragmentShaderSource		= textFileRead(m_dataFragmentShaderFile);

	const char * quadVertexShaderSource		= textFileRead(m_quadVertexShaderFile);
	const char * quadFragmentShaderSource	= textFileRead(m_quadFragmentShaderFile);
	const char * blurFragmentShaderSource	= textFileRead(m_blurFragmentShaderFile);

	glShaderSource(particleVertexShader, 1, &particleVertexShaderSource, NULL);
	glShaderSource(particleFragmentShader, 1, &particleFragmentShaderSource, NULL);
	glShaderSource(dataFragmentShader, 1, &dataFragmentShaderSource, NULL);

	glShaderSource(quadVertexShader, 1, &quadVertexShaderSource, NULL);
	glShaderSource(quadFragmentShader, 1, &quadFragmentShaderSource, NULL);
	glShaderSource(blurFragmentShader, 1, &blurFragmentShaderSource, NULL);

	glCompileShader(particleVertexShader);
	glCompileShader(particleFragmentShader);
	glCompileShader(dataFragmentShader);

	glCompileShader(quadVertexShader);
	glCompileShader(quadFragmentShader);
	glCompileShader(blurFragmentShader);

	int compileOK[6] = { 0, 0, 0, 0, 0, 0 };
	glGetShaderiv(particleVertexShader, GL_COMPILE_STATUS, &compileOK[0]);
	glGetShaderiv(particleFragmentShader, GL_COMPILE_STATUS, &compileOK[1]);
	glGetShaderiv(dataFragmentShader, GL_COMPILE_STATUS, &compileOK[2]);

	glGetShaderiv(quadVertexShader, GL_COMPILE_STATUS, &compileOK[3]);
	glGetShaderiv(quadFragmentShader, GL_COMPILE_STATUS, &compileOK[4]);
	glGetShaderiv(blurFragmentShader, GL_COMPILE_STATUS, &compileOK[5]);
	
	for (unsigned int i = 0; i < 6; i++)
		if (!compileOK[i])
			throw;
	//if (!(compileOK[0] + compileOK[1] + compileOK[2] + compileOK[3])) {
	//	fprintf(stderr, "Compilation error in %s shader\n",
	//		(!compileOK[0] ? "Vertex" :
	//		(!compileOK[1] ? "Fragement" :
	//		(!compileOK[2] ? "Data" : "Blur"))));
	//	//GLint infoLogLength;
	//	//glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
	//	//GLchar* strInfoLog = new GLchar[infoLogLength + 1];
	//	//glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);
	//	throw;
	//}

	m_dataProgram = glCreateProgram();
	glAttachShader(m_dataProgram, particleVertexShader);
	glAttachShader(m_dataProgram, dataFragmentShader);

	glLinkProgram(m_dataProgram);
	glValidateProgram(m_dataProgram);

	glUseProgram(m_dataProgram);
	glUniform1f(glGetUniformLocation(m_dataProgram, "sphereRadius"), particleSize);
	glUniform1f(glGetUniformLocation(m_dataProgram, "znear"), m_nearPlane);
	glUniform1f(glGetUniformLocation(m_dataProgram, "zfar"), m_farPlane);


	m_samplingProgram = glCreateProgram();
	glAttachShader(m_samplingProgram, quadVertexShader);
	glAttachShader(m_samplingProgram, quadFragmentShader);
	glLinkProgram(m_samplingProgram);
	glValidateProgram(m_samplingProgram);
	glUseProgram(m_samplingProgram);


	m_blurProgram = glCreateProgram();
	glAttachShader(m_blurProgram, quadVertexShader);
	glAttachShader(m_blurProgram, blurFragmentShader);
	glLinkProgram(m_blurProgram);
	glValidateProgram(m_blurProgram);
	glUseProgram(m_blurProgram);


	m_particleProgram = glCreateProgram();
	glAttachShader(m_particleProgram, particleVertexShader);
	glAttachShader(m_particleProgram, particleFragmentShader);
	glLinkProgram(m_particleProgram);
	glValidateProgram(m_particleProgram);
	glUseProgram(m_particleProgram);
	glUniform1f(glGetUniformLocation(m_particleProgram, "sphereRadius"), particleSize);
	glUniform1f(glGetUniformLocation(m_particleProgram, "znear"), m_nearPlane);
	glUniform1f(glGetUniformLocation(m_particleProgram, "zfar"), m_farPlane);

	// Load reflection map

	glGenTextures(1, &m_reflectionTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionTexture);

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


	glUseProgram(0);
}


void FluidParticleSystem::setPositions(std::vector<glm::vec3> positions){
	m_positions = positions;

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_positionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_positions.size(),
		&m_positions[0], GL_DYNAMIC_DRAW);
}

void FluidParticleSystem::update()
{
	// TODO: Keep in sync with fluid motion
}


void FluidParticleSystem::draw(GLuint targetFBO, const glm::mat4 view, const glm::mat4 proj)
{
	dataPass(view, proj);
	blurPass();

	glBindFramebuffer(GL_FRAMEBUFFER, targetFBO);
	renderPass(view, proj);
}


void FluidParticleSystem::dataPass(const glm::mat4 view, const glm::mat4 proj)
{
	static GLenum bufferTargets [3][2] = {
		{ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 },
		{ 0, GL_COLOR_ATTACHMENT1 },
		{ GL_COLOR_ATTACHMENT0, 0 }
	};

	glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);
	glDrawBuffers(2, bufferTargets[0]);

	glClearColor(0, 0, 0, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glDisablei(GL_BLEND, 0);
	glEnablei(GL_BLEND, 1);
	glBlendFunci(1, GL_ONE, GL_ONE);

	// Thickness (addative)
	glDisable(GL_DEPTH_TEST);
	glDrawBuffers(2, bufferTargets[1]);
	drawParticles(m_dataProgram, view, proj);

	// Particle depth
	glEnable(GL_DEPTH_TEST);
	glDrawBuffers(2, bufferTargets[2]);
	drawParticles(m_dataProgram, view, proj);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void FluidParticleSystem::blurPass()
{
	glm::vec2 blurSize = glm::vec2(
		SAMPLING_SCALAR * m_screenWidth,
		SAMPLING_SCALAR * m_screenHeight);

	// Active source textures
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(m_samplingProgram, "source"), 0);
	glUniform1i(glGetUniformLocation(m_blurProgram, "source"), 0);

	// Downsample
	glViewport(0, 0, blurSize.x, blurSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_samplingProgram);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glUniform2f(glGetUniformLocation(m_samplingProgram, "screenSize"),
		blurSize.x, blurSize.y);
	drawQuad();

	// Blur
	glUseProgram(m_blurProgram);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	GLint orientLoc = glGetUniformLocation(m_blurProgram, "vertical");
	for (unsigned int i = 0; i < 16; i++) {
		glUniform1f(orientLoc, 0); drawQuad();
		glUniform1f(orientLoc, 1); drawQuad();
	}

	// Upsample
	glViewport(0, 0, m_screenWidth, m_screenHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, m_dataFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(m_samplingProgram);
	glBindTexture(GL_TEXTURE_2D, m_blurTexture);
	glUniform2f(glGetUniformLocation(m_samplingProgram, "screenSize"),
		m_screenWidth, m_screenHeight);
	drawQuad();

	glUseProgram(0);
}

void FluidParticleSystem::renderPass(const glm::mat4 view, const glm::mat4 proj)
{
	glUseProgram(m_particleProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);
	glUniform1i(glGetUniformLocation(m_particleProgram, "depthTexture"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionTexture);
	glUniform1i(glGetUniformLocation(m_particleProgram, "reflectionTexture"), 1);

	drawParticles(m_particleProgram, view, proj);

#if defined(_DEBUG)
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_dataFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // NOTE: Screen

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, m_screenWidth, m_screenHeight,
		0, 0, m_screenWidth / 4, m_screenHeight / 4, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBlitFramebuffer(0, 0, m_screenWidth, m_screenHeight,
		m_screenWidth / 4, 0, m_screenWidth / 2, m_screenHeight / 4, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
#endif
}


void FluidParticleSystem::drawParticles(GLuint program,
	const glm::mat4 view, const glm::mat4 proj)
{
	glUseProgram(program);

	// Send the VP to the vertex shader
	glUniformMatrix4fv(glGetUniformLocation(program, "view"),
		1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"),
		1, GL_FALSE, glm::value_ptr(proj));

	glBindVertexArray(m_VAO);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
		m_positions.size());

	glUseProgram(0);
}


void FluidParticleSystem::drawQuad()
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