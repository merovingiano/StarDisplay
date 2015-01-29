//! \file ClusterStatistics.hpp Common measurements on sub-flocks.
//! \ingroup Analysis

#ifndef CLUSTERSTATISTICS_HPP_INCLUDED
#define CLUSTERSTATISTICS_HPP_INCLUDED


#include <map>
#include <vector>
#include "HistogramStatistic.hpp"
#include "visitors.hpp"
#include "Params.hpp"


class SubflockStatistic : public HistogramStatistic
{
public:
  SubflockStatistic();
  virtual void apply(double stat_dt);
  virtual std::string labelText() const;
  virtual void Display() const;
};


class SubflockPCAStatistic : public HistogramStatistic
{
public:
  SubflockPCAStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual void save(const char* fname, bool append) const;
  virtual void Display() const;

private:
  struct storage_entry
  {
    storage_entry(const Param::FeatureMap::Entries_type& fme, size_t flockSize)
    : flockSize_(flockSize),
      samples_(0),
      nnd_(fme),
      nndI_(fme),
      nndB_(fme),
      localD_(fme),
      polarization_(fme),
      speed_(fme),
      banking_(fme),
      LW_(0,5,100),
      I1_(0,50, 100),
      I3I1_(0,5,100),
      I3I2_(0,5,100),
      angleFI3_(0, 1, 100),
      balance_(-0.1,0.1,200),
      hide_(fme)
    {}
    bool operator<(const storage_entry& other) const { return flockSize_ < other.flockSize_; }

    size_t flockSize_;
    size_t samples_;
    hist_visitor<Param::FeatureMap::NND> nnd_;
    hist_visitor<Param::FeatureMap::NND> nndI_;
    hist_visitor<Param::FeatureMap::NND> nndB_;
    hist_visitor<Param::FeatureMap::LocalDensity> localD_;
    hist_visitor<Param::FeatureMap::Polarization> polarization_;
    hist_visitor<Param::FeatureMap::Speed> speed_;
    hist_visitor<Param::FeatureMap::Banking> banking_;
    histogram LW_;
    histogram I1_;
    histogram I3I1_;
    histogram I3I2_;
    histogram balance_;
    histogram angleFI3_; 
    accumulator<double> volume_;
    hist_visitor<Param::FeatureMap::AlertnessRelexation> hide_;
  };
  typedef std::map<size_t, storage_entry> storage_type;
  storage_type           storage_;
  std::vector<glm::mat4> LWM_;
  std::vector<glm::vec3> LWH_;
  std::vector<glm::vec3> Velocity_;
  std::vector<glm::vec3> I123_;
  std::vector<glm::mat4> PCAM_;
  double                 last_display_;
};



#endif  //CLUSTERSTATISTICS_HPP_INCLUDED
