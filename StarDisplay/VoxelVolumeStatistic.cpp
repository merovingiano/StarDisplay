#include "VoxelVolumeStatistic.hpp"
#include "HistOverlay.hpp"
#include "GLSLState.hpp"
#include "GLSLImm.hpp"
#include "Flock.hpp"
#include "Params.hpp"
#include "Globals.hpp"


using namespace Param;


VoxelVolumeStatistic::VoxelVolumeStatistic()
:  HistogramStatistic(FeatureMap::VoxelVolume),
   vox_(PFME(VoxelVolume).p[0])
{
  Reset();
}


void VoxelVolumeStatistic::apply(double stat_dt)
{
  HistogramStatistic::apply(stat_dt);   // default color
  vox_.reset();
  std::for_each(GFLOCK.prey_begin(), GFLOCK.prey_end(), [this] (const CBird& bird)
  {
    vox_(bird.position());
  });
  log_(static_cast<float>(vox_.volume()));
}


std::string VoxelVolumeStatistic::labelText() const
{
  char buf[128];
  _snprintf_s(buf, 127, "%s (%6.5g)", 
    PFME(VoxelVolume).title.c_str(),
    vox_.volume());
  return std::string(buf);
}


void VoxelVolumeStatistic::Display() const
{
  HistogramStatistic::Display();
  const float rh = 0.5f * vox_.floor_fact();
  std::for_each(vox_.rpos().begin(), vox_.rpos().end(), [this,rh] (const glm::vec3& v) 
  {
    GGl.imm3D->Box(v-rh, v+rh, 0.0f);
  });
}


