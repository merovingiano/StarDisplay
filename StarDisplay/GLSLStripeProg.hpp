#ifndef GLSLSTRIPEPROG_HPP_INCLUDED
#define GLSLSTRIPEPROG_HPP_INCLUDED

#include <vector>
#include <glsl/vertexarray.hpp>
#include "mapped_buffer.hpp"


class GLSLDiskProg
{
  static const int dr = 10;

public:
  GLSLDiskProg();
  ~GLSLDiskProg();

  void Flush();
  void Render();

private:
  glsl::buffer      buffer_;
  glsl::vertexarray vao_;
  float             worldRadius_;
  float             renderRadius_;
};


class GLSLGridProg
{
public:
  GLSLGridProg();
  ~GLSLGridProg();

  void Flush();
  void Render();

private:
  std::vector<glm::vec3>  vertices_, v_;
  glsl::buffer            buffer_;
  glsl::vertexarray       vao_;
};


#endif
