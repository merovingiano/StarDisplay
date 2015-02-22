#include <stddef.h>       // offsetof
#include <array>
#include <atomic>
#include <iostream>
#include <glsl/shader_pool.hpp>
#include <glsl/uniform_block.hpp>
#include <glmutils/avx/vec.hpp>
#include <glmutils/homogeneous.hpp>
#include "mapped_buffer.hpp"
#include "GLSLInstancingProg.hpp"
#include "GLSLState.hpp"
#include "GLSLModel.hpp"
#include "Params.hpp"
#include "Bird.hpp"
#include "Flock.hpp"
#include "Globals.hpp"
#include "ICamera.hpp"

# define M_PI           3.14159265358979323846
namespace avx = glmutils::avx;


namespace {

  struct LOD_single {};
  struct LOD_33 {};
  struct LOD_base_instance {};

}


template <typename LOD>
class GLSLInstancingProg : public IInstancingProg
{
  struct attrib_t
  {
    avx::vec4 c[4];
  };

public:
  GLSLInstancingProg(unsigned ModelId, unsigned MaxN);
  virtual ~GLSLInstancingProg();

  void Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const; 
  void Flush();
  void Render();

private:
  void do_instance(const CBird& bird, size_t ofs, int lod, int ignoreId) const;
  void do_render();

  mutable std::array<std::atomic<size_t>, 4>  lodCount_;
  mapped_buffer               mAttrib_;
  std::unique_ptr<GLSLModel>  model_;
  const size_t                MaxN_;
};


template <typename LOD>
GLSLInstancingProg<LOD>::GLSLInstancingProg(unsigned ModelId, unsigned MaxN)
  : lodCount_(), model_(CreateModel(ModelId)), MaxN_(MaxN)
{
  glsl::program* pprog = GGl.use_program("Instancing");
  pprog->uniform("Texture")->set1i(0);
  pprog->uniform("ColorTex")->set1i(1);
  pprog->uniform("ambient")->set1f(model_->material().amb.x);
  pprog->uniform("diffuse")->set1f(1.0f - model_->material().amb.x);
  pprog->uniform_block("Matrices")->binding(0);

  // Model vao still bound
  mAttrib_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(3);
  glEnableVertexAttribArray(4);
  glEnableVertexAttribArray(5);
  // 7 is the force attribute, a vec4, 1 = force, 2-4 will be other stuff
  glEnableVertexAttribArray(7);
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), 0);
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(4*sizeof(float)));
  glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(8*sizeof(float)));
  glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(12 * sizeof(float)));
  glVertexAttribDivisor(3, 1);
  glVertexAttribDivisor(4, 1);
  glVertexAttribDivisor(5, 1);
  glVertexAttribDivisor(7, 1);
}


template <typename LOD>
GLSLInstancingProg<LOD>::~GLSLInstancingProg() 
{
}


template <typename LOD>
void GLSLInstancingProg<LOD>::do_instance(const CBird& bird, size_t ofs, int lod, int ignoreId) const
{
  if (ofs < (lod + 1) * MaxN_)
  {
    attrib_t* dst = ((attrib_t*)mAttrib_.get() + lod * MaxN_ + ofs);
    const float scale = (bird.id() == ignoreId) ? 0 : bird.wingSpan() * model_->modelScale();
    avx::vec4 c0(bird.forward(), scale);
    avx::vec4 c1(bird.up(), 0);
    avx::vec4 c2(bird.position(), bird.getCurrentColorTex());
	avx::vec4 c3(bird.force(),Sim.SimulationTime());
    dst->c[0] = c0;
    dst->c[1] = c1;
    dst->c[2] = c2;
	dst->c[3] = c3;
  }
}


template <typename LOD>
void GLSLInstancingProg<LOD>::Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const
{
  if (!mAttrib_.is_mapped()) return;
  
  const int N = (int)std::distance(first, last);
  const glm::mat4 MV(GCAMERA.ModelViewMatrix());
  const float PointScale = 0.5f * model_->modelScale() * static_cast<float>((GCAMERA.GetViewport()[3] / std::tan(GCAMERA.Fovy() * 0.5 * glm::pi<double>() / 180.0))); 
# pragma omp parallel for schedule(static) firstprivate(first)  
  for (int i = 0; i < N; ++i)
  {
    CFlock::bird_const_iterator src(first + i);
    float eyeDist = length(glmutils::transformPoint(MV, src->position()));
    float pointSize = PointScale * src->wingSpan() / eyeDist;
    const std::array<float, 4>& pxCoverage = model_->pxCoverage();
    int lod = 3;
    if (pointSize >= pxCoverage[0]) lod = 0;
    else if (pointSize >= pxCoverage[1]) lod = 1;
    else if (pointSize >= pxCoverage[2]) lod = 2;
    const size_t ofs = lodCount_[lod].fetch_add(1);
    do_instance(*src, ofs, lod, ignoreId);
  }
}


template <>
void GLSLInstancingProg<LOD_single>::Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const
{
  if (!mAttrib_.is_mapped()) return;

  const int N = (int)std::distance(first, last);
# pragma omp parallel for schedule(static) firstprivate(first)
  for (int i = 0; i < N; ++i)
  {
    do_instance(*(first + i), i, 0, ignoreId);
  }
  lodCount_[0].store(size_t(N));
}


template <typename LOD>
void GLSLInstancingProg<LOD>::Flush()
{
  mAttrib_.bind(GL_ARRAY_BUFFER);
  for (int i=0; i < model_->nLods(); ++i)
  {
    mAttrib_.flush_range(sizeof(attrib_t) * i * MaxN_, sizeof(attrib_t) * lodCount_[i]);
  }
  mAttrib_.unmap();
}


template <typename LOD>
void GLSLInstancingProg<LOD>::Render()
{
  const bool colored = PFM.ColorMapping();
  model_->Bind();
  glsl::program* pprog = GGl.use_program("Instancing");

  GLuint locLoc = glGetUniformLocation(pprog->get(), "loc");
  float locs[9] = { model_->loc[0][0], model_->loc[0][1], model_->loc[0][2], model_->loc[0][0], model_->loc[0][1], model_->loc[0][2], model_->loc[0][0], model_->loc[0][1], model_->loc[0][2] };
  glUniform3fv(locLoc, 3, locs);
  glsl::uniform* alphaMask = pprog->uniform("alphaMask");
  if (PRENDERFLAGS.alphaMasking) 
  {
    alphaMask->set2f(GGl.alphaMaskCenter() - GGl.alphaMaskWidth(), GGl.alphaMaskCenter() + GGl.alphaMaskWidth());
  }
  else
  {
    alphaMask->set2f(-1000.0f, 1000.0f);
  }
  pprog->uniform("texMix")->set1f(colored ? model_->texMixAlt() : 0);
  int vertexOfs = 0;
  int indexOfs = 0;
  mAttrib_.bind(GL_ARRAY_BUFFER);
  if (mAttrib_.is_flushed()) 
  {
    do_render();
  }
  mAttrib_.map(sizeof(attrib_t) * model_->nLods() * MaxN_);
  model_->Unbind();
  std::for_each(lodCount_.begin(), lodCount_.end(), [] (std::atomic<size_t>& x) { x.store(0); });
}


// Single LOD
template <>
void GLSLInstancingProg<LOD_single>::do_render()
{
  const size_t N = std::min<size_t>(MaxN_, lodCount_[0].load());
  glDrawElementsInstanced(
    GL_TRIANGLES, model_->nIndices()[0], GL_UNSIGNED_INT, (GLvoid*)(0), GLsizei(N)
  );
}


// Multiple LODs using OpenGL 3.3
template <>
void GLSLInstancingProg<LOD_33>::do_render()
{
  int indexOfs = 0;
  int vertexOfs = 0;
  for (int i=0; i < model_->nLods(); ++i)
  {
    const size_t lodCount = std::min<size_t>(MaxN_, lodCount_[i].load());
    if (lodCount > 0)
    {
      glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(sizeof(attrib_t) * i * MaxN_));
      glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(sizeof(attrib_t) * i * MaxN_ + 4*sizeof(float)));
      glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)(sizeof(attrib_t) * i * MaxN_ + 8*sizeof(float)));
      glDrawElementsInstancedBaseVertex(
        GL_TRIANGLES, model_->nIndices()[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLint) * indexOfs), GLsizei(lodCount), vertexOfs
        );
    }
    indexOfs += model_->nIndices()[i];
    vertexOfs += model_->nVertices()[i];
  }
}


// Multiple LODs using OpenGL ARB_base_instance
template <>
void GLSLInstancingProg<LOD_base_instance>::do_render()
{
  int indexOfs = 0;
  int vertexOfs = 0;
  for (int i=0; i < model_->nLods(); ++i)
  {
   const size_t lodCount = std::min<size_t>(MaxN_, lodCount_[i].load());
   if (lodCount > 0)
    {
      glDrawElementsInstancedBaseVertexBaseInstance(
        GL_TRIANGLES, model_->nIndices()[i], GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLint) * indexOfs), GLsizei(lodCount), vertexOfs, GLuint(i * MaxN_)
        );
    }
    indexOfs += model_->nIndices()[i];
    vertexOfs += model_->nVertices()[i];
  }
}


class GLSLPointSpriteProg : public IInstancingProg
{
  struct attrib_t
  {
    glm::vec3 pos;
    float colorTexCoord;
    float pointScale;
  };

public:
  GLSLPointSpriteProg(float modelScale, unsigned MaxN);
  virtual ~GLSLPointSpriteProg();

  void Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const; 
  void Flush();
  void Render();

private:
  mutable size_t    instCount_;
  mapped_buffer     mAttrib_;
  glsl::vertexarray vao_;
  float             wingSpan_;
  bool              colored_;
  mutable float     PointScale_;
  float             PointRadius_;
  const unsigned    MaxN_;
};


GLSLPointSpriteProg::GLSLPointSpriteProg(float modelScale, unsigned MaxN)
  : instCount_(0), wingSpan_(1.0f), PointRadius_(modelScale), MaxN_(MaxN) 
{
  glsl::program* pprog = GGl.use_program("PointSprite");
  pprog->uniform("ColorTex")->set1i(1);
  pprog->uniform_block("Matrices")->binding(0);

  vao_.bind();
  mAttrib_.bind(GL_ARRAY_BUFFER);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(attrib_t), 0);
  glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)offsetof(attrib_t, colorTexCoord));
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(attrib_t), (void*)offsetof(attrib_t, pointScale));
}


GLSLPointSpriteProg::~GLSLPointSpriteProg() 
{
}


void GLSLPointSpriteProg::Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const
{
  PointScale_ = 0.5f * static_cast<float>((GCAMERA.GetViewport()[3] / std::tan(GCAMERA.Fovy() * 0.5 * glm::pi<double>() / 180.0))); 
  if (!mAttrib_.is_mapped()) return;

  const int N = (int)(instCount_ = std::distance(first, last));
  attrib_t* const pAttrib = (attrib_t*)mAttrib_.get();
  const float pScale = PointScale_ * PointRadius_;
# pragma omp parallel for schedule(static) firstprivate(first, pAttrib, pScale)
  for (int i = 0; i < N; ++i)
  {
    CFlock::bird_const_iterator src(first + i);
    attrib_t* dst = (pAttrib + i);
    dst->pos = src->position();
    dst->colorTexCoord = src->getCurrentColorTex();
    dst->pointScale = src->wingSpan() * pScale;
  }
}


void GLSLPointSpriteProg::Flush()
{
  if (mAttrib_.is_mapped())
  {
    mAttrib_.bind(GL_ARRAY_BUFFER);
    mAttrib_.flush_range(0, sizeof(attrib_t) * instCount_);
    mAttrib_.unmap();
    colored_ = PFM.ColorMapping();
  }
}


void GLSLPointSpriteProg::Render()
{
  mAttrib_.bind(GL_ARRAY_BUFFER);
  if (mAttrib_.is_flushed())
  {
    glsl::program* pprog = GGl.use_program("PointSprite");
    if (colored_)
    {
      pprog->uniform("Color")->set4f(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
      pprog->uniform("Color")->set4f(0.5f, 0.5f, 0.6f, 0.0f);
    }
    glsl::uniform* alphaMask = pprog->uniform("alphaMask");
    if (PRENDERFLAGS.alphaMasking) 
    {
      alphaMask->set2f(GGl.alphaMaskCenter() - GGl.alphaMaskWidth(), GGl.alphaMaskCenter() + GGl.alphaMaskWidth());
    }
    else
    {
      alphaMask->set2f(-1000.0f, 1000.0f);
    }
    vao_.bind();
    glDrawArrays(GL_POINTS, 0, GLsizei(instCount_));
  }
  mAttrib_.map(sizeof(attrib_t) * MaxN_);
  instCount_ = 0;
}


IInstancingProg* CreateInstancingProg(unsigned ModelId, unsigned MaxN)
{
  IInstancingProg* ptr = 0;
  if (PARAMS.ModelSet[ModelId].LOD.empty())
  {
    ptr =new GLSLPointSpriteProg(PARAMS.ModelSet[ModelId].Scale, MaxN);
  }
  else
  {
    if (PARAMS.ModelSet[ModelId].LOD.size() > 1)
    {
      if (ogl_ext_ARB_base_instance) 
        ptr = new GLSLInstancingProg<LOD_base_instance>(ModelId, MaxN);
      else
        ptr = new GLSLInstancingProg<LOD_33>(ModelId, MaxN);
    }
    else
    {
      ptr = new GLSLInstancingProg<LOD_single>(ModelId, MaxN);
    }
  }
  return ptr;
}


