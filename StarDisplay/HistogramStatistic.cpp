#include "HistogramStatistic.hpp"
#include <fstream>
#include <algorithm>
#include <glmutils/ostream.hpp>
#include "HistOverlay.hpp"
#include "GLSLState.hpp"
#include "Flock.hpp"
#include "features.hpp"
#include "visitors.hpp"
#include "Params.hpp"
#include "Globals.hpp"


using namespace Param;


HistogramStatistic::HistogramStatistic(FeatureMap::FMappings mapping)
  : DefaultStatistic(), mapping_(mapping), fme_(PFM.Entries[mapping])
{
  Reset();
}

void HistogramStatistic::Reset()
{
  log_.reset(fme_.hist);
}


void HistogramStatistic::apply(double stat_dt)
{ 
  log_.reduce(PFM.histKeepPercent / 100.0);
  const fpFeature mapping_fun = select_feature(mapping_);
  const val2TexCoord v2tex(fme_.hist.min, fme_.hist.max);
  const int N = static_cast<int>(GFLOCK.num_prey());
  values_.resize(N);
# pragma omp parallel firstprivate(mapping_fun, v2tex, N)
  {
    CFlock::prey_const_iterator first(GFLOCK.prey_begin());
#   pragma omp for
    for (int i=0; i<N; ++i)
    {
      const float value = values_[i] = mapping_fun(*(first + i));
      (*(first + i)).setCurrentColorTex( v2tex(value) );
    }
  }
  for (int i=0; i<N; ++i)
  {
    log_(values_[i]);
  }
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&] (const CPredator& pred) 
  {
    float value = mapping_fun(pred);
    pred.setCurrentColorTex( v2tex(value) );
  });
}


std::string HistogramStatistic::labelText() const
{
  return fme_.title;
}


void HistogramStatistic::Display() const 
{ 
  GGl.Overlay->Display(log_, val2TexCoord(fme_.hist.min, fme_.hist.max), labelText().c_str()); 
}


void HistogramStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  os << "% Histogram of " << fme_.title << '\n';
  glm::dvec3 q = log_.quartiles();
  os << "\nq25 = " << q.x << ";\n";
  os << "q50 = " << q.y << ";\n";
  os << "q75 = " << q.z << ";\n";
  os << "\n% bin_center counts\nH = [ ";
  for (size_t bin=0; bin < log_.num_bins(); ++bin)
  {
    glm::dvec2 r(log_[bin]);
    os << r.x << ' ' << r.y << ';';
  }
  os << " ];\n";
}


