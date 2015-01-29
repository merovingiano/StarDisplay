#include <iostream>
#include <stbi/stb_image.h>
#include "GLSLiotexture.hpp"
#include "Params.hpp"


namespace {

  void TexImageRGBA(GLenum target, GLint level, GLsizei width, GLsizei height, const GLvoid* pixels)
  {
    if (target == GL_TEXTURE_2D) {
      glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA8,
        width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    } else if (target == GL_TEXTURE_1D) {
      glTexImage1D(GL_TEXTURE_1D, level, GL_RGBA8,
        width, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    }
  }

}


glsl::texture LoadTexture(const std::string& FileName, bool mipmaps, glm::ivec2* pExt)
{
  GLint width, height, channels;
  width = height = channels = 0;
  unsigned char* texData = stbi_load(FileName.c_str(), &width, &height, &channels, STBI_rgb_alpha);
  if (pExt)
  {
    pExt->x = width;
    pExt->y = height;
  }
  if (0 == texData)
  {
    throw std::exception((std::string("Can't read texture ") + FileName).c_str());
  }
  glsl::texture tex(height == 1 ? GL_TEXTURE_1D : GL_TEXTURE_2D);
  tex.bind();
  TexImageRGBA(tex.target(), 0, (GLsizei)width, (GLsizei)height, texData);
  stbi_image_free(texData);
  if (mipmaps) 
  {
    glGenerateMipmap(tex.target());
  }
  return tex;
}


// Filenames expected to be +X,-X,+Y,-Y,+Z,-Z
glsl::texture LoadCubeMapTexture(const std::vector<std::string>& FileNames)
{
  glsl::texture tex(GL_TEXTURE_CUBE_MAP);
  tex.bind();
  for (unsigned i=0; i<6; ++i)
  {
    GLint width, height, channels;
    unsigned char* texData = 0;
    texData = stbi_load(FileNames[i].c_str(), &width, &height, &channels, STBI_rgb);
    if (0 == texData)
    {
      throw std::exception((std::string("Can't read cubemap face '") + FileNames[i]).c_str());
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texData);
    stbi_image_free(texData);
  }
  return tex;
}
