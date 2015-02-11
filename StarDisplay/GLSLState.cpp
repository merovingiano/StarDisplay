#include <iostream>
#include <memory>
#include <exception>
#include <strstream>
#include <set>
#include <future>
#include <glsl/wgl.h>
#include "GLSLState.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/homogeneous.hpp>
#include <glsl/swap_control.hpp>
#include <glsl/wgl_context.hpp>
#include <glsl/shader_pool.hpp>
#include "GLSLStripeProg.hpp"
#include "GLSLInstancingProg.hpp"
#include "GLSLSkybox.hpp"
#include "GLSLRibbonProg.hpp"
#include "GLSLLocals.hpp"
#include "GLSLText.hpp"
#include "GLSLImm.hpp"
#include "ICamera.hpp"
#include "HistOverlay.hpp"
#include "GLSLiotexture.hpp"
#include "glslModel.hpp"
#include "Params.hpp"
#include "Flock.hpp"
#include "Simulation.hpp"
#include "KeyState.hpp"
#include "Globals.hpp"
#include "debug.hpp"


namespace {

  const char header_fmt[] =
    "\\smallface{}\n[F1] Help\n[F2] Birds: %d + %d\n[F3] Boundary radius: %d\n[F4] HRTree level: %d\n\nh.hildenbrandt@rug.nl\n";


  const char footer_fmt[] =
    "\\smallface{}Sim. time: %02.0f:%02.0f:%02.0f\nupdate: %.1f ms\nfps: %d";

  
  const char footer_fmt_overload[] =
    "\\smallface{}Sim. time: %02.0f:%02.0f:%02.0f\nupdate:\\color{1 0 0} %.1f ms\\defcolor{}\nfps: %d";


  std::unique_ptr<IInstancingProg> InitModel(unsigned ModelId, unsigned MaxN)
  {
    std::unique_ptr<IInstancingProg> pprog;
    pprog.reset(CreateInstancingProg(ModelId, MaxN));
    return pprog;
  }


	HGLRC CreateContext(HDC hDC)
	{
    HGLRC hGLRC = NULL;
		int iAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 0,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0, 0,
      0, 0,
		};
    if (PARAMS.DebugLevel > 0)
    {
      iAttribs[6] = WGL_CONTEXT_FLAGS_ARB;
      iAttribs[7] = WGL_CONTEXT_DEBUG_BIT_ARB;
    }
		if ((hGLRC = glsl::CreateContext(hDC, iAttribs, PARAMS.FSAA[0], PARAMS.FSAA[1])) && glsl::init())
		{
      if (PARAMS.FSAA[0] > 0)
      {
			  glEnable(GL_MULTISAMPLE);
      }
      if (ogl_ext_ARB_debug_output && (PARAMS.DebugLevel > 0))
      {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB((PARAMS.DebugLogOnce ? debug::GLDebugLogOnce : debug::GLDebugLog), (void*)(PARAMS.DebugLogStackLevel));
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH_ARB, 0, NULL, GL_TRUE);
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM_ARB, 0, NULL, (PARAMS.DebugLevel > 1));
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW_ARB, 0, NULL, (PARAMS.DebugLevel > 2));
      }
		}
    else
    {
		  throw std::exception("CreateContext failed");
    }
    return hGLRC;
	}

}


GLSLState::GLSLState() 
: annotationElapsed_(0),
  alphaMaskWidth_(0.05f),
  alphaMaskCenter_(0.5f),
  currentPreyModel_(1),
  currentPredatorModel_(1),
  buBuf(new glsl::buffer()),
  hDC_(NULL), hGLRC_(NULL)
{
}


GLSLState::~GLSLState()
{
  if (hDC_) wglMakeCurrent((HDC)hDC_, NULL);
  if (hGLRC_) wglDeleteContext((HGLRC)hGLRC_);
}


void GLSLState::Init(void* hDC)
{
	hGLRC_ = (void*)CreateContext((HDC)hDC);
	hDC_ = wglGetCurrentDC();
	ShaderPool.reset( new glsl::shader_pool() );
  filesystem::path ShaderPath = luabind::object_cast<const char*>(Lua("ShaderPath"));
	std::ifstream fshader( (ShaderPath / "shader.glsl").string().c_str() );
	ShaderPool->parse(fshader);
  auto first = ShaderPool->begin();
  auto last = ShaderPool->end();
  for (; first != last; ++first)
  {
    if (! first->second.link())
    {
      throw std::exception((std::string(first->first) + " " + first->second.log_info()).c_str());
    }
  }
  LoadModels();
	imm3D.reset( new GLSLImm("NoLit") );
	imm2D.reset( new GLSLImm("NoLit2D") );
	Fonts.reset( new GLSLText() );
	Overlay.reset( new HistOverlay() );

	DiskProg.reset( new GLSLDiskProg() );  
	GridProg.reset( new GLSLGridProg() );  
  Locals.reset( new GLSLLocals("NoLit") );
  RibbonProg.reset( new GLSLRibbonProg() );
 
  glsl::program* pprog = use_program("PointSprite");
  pprog->uniform_block("Matrices")->binding(0);
  buBuf->bind(GL_ARRAY_BUFFER);
  buBuf->data(4 * sizeof(glm::mat4), 0, GL_STREAM_DRAW);
  buBuf->unbind();

  filesystem::path MediaPath = luabind::object_cast<const char*>(Lua("MediaPath"));
	glsl::texture tex = LoadTexture((MediaPath / "spectrum.png").string(), false);
	tex.set_wrap_filter(GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	tex.bind(1);
	SpectrumTex.reset( new glsl::texture(tex) );
	LoadSkybox();
	glsl::swap_control(PARAMS.swap_control);

	// Set default OpenGL state. Any shader that needs something else
	// is requested to set the state back to the following values
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  glm::ivec2 ext = Fonts->extent(footer_fmt);
  Lua.CreateTextBox(Fonts.get(), "sim_footer", glm::ivec4(4, -ext.y + 8, ext.x + 4, ext.y + 4), &GCAMERA);
}


void GLSLState::Resize() const
{
  Overlay->Resize();
}


void GLSLState::LoadSkybox()
{
  Skybox.reset( new GLSLSkybox() );
}


void GLSLState::LoadModels()
{
  InstancingPrey.reset(0);
  InstancingPred.reset(0);
  InstancingPrey = InitModel(currentPreyModel_, PARAMS.maxPrey);
  InstancingPred = InitModel(currentPredatorModel_, PARAMS.maxPredators);
}


glm::vec4 GLSLState::textColor() const
{
  return PRENDERFLAGS.altBackground ? glm::vec4(PARAMS.TextColorAlt, 1.0f) : glm::vec4(PARAMS.TextColor, 1.0f);
}


void GLSLState::Flush()
{
  flags_ = PRENDERFLAGS;
  const CBird* focal = GCAMERA.GetFocalBird();
  const int ignoreId = (focal && GCAMERA.HideFocal()) ? focal->id() : -1;
  
  InstancingPrey->Instance(GFLOCK.prey_begin(), GFLOCK.prey_end(), ignoreId);
  InstancingPred->Instance(GFLOCK.predator_begin(), GFLOCK.predator_end(), ignoreId);
  RibbonProg->Flush(ignoreId);
  std::future<void> future = std::async([ignoreId, this] () {
    Locals->Emmit();
  });
  InstancingPrey->Flush();
  InstancingPred->Flush();
  Sim.DisplayStatistic();
  PrintInfoText();

  Skybox->Flush();
  Fonts->Flush();
  imm2D->Flush();
  imm3D->Flush();
  if (flags_.show_world) DiskProg->Flush();  
  if (flags_.show_rulers) GridProg->Flush();
  UploadMatrices();

  future.get(); 
  Locals->Flush();
}


void GLSLState::Render()
{
  buBuf->bind_base_uniform(0);
  UseSimViewport();
  Skybox->Render();
  imm3D->Render();
  Locals->Render();
  if (flags_.wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  RibbonProg->Render();
  InstancingPrey->Render();
  InstancingPred->Render();
  if (flags_.wireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);   // reset
  glDisable(GL_CULL_FACE);
  if (flags_.show_world) DiskProg->Render();  
  if (flags_.show_rulers) GridProg->Render();
  imm2D->Render();
  UseFullViewport();
  Fonts->Render();
  glEnable(GL_CULL_FACE);
  ::SwapBuffers((HDC)hDC_);
}


void GLSLState::PrintInfoText()
{
  const Param::RenderFlags& flags = PRENDERFLAGS;
  char buf[256];

  glm::ivec4 vpc(GCAMERA.GetViewport());
  Fonts->set_color(textColor());
  Fonts->set_orig(glm::ivec2(vpc.x + 4, vpc.y + 4));

  // Help
  if (! flags.helpMsg.empty()) 
  {
    Fonts->print(flags.helpMsg.c_str());
    return;
  }

  // Header
  if (flags.show_header) 
  {
    _snprintf_s(buf, 255, header_fmt, Sim.getNumPrey(), Sim.getNumPredators(), static_cast<int>(PROOST.Radius), flags.rtreeLevel);
    Fonts->print(buf);
  }

  // Annotation
  if (annotationElapsed_ > 0.0) 
  {
    // Place the message below the header.
    if (flags.show_annotation)
      Fonts->print((std::string("\\mediumface{}\n") + annotation_).c_str());
    annotationElapsed_ -= Sim.FrameTime();
  }

  // FPS
  if (flags.show_fps)
  {
    const char* fmt = (Sim.UpdateTime() > PARAMS.IntegrationTimeStep) ? footer_fmt_overload : footer_fmt;
    double hours = std::floor(Sim.SimulationTime() / (60.0*60.0));
    double minutes = std::floor((Sim.SimulationTime() - hours*60.0*60.0) / 60.0);
    double seconds = std::floor(Sim.SimulationTime() - hours*60.0*60.0 - minutes*60.0);
    _snprintf_s(buf, 255, fmt, 
      hours, minutes, seconds,
      1000.0 * Sim.UpdateTime(),
      static_cast<int>(0.5f + 0.5/Sim.FrameTime()));
    Fonts->get_text_box("sim_footer")->SetText(buf);
  }
}


void GLSLState::setAnnotation(const char* str, double duration)
{
  if (str) 
  {
    annotation_ = str;
    annotationElapsed_ = duration;
  }
}


void GLSLState::UploadMatrices() 
{
  buBuf->bind_base_uniform(0);
  glm::vec4 viewport(GCAMERA.WindowViewport());
  glm::mat4* pM = (glm::mat4*)buBuf->map_range_write(0, 4 * sizeof(glm::mat4), GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
  new (pM + 0) glm::mat4(GCAMERA.ModelViewProjectionMatrix());
  new (pM + 1) glm::mat4(GCAMERA.ModelViewMatrix()); 
  new (pM + 2) glm::mat4(GCAMERA.ProjectionMatrix());
	new (pM + 3) glm::mat4(glm::ortho(viewport[0], viewport[2], viewport[3], viewport[1]));
  buBuf->flush_mapped_range(0, 4 * sizeof(glm::mat4));
  buBuf->unmap();
}


void GLSLState::UseSimViewport() const
{
	glm::ivec4 SimViewport(GCAMERA.GetViewport());
	glViewport(SimViewport[0], SimViewport[1], SimViewport[2], SimViewport[3]);
}


void GLSLState::UseFullViewport() const
{
	glm::ivec4 WindowViewport(GCAMERA.WindowViewport());
	glViewport(WindowViewport[0], WindowViewport[1], WindowViewport[2], WindowViewport[3]);
}


glsl::program* GLSLState::use_program(const char* prog)
{
  glsl::program* pprog = (*ShaderPool)(prog);
  pprog->use();
  return pprog;
}


