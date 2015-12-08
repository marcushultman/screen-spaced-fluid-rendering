#include "FirstPersonCamera.h"

#define CAM_MOVE_SPEED glm::vec3(0.3f, 0.3f, 0.3f)
#define CAM_ROTATION_SPEED glm::vec2(0.003f, -0.003f)

static const glm::vec3 X_AXIS(1, 0, 0);
static const glm::vec3 Y_AXIS(0, 1, 0);
static const glm::vec3 Z_AXIS(0, 0, 1);


FirstPersonCamera::FirstPersonCamera() :
FirstPersonCamera(glm::vec3(0), glm::vec2(0), 1)
{ }
FirstPersonCamera::FirstPersonCamera(glm::vec3 position, glm::vec2 rotation, float speed)
{
	m_position = position;
	m_rotation = rotation;
	m_speed = speed;
}

FirstPersonCamera::~FirstPersonCamera()
{
}

void FirstPersonCamera::move(float dx, float dy, float dz)
{
	move(glm::vec3(dx, dy, dz));
}
void FirstPersonCamera::move(glm::vec3 offset)
{
	m_position += CAM_MOVE_SPEED * offset;
}
void FirstPersonCamera::rotate(float dx, float dy)
{
	rotate(glm::vec2(dx, dy));
}
void FirstPersonCamera::rotate(glm::vec2 offset)
{
	m_rotation += CAM_ROTATION_SPEED * offset;
}

void FirstPersonCamera::setSpeed(float speed)
{
	m_speed = speed;
}

void FirstPersonCamera::setVelocityX(int mode)
{
	m_velocity.x = (mode == 0 ? 0 : 
		(mode < 0 ? -m_speed : m_speed));
}
void FirstPersonCamera::setVelocityY(int mode)
{
	m_velocity.y = (mode == 0 ? 0 :
		(mode < 0 ? -m_speed : m_speed));
}
void FirstPersonCamera::setVelocityZ(int mode)
{
	m_velocity.z = (mode == 0 ? 0 :
		(mode < 0 ? -m_speed : m_speed));
}
void FirstPersonCamera::update(float elapsedTime)
{
	move(elapsedTime * m_velocity *
		glm::angleAxis(m_rotation.y, X_AXIS) *
		glm::angleAxis(m_rotation.x, Y_AXIS));
}

glm::vec3 FirstPersonCamera::getPosition()
{
	return m_position;
}

glm::mat4 FirstPersonCamera::getView()
{
	return glm::lookAt(m_position, m_position - Z_AXIS *
		glm::angleAxis(m_rotation.y, X_AXIS) *
		glm::angleAxis(m_rotation.x, Y_AXIS), Y_AXIS);
}