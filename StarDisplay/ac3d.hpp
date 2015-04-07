//! \file ac3d.hpp AC3D file importer
//! \ingroup Graphics

#ifndef AC3D_HPP_INCLUDED
#define AC3D_HPP_INCLUDED

#include <string>
#include <vector>
#include "filesystem.hpp"
#include <glsl/glsl.hpp>
#include "glmfwd.hpp"


struct T2F_N3F_V3F {
  glm::vec2 t;    
  glm::vec3 n;    
  glm::vec3 v;    
  //! Another element to this struct
  float part;
};


struct ac3d_material
{
  glm::vec3 rgb;
  glm::vec3 amb;
  glm::vec3 emis;
  glm::vec3 spec;
  float shi;
  float trans;
};

//!  added a loc file for indiviual parts of the bird
struct ac3d_model
{
  std::vector<T2F_N3F_V3F> vertices; 
  std::vector<GLuint> indices;
  std::string texFile;
  bool twoSided;
  ac3d_material material;
  glmutils::bbox3 bbox;
  glm::vec3 loc[3];
};


//! \brief Import ac3d file. May throw.
ac3d_model ImportAC3D(const filesystem::path& ShaderPath, const std::string& ac3dFile);


#endif
