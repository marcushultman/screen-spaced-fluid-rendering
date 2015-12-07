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

#include "textfile.h"

using glm::mat4;
using glm::vec3;
using glm::vec4;

class FluidParticle
{
private:
	//vec3 position;

	const char* m_particleVertexShaderFile		= "resource/shaders/particle.vert";
	const char* m_particleFragmentShaderFile	= "resource/shaders/particle.frag";
	const char* m_dataFragmentShaderFile		= "resource/shaders/particledata.frag";

	const char* m_quadVertexShaderFile		= "resource/shaders/quad.vert";
	const char* m_quadFragmentShaderFile	= "resource/shaders/quad.frag";
	const char* m_blurFragmentShaderFile	= "resource/shaders/blur.frag";

	GLuint vertexArrayObject;
	GLuint transformBuffer; // m_buffers[VB_POSITIONS]
	
	GLuint m_dataFBO, m_blurFBO;
	float m_width, m_height;

	GLuint m_dataTexture, m_colorTexture; // Geomery pass
	GLuint m_blurTexture; // Blur pass

	GLuint
		m_particleProgram,
		m_dataProgram,
		m_samplingProgram,
		m_blurProgram;


	std::vector<glm::vec3> m_positions;

public:

	FluidParticle(float size, int width, int height);
	~FluidParticle();

	void setupVAO(float size);

	void setupFBO();
	void setupBlurFBO();

	void setupShaders(float size);

	void SetPositions(std::vector<glm::vec3> positions){
		m_positions = positions;

		glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_positions.size(),
			&m_positions[0], GL_DYNAMIC_DRAW);
	}

	void DrawData(mat4 view, mat4 projection);
	void Draw(mat4 view, mat4 projection, int renderDepth);

	void blur();

private:
	void DrawShader(GLuint program, const glm::mat4 view, const glm::mat4 projection);

	void drawQuad();
};

