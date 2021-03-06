#ifndef EVOLVEPN_HPP_INCLUDED
#define EVOLVEPN_HPP_INCLUDED

#include <vector>
#include "glmfwd.hpp"
#include "DefaultStatistic.hpp"
#include <string>
#include <iterator>



class EvolvePN : public DefaultStatistic
{
public:
	EvolvePN();
	virtual void Reset();
	virtual void apply();
	virtual void loadFiles();
	virtual void EvolvePN::loadOldData(std::string filename);
	virtual void save(const char* fname, bool append) const;
	virtual void loadPositions(const char* fname) const;
	int getGeneration()	{ return Generation_; };

private:
	void Shuffle();
	void PrepareSave(); 
	double lastShuffle_;
	int Generation_;
	
	

	  
	typedef std::vector<float> one_allele;
	typedef std::vector<one_allele>   allele_type;
	typedef std::vector<glm::vec4>   allele_type2;
	typedef std::vector<allele_type2> alleles_vect2;
	typedef std::vector<allele_type> alleles_vect;
	alleles_vect alleles_;
	std::vector<allele_type2> positionsAndSpeed_;
	alleles_vect2 allPandS_;
	alleles_vect2 allPandSPrey_;
	typedef std::vector<float> data_per_pred;
	typedef std::vector<data_per_pred> data_all_pred;
	typedef std::vector<data_all_pred> data_all_generations;
	data_all_generations save_data_;
	std::vector<std::string> names_;
	std::vector<std::string> namesParameters_;
	std::vector<float> ValuesParameters_;
	std::vector<std::string> StringParameters_;
};


#endif
