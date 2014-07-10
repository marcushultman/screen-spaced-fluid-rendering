#pragma once

//Include GLEW
#include <GL/glew.h>

//Include GLM
#include <glm\glm.hpp>

class Box
{
private:
	GLuint vao;
	float size;
public:
	Box(float);
	~Box();
	void Draw();
};

