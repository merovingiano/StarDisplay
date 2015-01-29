#ifndef TIMESERIESSTATISTICS_HPP_INCLUDED
#define TIMESERIESSTATISTICS_HPP_INCLUDED


#include "DefaultStatistic.hpp"
#include "histogram.hpp"


class TimeSeriesStatistic : public DefaultStatistic
{
public:
  TimeSeriesStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual std::string labelText() const;
  virtual void save(const char* fname, bool append) const;
  virtual void Display() const;

private:
  typedef histogram::quartiles_t quartiles_t;

  std::vector<double>       time_;
  std::vector<double>       volume_;
  std::vector<glm::dvec3>   pos_;
  std::vector<glm::dvec3>   vel_;
  std::vector<glm::dvec3>   I123_;
  std::vector<glm::dvec3>   Ext_;
  std::vector<int>          numFlocks_;
  std::vector<quartiles_t>  FlockSize_;
  std::vector<int>          attacks_;
  std::vector<int>          locks_;
  std::vector<int>          catches_;
  std::vector<double>       balanceShift_;
  std::vector<quartiles_t>  banking_;
  std::vector<quartiles_t>  vario_;
  std::vector<quartiles_t>  nndB_;
  std::vector<quartiles_t>  nndI_;
  std::vector<quartiles_t>  nnd_;
  std::vector<quartiles_t>  localD_;
  std::vector<quartiles_t>  lift_;
  std::vector<quartiles_t>  polarization_;
  std::vector<quartiles_t>  frobenius_;
  std::vector<quartiles_t>  bearing_;
};


#endif  //TIMESERIESSTATISTICS_HPP_INCLUDED
