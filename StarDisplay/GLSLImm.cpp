#include <glsl/shader_pool.hpp>
#include <glmutils/homogeneous.hpp>
#include "GLSLState.hpp"
#include "GLSLImm.hpp"
#include "Globals.hpp"


const GLenum GLSLImm::IMM_GL_MAP[IMM_MAX_PRIMITIVE] = 
{
  GL_POINTS,
  GL_LINES,
  GL_LINE_STRIP,
  GL_TRIANGLES,
  GL_TRIANGLE_STRIP,
  GL_TRIANGLES,
  GL_TRIANGLE_STRIP
};


GLSLImm::GLSLImm(const char* ProgName) 
: primitive(IMM_POINT), start(0), ProgName_(ProgName)
{
  glsl::program* pprog = GGl.use_program(ProgName_.c_str());
  pprog->uniform("ColorTex")->set1i(1);
  pprog->uniform_block("Matrices")->binding(0);
  vao_.bind();
  buffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(T1_V3_C4), (void*)(char*)(offsetof(T1_V3_C4, v3)));
  glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T1_V3_C4), (void*)(char*)(offsetof(T1_V3_C4, c4)));
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(T1_V3_C4),  (void*)(char*)(offsetof(T1_V3_C4, t1)));
}


void GLSLImm::Emit(const glm::vec3& vert, const color32& color, float tex)
{
  switch (primitive)
  {
  case IMM_LINE_STRIP : 
  case IMM_TRIANGLE_STRIP :
  case IMM_FILLED_TRIANGLE_STRIP :
    if (start == attribs[primitive].size()) 
    {
      attribs[primitive].emplace_back( vert, color, discardTexCoord );
    }
    break;
  }
  attribs[primitive].emplace_back( vert, color, tex ); 
}


void GLSLImm::Begin(IMMprimitive Primitive) 
{ 
  primitive = Primitive;
  start = attribs[primitive].size();
}


void GLSLImm::End()
{
  switch (primitive)
  {
  case IMM_LINE_STRIP : 
  case IMM_TRIANGLE_STRIP :
  case IMM_FILLED_TRIANGLE_STRIP :
    // Add degenerated vertex
    if (start != attribs[primitive].size()) 
    {
      attribs[primitive].push_back(attribs[primitive].back());
      attribs[primitive].back().t1 = discardTexCoord;
    }
    break;
  }
}


void GLSLImm::Flush()
{
  tot_primitives_ = 0;
  for (int i=IMM_POINT; i<IMM_MAX_PRIMITIVE; ++i)
  {
    tot_primitives_ += attribs[i].size();
  }
  if (tot_primitives_)
  {
    buffer_.bind(GL_ARRAY_BUFFER);
    buffer_.data(sizeof(T1_V3_C4) * tot_primitives_, 0, GL_STREAM_DRAW);
    size_t tpc = 0;
    for (int i=IMM_POINT; i<IMM_MAX_PRIMITIVE ; ++i)
    {
      primitives_[i] = attribs[i].size();
      if (primitives_[i])
      {
        buffer_.sub_data(sizeof(T1_V3_C4) * tpc, sizeof(T1_V3_C4) * primitives_[i], &*attribs[i].begin());
        attribs[i].clear(); 
      }
      tpc += primitives_[i];
    }  
  }
}


void GLSLImm::Render()
{
  if (tot_primitives_)
  {
    GGl.use_program(ProgName_.c_str());
    vao_.bind();
    buffer_.bind(GL_ARRAY_BUFFER);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    size_t tpc = 0;
    for (int i=IMM_POINT; i<IMM_MAX_PRIMITIVE ; ++i)
    {
      if (i == IMM_FILLED_TRIANGLES) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      if (primitives_[i])
      {
        glDrawArrays(IMM_GL_MAP[i], (GLint)tpc, (GLsizei)primitives_[i]);
      }
      tpc += primitives_[i];
    }
  }
}


void GLSLImm::Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex)
{
  Begin(IMM_LINE_STRIP);
  Emit(glm::vec3(p0.x, p0.y, p0.z), color, ctex);
  Emit(glm::vec3(p1.x, p0.y, p0.z), color, ctex);
  Emit(glm::vec3(p1.x, p1.y, p0.z), color, ctex);
  Emit(glm::vec3(p0.x, p1.y, p0.z), color, ctex);
  Emit(glm::vec3(p0.x, p0.y, p0.z), color, ctex);
  Emit(glm::vec3(p0.x, p0.y, p1.z), color, ctex);
  Emit(glm::vec3(p1.x, p0.y, p1.z), color, ctex);
  Emit(glm::vec3(p1.x, p1.y, p1.z), color, ctex);
  Emit(glm::vec3(p0.x, p1.y, p1.z), color, ctex);
  Emit(glm::vec3(p0.x, p0.y, p1.z), color, ctex);
  End();
  Begin(IMM_LINES);
  Emit(glm::vec3(p1.x, p0.y, p0.z), color, ctex);
  Emit(glm::vec3(p1.x, p0.y, p1.z), color, ctex);
  Emit(glm::vec3(p0.x, p1.y, p0.z), color, ctex);
  Emit(glm::vec3(p0.x, p1.y, p1.z), color, ctex);
  Emit(glm::vec3(p1.x, p1.y, p0.z), color, ctex);
  Emit(glm::vec3(p1.x, p1.y, p1.z), color, ctex);
  End();
}


void GLSLImm::Box(const glm::vec3& p0, const glm::vec3& p1, const color32& color)
{
  Box(p0, p1, color, ignoreTexCoord);
}


void GLSLImm::Box(const glm::vec3& p0, const glm::vec3& p1, float ctex)
{
  Box(p0, p1, defaultColor, ctex);
}


void GLSLImm::Box(const glmutils::bbox3& box, const color32& color)
{ 
  Box(box.p0(), box.p1(), color, ignoreTexCoord); 
}


void GLSLImm::Box(const glmutils::bbox3& box, float ctex)
{ 
  Box(box.p0(), box.p1(), defaultColor, ctex); 
}


void GLSLImm::Box(const glm::mat4 M, const glm::vec3& p0, const glm::vec3& p1, const color32& color)
{
  Box(M, p0, p1, color, ignoreTexCoord);
}


void GLSLImm::Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, float ctex)
{
  Box(M, p0, p1, defaultColor, ctex);
}


void GLSLImm::Box(const glm::mat4& M, const glmutils::bbox3& box, const color32& color)
{ 
  Box(M, box.p0(), box.p1(), color, ignoreTexCoord); 
}


void GLSLImm::Box(const glm::mat4& M, const glmutils::bbox3& box, float ctex)
{ 
  Box(M, box.p0(), box.p1(), defaultColor, ctex); 
}


void GLSLImm::Box(const glm::mat4& M, const glm::vec3& p0, const glm::vec3& p1, const color32& color, float ctex)
{
  using glmutils::transformPoint;

  Begin(IMM_LINE_STRIP);
  Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p0.y, p1.z)), color, ctex);
  End();
  Begin(IMM_LINES);
  Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p0.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p0.x, p1.y, p1.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p0.z)), color, ctex);
  Emit(transformPoint(M, glm::vec3(p1.x, p1.y, p1.z)), color, ctex);
  End();
}


