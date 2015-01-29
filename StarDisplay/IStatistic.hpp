//! \file IStatistic.hpp Statistics interface
//! \ingroup Analysis

#ifndef ISTATISTIC_HPP_INCLUDED
#define ISTATISTIC_HPP_INCLUDED

#include <iosfwd>
#include <string>


//! IStatistic class.
//! Abstract interface
class __declspec(novtable) IStatistic
{
public:
  virtual ~IStatistic() {}
  virtual void finalize() = 0;

  //! Resets statistics.
  virtual void Reset() = 0;        
  
  //! Apply the statistic.
  //! Called once per frame.
  //! \note Called from a non-rendering thread. 
  virtual void apply(double stat_dt) = 0;

  //! Chance to return some information for 2D annotation.
  virtual std::string labelText() const = 0;
  
  //! Onscreen Display.
  virtual void Display() const = 0;

  //! Prints results to file.
  virtual void save(const char* fname, bool append) const = 0;
};


#endif
