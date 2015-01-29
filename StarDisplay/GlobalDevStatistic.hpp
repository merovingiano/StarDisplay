//! \file GlobalDevStatistic.hpp Global deviations.
//! \ingroup Analysis

#ifndef GLOBALDEVSTATISTIC_HPP_INCLUDED
#define GLOBALDEVSTATISTIC_HPP_INCLUDED

#include "HistogramStatistic.hpp"
#include "accumulators.hpp"


class GlobalDevStatistic : public HistogramStatistic
{
  typedef std::pair< accumulator<float>, accumulator<float> > idata_type;

public:
  GlobalDevStatistic(Param::FeatureMap::FMappings mapping);
  virtual void apply(double stat_dt);
  virtual void Reset();
  virtual void Display() const;
  virtual void save(const char* fname, bool append) const;

private:
  std::vector<idata_type> idata_;  // individual time series (dev, Ci)
  std::vector<double>     time_;
  std::vector<double>     L_;
  int maxSamples_;
};


#endif  //GLOBALDEVSTATISTIC_HPP_INCLUDED
