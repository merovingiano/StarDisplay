#ifndef GLSLRIBBONPROG_HPP_INCLUDED
#define GLSLRIBBONPROG_HPP_INCLUDED

#include <glsl/vertexarray.hpp>
#include "mapped_buffer.hpp"


class GLSLRibbonProg
{
public:
  GLSLRibbonProg();
  ~GLSLRibbonProg();

  void Flush(int ignoreId);
  void Render();
  
private:
  mapped_buffer     mbuffer_;
  glsl::vertexarray vao_;
  size_t            primitives_;
};


#endif
