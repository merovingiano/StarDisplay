#include <algorithm>
#include <numeric>
#include <glmutils/istream.hpp>
#include "GLSLModel.hpp"
#include "GLSLiotexture.hpp"
#include "Params.hpp"
#include "ac3d.hpp"
#include "Globals.hpp"


GLSLModel* CreateModel(unsigned ModelId)
{
  const Param::ModelDef& def = PARAMS.ModelSet[ModelId];
  if (def.LOD.empty()) return 0;
  return new GLSLModel(def.LOD, def.texMix, def.Scale);
}


GLSLModel::GLSLModel(const std::vector<Param::ModelLod>& lods, 
                     float texMixSpectrum,
                     float modelScale)
: texMix_(texMixSpectrum),
  modelScale_(modelScale)
{
  nLods_ = std::min(GLSLModel::MAX_LOD, static_cast<int>(lods.size()));
  nIndices_.assign(0);
  nVertices_.assign(0);
  pxCoverage_.assign(0.0f);
  ac3d_model model[GLSLModel::MAX_LOD];
  GLuint cumVertices = 0;
  GLuint cumIndices = 0;
  for (int lod = 0; lod < nLods_; ++lod)
  {
    filesystem::path ShaderPath = luabind::object_cast<const char*>(Lua("ShaderPath"));
    model[lod] = ImportAC3D(ShaderPath, lods[lod].acFile);
    pxCoverage_[lod] = lods[lod].pxCoverage;
    nVertices_[lod] = (GLsizei)model[lod].vertices.size();
    nIndices_[lod] = (GLsizei)model[lod].indices.size();
    cumVertices += nVertices_[lod];
    cumIndices += nIndices_[lod];
	for (int i = 0; i < 3; i++){
		loc[i] = model[lod].loc[i];
		//std::cout << "\n model location: " << i << "is: " << loc[i][0] <<  loc[i][1] << loc[i][2];
	}
  }
  texture_ = LoadTexture(model[0].texFile, true);
  texture_.set_wrap_filter(GL_CLAMP_TO_EDGE, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR);
  twoSided_ = model[0].twoSided;
  material_ = model[0].material;
  vao_.bind();
  vbuffer_.bind(GL_ARRAY_BUFFER);
  ibuffer_.bind(GL_ELEMENT_ARRAY_BUFFER);
  vbuffer_.data(sizeof(T2F_N3F_V3F) * cumVertices, 0, GL_STATIC_DRAW);
  ibuffer_.data(sizeof(GLint) * cumIndices, 0, GL_STATIC_DRAW);
  cumVertices = 0;
  cumIndices = 0;
  
  for (int lod = 0; lod < nLods_; ++lod)
  {
    vbuffer_.sub_data(sizeof(T2F_N3F_V3F) * cumVertices, sizeof(T2F_N3F_V3F) * nVertices_[lod], (GLvoid*)(&model[lod].vertices[0]));
    ibuffer_.sub_data(sizeof(GLint) * cumIndices, sizeof(GLint) * nIndices_[lod], (GLvoid*)(&model[lod].indices[0]));
    cumVertices += nVertices_[lod];
    cumIndices += nIndices_[lod];
  }
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(6);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(T2F_N3F_V3F), 0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(T2F_N3F_V3F), (void*)(2*sizeof(float)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(T2F_N3F_V3F), (void*)(5*sizeof(float)));
  glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, sizeof(T2F_N3F_V3F), (void*)(8 * sizeof(float)));
}


GLSLModel::~GLSLModel()
{
}


void GLSLModel::Bind()
{
  if (twoSided_) glDisable(GL_CULL_FACE);    // hints to bad model design
  texture_.bind(0);
  vao_.bind();
}


void GLSLModel::Unbind()
{
  if (twoSided_) glEnable(GL_CULL_FACE);    // back to default
}


