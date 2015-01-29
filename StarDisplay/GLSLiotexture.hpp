#ifndef IOTEXTURE_HPP_INCLUDED
#define IOTEXTURE_HPP_INCLUDED

#include <string>
#include <vector>
#include <glsl/texture.hpp>
#include "glmfwd.hpp"


glsl::texture LoadTexture(const std::string& FileName, bool mipmaps, glm::ivec2* pExt = 0);

// Filenames expected to be +X,-X,+Y,-Y,+Z,-Z
glsl::texture LoadCubeMapTexture(const std::vector<std::string>& FileNames);


#endif
