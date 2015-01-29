#ifndef GLSLSTATE_HPP_INCLUDED
#define GLSLSTATE_HPP_INCLUDED

#include <memory>
#include <array>
#include "glmfwd.hpp"
#include "color32.hpp"
#include "Params.hpp"


namespace glsl { 
  class texture;
  class block_uniform;
  class uniform;
  class uniform_block;
  class shader_pool;
  class program;
  class buffer;
}


class GLSLState
{
  GLSLState(const GLSLState&);
  const GLSLState& operator=(const GLSLState&);

public:
	GLSLState();
	~GLSLState();
	void Init(void* hDC);
  void Resize() const;
  void LoadModels();
  
  void Flush();
  void Render();

  glm::vec4 textColor() const;
  float alphaMaskCenter() const { return alphaMaskCenter_; }
  float alphaMaskWidth() const { return alphaMaskWidth_; }
  float& alphaMaskCenter() { return alphaMaskCenter_; }
  float& alphaMaskWidth() { return alphaMaskWidth_; }
  unsigned currentPreyModel() const { return currentPreyModel_; }
  unsigned& currentPreyModel() { return currentPreyModel_; }
  unsigned currentPredatorModel() const { return currentPredatorModel_; }
  unsigned& currentPredatorModel() { return currentPredatorModel_; }
  void setAnnotation(const char* str, double duration = 1.0);

  // Interface to shader pool
  glsl::program* use_program(const char* prog);

private:
  void LoadSkybox();
  void UploadMatrices();
  void UseSimViewport() const;
  void UseFullViewport() const;
  void PrintInfoText();

  std::unique_ptr<glsl::shader_pool>        ShaderPool;
  std::unique_ptr<class GLSLSkybox>         Skybox;
  std::unique_ptr<class glsl::texture>      SpectrumTex;
  std::unique_ptr<class IInstancingProg>    InstancingPrey;
  std::unique_ptr<class IInstancingProg>    InstancingPred;
  std::unique_ptr<class GLSLRibbonProg>     RibbonProg;
  std::unique_ptr<class GLSLDiskProg>       DiskProg;
  std::unique_ptr<class GLSLGridProg>       GridProg;
  std::unique_ptr<class GLSLLocals>         Locals;
  std::unique_ptr<class glsl::buffer>       buBuf;

public:
  std::unique_ptr<class HistOverlay>        Overlay;
  std::unique_ptr<class GLSLImm>            imm3D;
  std::unique_ptr<class GLSLImm>            imm2D;
  std::unique_ptr<class IText>              Fonts;

private:
  float alphaMaskCenter_;
  float alphaMaskWidth_;
  unsigned currentPreyModel_;
  unsigned currentPredatorModel_;
  std::string helpMsg_;
  std::string annotation_;
  double annotationElapsed_;
  Param::RenderFlags flags_;
	void* hDC_;
  void* hGLRC_;
};


#endif
