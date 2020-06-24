/*
 * Copyright © 2015, Marcus Hultman
 */
#pragma once

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include "Camera.h"

class OrbitCamera : public Camera {
 public:
  enum Mode { NONE, ARC, PAN };

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

  void setCenter(glm::vec3);
  void setPosition(glm::vec3);
  void setZoom(float);

  glm::vec3 getCenter() const;
  float getZoom() const;

  void update(float elapsed_time) override;

  glm::mat4 getView() const override;
  glm::vec3 getPosition() const override;

  OrbitCamera::Mode getMode() const;
  void setMode(OrbitCamera::Mode);

 private:
  glm::vec3 _center;
  glm::vec2 _rotation;
  float _zoom;
  Mode _mode = NONE;
};
