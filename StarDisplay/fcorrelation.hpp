//! \file fcorrelation.hpp Correlation function of fluctuations
//! \ingroup Analysis


#ifndef FCORRELATION_HPP_INCLUDED
#define FCORRELATION_HPP_INCLUDED

#include "Flock.hpp"
#include "accumulators.hpp"


//! \brief ToDo
class correlation
{
public:
  struct corrData
  {
    corrData(const glm::vec3& p, const glm::vec3& v, const glm::vec3& f, float s)
      : pos(p), vel(v), forward(f), speed(s)
    {}
    glm::vec3 pos;
    glm::vec3 vel;
    glm::vec3 forward;
    float     speed;
  };

public:
  typedef std::vector< average<double> > corr_function;
  typedef std::vector<corrData>          corr_vect;

  correlation();
  void reset(size_t maxLI);
  void operator()(const CBird& bird) const
  {
    glm::vec3 vel(bird.velocity());
    data_.emplace_back(bird.position(), vel, bird.forward(), bird.speed());
    avel_(vel);
  }
  void resume();
  float L() const { return L_; }            //!< Returns flock size L (m). Valid after \c resume.
  size_t size() const { return Cv_.size(); }      //!< Returns flock size N. Valid after \c resume.
  const corr_function& Cv() const { return Cv_; }    //!< Returns Cv(r). Valid after \c resume.
  const corr_function& Cp() const { return Cp_; }    //!< Returns Cp(r). Valid after \c resume.
  const corr_function& Csp() const { return Csp_; }  //!< Returns Csp(r). Valid after \c resume.

private:
  mutable corr_vect data_;
  mutable average<glm::dvec3> avel_;
  corr_function Cv_;    // velocity
  corr_function Cp_;    // polarization
  corr_function Csp_;   // speed
  float L_;             // Flock 'size' L (m)
  size_t maxLI_;        // Index for max. L.
};


////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////


inline correlation::correlation()
{
  reset(100);
}


inline void correlation::reset(size_t maxLI)
{
  data_.clear();
  avel_.reset();
  maxLI_ = maxLI;
}


inline void correlation::resume()
{
  corr_vect::iterator first(data_.begin());
  corr_vect::iterator last(data_.end());
  glm::vec3 avel(avel_.mean());
  float aspeed(static_cast<float>(glm::length(avel_.mean())));
  if (aspeed < 0.000001f) return;
  glm::vec3 aforward(avel_.mean() / double(aspeed));
  average<double> aapol;
  for (corr_vect::iterator it = first; it != last; ++it)
  {
    (*it).vel -= avel;
    (*it).speed -= aspeed;
    (*it).forward.x = glm::dot((*it).forward, aforward);
    aapol((*it).forward.x);
  }
  float apol = static_cast<float>(aapol.mean());
  Cv_.clear(); Cv_.resize(maxLI_);
  Cp_.clear(); Cp_.resize(maxLI_);
  Csp_.clear(); Csp_.resize(maxLI_);
  L_ = 0;
  for (corr_vect::iterator it = first; it != last; ++it)
  {
    glm::vec3 posi = (*it).pos;
    glm::vec3 ui = (*it).vel;
    float dpi = (*it).forward.x - apol;
    float si = (*it).speed;
    for (corr_vect::iterator jt = it; jt != last; ++jt)
    {
      const float radius = glm::distance(posi, (*jt).pos);
      L_ = std::max(L_, radius);
      size_t radius_index = static_cast<size_t>(radius);
      if (radius_index < maxLI_) 
      {
        Cv_[radius_index](glm::dot(ui, (*jt).vel));
        float dpj = (*jt).forward.x - apol;
        Cp_[radius_index](dpi * dpj);
        Csp_[radius_index](si * (*jt).speed);
      }
    }
  }
}



#endif
