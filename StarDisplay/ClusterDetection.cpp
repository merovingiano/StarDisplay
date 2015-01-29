#include <numeric>
#include <glmutils/bbox_bbox.hpp>
#include "ClusterDetection.hpp"
#include "Flock.hpp"
#include "Globals.hpp"


ClusterDetection::ClusterDetection()
  : T0_(0), T00_(0)
{
}


ClusterDetection::~ClusterDetection()
{
  if (future_.valid())
  {
    future_.get();
  }
}


void ClusterDetection::update(const CFlock* flock, cluster_container& res, double T0)
{
  if (future_.valid())
  {
    future_.get();
    res.swap(clusters_);
  }
  T00_ = T0_;
  T0_ = T0;
  boxes_.resize(flock->num_prey());
  std::copy(flock->level_begin(0), flock->level_end(0), boxes_.begin());
  vel_.resize(flock->num_prey());
  std::transform(flock->prey_begin(), flock->prey_end(), vel_.begin(), [] (const CPrey& prey) 
  { 
    return prey.velocity(); 
  });

  future_ = std::async([this] (float dist1D, float dist3D) {
    stage1(dist1D);
    stage2(dist3D);
  }, PARAMS.ClusterDistance1D, PARAMS.ClusterDistance3D);
}


void ClusterDetection::stage1(float clusterDist1D)
{
  // Stage 1: Detect 1D clusters along the Hilbert-curve.
  clusters_.erase(clusters_.begin(), clusters_.end());
  CFlock::build_policy build;
  std::vector<glmutils::bbox3>::const_iterator first_box(boxes_.begin());
  std::vector<glmutils::bbox3>::const_iterator last_box(boxes_.end());
  std::vector<glmutils::bbox3>::const_iterator cluster_head(first_box);
  std::vector<glm::vec3>::const_iterator first_vel(vel_.begin());
  std::vector<glm::vec3>::const_iterator last_vel(vel_.end());
  std::vector<glm::vec3>::const_iterator v0(first_vel);
  const float clusterSqDist1D = clusterDist1D * clusterDist1D;
  for (++first_box, ++first_vel; first_box != last_box; ++first_box, ++first_vel)
  {
    if (glmutils::squaredDistance<glmutils::bbox3>(*(first_box-1), *first_box) > clusterSqDist1D)
    {
      // Box *first_box is too far away from the tail of the cluster.
      // Finalize the cluster and start a new one with *first_box. 
      //
      clusters_.emplace_back( *cluster_head, static_cast<unsigned>(std::distance(cluster_head, first_box)), *v0 );
      cluster_entry& cb = clusters_.back();
      build(cluster_head+1, first_box, &cb.bbox);
      cb.velocity = std::accumulate(v0+1, first_vel, cb.velocity);
      cluster_head = first_box;
      v0 = first_vel;
    }
  }
  clusters_.emplace_back( *cluster_head, static_cast<unsigned>(std::distance(cluster_head, last_box)), *v0 );
  cluster_entry& cb = clusters_.back();
  build(cluster_head+1, last_box, &cb.bbox);
  cb.velocity = std::accumulate(v0+1, last_vel, cb.velocity);
}


void ClusterDetection::stage2(float clusterDist3D)
{
  // Stage 2: Collect 1D-cluster to flocks.
  //
  typedef cluster_container::iterator cluster_iterator;

  std::vector<cluster_entry> flocks;
  flocks.reserve(boxes_.size());        // worst case
  cluster_iterator first_cluster(clusters_.begin());
  cluster_iterator last_cluster(clusters_.end());
  while ( first_cluster != last_cluster ) 
  {
    flocks.clear();
    flocks.push_back(*first_cluster++);
    cluster_iterator first_flock(flocks.begin());
    for (; first_flock != flocks.end(); ++first_flock)
    {
      CFlock::intersect_policy test(glmutils::bbox3(first_flock->bbox.p0()-clusterDist3D, first_flock->bbox.p1()+clusterDist3D));
      for (cluster_iterator it=first_cluster; it != last_cluster; ) 
      {
        if (test(it->bbox)) 
        {
          flocks.push_back(*it);
          flocks.front().size += it->size;
          flocks.front().velocity += it->velocity;
          glmutils::include(flocks.front().bbox, it->bbox);
          *it = *--last_cluster;  // remove this cluster from list.
        }
        else 
        {
          ++it;  // try next cluster in list.
        }
      }
    }
    *(first_cluster-1) = flocks.front();
  }
  clusters_.erase(last_cluster, clusters_.end());
  std::for_each(clusters_.begin(), clusters_.end(), [] (cluster_entry& ce) { ce.velocity /= float(ce.size ? ce.size : 1); });
  std::sort(clusters_.begin(), clusters_.end(), [] (const cluster_entry& a, const cluster_entry& b) 
  { 
    return a.size > b.size; 
  });
}


