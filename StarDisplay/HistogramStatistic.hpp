//! \file HistogramStatistic.hpp Simple color mapping.
//! \ingroup Analysis

#ifndef HISTOGRAM_STATISTIC_HPP_INCLUDED
#define HISTOGRAM_STATISTIC_HPP_INCLUDED

#include "DefaultStatistic.hpp"
#include <vector>
#include "histogram.hpp"
#include "Params.hpp"


//! Histogram statistic class.
//! Simple single variable histogram logging.
//! \ingroup Statistics Visualization
class HistogramStatistic : public DefaultStatistic
{
public:
  HistogramStatistic(Param::FeatureMap::FMappings mapping);
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual std::string labelText() const;
  virtual void Display() const;
  virtual void save(const char* fname, bool append) const;

protected:
  Param::FeatureMap::FMappings mapping_;
  Param::FeatureMap::Entry fme_;
  std::vector<float> values_;
  histogram log_;
};


#endif
