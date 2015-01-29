#ifndef COLOR32_HPP_INCLUDED
#define COLOR32_HPP_INCLUDED

#include <stdint.h>
#include "glmfwd.hpp"


class color32
{
public:
  color32() {}
  explicit color32(const glm::vec4& cf) : cub(cast(cf.x, cf.y, cf.z, cf.w)) {}
  explicit color32(const glm::vec3& cf) : cub(cast(cf.x, cf.y, cf.z, 1.0f)) {}
  color32(const glm::vec3& cf, float alpha) : cub(cast(cf.x, cf.y, cf.z, alpha)) {}
  color32(float r, float g, float b) : cub(cast(r, g, b, 1.0f)) {}
  color32(float r, float g, float b, float a) : cub(cast(r, g, b, a)) {}

private:
  static uint32_t cast(float r, float g, float b, float a)
  {
    return uint32_t(255.f * r) | (uint32_t(255.f * g) << 8) | (uint32_t(255.f * b) << 16) | (uint32_t(255.f * a) << 24);
  }
  uint32_t cub;
};


#endif
