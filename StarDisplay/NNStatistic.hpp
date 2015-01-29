#ifndef NNSTATISTICS_HPP_INCLUDED
#define NNSTATISTICS_HPP_INCLUDED


#include "DefaultStatistic.hpp"
#include "histogram.hpp"
#include "accumulators.hpp"


//! Records distances for the N=20 nearest neighbors.
class NNStatistic : public DefaultStatistic
{
public:
  NNStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual std::string labelText() const;
  virtual void save(const char* fname, bool append) const;

private:
  std::vector<histogram> nnd_;
  std::vector<accumulator<double> > acc_;
};


#endif  //NNSTATISTICS_HPP_INCLUDED
