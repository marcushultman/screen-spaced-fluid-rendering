
#pragma once

#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

class FirstPersonCamera : Camera
{
private:
	glm::vec3	m_position, m_velocity;
	glm::vec2	m_rotation;
	float		m_speed;
public:
	FirstPersonCamera();
	FirstPersonCamera(glm::vec3 position, glm::vec2 rotation, float speed);
	~FirstPersonCamera();

	void move(float dx, float dy, float dz);
	void move(glm::vec3);
	void rotate(float dx, float dy);
	void rotate(glm::vec2);

	void setSpeed(float speed);

	void setVelocityX(int mode);
	void setVelocityY(int mode);
	void setVelocityZ(int mode);

	void update(float elapsedTime);

	glm::vec3	getPosition();
	glm::mat4	getView();
};

