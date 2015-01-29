#ifndef GLMDEF_HPP_INCLUDED
#define GLMDEF_HPP_INCLUDED

#include <cmath>
#include <limits>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glmutils/bbox.hpp>
#include <glmutils/save_normalize.hpp>
#include <glmutils/fpclassify.hpp>


const float qNan_val = std::numeric_limits<float>::quiet_NaN();    //!< Return value 'mapping impossible'


inline float BlindCosine(float blindAngleDeg)
{
  return ::cos(glm::radians(180.0f - 0.5f * blindAngleDeg));
}


inline float FOVCosine(float FOVDeg)
{
  return ::cos(glm::radians(0.5f * FOVDeg));
}


template <typename T>
inline T smoothhump(const glm::detail::tvec4<T>& edges, T x)
{
  return glm::smoothstep(edges.x, edges.y, x) * (T(1) - glm::smoothstep(edges.z, edges.w, x));
}


template <typename T>
inline bool approx_eq(T const& a, T const& b)
{
  return std::abs(a - b) < T(0.000001);
}


template <typename T>
inline bool approx_ge(T const& a, T const& b)
{
  return (a > b) || (std::abs(a - b) < T(0.000001));
}


#endif  // GLMDEF_HPP_INCLUDED
