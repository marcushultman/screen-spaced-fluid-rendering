#pragma once

#include <stdlib.h>
#include <stdio.h>

//Include GLEW
#include <GL/glew.h>

#include <glm\matrix.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "textfile.h"

using glm::mat4;

class Plane
{
private:
	const char* vertexShaderFile = "shaders/simple.vert";
	const char* fragmentShaderFile = "shaders/simple.frag";
	GLuint vao, shaderProgram;
public:
	Plane(float);
	Plane(float, float);
	~Plane();
	void Draw(mat4 view, mat4 projection);
};

