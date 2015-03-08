#ifndef EVOLVEPN_HPP_INCLUDED
#define EVOLVEPN_HPP_INCLUDED

#include <vector>
#include "glmfwd.hpp"
#include "DefaultStatistic.hpp"


class EvolvePN : public DefaultStatistic
{
public:
	EvolvePN();
	virtual void Reset();
	virtual void apply(double stat_dt);
	virtual void save(const char* fname, bool append) const;
	virtual void Display() const;

private:
	void Shuffle();
	double lastShuffle_;
	int Generation_;

	typedef std::vector<glm::vec4>   allele_type;      // Deflection, minDist
	typedef std::vector<allele_type> alleles_vect;
	alleles_vect alleles_;
};


#endif
