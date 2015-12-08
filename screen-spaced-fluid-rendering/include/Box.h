#pragma once

#include <GL/glew.h>

class Box
{
private:
	GLuint m_VAO;
public:
	Box();
	Box(float size);
	~Box();
	void draw();
};

