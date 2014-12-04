#pragma once

#include "GL\glew.h"

class FrameBuffer
{
private:
	GLuint frameBufferObject;
	GLuint textures[];
public:
	FrameBuffer();
	~FrameBuffer();
};

