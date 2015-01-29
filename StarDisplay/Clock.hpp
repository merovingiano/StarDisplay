#ifndef CLOCK_INCLUDED
#define CLOCK_INCLUDED


// Returns the current time elapsed is seconds.
double GlobalTimerNow();

// Returns the time interval elapsed since lastTime.
double GlobalTimerSince(double lastTime);

// Returns the time interval elapsed since lastTime.
// lastTime is replaced by the current time.
double GlobalTimerSinceReplace(double& lastTime);

// Returns the time interval elapsed since lastTime.
// Copies the current time into \c now.
double GlobalTimerSinceCopy(double lastTime, double& now);

// Sets the current time to zero.
void GlobalTimerRestart();


#endif  // CLOCK_INCLUDED
