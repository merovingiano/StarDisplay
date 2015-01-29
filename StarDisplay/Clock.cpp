#include "glmfwd.hpp"
#include <glmutils/hd_timer.hpp>
#include "Clock.hpp"


glmutils::HDTimer gTimer;


double GlobalTimerNow() 
{ 
  return gTimer.elapsed(); 
}


double GlobalTimerSince(double lastTime) 
{ 
  double now = gTimer.elapsed();
  return now - lastTime;
}


double GlobalTimerSinceReplace(double& lastTime) 
{ 
  double now = gTimer.elapsed();
  double tmp = lastTime;
  return (lastTime = now) - tmp;
}


double GlobalTimerSinceCopy(double lastTime, double& now)
{
  now = gTimer.elapsed();
  return now - lastTime;
}


void GlobalTimerRestart() 
{ 
  gTimer.restart(); 
}


