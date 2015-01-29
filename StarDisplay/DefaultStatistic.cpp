#include "DefaultStatistic.hpp"


DefaultStatistic::DefaultStatistic()
  : IStatistic()
{
}


DefaultStatistic::~DefaultStatistic()
{
}


void DefaultStatistic::finalize()
{
}


void DefaultStatistic::Reset()
{
}


void DefaultStatistic::apply(double stat_dt)
{
}


std::string DefaultStatistic::labelText() const
{
  return std::string();
}


void DefaultStatistic::Display() const
{
}


void DefaultStatistic::save(const char* fname, bool append) const
{
}


