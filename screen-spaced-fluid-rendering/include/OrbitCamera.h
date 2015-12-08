/*
* Copyright © 2015, Marcus Hultman
*/
#pragma once

#include "Camera.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

class OrbitCamera : Camera
{
public:
	enum Mode{
		NONE,
		ARC,
		PAN
	};

	OrbitCamera();
	OrbitCamera(glm::vec3 center);
	OrbitCamera(glm::vec3 center, glm::vec2 rotation);
	OrbitCamera(glm::vec3 center, glm::vec2 rotation, float zoom);
	~OrbitCamera();

	void pan(float dx, float dy);
	void pan(glm::vec2);
	void rotate(float dx, float dy);
	void rotate(glm::vec2);
	void zoom(float dz);

	glm::vec3	getCenter();
	void		setCenter(glm::vec3);
	glm::vec3	getPosition();
	void		setPosition(glm::vec3);
	float		getZoom();
	void		setZoom(float);

	glm::mat4	getView();

	OrbitCamera::Mode	getMode();
	void			setMode(OrbitCamera::Mode);
private:
	glm::vec3		m_center;
	glm::vec2		m_rotation;
	float			m_zoom;
	OrbitCamera::Mode	m_mode;
};

