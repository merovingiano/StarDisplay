//! \file initial.hpp Initial positions and orientations
//! \ingroup Model

#ifndef INITIAL_HPP_INCLUDED
#define INITIAL_HPP_INCLUDED

#include "vector.hpp"
#include "glmfwd.hpp"
#include "libParam.hpp"


class CInitializer 
{
  enum InitPdf 
  {
    PDF_CONST,
    PDF_GEOMETRIC
  };

public:
  CInitializer(const libParam::Initial& param);
  vec3 position();
  vec3 forward();
  unsigned& numPrey() { return param_.numPrey; }

private:
  vec3 do_const_position();
  vec3 do_geometric_position();

  libParam::Initial param_;
  InitPdf pdf_;
  size_t bucket_;
  vector<vec3> centers_;
  vector<vec3> forwards_;
};


#endif
