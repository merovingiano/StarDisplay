//! \file Statistics.hpp Voxel volume statistics
//! \ingroup Analysis

#ifndef VOXELVOLUMESTATISTIC_HPP_INCLUDED
#define VOXELVOLUMESTATISTIC_HPP_INCLUDED

#include "HistogramStatistic.hpp"
#include "voxel_volume.hpp"


//! Records voxel volume of the flocks
//! \ingroup Statistics Visualization
class VoxelVolumeStatistic : public HistogramStatistic
{
public:
	VoxelVolumeStatistic();
	virtual void apply(double stat_dt);
	virtual void Display() const;
	virtual std::string labelText() const;

private:
	voxel_volume vox_;
};


#endif
