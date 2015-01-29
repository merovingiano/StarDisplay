#include <hrtree/isfc/key_gen.hpp>
#include <glmutils/bbox_bbox.hpp>
#include <glmutils/frustum.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/bbox_ray.hpp>
#include <glmutils/avx/vec.hpp>
#include <hrtree/sorting/parallel_radix_sort.hpp>
#include "Flock.hpp"
#include "Globals.hpp"


using namespace Param;
namespace avx = glmutils::avx;

namespace detail {

  
  // radix sort support
  template <typename KEY, typename P>
  struct proxy2key : std::unary_function<P, const unsigned char*>
  {
    BOOST_STATIC_CONSTANT(int, key_bytes = KEY::key_bytes);
    const unsigned char* operator()(const P& x) const { return x.key_.data(); }
  };


  class frustum_culler_qf
  {
    const glmutils::frustum frustum_;

  public:
    frustum_culler_qf(const glm::dmat4& ModelViewMatrix, const glm::dmat4& ProjectionMatrix)
    : frustum_(glm::mat4(ModelViewMatrix), glm::mat4(ProjectionMatrix))
    {}

    bool operator()(const CFlock::bbox& aabb) const
    {
      return !glmutils::is_outside(frustum_, glmutils::bbox3(aabb));
    }
  };


  struct bbox_ctor
  {
    void operator()(CFlock::bbox* rawptr, const CBird& bird) const 
    {
      avx::vec3 pos(bird.position());
      const avx::vec3 hw(0.5f * bird.wingSpan());
      (pos - hw).store(glm::value_ptr(rawptr->p0()));
      (pos + hw).store(glm::value_ptr(rawptr->p1()));
    }
  };


  struct assign_flockId_qf
  {
    assign_flockId_qf(int id, unsigned size): id_(id), size_(size), n_(0) {}
    void operator()(CPrey& prey) 
    {
      prey.setFlockId(id_, size_);
      avelocity_ += prey.velocity();
      ++n_;
    }
    int       id_;
    unsigned  size_;
    glm::vec3 avelocity_;
    int       n_;
  };


  struct pick_qf
  {
    glm::vec3 ray_position_;
    glm::vec3 ray_direction_;
    CBird*    result_;
    float     distSq_;

    pick_qf(const glm::vec3& ray_position, const glm::vec3& ray_direction)
    : ray_position_(ray_position),
      ray_direction_(ray_direction),
      result_(0),
      distSq_(std::numeric_limits<float>::max()) 
    {}

    void operator()(CBird& bird)
    {
      const float distSq = glmutils::distanceSqRayPoint(ray_position_, ray_direction_, bird.position());
      if (distSq < distSq_) 
      {
        distSq_ = distSq;
        result_ = &bird;
      }
    }
  };


  class bbox_ray_intersect
  {
    glm::vec3   orig;
    glm::vec3   inv_dir;
    glm::ivec3  sign;

  public:
    bbox_ray_intersect(const glm::vec3& ray_position, const glm::vec3& ray_direction)
      : orig(ray_position), inv_dir(1.0f / ray_direction) 
    {
      sign.x = (inv_dir.x < 0.0f);
      sign.y = (inv_dir.y < 0.0f);
      sign.z = (inv_dir.z < 0.0f);
    }

    bool operator()(const CFlock::bbox& aabb) const
    {
      CFlock::bbox tmp(aabb); glmutils::inflate(tmp, glm::vec3(1.0f));
      return glmutils::intersect(tmp, orig, inv_dir, sign, 0, std::numeric_limits<float>::max());
    }
  };

}


CFlock::CFlock(unsigned numPrey)
: nextID_(1)
{
  prey_.reserve(numPrey);
  buffer_.reserve(numPrey);
}


CFlock::~CFlock()
{
}


void CFlock::update(float sim_dt, bool recluster)
{
  const int N = static_cast<int>(prey_.size());
  buffer_.resize(N);
  if (0 == N) return;
  bbox ext(rtree_.total_bv());
  ext.p0() -= 10.0f;  
  ext.p1() += 10.0f;
  hrtree::key_gen<key_type, glm::vec3> hilbert_domain(ext.p0(), ext.p1());
# pragma omp parallel firstprivate(hilbert_domain)
  {
    proxy_iterator first(prey_.begin());
#   pragma omp for
    for (int i=0; i<N; ++i)
    {
      (*(first + i)).key_ = hilbert_domain((first + i)->ptr_->position());
    }
  }
  bool swapped = hrtree::parallel_radix_sort(prey_.begin(), prey_.end(), buffer_.begin(), detail::proxy2key<key_type, proxy>());
  if (swapped) std::swap(prey_, buffer_);
  rtree_.parallel_construct(prey_begin(), prey_end(), detail::bbox_ctor());
  cluster_detection(sim_dt, recluster);
}


void CFlock::refresh()
{
  rtree_.construct(prey_begin(), prey_end(), detail::bbox_ctor());
  update(0.0f, true);
}


CBird* CFlock::pickNearest2Ray(const glm::vec3& ray_position,
                               const glm::vec3& ray_direction)
{
  detail::pick_qf qf(ray_position, ray_direction);
  qf = std::for_each(predator_begin(), predator_end(), qf);
  detail::bbox_ray_intersect policy(ray_position, ray_direction);
  rtree_.query(prey_begin(), policy, qf);
  return (qf.distSq_ < 4.0f) ? qf.result_ : 0;
}


void CFlock::remove_invisibles(const glm::dmat4& ModelViewMatrix, const glm::dmat4& ProjectionMatrix)
{
  buffer_.clear();
  detail::frustum_culler_qf qf(ModelViewMatrix, ProjectionMatrix);
  rtree_.query(prey_.begin(), qf, [this] (proxy& Proxy) 
  { 
    buffer_.push_back(std::move(Proxy)); 
  });
  prey_.clear();
  prey_.swap(buffer_);
  update(0.0f, true);
}


void CFlock::cluster_detection(float sim_dt, bool recluster)
{
  float dt = sim_dt;
  if (recluster) 
  {
    clusterDetection_.update(this, clusters_, Sim.SimulationTime());
    dt = static_cast<float>(Sim.SimulationTime() - clusterDetection_.T0());
  }
  std::for_each(clusters_.begin(), clusters_.end(), [dt] (cluster_entry& ce)
  {
    glm::vec3 dp = dt * ce.velocity;
    glmutils::move(ce.bbox, dp);
  });
  if (recluster)
  {
    assignFlockId();
  }
}


void CFlock::assignFlockId()
{
  prey_iterator first = prey_begin();
  const int N = static_cast<int>(num_prey());
  const int F = static_cast<int>(clusters_.size());
# pragma omp parallel firstprivate(first)
  {
#   pragma omp for
    for (int i=0; i<N; ++i) 
    {
      (*(first + i)).invalidateFlockId();
    }
#   pragma omp for schedule(dynamic, 1)
    for (int f=0; f<F; ++f)
    {
      cluster_entry& ce = clusters_[f];
      detail::assign_flockId_qf qf(f, ce.size);
      rtree_.query(prey_begin(), intersect_policy(ce.bbox), qf);
    }
#   pragma omp for nowait
    for (int f=0; f<F; ++f)
    {
      const glm::vec3 pos(glmutils::center(clusters_[f].bbox));
      float noise = 0.0f;
      clusters_[f].loudest = f; 
      for (int k=0; k<F; ++k)
      {
        if (k != f)
        {
          float nk = clusters_[k].size / glm::distance(pos, glmutils::center(clusters_[k].bbox));
          if (nk >= noise) 
          { 
            noise = nk; 
            clusters_[f].loudest = k; 
          }
        }
      }    
    }
  }
}


