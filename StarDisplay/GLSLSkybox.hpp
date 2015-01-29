#ifndef SLSKYBOX_HPP_INCLUDED
#define SLSKYBOX_HPP_INCLUDED

#include "glmfwd.hpp"
#include <glsl/texture.hpp>
#include <glsl/buffer.hpp>
#include <glsl/vertexarray.hpp>


class GLSLSkybox
{
public:
  GLSLSkybox();
  virtual ~GLSLSkybox();
  void Flush();
  void Render();

private:
  glsl::vertexarray vao_;
  glsl::buffer      vertices_;
  glsl::texture     CubeMap_;
  glm::ivec4        viewPort_;
  glm::vec4         colorCorr_;
  double            fovy_;
  glm::dmat4        MV_;
};


#endif  // GLSLSKYBOX_HPP_INCLUDED
