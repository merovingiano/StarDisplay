#ifndef GLSLIMM_HPP_INCLUDED
#define GLSLIMM_HPP_INCLUDED

#include <vector>
#include <glsl/buffer.hpp>
#include <glsl/vertexarray.hpp>
#include "glmfwd.hpp"
#include "color32.hpp"


const color32 defaultColor(1,1,1,1);
const float ignoreTexCoord(100.0f);
const float discardTexCoord(-100.0f);


enum IMMprimitive
{
  IMM_POINT = 0,
  IMM_LINES,
  IMM_LINE_STRIP,
  IMM_TRIANGLES,
  IMM_TRIANGLE_STRIP,
  IMM_FILLED_TRIANGLES,
  IMM_FILLED_TRIANGLE_STRIP,
  IMM_MAX_PRIMITIVE
};


class GLSLImm
{
  static const GLenum IMM_GL_MAP[IMM_MAX_PRIMITIVE];

  struct T1_V3_C4
  {
    T1_V3_C4() {}
    T1_V3_C4(const glm::vec3& vert, const color32& color, float tex)
      : t1(tex), v3(vert), c4(color) {}

    glm::vec3 v3;
    float     t1;
    color32     c4;
  };

public:
  GLSLImm(const char* Progname);
  void Begin(IMMprimitive Primitive);
  void Emit(const glm::vec3& vert, const color32& color, float tex);
  void Emit(const glm::vec3& vert, float tex) { Emit(vert, defaultColor, tex); }
  void Emit(const glm::vec3& vert, const color32& color) { Emit(vert, color, ignoreTexCoord); }
  void Emit(const glm::vec2& vert, const color32& color)  { Emit(glm::vec3(vert, 0.0f), color, ignoreTexCoord); }
  void Emit(const glm::vec2& vert, float tex) { Emit(glm::vec3(vert, 0.0f), defaultColor, tex); }
  void Emit(const glm::vec2& vert) { Emit(glm::vec3(vert, 0.0f), defaultColor, ignoreTexCoord); }
  void Emit(float x, float y, float z, const color32& color, float tex) { Emit(glm::vec3(x, y, z), color, tex); }
  void Emit(float x, float y, float z, const color32& color) { Emit(x, y, z, color, ignoreTexCoord); }
  void Emit(float x, float y, float z, float tex) { Emit(x, y, z, defaultColor, tex); }
  void Emit(float x, float y, const color32& color, float tex) { Emit(x, y, 0.0f, color, tex); }
  void Emit(float x, float y, const color32& color) { Emit(x, y, 0.0f, color, ignoreTexCoord); }
  void Emit(float x, float y, float tex) { Emit(x, y, 0.0f, defaultColor, tex); }
  void End();

  void Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex);
  void Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex);
  void Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color);
  void Box(const glm::vec3& p0, const glm::vec3& p1, float ctex);
  void Box(const glmutils::bbox3& box, const color32& color);
  void Box(const glmutils::bbox3& box, float ctex); 
  void Box(const glm::mat4 M, const glm::vec3& p0, const glm::vec3& p1, const color32& color);
  void Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, float ctex);
  void Box(const glm::mat4& M, const glmutils::bbox3& box, const color32& color); 
  void Box(const glm::mat4& M, const glmutils::bbox3& box, float ctex); 

  void Flush();
  void Render();

private:
  IMMprimitive primitive;
  size_t start;
  std::vector<T1_V3_C4> attribs[IMM_MAX_PRIMITIVE];
  glsl::buffer buffer_;
  glsl::vertexarray vao_;
  std::string ProgName_;
  size_t tot_primitives_;
  size_t primitives_[IMM_MAX_PRIMITIVE];
};


#endif
