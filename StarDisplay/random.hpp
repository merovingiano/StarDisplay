#ifndef STARDISPLAY_RANDOM_HPP_INCLUDED
#define STARDISPLAY_RANDOM_HPP_INCLUDED

#include <random>


typedef std::mt19937 rnd_eng_type;


// Seeding
void rnd_seed(unsigned long seed);

float cauchy(void);

// Returns thread local random number engine
rnd_eng_type& rnd_eng();


#endif  // OMP_RANDOM_HPP_INCLUDED
