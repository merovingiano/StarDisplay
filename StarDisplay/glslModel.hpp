//! \file GLSLModel.hpp 3D triangulated models.
//! \defgroup Models Triangulated 3D Models for OpenGL  

#ifndef GLSLMODEL_HPP_INCLUDED
#define GLSLMODEL_HPP_INCLUDED

#include <string>
#include <array>
#include <vector>
#include <glsl/glsl.hpp>
#include <glsl/buffer.hpp>
#include <glsl/vertexarray.hpp>
#include "ac3d.hpp"
#include "GLSLiotexture.hpp"
#include "glmfwd.hpp"
#include "Params.hpp"

class GLSLModel;


//! \return 3D Model
//! \ingroup Models Visualization
GLSLModel* CreateModel(unsigned ModelId);


//! Base class for OpenGL models.
//! \ingroup Models Visualization
class GLSLModel
{
public:
  BOOST_STATIC_CONSTANT(int, MAX_LOD = 4);

  //! Constructs 3D model from .ac file
  GLSLModel(const std::vector<Param::ModelLod>& lods, 
            float texMixSpectrum,
            float modelScale);
  ~GLSLModel();

  void Bind();            //! bind OpenGL state 
  void Unbind();          //! unbind OpenGL state
  const ac3d_material& material() const { return material_; }
  float modelScale() const { return modelScale_; }  //! return model scale
  const std::array<unsigned, 4>& nVertices() const { return nVertices_; }
  const std::array<unsigned, 4>& nIndices() const { return nIndices_; }
  const std::array<float, 4>& pxCoverage() const { return pxCoverage_; }
  int nLods() const { return nLods_; }
  float texMixAlt() const { return texMix_; }

private:
  int                     nLods_;
  std::array<unsigned, 4> nVertices_;
  std::array<unsigned, 4> nIndices_;
  std::array<float, 4>    pxCoverage_;
  bool                    twoSided_;
  ac3d_material           material_;
  glsl::texture           texture_;
  glsl::buffer            vbuffer_;
  glsl::buffer            ibuffer_;
  glsl::vertexarray       vao_;
  float                   texMix_;
  float                   modelScale_;
};


#endif
