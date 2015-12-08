#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>

#include <vector>

#include <glm\glm.hpp>
#include <glm\matrix.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_access.hpp>

#include <IL\il.h>

#include "textfile.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;

class FluidParticleSystem
{
private:
	
	GLuint m_VAO;
	GLuint m_positionBuffer;

	std::vector<glm::vec3> m_positions;


	GLuint m_dataFBO, m_blurFBO;
	unsigned int m_screenWidth, m_screenHeight;
	float m_nearPlane, m_farPlane;

	GLuint m_dataTexture, m_thicknessTexture,  // Geomery pass
		m_colorTexture;
	GLuint m_blurTexture; // Blur pass


	const char* m_particleVertexShaderFile = "resource/shaders/particle.vert";
	const char* m_particleFragmentShaderFile = "resource/shaders/particle.frag";
	const char* m_dataFragmentShaderFile = "resource/shaders/particledata.frag";

	const char* m_quadVertexShaderFile = "resource/shaders/quad.vert";
	const char* m_quadFragmentShaderFile = "resource/shaders/quad.frag";
	const char* m_blurFragmentShaderFile = "resource/shaders/blur.frag";

	GLuint
		m_particleProgram,
		m_dataProgram,
		m_samplingProgram,
		m_blurProgram,
		m_postProcessProgram;

	GLuint m_reflectionTexture;

public:

	FluidParticleSystem(float particleSize,
		unsigned int screenWidth, unsigned int screenHeight,
		float nearPlane, float farPlane);
	~FluidParticleSystem();

	void setPositions(std::vector<glm::vec3> positions);

	void update();

	void preProcessPass(const glm::mat4 view, const glm::mat4 projection);
	void postProcessPass(GLuint backgroundTexture,
		const glm::mat4 view, const glm::mat4 proj);

private:
	void setupVAO(float size);
	void setupDataFBO();
	void setupBlurFBO();
	void setupShaders(float size);

	void dataPass(const glm::mat4 view, const glm::mat4 proj);
	void blurPass();
	void renderPass(const glm::mat4 view, const glm::mat4 proj);

	void drawParticles(GLuint program,
		const glm::mat4 view, const glm::mat4 proj);
	void drawQuad();
};

