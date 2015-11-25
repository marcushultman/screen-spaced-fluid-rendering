#pragma once

#include <string>

#include <GL\glew.h>
#include <IL\il.h>

#include "textfile.h"

#include "Box.h"

#include <glm\gtc\type_ptr.hpp>

using glm::mat4;
using glm::vec3;

class SkyBox
{
private:
	Box* box;
	GLuint texture;
	GLuint shader;
public:
	SkyBox();
	~SkyBox();
	void Load();
	void Draw(mat4 view, mat4 projection);
};

