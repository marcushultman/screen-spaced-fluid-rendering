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

	const char* m_vertexShaderFile = "resource/shaders/particle.vert";
	const char* m_fragementShaderFile = "resource/shaders/particle.frag";
	const char* m_fragementDataShaderFile = "resource/shaders/particledata.frag";

	GLuint vertexArrayObject, shaderProgram, particleDataShaderProgram;
	GLuint transformBuffer; // m_buffers[VB_POSITIONS]

	GLuint m_FBO;
	GLuint m_dataTexture; //, m_additionalTexture;

	std::vector<glm::vec3> m_positions;

public:

	FluidParticle(float size);
	~FluidParticle();

	void setupFBO(int width, int height);

	void SetPositions(std::vector<glm::vec3> positions){
		m_positions = positions;

		glBindBuffer(GL_ARRAY_BUFFER, transformBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_positions.size(),
			&m_positions[0], GL_DYNAMIC_DRAW);
	}

	void DrawData(mat4 view, mat4 projection);
	void Draw(mat4 view, mat4 projection);

private:
	void DrawShader(GLuint program, const glm::mat4 view, const glm::mat4 projection);
};

