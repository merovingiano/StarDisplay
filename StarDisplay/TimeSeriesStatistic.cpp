#include <fstream>
#include <glmutils/ostream.hpp>
#include "HistOverlay.hpp"
#include "GLSLState.hpp"
#include "TimeSeriesStatistic.hpp"
#include "bounding_box.hpp"
#include "accumulators.hpp"
#include "voxel_volume.hpp"
#include "Flock.hpp"
#include "visitors.hpp"
#include "Simulation.hpp"


using namespace Param;


TimeSeriesStatistic::TimeSeriesStatistic()
: DefaultStatistic()
{
  Reset();
}


void TimeSeriesStatistic::Reset()
{
  time_.clear();
  volume_.clear();
  pos_.clear();
  vel_.clear();
  I123_.clear();
  numFlocks_.clear();
  FlockSize_.clear();
  attacks_.clear();
  locks_.clear();
  catches_.clear();
  Ext_.clear();
  nndB_.clear();
  nndI_.clear();
  nnd_.clear();
  localD_.clear();
  banking_.clear();
  lift_.clear();
  polarization_.clear();
  frobenius_.clear();
  bearing_.clear();
}


void TimeSeriesStatistic::apply(double stat_dt)
{
  
  CFlock::prey_const_iterator first(GFLOCK.prey_begin());
  CFlock::prey_const_iterator last(GFLOCK.prey_end());

  time_.push_back(Sim.SimulationTime());
  voxel_volume vox(PFME(VoxelVolume).p[0]);
  bounding_box bb;
  for (CFlock::prey_const_iterator it(first); it < last; ++it)
  {
    vox((*it).position());
    bb(*it);
  }
  volume_.push_back(vox.volume());
  I123_.push_back(bb.pca_I123());
  Ext_.push_back(bb.lw_extent());
  numFlocks_.push_back(GFLOCK.num_clusters());
  pos_.push_back(bb.centerOfMass());
  balanceShift_.push_back(bb.pca_balanceShift());
  vel_.push_back(bb.velocity());

  histogram hFlockSize(PFME(Subflock).hist);
  std::for_each(GFLOCK.clusters_begin(), GFLOCK.clusters_end(), [&hFlockSize] (const cluster_entry& ce) 
  {
    hFlockSize(ce.size, ce.size);
  });
  FlockSize_.push_back(hFlockSize.quartiles());

  int n_attack = 0;
  int n_locked = 0;
  int n_catches = 0;
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&n_attack, &n_locked, &n_catches] (const CPredator& pred) 
  {
    if (pred.is_attacking()) n_attack += 1;
    if (pred.GetLockedOn()) n_locked += 1;
    n_catches += pred.hunts().success;
  });
  attacks_.push_back(n_attack);
  locks_.push_back(n_locked);
  catches_.push_back(n_catches);

  histogram hbank(PFME(Banking).hist);
  histogram hnndB(PFME(NND).hist);
  histogram hnndI(PFME(NND).hist);
  histogram hnnd(PFME(NND).hist);
  histogram hlocalD(PFME(LocalDensity).hist);
  histogram hvario(PFME(Vario).hist);
  histogram hlift(PFME(EffectiveLift).hist);
  histogram hpolarization(PFME(Polarization).hist);
  histogram hfrobenius(PFME(Frobenius).hist);
  histogram hbearing(PFME(Bearing3D).hist);

  for (; first != last; ++first) 
  {
    hbank(visit<FeatureMap::Banking>(*first));
    hlift(visit<FeatureMap::EffectiveLift>(*first));
    float nnd = visit<FeatureMap::NND>(*first);  
    hnnd(nnd);
    if (true_val == visit<FeatureMap::Border>(*first))  
      hnndB(nnd);
    else
      hnndI(nnd);
    hlocalD(visit<FeatureMap::LocalDensity>(*first));
    hvario(visit<FeatureMap::Vario>(*first));
    hpolarization(visit<FeatureMap::Polarization>(*first));
    hfrobenius(visit<FeatureMap::Frobenius>(*first));
  }
  nndB_.push_back(hnndB.quartiles());
  nndI_.push_back(hnndI.quartiles());
  nnd_.push_back(hnnd.quartiles());
  localD_.push_back(hlocalD.quartiles());
  banking_.push_back(hbank.quartiles());
  vario_.push_back(hvario.quartiles());
  lift_.push_back(hlift.quartiles());
  polarization_.push_back(hpolarization.quartiles());
  frobenius_.push_back(hfrobenius.quartiles());
  bearing_.push_back(hbearing.quartiles());
}


std::string TimeSeriesStatistic::labelText() const
{
  char buf[128];
  _snprintf_s(buf, 127, "%s", PFME(TimeSeries).title.c_str());
  return std::string(buf);
}


void TimeSeriesStatistic::Display() const 
{ 
  GGl.Overlay->Display(labelText().c_str()); 
}



namespace {

  template<typename C>
  void saveVect(std::ostream& os, const std::string& what, const C& coll)
  {
    std::ostream_iterator<typename C::value_type> osit(os, ";");
    os << what << " = [ "; std::copy(coll.begin(), coll.end(), osit); os << " ];\n";
  }

}


void TimeSeriesStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  saveVect(os, "T", time_);
  saveVect(os, "nnd", nnd_);
  saveVect(os, "nndI", nndI_);
  saveVect(os, "nndB", nndB_);
  saveVect(os, "localD", localD_);
  saveVect(os, "Pos", pos_);
  saveVect(os, "Volume", volume_);
  saveVect(os, "Bank", banking_);
  saveVect(os, "EffLift", lift_);
  saveVect(os, "Extent", Ext_);
  saveVect(os, "numFlocks", numFlocks_);
  saveVect(os, "FlockSize", FlockSize_);
  saveVect(os, "attacks", attacks_);
  saveVect(os, "locks", locks_);
  saveVect(os, "catches", catches_);
  saveVect(os, "I123", I123_);
  saveVect(os, "Polarization", polarization_);
  saveVect(os, "Frobenius", frobenius_);
  saveVect(os, "Bearing", bearing_);
  }





