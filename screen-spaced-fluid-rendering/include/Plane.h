#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <string>

#include <GL/glew.h>

#include <glm\matrix.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <IL/il.h>

#include "textfile.h"


class Plane
{
private:
	GLuint m_VAO;
	GLuint m_texture;
	GLuint m_shader;
public:
	Plane(float size);
	Plane(float width, float height);
	~Plane();
	void draw(const glm::mat4 view, const glm::mat4 proj);
private:
	void load(float width, float height);
};

