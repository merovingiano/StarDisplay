#include <atomic>
#include <boost/circular_buffer.hpp>
#include <glmutils/avx/vec.hpp>
#include "trail_buffer.hpp"
#include "Params.hpp"
#include "Globals.hpp"


using namespace Param;
namespace avx = glmutils::avx;

size_t trailVertices()
{
  return size_t((PARAMS.TrailLength / (PARAMS.TrailSkip + 1)) / PARAMS.IntegrationTimeStep); 
}


struct trail_elem {
  trail_elem() : side_t(0), pos_u(0) {};
  trail_elem(const glm::vec3& side, float t, const glm::vec3& pos, float u): side_t(side, t), pos_u(pos, u) {}
  avx::vec4 side_t;
  avx::vec4 pos_u;
};


class trail_buffer
{
public:
  trail_buffer(int id)
    : buf_(trailVertices()), id_(id), skiped_(1), skip_(PARAMS.TrailSkip), time_(0.0f) 
  {
  }

  ~trail_buffer() {}

  void append(const glm::vec3& position, const glm::vec3& side, float colorTex, float dt)
  {
    time_ += dt;
    if (--skiped_ == 0)
    {
      buf_.push_back( trail_elem(side, time_, position, colorTex) );
      skiped_ = skip_ + 1;
    }
  }

  size_t vertcount(int ignoreId) const
  {
    return (id_ == ignoreId) ? 0 : buf_.size() + 1;
  }

  void flatten(trail_elem* dst, int ignoreId) const  // nothrow
  {
    if (id_ == ignoreId) return;
    *dst++ = trail_elem();    // start vertex
    const auto ar1 = buf_.array_one(); 
    const auto ar2 = buf_.array_two();
    memcpy((void*)(dst), ar1.first, ar1.second * sizeof(trail_elem));
    memcpy((void*)(dst + ar1.second), ar2.first, ar2.second * sizeof(trail_elem));
  }

  boost::circular_buffer<trail_elem>  buf_;
  int   id_;
  int   skiped_;
  int   skip_;
  float time_;
};



size_t trail_buffer_pool::recommendedCapacity() const
{ 
  return PARAMS.renderFlags.show_trails ? (pool_.size() * (trailVertices() + 1) * sizeof(trail_elem)) : 0; 
}


trail_buffer* trail_buffer_pool::create(int id) const
{
  pool_.push_front( new trail_buffer(id) );
  return pool_.front();
}


void trail_buffer_pool::destroy(trail_buffer* ptb) const
{
  auto it = std::find(pool_.begin(), pool_.end(), ptb);
  if (it != pool_.end())
  {
    pool_.erase(it);
  }
  if (ptb) (ptb)->~trail_buffer();
}


size_t trail_buffer_pool::flatten(void* buf, size_t capacity, int ignoreId) const
{
  if (nullptr == buf) return 0;
  const int maxTrail = std::min<int>(int(pool_.size()), int(capacity / ((trailVertices() + 1) * sizeof(trail_elem))));
  tbs_.resize(maxTrail);
  auto it = pool_.cbegin();
  for (int i=0; i<maxTrail; ++i, ++it)
  {
    tbs_[i].first = *it;
    tbs_[i].second = (*it)->vertcount(ignoreId);
  }
  std::atomic<size_t> vertCount(0);
  trail_elem* dst = reinterpret_cast<trail_elem*>(buf);
# pragma omp parallel for schedule(static)
  for (int i=0; i<maxTrail; ++i)
  {
    tbs_[i].first->flatten(dst + vertCount.fetch_add(tbs_[i].second), ignoreId);
  }
  return vertCount.load();
}


void appendTrail(class trail_buffer* ptb, const glm::vec3& position, const glm::vec3& side, float colorTex, float dt)
{
  if (ptb != nullptr) ptb->append(position, side, colorTex, dt);
}


