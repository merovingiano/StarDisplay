//! \file bounding_box.hpp Flock bounding box
//! \ingroup Analysis


#ifndef BOUNDING_BOX_HPP_INCLUDED
#define BOUNDING_BOX_HPP_INCLUDED

#include <vector>
#include <glmutils/oobb.hpp>
#include "glmfwd.hpp"
#include "accumulators.hpp"
#include "Bird.hpp"
#include "Flock.hpp"


struct bounding_box_base
{
  bounding_box_base();
  void reset();
  void operator()(const CBird& bird) const
  {
    forward_(glm::dvec3(bird.forward()));
    side_(glm::dvec3(bird.side()));
    centerOfMass_(glm::dvec3(bird.position()));
    velocity_(glm::dvec3(bird.velocity()));
  }
  mutable average<glm::dvec3> forward_;
  mutable average<glm::dvec3> side_;
  mutable average<glm::dvec3> centerOfMass_;
  mutable average<glm::dvec3> velocity_;
};


//! \brief Capture velocity aligned bounding box and pca based bounding box.
class bounding_box : private bounding_box_base
{
public:
  bounding_box();
  void reset();
  
  void operator()(const CBird& bird) const
  {
    bounding_box_base::operator()(bird);
    pos_.emplace_back(bird.position());
    side_(glm::dvec3(bird.side()));
    dirty_ = true;
  }

  // valid for both bounding boxes
  glm::dvec3 velocity() const  { resume(); return velocity_.mean(); }          //!< Mean velocity
  glm::dvec3 centerOfMass() const  { resume(); return centerOfMass_.mean(); }  //!< Center of mass

  // length-width specific
  const glm::dmat4& lw_H() const          { resume(); return lw_H_; }           //!< Homogeneous transformation Matrix -> BB
  const glm::dvec3& lw_extent() const     { resume(); return lw_extent_; }      //!< (length, height, width)
  const glm::dvec3& lw_geoCenter() const  { resume(); return lw_geoCenter_; }   //!< Center of the BB
  double lw_balanceShift() const          { resume(); return lw_balance_;  }    //!< Balance shift

  // pca specific
  const glm::dmat4& pca_H() const         { resume(); return pca_H_; }            //!< Homogeneous transformation Matrix -> BB
  glm::dvec3 pca_EV3() const              { resume(); return glm::dvec3(pca_H_[2]); }  //!< Biggest eigenvector, i.e. direction of I3 axis
  const glm::dvec3& pca_I123() const      { resume(); return pca_I123_; }         //!< I123
  const glm::dvec3& pca_geoCenter() const { resume(); return pca_geoCenter_; }    //!< Center of the BB
  double pca_balanceShift() const         { resume(); return pca_balance_; }      //!< Balance shift

private:
  void resume() const;

  mutable glm::dmat4  lw_H_;
  mutable glm::dvec3  lw_extent_;
  mutable glm::dvec3  lw_geoCenter_;
  mutable double      lw_balance_;

  mutable glm::dmat4  pca_H_;
  mutable glm::dvec3  pca_I123_;
  mutable glm::dvec3  pca_geoCenter_;
  mutable double      pca_balance_;

  typedef std::vector<glm::dvec3> pos_vect;
  mutable pos_vect pos_;
  mutable bool dirty_;
};


/////////////////////////////////////////////////////
// Implementation
/////////////////////////////////////////////////////


inline bounding_box_base::bounding_box_base() 
{
  reset();
}


inline void bounding_box_base::reset()
{
  forward_.reset();
  side_.reset();
  centerOfMass_.reset();
  velocity_.reset();
}


inline bounding_box::bounding_box() 
{
  reset();
}


inline void bounding_box::reset()
{
  bounding_box_base::reset();
  pos_.clear();
  dirty_ = true;
}


namespace {

  template<typename IT>
  struct dpos_iter
  {
    typedef std::random_access_iterator_tag iterator_category;
    typedef glm::dvec3 value_type;
    typedef ptrdiff_t difference_type;
    typedef ptrdiff_t distance_type;  
    typedef glm::dvec3* pointer;
    typedef glm::dvec3& reference;

    dpos_iter(IT it): it_(it) {}
    value_type operator[](size_t i) const { return glm::dvec3(it_[i]); }
    IT it_;
  };

}


inline void bounding_box::resume() const
{
  if (dirty_)
  {
    dirty_ = false;
    glm::dmat3 T(0);
    T[0] = glm::normalize(forward_.mean());
    T[2] = glm::normalize(side_.mean());
    T[1] = glm::normalize( glm::cross(T[0], T[2]) );

    pos_vect::const_iterator first(pos_.begin());
    pos_vect::const_iterator last(pos_.end());
    if (first == last) return;
    glmutils::dbbox3 aabb((*first++) * T);
    for (; first != last; ++first) 
    {
      include(aabb, (*first) * T);
    }
    const glm::dvec3 fdir = glm::normalize(velocity_.mean());
    lw_extent_ = glmutils::extent(aabb);
    lw_geoCenter_ = glmutils::center(aabb);
    lw_H_ = glm::dmat4(T); lw_H_[3] = glm::dvec4(T * lw_geoCenter_, 1.0);
    glm::dvec3 ofs = centerOfMass_.mean() - lw_geoCenter_;
    lw_balance_ = glm::dot(ofs, fdir) / lw_extent_.x;

    glm::dvec3 ext;
    pca_H_ = glm::dmat4(glmutils::oobb(static_cast<int>(pos_.size()), dpos_iter<pos_vect::const_iterator>(pos_.begin()), ext));
    pca_geoCenter_ = glm::dvec3(pca_H_[3]);
    pca_I123_ = glm::dvec3(ext);
    ofs = centerOfMass_.mean() - pca_geoCenter_;
    pca_balance_ = glm::dot(ofs, fdir) / lw_extent_.x;
  }
}



#endif
