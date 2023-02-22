#pragma once

#include <GL/glew.h>
#include "constraint.h"

class FluidPositions {
 public:
  FluidPositions();

  void update(double elapsed_time);

  size_t size() const { return _positions.size(); }
  const glm::vec3 *data() const { return _positions.data(); }

 private:
  std::vector<glm::vec3> _rest_positions;
  std::vector<glm::vec3> _positions;
  std::vector<glm::vec3> _velocities;
  std::vector<glm::vec3> _predicted_positions;

  Constraint::Set _constraints;
  Constraint::Set _collision_constraints;
};
