#ifndef QMSTATISTICS_HPP_INCLUDED
#define QMSTATISTICS_HPP_INCLUDED

#include <map>
#include <vector>
#include "DefaultStatistic.hpp"


static const int Qmp[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,340 };
static const int Qmpn = sizeof(Qmp) / sizeof(int);


//! Records neighbor reshuffling for the T=3 s.
class QmStatistic : public DefaultStatistic
{
public:
  QmStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual std::string labelText() const;
  virtual void save(const char* fname, bool append) const;

private:
  std::map<int, std::vector<int> > M0_;
  std::vector<double> Qm_[Qmpn];
  std::vector<double> T_;
};


#endif  //QMSTATISTICS_HPP_INCLUDED
