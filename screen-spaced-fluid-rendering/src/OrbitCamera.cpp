#include "OrbitCamera.h"

#define CAM_INITIAL_CENTER glm::vec3(0.0f, 0.0f, 0.0f)
#define CAM_INITIAL_ROTATION glm::vec2(0.0f, 0.0f)
#define CAM_INITIAL_ZOOM 250.0f

#define CAM_ZOOM_SPEED 10.0f
#define CAM_ZOOM_MIN 1.0f
#define CAM_ZOOM_MAX 1000.0f

#define CAM_PAN_SPEED glm::vec2(-0.3f, 0.3f)
#define CAM_ARC_SPEED glm::vec2(0.003f, -0.003f)

static const glm::vec3 X_AXIS(1, 0, 0);
static const glm::vec3 Y_AXIS(0, 1, 0);
static const glm::vec3 Z_AXIS(0, 0, 1);


OrbitCamera::OrbitCamera() : 
	OrbitCamera(CAM_INITIAL_CENTER, CAM_INITIAL_ROTATION, CAM_INITIAL_ZOOM)
{ }
OrbitCamera::OrbitCamera(glm::vec3 center) :
OrbitCamera(center, CAM_INITIAL_ROTATION, CAM_INITIAL_ZOOM)
{ }
OrbitCamera::OrbitCamera(glm::vec3 center, glm::vec2 rotation) :
OrbitCamera(center, rotation, CAM_INITIAL_ZOOM)
{ }
OrbitCamera::OrbitCamera(glm::vec3 center, glm::vec2 rotation, float zoom)
{
	m_center = center;
	m_rotation = rotation;
	m_zoom = zoom;
	m_mode = NONE;
}

OrbitCamera::~OrbitCamera()
{
}

void OrbitCamera::pan(float dx, float dy)
{
	pan(glm::vec2(dx, dy));
}
void OrbitCamera::pan(glm::vec2 offset)
{
	glm::vec3 dir = m_center - getPosition();
	glm::vec3 loc_x = glm::normalize(glm::cross(dir, Y_AXIS));
	glm::vec3 loc_y = glm::normalize(glm::cross(dir, loc_x));
	offset *= CAM_PAN_SPEED;
	m_center += offset.x * loc_x + offset.y * loc_y;
}
void OrbitCamera::rotate(float dx, float dy)
{
	rotate(glm::vec2(dx, dy));
}
void OrbitCamera::rotate(glm::vec2 offset)
{
	m_rotation += CAM_ARC_SPEED * offset;
}
void OrbitCamera::zoom(float dz)
{
	setZoom(m_zoom - CAM_ZOOM_SPEED * dz);
}
glm::vec3 OrbitCamera::getCenter()
{
	return m_center;
}
void OrbitCamera::setCenter(glm::vec3 center)
{
	m_center = center;
}
glm::vec3 OrbitCamera::getPosition()
{
	return m_center + (m_zoom * Z_AXIS) *
		glm::angleAxis(m_rotation.y, X_AXIS) *
		glm::angleAxis(m_rotation.x, Y_AXIS);
}
float OrbitCamera::getZoom()
{
	return m_zoom;
}
void OrbitCamera::setZoom(float zoom)
{
	m_zoom = std::max(CAM_ZOOM_MIN,
		std::min(zoom, CAM_ZOOM_MAX));
}

glm::mat4 OrbitCamera::getView()
{
	return glm::lookAt(getPosition(), m_center, Y_AXIS);
}

OrbitCamera::Mode OrbitCamera::getMode()
{
	return m_mode;
}
void OrbitCamera::setMode(OrbitCamera::Mode mode)
{
	m_mode = mode;
}