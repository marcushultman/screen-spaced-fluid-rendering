
#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	virtual void update() { }

	virtual glm::vec3 getPosition() = 0;
	virtual glm::mat4 getView() = 0;
};