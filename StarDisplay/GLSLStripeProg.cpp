#include <vector>
#include "glmfwd.hpp"
#include <glmutils/homogeneous.hpp>
#include <glsl/shader_pool.hpp>
#include "bounding_box.hpp"
#include "Flock.hpp"
#include "Params.hpp"
#include "GLSLStripeProg.hpp"
#include "GLSLState.hpp"
#include "Globals.hpp"


GLSLDiskProg::GLSLDiskProg() 
: worldRadius_(-1.0f), renderRadius_(-1.0f)
{
  glsl::program* pprog = GGl.use_program("DiskRadii");
  pprog->uniform("stripeLen")->set1f(5.0f);
  pprog = GGl.use_program("DiskCircles");
  pprog->uniform("stripeLen")->set1f(5.0f);

  vao_.bind();
  buffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
}


GLSLDiskProg::~GLSLDiskProg()
{
}


void GLSLDiskProg::Flush()
{
  worldRadius_ = PROOST.Radius;
  if (worldRadius_ != renderRadius_)
  {
    renderRadius_ = -1.0f;
    buffer_.bind(GL_ARRAY_BUFFER);
    buffer_.data(sizeof(float) * (size_t(worldRadius_ / dr) + 3), 0, GL_STATIC_DRAW);
    float* p = (float*)buffer_.map_write();
    if (p)
    {
      renderRadius_ = worldRadius_;
      *p++ = renderRadius_;
      float r = dr;
      while (r < renderRadius_) {
        *p++ = r;
        r += dr;
      }
      *p++ = renderRadius_;
      *p = renderRadius_;      // guard
    }
    buffer_.unmap();
  }
}


void GLSLDiskProg::Render()
{
  if (renderRadius_ > 0.0f)
  {
    buffer_.bind(GL_ARRAY_BUFFER);
    vao_.bind();
    GGl.use_program("DiskRadii");
    glDrawArrays(GL_POINTS, 0, 1);
    GGl.use_program("DiskCircles");
    glDrawArrays(GL_POINTS, 1, 1 + GLsizei(renderRadius_ / dr));
  }
}


GLSLGridProg::GLSLGridProg()
{
  glsl::program* pprog = GGl.use_program("Grid");
  pprog->uniform("stripeLen")->set1f(PARAMS.RulerTick);
  vao_.bind();
  buffer_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
}


GLSLGridProg::~GLSLGridProg()
{
}


void GLSLGridProg::Flush()
{
  using glmutils::transformPoint;

  vertices_.swap(v_);
  bounding_box bb;
  for (unsigned i=0; i<GFLOCK.num_clusters(); ++i)
  {
    if (GFLOCK.cluster(i)->size > static_cast<size_t>(PARAMS.RulerMinFlockSize)) 
    {
      bb.reset();
      GFLOCK.query(bb, GFLOCK.cluster(i)->bbox);
      if (glmutils::any_inf(bb.pca_I123())) break;
      glm::mat4 M(bb.pca_H());
      glmutils::bbox3 fabb(glm::vec3(-0.5*bb.pca_I123()), glm::vec3(0.5*bb.pca_I123()));
      fabb.p0().x = fabb.p1().x = 0.0f;
      glm::vec3 p0 = fabb.p0();
      glm::vec3 p1 = glm::vec3(0, fabb.p0().y, fabb.p1().z);
      glm::vec3 p2 = glm::vec3(0, fabb.p1().y, fabb.p1().z);
      glm::vec3 p3 = glm::vec3(0, fabb.p1().y, fabb.p0().z);
      const float tick = PARAMS.RulerTick;
      glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
      while (p0.z < p2.z)
      {
        vertices_.push_back( transformPoint(M, p0) );
        vertices_.push_back( transformPoint(M, p3) );
        p0.z += tick;
        p3.z += tick;
      }
      p0 = fabb.p0();
      while (p0.y < p2.y)
      {
        vertices_.push_back( transformPoint(M, p0) );
        vertices_.push_back( transformPoint(M, p1) );
        p0.y += tick;
        p1.y += tick;
      }
    }
  }
  buffer_.bind(GL_ARRAY_BUFFER);
  buffer_.data(sizeof(glm::vec3) * v_.size(), 0, GL_STREAM_DRAW);
  buffer_.sub_data(0, sizeof(glm::vec3) * v_.size(), (void*)&v_[0]);
}


void GLSLGridProg::Render()
{
  if (!v_.empty())
  {
    buffer_.bind(GL_ARRAY_BUFFER);
    GGl.use_program("Grid");
    vao_.bind();
    glDrawArrays(GL_LINES, 0, (GLsizei)v_.size());
    v_.clear();
  }
}


