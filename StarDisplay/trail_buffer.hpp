#ifndef TRAIL_BUFFER_HPP_INCLUDED
#define TRAIL_BUFFER_HPP_INCLUDED


#include <list>
#include <vector>
#include <utility>
#include "glmfwd.hpp"


class trail_buffer_pool
{
public:
  class trail_buffer* create(int id) const;
  void destroy(class trail_buffer* ptb) const;
  
  size_t recommendedCapacity() const;
  size_t flatten(void* buf, size_t capacity, int ignoreId) const;

private:
  typedef std::pair<class trail_buffer const*, size_t> tbs_t;
  mutable std::list<trail_buffer* > pool_;
  mutable std::vector<tbs_t>        tbs_;
};


void appendTrail(class trail_buffer* ptb, const glm::vec3& position, const glm::vec3& side, float colorTex, float dt);


#endif
