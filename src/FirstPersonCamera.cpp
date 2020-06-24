#include "FirstPersonCamera.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace {

static const glm::vec3 kCamMoveSpeed(0.3f, 0.3f, 0.3f);
static const glm::vec2 kCamRotationSpeed(0.003f, -0.003f);
static const glm::vec3 kXAxis(1, 0, 0);
static const glm::vec3 kYAxis(0, 1, 0);
static const glm::vec3 kZAxis(0, 0, 1);

}  // namespace

FirstPersonCamera::FirstPersonCamera() : FirstPersonCamera(glm::vec3(0), glm::vec2(0), 1) {}
FirstPersonCamera::FirstPersonCamera(glm::vec3 position, glm::vec2 rotation, float speed)
    : _position{position}, _rotation{rotation}, _speed{speed} {}

FirstPersonCamera::~FirstPersonCamera() = default;

void FirstPersonCamera::move(float dx, float dy, float dz) {
  move(glm::vec3(dx, dy, dz));
}

void FirstPersonCamera::move(glm::vec3 offset) {
  _position += kCamMoveSpeed * offset;
}

void FirstPersonCamera::rotate(float dx, float dy) {
  rotate(glm::vec2(dx, dy));
}

void FirstPersonCamera::rotate(glm::vec2 offset) {
  _rotation += kCamRotationSpeed * offset;
}

void FirstPersonCamera::setSpeed(float speed) {
  _speed = speed;
}

void FirstPersonCamera::setVelocityX(int mode) {
  _velocity.x = (mode == 0 ? 0 : (mode < 0 ? -_speed : _speed));
}

void FirstPersonCamera::setVelocityY(int mode) {
  _velocity.y = (mode == 0 ? 0 : (mode < 0 ? -_speed : _speed));
}

void FirstPersonCamera::setVelocityZ(int mode) {
  _velocity.z = (mode == 0 ? 0 : (mode < 0 ? -_speed : _speed));
}

void FirstPersonCamera::update(float elapsed_time) {
  move(elapsed_time * _velocity * glm::angleAxis(_rotation.y, kXAxis) *
       glm::angleAxis(_rotation.x, kYAxis));
}

glm::vec3 FirstPersonCamera::getPosition() const {
  return _position;
}

glm::mat4 FirstPersonCamera::getView() const {
  auto center = _position -
                kZAxis * glm::angleAxis(_rotation.y, kXAxis) * glm::angleAxis(_rotation.x, kYAxis);
  return glm::lookAt(_position, center, kYAxis);
}
