#include "OrbitCamera.h"

#include <algorithm>

namespace {

static const auto kCamInitialCenter = glm::vec3(0.0f, 0.0f, 0.0f);
static const auto kCamInitialRotation = glm::vec2(0.0f, 0.0f);
static const auto kCamInitialZoom = 250.0f;

static const auto kCamZoomSpeed = 10.0f;
static const auto kCamZoomMin = 1.0f;
static const auto kCamZoomMax = 1000.0f;

static const auto kCamPanSpeed = glm::vec2(-0.3f, 0.3f);
static const auto kCamArcSpeed = glm::vec2(0.003f, -0.003f);

static const auto kCamMoveSpeed = glm::vec3(0.3f, 0.3f, 0.3f);
static const auto kCamRotationSpeed = glm::vec2(0.003f, -0.003f);
static const auto kXAxis = glm::vec3(1, 0, 0);
static const auto kYAxis = glm::vec3(0, 1, 0);
static const auto kZAxis = glm::vec3(0, 0, 1);

}  // namespace

OrbitCamera::OrbitCamera() : OrbitCamera(kCamInitialCenter, kCamInitialRotation, kCamInitialZoom) {}
OrbitCamera::OrbitCamera(glm::vec3 center)
    : OrbitCamera(center, kCamInitialRotation, kCamInitialZoom) {}
OrbitCamera::OrbitCamera(glm::vec3 center, glm::vec2 rotation)
    : OrbitCamera(center, rotation, kCamInitialZoom) {}
OrbitCamera::OrbitCamera(glm::vec3 center, glm::vec2 rotation, float zoom)
    : _center{center}, _rotation{rotation}, _zoom{zoom} {}

OrbitCamera::~OrbitCamera() = default;

void OrbitCamera::pan(float dx, float dy) {
  pan(glm::vec2(dx, dy));
}

void OrbitCamera::pan(glm::vec2 offset) {
  auto dir = _center - getPosition();
  auto loc_x = glm::normalize(glm::cross(dir, kYAxis));
  auto loc_y = glm::normalize(glm::cross(dir, loc_x));
  offset *= kCamPanSpeed;
  _center += offset.x * loc_x + offset.y * loc_y;
}

void OrbitCamera::rotate(float dx, float dy) {
  rotate(glm::vec2(dx, dy));
}

void OrbitCamera::rotate(glm::vec2 offset) {
  _rotation += kCamArcSpeed * offset;
}

void OrbitCamera::zoom(float dz) {
  setZoom(_zoom - kCamZoomSpeed * dz);
}

void OrbitCamera::setCenter(glm::vec3 center) {
  _center = center;
}

void OrbitCamera::setZoom(float zoom) {
  _zoom = std::clamp(zoom, kCamZoomMin, kCamZoomMax);
}

glm::vec3 OrbitCamera::getCenter() const {
  return _center;
}

float OrbitCamera::getZoom() const {
  return _zoom;
}

void OrbitCamera::update(float elapsed_time) {
  if (_mode == OrbitCamera::Mode::NONE) {
    rotate(100 * elapsed_time, 0);
  }
}

glm::vec3 OrbitCamera::getPosition() const {
  return _center + (_zoom * kZAxis) * glm::angleAxis(_rotation.y, kXAxis) *
                       glm::angleAxis(_rotation.x, kYAxis);
}

glm::mat4 OrbitCamera::getView() const {
  return glm::lookAt(getPosition(), _center, kYAxis);
}

OrbitCamera::Mode OrbitCamera::getMode() const {
  return _mode;
}

void OrbitCamera::setMode(OrbitCamera::Mode mode) {
  _mode = mode;
}
