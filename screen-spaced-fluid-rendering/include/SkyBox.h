#pragma once

#include <string>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <IL/il.h>

#include "textfile.h"

#include "Box.h"

class SkyBox
{
private:
	Box m_box;
	GLuint m_texture;
	GLuint m_shader;
public:
	SkyBox();
	~SkyBox();
	void draw(const glm::mat4 view, const glm::mat4 proj);
private:
	void load();
};

