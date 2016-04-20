#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <glmutils/ostream.hpp>
#include <glmutils/random.hpp>
#include "EvolveDeflection.hpp"
#include "random.hpp"
#include "Flock.hpp"
#include "IText.hpp"
#include "ICamera.hpp"
#include "GLSLState.hpp"
#include "Params.hpp"
#include "visitors.hpp"
#include "Globals.hpp"


using namespace Param;


namespace {

  struct cmp_min_dist
  {
    bool operator () (const glm::vec4& a, const glm::vec4& b) const 
    { 
      return a.w < b.w; 
    }
  };

}


EvolveDeflection::EvolveDeflection()
  : DefaultStatistic()
{
  Reset();
}


void EvolveDeflection::Reset()
{
  lastShuffle_ = Sim.SimulationTime();
  Generation_ = 1;
  std::for_each(GFLOCKNC.predator_begin(), GFLOCKNC.predator_end(), [] (CPredator& pred)
  {
    pred.ResetHunt();
  });
  alleles_.clear();
}


void EvolveDeflection::apply(double stat_dt)
{
  if (alleles_.empty()) 
  {
    alleles_.emplace_back();
  }
  allele_type& allele = alleles_.back();
  allele.clear();
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&allele] (const CPredator& pred)
  {
    const glm::vec3& defl = glm::vec3(pred.getDeflection());
    pred.setCurrentColorTex( visit<FeatureMap::Default>(pred) );
    allele.push_back( glm::vec4(defl.x, defl.y, defl.z, pred.hunts().minDistLockedOn) );
  });
  std::sort(allele.begin(), allele.end(), cmp_min_dist());
  if (PCFME.p[0] <= static_cast<float>(Sim.SimulationTime() - lastShuffle_))
  {
    lastShuffle_ = Sim.SimulationTime();
    Shuffle();
  }
}


void EvolveDeflection::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  for (size_t i=0; i<alleles_.size(); ++i)
  {
    std::ostream_iterator<glm::vec4> oit(os, " ");
    std::copy(alleles_[i].begin(), alleles_[i].end(), oit);
    os << '\n';
  }
}


void EvolveDeflection::Display() const
{
  if (alleles_.empty() || alleles_.back().empty())
  {
    return;
  }
  const allele_type& allele = alleles_.back();
  GTEXT->print("\\monoface{}");
  GTEXT->set_orig(glm::ivec2(GCAMERA.GetViewport()[2] - 300, GTEXT->base()));
  GTEXT->set_tabsize(80);
  GTEXT->print("\n");
  char buf[256];
  _snprintf_s(buf, 255, "Generation: %d\n\n", Generation_);
  GTEXT->print(buf);
  size_t n = std::min(size_t(25), allele.size());
  for (size_t i=0; i<n; ++i)
  {
    const glm::vec4& a = allele[i];
    _snprintf_s(buf, 255, "%7.3f %7.3f %7.3f %7.3f\n", a.x, a.y, a.z, a.w);
    GTEXT->print(buf);
  }
  GTEXT->print("\n");
  n = allele.size() - 5;
  for (size_t i=n; i<allele.size(); ++i)
  {
    const glm::vec4& a = allele[i];
    _snprintf_s(buf, 255, "%7.3f %7.3f %7.3f %7.3f\n", a.x, a.y, a.z, a.w);
    GTEXT->print(buf);
  }
  glm::vec4 ave(0);
  for (size_t i=0; i<allele.size(); ++i)
  {
    ave += allele[i];
  }
  ave /= allele.size();
  _snprintf_s(buf, 255, "\n%7.3f %7.3f %7.3f %7.3f\n", ave.x, ave.y, ave.z, ave.w);
  GTEXT->print(buf);
}


void EvolveDeflection::Shuffle()
{
	//next generation starts here
  ++Generation_;
  // just a vector of type vec4 having three deflection parameters and the min distance
  allele_type allele;
  //looping over the predators and placing all 
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&allele] (const CPredator& pred)
  {
    const glm::vec3& defl = glm::vec3(pred.getDeflection());
    allele.push_back( glm::vec4(defl.x, defl.y, defl.z, pred.hunts().minDist) );
  });
  //resort on having the minimum distance
  std::sort(allele.begin(), allele.end(), cmp_min_dist());
  // placing all alleles into the total bunch of alleles over time
  alleles_.emplace_back(allele);
  //how big is the total amount of alleles in the population?
  unsigned N = static_cast<unsigned>(allele.size());
  
  // 50% overwritten + mutation
  std::copy(allele.begin(), allele.begin() + (N >> 1), allele.begin() + (N >> 1));
  std::uniform_real_distribution<float> rnd(-0.5f, 0.5f);
  //each generation the mutation becomes less..
  for (unsigned i=(N >> 1); i < N; ++i)
  {
    allele[i].x += (1.0f / Generation_) * rnd(rnd_eng());
    allele[i].y += (1.0f / Generation_) * rnd(rnd_eng());
  }
  CFlock::pred_iterator first(GFLOCKNC.predator_begin());
  CFlock::pred_iterator last(GFLOCKNC.predator_end());

  //change all of the settings of the predators after mutation
  for (unsigned i=0; first != last; ++first, ++i)
  {
    first->setDeflection(*(const glm::vec3*)&allele[i]);
  }
  // Average of top 1%
  unsigned n = unsigned(double(allele.size()) * 0.01);
  glm::vec4 top1(0);
  for (unsigned i=0; i<n; ++i)
  {
    top1 += allele[i];
  }
  top1 /= n;

  first = GFLOCKNC.predator_end() - n;
  //set the the last n to the average of the top 1%, This is possibly wrong, you want the sorted vector to be adapted
  for (; first != last; ++first)
  {
    first->setDeflection(*(const glm::vec3*)&top1);
  }
  const float R = PROOST.Radius;
  std::for_each(GFLOCKNC.predator_begin(), GFLOCKNC.predator_end(), [R] (CPredator& pred)
  {
    pred.ResetHunt();
    pred.setTrail(false);
    pred.position_ = R * glmutils::vec3_in_sphere(rnd_eng());
    pred.position_.y = pred.GetBirdParams().altitude;
    pred.setTrail(true);
  });
}
