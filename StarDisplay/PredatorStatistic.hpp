#ifndef PREDATORSTATISTIC_HPP_INCLUDED
#define PREDATORSTATISTIC_HPP_INCLUDED

#include <vector>
#include "glmfwd.hpp"
#include "DefaultStatistic.hpp"


//! Records distances for the N=20 nearest neighbors.
class PredatorStatistic : public DefaultStatistic
{
public:
  PredatorStatistic();
  virtual void Reset();
  virtual void Display() const;
  virtual void save(const char* fname, bool append) const;
};


//! Video comparison.
//! Single predator
class PredatorVidStatistic : public DefaultStatistic
{
public:
  PredatorVidStatistic();
  virtual void Reset();
  virtual void apply(double stat_dt);
  virtual void save(const char* fname, bool append) const;
  virtual std::string labelText() const;
  
private:
  std::vector<double> time;
  std::vector<int>    lockedOn;
  std::vector<float>  banking;   
  std::vector<float>  speed;
  std::vector<float>  distance;
  std::vector<glm::vec3>                predPos; 
  std::vector< std::vector<glm::vec3> > preyPos;    // prey position   
  std::vector< std::vector<glm::vec3> > project;    // projected coordinates
  int catched;
};




#endif

