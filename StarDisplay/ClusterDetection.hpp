#ifndef CLUSTERDETECTION_HPP_INCLUDED
#define CLUSTERDETECTION_HPP_INCLUDED

#include <future>
#include "glmfwd.hpp"


//! Result type of cluster detection.
//!
struct cluster_entry
{
  glmutils::bbox3 bbox;       //!< axis-aligned bounding box of the cluster
  glm::vec3       velocity;   //!< cluster velocity
  unsigned        size;       //!< number of individuals in the cluster
  int             loudest;    //!< id of 'loudest' flock

  cluster_entry() {}
  cluster_entry(glmutils::bbox3 const& b, unsigned s): bbox(b), size(s), loudest(-1), velocity(0.0f) {}
  cluster_entry(glmutils::bbox3 const& b, unsigned s, const glm::vec3& vel):  bbox(b), size(s), loudest(-1), velocity(vel) {}
};


class ClusterDetection
{
public:
  typedef std::vector<cluster_entry> cluster_container;

  ClusterDetection();
  ~ClusterDetection();
  void update(const class CFlock* flock, cluster_container& res, double T0);
  double T0() const { return T00_; }

private:
  void stage1(float clusterDist1D);
  void stage2(float clusterDist3D);

  cluster_container             clusters_;
  std::vector<glmutils::bbox3>  boxes_;
  std::vector<glm::vec3>        vel_;
  std::future<void>             future_;
  double                        T0_, T00_;
};


#endif
