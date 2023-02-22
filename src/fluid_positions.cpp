#include "fluid_positions.h"

#include <cmath>
#include <vector>
#include <glm/glm.hpp>

namespace {

const auto kParticleSep = 5.0f;

}  // namespace

FluidPositions::FluidPositions() {
  int num = 10;
  _positions.clear();
  for (int x = -num; x < num; x++) {
    for (int y = 0; y < num / 2; y++) {
      for (int z = -num; z < num; z++) {
        auto position = kParticleSep * glm::vec3(x, y + 0.75f, z);
        _rest_positions.push_back(position);
        _positions.push_back(position);
        _velocities.push_back(glm::vec3(0));
        if (x < num - 1 && y < 4 && z < num - 1) {
          size_t i[2];
          i[0] = x * 5 * num + y * num + z;
          i[1] = (x + 1) * 5 * num + y * num + z;
          _constraints.insert(std::make_unique<DistanceConstraint>(i[0], i[1]));
          i[0] = x * 5 * num + y * num + z;
          i[1] = x * 5 * num + (y + 1) * num + z;
          _constraints.insert(std::make_unique<DistanceConstraint>(i[0], i[1]));
          i[0] = x * 5 * num + y * num + z;
          i[1] = x * 5 * num + y * num + (z + 1);
          _constraints.insert(std::make_unique<DistanceConstraint>(i[0], i[1]));
        }
      }
    }
  }

  _predicted_positions.resize(_positions.size());
}

void FluidPositions::update(double elapsed_time) {
  return;

  // Symplectic Euler
  for (auto i = 0; i < _positions.size(); ++i) {
    auto ft = static_cast<float>(elapsed_time);
    // _velocities[i] += ft * glm::vec3(0, -9.8f, 0);
    _predicted_positions[i] = _positions[i] + ft * _velocities[i];
  }

  _collision_constraints.clear();
  for (auto i = 0; i < _predicted_positions.size(); ++i) {
    auto p = _predicted_positions[i];
    p.x = std::clamp(p.x, kParticleSep * -10, kParticleSep * 10);
    p.y = std::clamp(p.y, kParticleSep * 0.75f, kParticleSep * (0.75f + 5));
    p.x = std::clamp(p.x, kParticleSep * -10, kParticleSep * 10);
    if (p != _predicted_positions[i]) {
      _collision_constraints.insert(std::make_unique<CollisionConstraint>(i, p));
    }
  }

  // Drag
  static float t = 0;
  t += (float)elapsed_time;
  // _collision_constraints.insert(std::make_unique<CollisionConstraint>(
  //     42, glm::vec3(0, 15, 0) + (glm::angleAxis(t, glm::vec3(0, 0, 1)) * glm::vec3(0, 5, 0))));
  for (auto i = 500; i < 750; ++i) {
    _collision_constraints.insert(
        std::make_unique<CollisionConstraint>(i, glm::vec3(0, 0, 40 + 10 * std::sin(t))));
  }

  auto iterations = 4;
  for (auto i = 0; i < iterations; ++i) {
    // projectConstraints(C1,...,CM+Mcoll, p1,...,pN, s1,...,sN)
    for (auto &c : _constraints) {
      (*c)(iterations, _predicted_positions, _rest_positions);
    }
    for (auto &c : _collision_constraints) {
      (*c)(iterations, _predicted_positions, _rest_positions);
    }
  }

  // Calculate velocities from updated positions
  if (elapsed_time == 0) {
    return;
  }

  for (auto i = 0; i < _positions.size(); ++i) {
    _velocities[i] = (_predicted_positions[i] - _positions[i]) / static_cast<float>(elapsed_time);
    _positions[i] = _predicted_positions[i];
  }
}
