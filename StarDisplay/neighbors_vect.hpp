#ifndef STARDISPLAY_NEIGHBOR_VEC_HPP_INCLUDED
#define STARDISPLAY_NEIGHBOR_VEC_HPP_INCLUDED

#include <malloc.h>
#include <hrtree/config.hpp>
#include <hrtree/sorting/insertion_sort.hpp>
#include <glmutils/avx/vec.hpp>
#include "glmfwd.hpp"


template<typename T>
inline bool test_distance(T const& neighbor, glmutils::avx::vec3 position, float rr, glmutils::avx::vec3& dir, float& distance)
{
  const glmutils::avx::vec3 ofs = glmutils::avx::vec3(neighbor.position()) - position;
  const float distance2 = glmutils::avx::length2(ofs);
  if (distance2 >= 0.0000001f && distance2 <= rr)
  {
    dir = glmutils::avx::fast_normalize_length(ofs, distance);
    return true;
  }
  return false;
}


//! What a bird knows about another
HRTREE_ALIGN(16) struct neighborInfo
{
  neighborInfo() {}

  template <typename T>
  neighborInfo(T const& neighbor, float Distance, glmutils::avx::vec3 dir, float CosAngle, float Azimuth)
  : position(neighbor.position()),
    forward(neighbor.forward()),
    side(neighbor.side()),
    direction(glmutils::avx::cast(dir)),
    distance(std::max(Distance, 0.0000001f)),
    cosAngle(CosAngle),
    azimuth(Azimuth),
    speed(neighbor.speed()),
    predatorReaction(neighbor.predatorReaction_),
    panicOnset(neighbor.panicOnset_),
    panicCopy(neighbor.panicCopy_),
    id(neighbor.id()),
    interacting(false)
  {
  }

  HRTREE_ALIGN(16) glm::vec3 position;
  HRTREE_ALIGN(16) glm::vec3 forward;
  HRTREE_ALIGN(16) glm::vec3 side;    
  HRTREE_ALIGN(16) glm::vec3 direction;            // in global space
  float     distance;
  float     cosAngle;                              // against forward
  float     azimuth;                               // in head system
  float     speed;     
  int       predatorReaction;
  double    panicOnset;
  int       panicCopy;
  int       id;
  bool      interacting;
  char padding_[8];
};


class neighbors_vect
{
  neighbors_vect(const neighbors_vect&);
  neighbors_vect& operator=(const neighbors_vect&);

public:
  typedef neighborInfo*       iterator;
  typedef neighborInfo const* const_iterator;
  typedef size_t              size_type;
  typedef neighborInfo        value_type;

  explicit neighbors_vect(size_t capacity)
  : neighbors_((neighborInfo*)_aligned_malloc(capacity*sizeof(neighborInfo), 64)), 
    size_(0),
    capacity_(capacity)
  {}
  ~neighbors_vect()
  {
    _aligned_free((void*)neighbors_);
  }

  bool empty() const { return size_ == 0; }
  size_t size() const { return size_; }
  size_t capacity() const { return capacity_; }
  iterator begin() { return neighbors_; }
  iterator end() { return neighbors_ + size_; }
  const_iterator begin() const { return neighbors_; }
  const_iterator end() const { return neighbors_ + size_; }
  
  neighborInfo& operator[](size_t i) { return neighbors_[i]; }
  const neighborInfo& operator[](size_t i) const { return neighbors_[i]; }

  void clear() { size_ = 0; }
  void insertion_sort() 
  { 
    hrtree::insertion_sort(begin(), end(), [] (const neighborInfo& a, const neighborInfo& b) { return a.distance < b.distance; });
  }

  template <typename T>
  void insert(const T& neighbor, float Distance, glmutils::avx::vec3 dir, float CosAngle, float Azimut)  
  {
    iterator it = try_insert(Distance);
    if (it) *it = neighborInfo(neighbor, Distance, dir, CosAngle, Azimut);
  }

  iterator try_insert(float Distance)  
  {
    iterator last = neighbors_ + size_;
    if (size_ < capacity_)
    {
      ++size_;
      return last;
    }
    iterator it = std::max_element(neighbors_, last, 
      [](neighborInfo const& a, neighborInfo const& b) { return a.distance < b.distance; }
    );
    return ((it == last) || (it->distance < Distance)) 
      ? nullptr 
      : it;
  }

private:
  neighborInfo* neighbors_;
  size_t size_;
  const size_t capacity_;
};


#endif
