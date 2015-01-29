#ifndef GLSLINSTANCINGPROG_HPP_INCLUDED
#define GLSLINSTANCINGPROG_HPP_INCLUDED

#include "glmfwd.hpp"
#include "Flock.hpp"


class IInstancingProg
{
public:
  virtual void Instance(CFlock::bird_const_iterator first, CFlock::bird_const_iterator last, int ignoreId) const = 0; 
  virtual void Flush() = 0;
  virtual void Render() = 0;
};


IInstancingProg* CreateInstancingProg(unsigned ModelId, unsigned MaxN);


#endif
