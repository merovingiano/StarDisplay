//! \file DefaultStatistic.hpp Default (nop) Statistics
//! \ingroup Analysis

#ifndef DEFAULT_STATISTIC_HPP_INCLUDED
#define DEFAULT_STATISTIC_HPP_INCLUDED

#include "IStatistic.hpp"


//! DefaultStatistic class.
//! Empty implementation of all members.
//! \ingroup Statistics Visualization
class DefaultStatistic : public IStatistic
{
public:
  DefaultStatistic();
  virtual ~DefaultStatistic();

  virtual void finalize();

  //! Resets statistics.
  virtual void Reset();        

  //! Apply the statistic.
  //! Called once per frame.
  //! \note Called from a non-rendering thread. 
  virtual void apply(double stat_dt);

  //! Chance to return some information for 2D annotation.
  virtual std::string labelText() const;

  //! Onscreen Display.
  virtual void Display() const;

  //! Prints results to file.
  virtual void save(const char* fname, bool append) const;
};


#endif
