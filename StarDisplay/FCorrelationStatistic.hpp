//! \file FCorrelationStatistic.hpp Correlation of fluctuations
//! \ingroup Analysis

#ifndef FCORRELATIONSTATISTIC_HPP_INCLUDED
#define FCORRELATIONSTAIISTIC_HPP_INCLUDED

#include <map>
#include <vector>
#include "DefaultStatistic.hpp"
#include "accumulators.hpp"


class FCorrelationStatistic : public DefaultStatistic
{
public:
  FCorrelationStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual void save(const char* fname, bool append) const;
  virtual void Display() const;

private:
  struct storage_entry
  {
    storage_entry(size_t size)
    :  flockSize(size),
      samples(0)
    {
    }
    
    size_t flockSize;
    size_t samples;
    accumulator<double> L;
    accumulator<double> Volume;
    std::vector< average<double> > Cv;
    std::vector< average<double> > Cp;
    std::vector< average<double> > Csp;
  };
  typedef std::map<size_t, storage_entry> storage_type;
  storage_type storage_;
};


#endif
