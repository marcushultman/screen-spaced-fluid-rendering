#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>

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

	const char* particleDataVertexShaderFile = "shaders/particledata.vert";
	const char* particleDataFragmentShaderFile = "shaders/particledata.frag";
	
	const char* vertexShaderFile = "shaders/particle.vert";
	const char* fragmentShaderFile = "shaders/particle.frag";
	GLuint 
		vertexArrayObject, 
		shaderProgram;

	GLuint
		transformBuffer;
public:

	GLuint
		particleDataShaderProgram;

	FluidParticle(float size);
	~FluidParticle();

	vec3 position;

	vec3 GetPosition(){
		return FluidParticle::position;
	}
	void SetPosition(vec3 position){
		FluidParticle::position = position;
	}

	void DrawParticleData(mat4 view, mat4 projection);
	void Draw(mat4 view, mat4 projection);

	void DrawParticleDataInstanced(mat4 view, mat4 projection, unsigned int numelem, const mat4* transforms);
	void DrawInstanced(mat4 view, mat4 projection, unsigned int numelem, const mat4* transforms);
};

