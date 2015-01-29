#include <fstream>
#include <glmutils/matrix_norm.hpp>
#include "GlobalDevStatistic.hpp"
#include "GLSLImm.hpp"
#include "IText.hpp"
#include "ICamera.hpp"
#include "HistOverlay.hpp"
#include "GLSLState.hpp"
#include "Flock.hpp"
#include "bounding_box.hpp"
#include "Params.hpp"
#include "Globals.hpp"


using namespace Param;


//! Helper query functor
struct global_mean_qf
{
  void operator()(const CBird& bird) const
  {
    bb_(bird);
  }
  bounding_box_base bb_;
};


struct global_dev_qf_base
{
  global_dev_qf_base(histogram& log, const val2TexCoord& v2tex)
    : log_(log), v2tex_(v2tex)
  {
  }

  void log(const CBird& bird, float dev)
  {
    bird.setCurrentColorTex(v2tex_(dev));
    log_(dev);
  }

  histogram& log_;
  val2TexCoord v2tex_;
};


struct global_pol_dev_qf : global_dev_qf_base
{
  global_pol_dev_qf(const global_mean_qf& gm, histogram& log, const val2TexCoord& v2tex)
    : global_dev_qf_base(log, v2tex), forward_(gm.bb_.forward_.mean())
  {
  }

  void operator()(const CBird& bird)
  {
    log( bird, glm::dot(forward_, bird.forward()) );
  }

  const glm::vec3 forward_;
};


struct global_vel_dev_qf : global_dev_qf_base
{
  global_vel_dev_qf(const global_mean_qf& gm, histogram& log, const val2TexCoord& v2tex)
    : global_dev_qf_base(log, v2tex), velocity_(gm.bb_.velocity_.mean())
  {}

  void operator()(const CBird& bird)
  {
    log( bird, glm::length(bird.velocity() - velocity_) );
  }

  const glm::vec3 velocity_;
};


struct global_speed_dev_qf : global_dev_qf_base
{
  global_speed_dev_qf(const global_mean_qf& gm, histogram& log, const val2TexCoord& v2tex)
    : global_dev_qf_base(log, v2tex), speed_(static_cast<float>(glm::length(gm.bb_.velocity_.mean())))
  {}

  void operator()(const CBird& bird)
  {
    log( bird, bird.speed() - speed_ );
  }

  const float speed_;
};


struct global_frobenius_dev_qf : global_dev_qf_base
{
  global_frobenius_dev_qf(const global_mean_qf& gm, histogram& log, const val2TexCoord& v2tex)
    : global_dev_qf_base(log, v2tex), G_(glm::vec3(gm.bb_.forward_.mean()), glm::vec3(gm.bb_.side_.mean()), glm::vec3(0))
  {
    G_[2] = glm::cross(G_[0], G_[1]);
  }

  void operator()(const CBird& bird)
  {
    glm::mat3 A( bird.forward(), bird.side(), glm::cross(bird.forward(), bird.side()) ); 
    log ( bird, glmutils::frobenius_norm(G_ - A) );
  }

  glm::mat3 G_;          // mean orientation
};


template <typename QF>
void apply_global_dev(const CFlock& flock, histogram& log, const val2TexCoord& v2tex)
{
  CFlock::cluster_const_iterator first(flock.clusters_begin());
  CFlock::cluster_const_iterator last(flock.clusters_end());
  for (; first != last; ++first) 
  {
    global_mean_qf qfb;
    flock.query(qfb, (*first).bbox);
    QF qf(qfb, log, v2tex);
    flock.query(qf, (*first).bbox);
  }
}


GlobalDevStatistic::GlobalDevStatistic(FeatureMap::FMappings mapping)
:  HistogramStatistic(mapping)
{
  Reset();
};


void GlobalDevStatistic::Reset()
{
  HistogramStatistic::Reset();
  idata_.clear();
  time_.clear();
  L_.clear();
  idata_.resize(std::min(GFLOCK.num_prey(), static_cast<unsigned>(PCFME.p[0])));
  maxSamples_ = static_cast<int>(PCFME.p[1]);
}


void GlobalDevStatistic::apply(double stat_dt)
{
  val2TexCoord v2tex(fme_.hist.min, fme_.hist.max);
  switch (mapping_)
  {
  case FeatureMap::GlobalVelDev:
    apply_global_dev<global_vel_dev_qf>(GFLOCK, log_, v2tex);
    break;
  case FeatureMap::GlobalSpeedDev:
    apply_global_dev<global_speed_dev_qf>(GFLOCK, log_, v2tex);
    break;
  case FeatureMap::GlobalPolDev:
    apply_global_dev<global_pol_dev_qf>(GFLOCK, log_, v2tex);
    break;
  case FeatureMap::GlobalFrobenius:
    apply_global_dev<global_frobenius_dev_qf>(GFLOCK, log_, v2tex);
    break;
  }

  if (time_.size() >= static_cast<size_t>(maxSamples_))
    return;

  //time_.push_back(Sim.SimulationTime());
  //CFlock::const_iterator it = GFlock.prey_begin();
  //const size_t n = std::min(GFlock.num_prey(), int(GCFmE.p[0]));
  //for (size_t i=0; i<n; ++i, ++it) 
  //{
  //  idata_[i].first(v2tex.inverse(it->getCurrentColorTex()));
  //  idata_[i].second(it->circularity());
  //}
  //const size_t N = GFlock.num_prey();
  //it = GFlock.prey_begin();
  //float maxDist = 0.0f;
  //for (size_t i = 0; i < (N-1); ++i, ++it)
  //{
  //  const glm::vec3 pos = it->position();
  //  CFlock::const_iterator jt = it + 1;
  //  for (size_t j = i+1; j < N; ++j, ++jt)
  //  {
  //    maxDist = std::max(maxDist, glm::distance2(pos, jt->position()));
  //  }
  //}
  //L_.push_back(std::sqrt(maxDist));
}


void GlobalDevStatistic::Display() const
{
  HistogramStatistic::Display();
  if (!L_.empty())
  {
    GTEXT->print("\\smallface{}");
    char buf[128];
    _snprintf_s(buf, 127, "L = %.4g m ", L_.back());
    glm::ivec2 ext = GTEXT->extent(buf);
    glm::ivec4 vp = GCAMERA.GetViewport();
    glm::ivec2 orig = GTEXT->orig();
    GTEXT->set_orig(glm::ivec2(vp[2] - ext.x, ext.y));
    GTEXT->print(buf);
    GTEXT->set_orig(orig);
  }
}


void GlobalDevStatistic::save(const char* fname, bool append) const
{
  HistogramStatistic::save(fname, append);
  /*
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
//  os << "% Global deviation statistic\n";
//  os << "T = [ ";
  std::copy(time_.begin(), time_.end(), std::ostream_iterator<double>(os, " "));
  os << ";\n\n";
  std::copy(L_.begin(), L_.end(), std::ostream_iterator<double>(os, " "));
  os << ";\n\n";
  for (size_t i=0; i<idata_.size(); ++i)
  {
    std::copy(idata_[i].first.data().begin(), idata_[i].first.data().end(), std::ostream_iterator<float>(os, " "));
    os << ";\n";
  }
  os << '\n';
  //  os << "];\n\nCi = [\n";
  //for (size_t i=0; i<idata_.size(); ++i)
  //{
  //  std::copy(idata_[i].second.data().begin(), idata_[i].second.data().end(), std::ostream_iterator<float>(os, " "));
  //  os << ";\n";
  //}
  //os << "];\n\n";
  //return cblog_->save(os);
  */
}





