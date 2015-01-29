#include <glsl/shader_pool.hpp>
#include "GLSLRibbonProg.hpp"
#include "GLSLState.hpp"
#include "trail_buffer.hpp"
#include "Params.hpp"
#include "glmfwd.hpp"
#include "Globals.hpp"
#include "ICamera.hpp"
#include "Bird.hpp"


GLSLRibbonProg::GLSLRibbonProg() 
: primitives_(0)
{
  glsl::program* pprog = GGl.use_program("Ribbon");
  pprog->uniform("colorTex")->set1i(1);
  pprog->uniform("oneOverTickInterval")->set1f(1.0f / PARAMS.TrailTickInterval);
  pprog->uniform("oneMinusTickWidth")->set1f(1.0f - PARAMS.TrailTickWidth);
  pprog->uniform("halfWidth")->set1f(0.5f * PARAMS.TrailWidth);
  pprog->uniform_block("Matrices")->binding(0);

  vao_.bind();
  mbuffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, 0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (char*)(16));
}


GLSLRibbonProg::~GLSLRibbonProg()
{
}


void GLSLRibbonProg::Flush(int ignoreId)
{
  primitives_ = GTRAILS.flatten(mbuffer_.get(), mbuffer_.size(), ignoreId);
  mbuffer_.bind(GL_ARRAY_BUFFER);
  mbuffer_.flush();
  mbuffer_.unmap();
}


void GLSLRibbonProg::Render()
{
  mbuffer_.bind(GL_ARRAY_BUFFER);
  if (mbuffer_.is_flushed() && primitives_)
  {
    vao_.bind();
    GGl.use_program("Ribbon");
    glDisable(GL_CULL_FACE);
    glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, GLsizei(primitives_));
    glEnable(GL_CULL_FACE);
  }
  mbuffer_.map(GTRAILS.recommendedCapacity());
}


