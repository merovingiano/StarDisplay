//! \file voxel_volume.hpp Voxel volume.
//! \ingroup Analysis

#ifndef VOXEL_VOLUME_HPP_INCLUDED
#define VOXEL_VOLUME_HPP_INCLUDED

#include <vector>
#include "glmfwd.hpp"
#include "Flock.hpp"


namespace detail 
{
  struct voxel_volume_cmp
  {
    bool operator()(const glm::vec3& a, const glm::vec3& b) const
    {
      if (a.x < b.x) return true;
      if (a.x > b.x) return false;
      if (a.y < b.y) return true;
      if (a.y > b.y) return false;
      return (a.z < b.z);
    }
  };

  inline bool vec3_equal(const glm::vec3& a, const glm::vec3& b) 
  {
    return glm::all(glm::equal(a, b));
  }
}


//! \brief Voxel volume calculator.
class voxel_volume
{
  typedef std::vector<glm::vec3> rpos_t;

public:
  voxel_volume() : floor_fact_(1.0f), floor_rfact_(1.0f / 1.0f)
  {
    reset();
  }

  voxel_volume(float cellSize): floor_fact_(cellSize), floor_rfact_(1.0f/cellSize) 
  { 
    reset(); 
  }

  void reset()
  {
    volume_ = 0;
    rpos_.clear();
  }

  void reset(float cellSize)
  {
    floor_fact_ = cellSize;
    floor_rfact_ = 1.0f / cellSize;
    volume_ = 0;
    rpos_.clear();
  }

  void operator()(const glm::vec3& pos) const
  {
    volume_ = 0;  // invalid flag
    rpos_.push_back(glm::floor(floor_rfact_ * pos) * floor_fact_ + 0.5f * floor_fact_);
  }

  void operator()(const CBird& bird) const
  {
    this->operator()(bird.position());
  }

  double volume() const
  {
    if (volume_ == 0)
    {
      std::sort(rpos_.begin(), rpos_.end(), detail::voxel_volume_cmp());
      rpos_t::iterator uend = std::unique(rpos_.begin(), rpos_.end(), detail::vec3_equal);
      rpos_.erase(uend, rpos_.end());
      volume_ = std::powf(floor_fact_, 3.0f) * float(rpos_.size());
    }
    return volume_;
  }

  float floor_fact() const { return floor_fact_; }
  const rpos_t& rpos() const { return rpos_; }

private:
  mutable rpos_t rpos_;  // rpos = floor(pos / floor_fact_) * floor_fact
  mutable double  volume_;
  float  floor_fact_;
  float  floor_rfact_;
};


#endif
