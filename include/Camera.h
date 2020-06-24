#pragma once

#include <glm/glm.hpp>

class Camera {
 public:
  virtual void update(float elapsed_time) {}

  virtual glm::vec3 getPosition() const = 0;
  virtual glm::mat4 getView() const = 0;
};
