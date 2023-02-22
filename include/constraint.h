#pragma once

#include <set>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

class Constraint {
  using ConstraintPtr = std::unique_ptr<Constraint>;
  struct ByIndex {
    bool operator()(const ConstraintPtr &lhs, const ConstraintPtr &rhs) {
      return lhs->_indices < rhs->_indices;
    }
  };

 public:
  using Set = std::set<ConstraintPtr, ByIndex>;

  virtual ~Constraint() {}

  virtual void operator()(int iterations,
                          std::vector<glm::vec3> &positions,
                          std::vector<glm::vec3> &rest_position) const = 0;

 protected:
  static constexpr auto kStiffness = 1.0f;

  std::vector<size_t> _indices;
};

class DistanceConstraint : public Constraint {
 public:
  DistanceConstraint(size_t i1, size_t i2) {
    _indices = {i1, i2};
    std::sort(_indices.begin(), _indices.end());
  }
  void operator()(int iterations,
                  std::vector<glm::vec3> &positions,
                  std::vector<glm::vec3> &rest_positions) const {
    auto i1 = _indices[0];
    auto i2 = _indices[1];

    auto diff = positions[i2] - positions[i1];
    auto diff_dist = glm::length(diff);
    auto diff_norm = diff / diff_dist;
    auto rest_diff = rest_positions[i2] - rest_positions[i1];

    auto delta = diff_norm * (diff_dist - glm::length(rest_diff)) / 2.0f;

    auto k = 1 - pow(1 - kStiffness, 1 / static_cast<float>(iterations));
    delta *= k;

    positions[i1] += delta;
    positions[i2] -= delta;
  }
};

class BendingConstraint : public Constraint {
 public:
  BendingConstraint(size_t i1, size_t i2, size_t i3, size_t i4) {
    _indices = {i1, i2, i3, i4};
    std::sort(_indices.begin(), _indices.begin() + 2);
    std::sort(_indices.begin() + 2, _indices.end());
  }
  void operator()(int iterations,
                  std::vector<glm::vec3> &positions,
                  std::vector<glm::vec3> &rest_positions) const {
    auto i1 = _indices[0];
    auto i2 = _indices[1];
    auto i3 = _indices[2];
    auto i4 = _indices[3];

    auto p2 = positions[i2] - positions[i1];
    auto p3 = positions[i3] - positions[i1];
    auto p4 = positions[i4] - positions[i1];

    auto cp2p3 = glm::cross(p2, p3);
    auto cp2p4 = glm::cross(p2, p4);

    auto cp2p3_len = glm::length(cp2p3);
    auto cp2p4_len = glm::length(cp2p4);

    auto n1 = glm::normalize(cp2p3);
    auto n2 = glm::normalize(cp2p4);
    auto d = std::max(0.0f, std::min(glm::dot(n1, n2), 1.0f));

    auto q3 = (glm::cross(p2, n2) + glm::cross(n1, p2) * d) / cp2p3_len;
    auto q4 = (glm::cross(p2, n1) + glm::cross(n2, p2) * d) / cp2p4_len;
    auto q2 = -(glm::cross(p3, n2) + glm::cross(n1, p3) * d) / cp2p3_len -
              (glm::cross(p4, n1) + glm::cross(n2, p4) * d) / cp2p4_len;
    auto q1 = -q2 - q3 - q4;

    auto s2 = rest_positions[i2] - rest_positions[i1];
    auto s3 = rest_positions[i3] - rest_positions[i1];
    auto s4 = rest_positions[i4] - rest_positions[i1];

    auto sn1 = glm::normalize(glm::cross(s2, s3));
    auto sn2 = glm::normalize(glm::cross(s2, s4));
    auto rd = std::clamp(glm::dot(sn1, sn2), 0.0f, 1.0f);

    auto sum = glm::length2(q1) + glm::length2(q2) + glm::length2(q3) + glm::length2(q4);
    auto coe = sum != 0 ? (-glm::sqrt(1 - d * d) * (acos(d) - acos(rd)) / sum) : 0;

    auto k = 1 - pow(1 - kStiffness, 1 / static_cast<float>(iterations));
    coe *= k;

    positions[i1] += coe * q1;
    positions[i2] += coe * q2;
    positions[i3] += coe * q3;
    positions[i4] += coe * q4;
  }
};

class CollisionConstraint : public Constraint {
 public:
  CollisionConstraint(size_t i, glm::vec3 target) {
    _indices = {i};
    _target = target;
  }

  void operator()(int iterations,
                  std::vector<glm::vec3> &positions,
                  std::vector<glm::vec3> &rest_positions) const {
    positions[_indices[0]] = _target;
  }

 private:
  glm::vec3 _target;
};
