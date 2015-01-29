#include <fstream>
#include <functional>
#include <hrtree/config.hpp>  // OpenMP stub
#include "HistOverlay.hpp"
#include "GLSLState.hpp"
#include "FCorrelationStatistic.hpp"
#include "fcorrelation.hpp"
#include "voxel_volume.hpp"
#include "Params.hpp"
#include "Globals.hpp"


using namespace Param;


FCorrelationStatistic::FCorrelationStatistic()
:  DefaultStatistic()
{
  Reset();
};


void FCorrelationStatistic::Reset()
{
  storage_.clear();
}


void FCorrelationStatistic::apply(double stat_dt)
{
  std::mutex emutex;
  std::exception_ptr eptr;

  const int c = static_cast<int>(GFLOCK.num_clusters());
  CFlock::cluster_const_iterator cfirst(GFLOCK.clusters_begin());
  correlation corr[HRTREE_OMP_MAX_THREADS];
  voxel_volume vox[HRTREE_OMP_MAX_THREADS];
  const size_t maxLI(static_cast<size_t>(PFME(Correlation).p[0]));
  const float cellSize(static_cast<float>(PFME(Correlation).p[1]));
# pragma omp parallel firstprivate(cfirst)
  {
    try
    {
#     pragma omp for schedule(dynamic, 1)
      for (int i = 0; i < c; ++i)
      {
        int tid = omp_get_thread_num();
        corr[tid].reset(maxLI);
        vox[tid].reset(cellSize);
        GFLOCK.query(corr[tid], (*(cfirst + i)).bbox);
        GFLOCK.query(vox[tid], (*(cfirst + i)).bbox);
        corr[tid].resume();
#       pragma omp critical (fcorr_apply)
        {
          size_t flockSize = (*(cfirst + i)).size;
          storage_type::iterator sit = storage_.find(flockSize);
          if (sit == storage_.end())
          {
            sit = storage_.insert(storage_type::value_type(flockSize, storage_entry(flockSize))).first;
          }
          // append averages
          (*sit).second.samples += 1;
          (*sit).second.L(corr[tid].L());
          (*sit).second.Volume(vox[tid].volume());
          size_t r = corr[tid].size();
          if (r > (*sit).second.Cv.size()) 
          {
            (*sit).second.Cv.resize(r);
            (*sit).second.Cp.resize(r);
            (*sit).second.Csp.resize(r);
          }
          for (size_t i=0; i<r; ++i)
          {
            (*sit).second.Cv[i].append(corr[tid].Cv()[i]);
            (*sit).second.Cp[i].append(corr[tid].Cp()[i]);
            (*sit).second.Csp[i].append(corr[tid].Csp()[i]);
          }
        }
      }
    }
    catch (...)
    {
      std::lock_guard<std::mutex> lock(emutex);
      eptr = std::current_exception();
    }
  }
  if (eptr != nullptr) std::rethrow_exception(eptr);
}


namespace {

  template<typename C>
  void print_means(std::ostream& os, const C& cr, size_t maxr)
  {
    size_t mi = std::min(maxr, cr.size());
    for (size_t i=0; i<mi; ++i) {
      os << cr[i].mean() << ' ';
    }
    for (size_t i=mi; i<maxr; ++i) {
      os << "NaN ";
    }
  }

}


void FCorrelationStatistic::Display() const
{
  GGl.Overlay->Display(PFME(Correlation).title.c_str());
}


void FCorrelationStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  const size_t maxr = static_cast<size_t>(PFME(Correlation).p[0]);
  os << "% Correlations of fluctuations\n"
    << "% Indices of X:\n"
    << "iN = 1;                 % subflock size\n"
    << "isamples = 2;           % samples\n"
    << "iL = 3:4;               % L [mean, variance]\n"
    << "iV = 5:6;               % Volume [mean, variance]\n"
    << "iCv = 7:" << 7+maxr-1 << ";             % Cv(r)\n"
    << "iCp = " << 7+maxr << ':' << 7+2*maxr-1 << ";           % Cp(r)\n"
    << "iCsp =" << 7+2*maxr << ':' << 7+3*maxr-1 << ";          % Csp(r)\n\n";
  os << "X = [\n";
  storage_type::const_iterator first(storage_.begin());
  storage_type::const_iterator last(storage_.end());
  for (; first != last; ++first)
  {
    os << (*first).second.flockSize << ' '
      << (*first).second.samples << ' '
      << (*first).second.L.mean() << ' '
      << (*first).second.L.variance() << ' '
      << (*first).second.Volume.mean() << ' '
      << (*first).second.Volume.variance() << ' ';
    print_means(os, (*first).second.Cv, maxr);
    print_means(os, (*first).second.Cp, maxr);
    print_means(os, (*first).second.Csp, maxr);
    os << '\n';
  }
  os << "];" << std::endl;
}
