#include <string>
#include <exception>
#include "filesystem.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glsl/buffer.hpp>
#include <glsl/texture.hpp>
#include <glsl/vertexarray.hpp>
#include <glsl/shader_pool.hpp>
#include "GLSLSkybox.hpp"
#include "GLSLState.hpp"
#include "GLSLiotexture.hpp"
#include "Params.hpp"
#include "Globals.hpp"
#include "ICamera.hpp"


using namespace glsl;


namespace {

  static const GLfloat vertices[] = 
  {
     1,  1,  1,   1, -1, -1,   1, -1,  1,
     1, -1, -1,   1,  1,  1,   1,  1, -1,
    -1,  1, -1,  -1, -1,  1,  -1, -1, -1,
    -1, -1,  1,  -1,  1, -1,  -1,  1,  1,
     1, -1, -1,  -1,  1, -1,  -1, -1, -1,
    -1,  1, -1,   1, -1, -1,   1,  1, -1,
     1,  1,  1,  -1, -1,  1,  -1,  1,  1,
    -1, -1,  1,   1,  1,  1,   1, -1,  1,
     1,  1,  1,  -1,  1, -1,   1,  1, -1,
    -1,  1, -1,   1,  1,  1,  -1,  1,  1,
    -1, -1,  1,   1, -1, -1,  -1, -1, -1,
     1, -1, -1,  -1, -1,  1,   1, -1,  1,
  };

}


GLSLSkybox::GLSLSkybox()
{
  if (PSKYBOX.name.empty())
  {
    CubeMap_ = glsl::texture(GL_TEXTURE_CUBE_MAP);
    CubeMap_.bind();
    const unsigned char pix[4] = { 255, 255, 255 };
    for (unsigned i=0; i<6; ++i)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pix);
    }
  }
  else
  {
    filesystem::path MediaPath = luabind::object_cast<const char*>(Lua("MediaPath"));
    std::vector< std::string > FileNames;
    for (int i=0; i<6; ++i)
    {
      // 0, 90, 180, 270, up, down
      char buf[32] = {0};
      _snprintf_s(buf, 31, "%d.png", i);
      FileNames.push_back((MediaPath / PSKYBOX.name / buf).string());
    }
    CubeMap_ = LoadCubeMapTexture(FileNames);
  }
  CubeMap_.set_wrap_filter(GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
  GGl.use_program("SkyBox")->uniform("CubeMap")->set1i(3);

  vao_.bind();
  vertices_.bind(GL_ARRAY_BUFFER);
  vertices_.data(sizeof(vertices), (void*)vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  CubeMap_.bind(3);
}


GLSLSkybox::~GLSLSkybox()
{
}


void GLSLSkybox::Flush()
{
  viewPort_ = GCAMERA.GetViewport();
  colorCorr_ = glm::vec4(PRENDERFLAGS.altBackground ? PSKYBOX.ColorCorrAlt : PSKYBOX.ColorCorr, 1.0f);
  MV_ = GCAMERA.ModelViewMatrix();
  fovy_ = PSKYBOX.fovy;
}


void GLSLSkybox::Render()
{
  double aspect = static_cast<double>(viewPort_[2]) / viewPort_[3];
  glsl::program* pprog = GGl.use_program("SkyBox");
  pprog->uniform("colorFact")->set4fv(1, glm::value_ptr(colorCorr_));
  MV_[3] = glm::dvec4(0,0,0,1);    // move to origin
  glm::mat4 MVP( glm::perspective(fovy_, aspect, 0.1, 2.0) * MV_ );
  pprog->uniform("ModelViewProjectionMatrix")->set4x4fv(1, glm::value_ptr(MVP));
  glDepthFunc(GL_ALWAYS);
  vao_.bind();
  vertices_.bind(GL_ARRAY_BUFFER);
  glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices));
  glDepthFunc(GL_LEQUAL);
}


