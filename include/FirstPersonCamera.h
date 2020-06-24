#pragma once

#include <glm/glm.hpp>
#include "Camera.h"

class FirstPersonCamera : public Camera {
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

  void update(float elapsed_time) override;

  glm::vec3 getPosition() const override;
  glm::mat4 getView() const override;

 private:
  glm::vec3 _position;
  glm::vec3 _velocity;
  glm::vec2 _rotation;
  float _speed;
};
