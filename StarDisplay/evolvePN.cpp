#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <glmutils/ostream.hpp>
#include <glmutils/random.hpp>

#include "random.hpp"
#include "Flock.hpp"
#include "IText.hpp"
#include "ICamera.hpp"
#include "GLSLState.hpp"
#include "Params.hpp"
#include "visitors.hpp"
#include "Globals.hpp"
#include <string>
#include "Simulation.hpp"
#include "Globals.hpp"
#include <iostream>
#include <sstream>
#include <time.h>
#include <iomanip> 
#include "Params.hpp"
#include <iostream>
#include "EvolvePN.hpp"
#include <boost/filesystem.hpp>

using namespace Param;

typedef std::vector<float> one_allele;
namespace {

	struct cmp_min_dist
	{
		bool operator () (const one_allele a, const one_allele b) const
		{
			return a[a.size() - 1] < b[b.size() - 1];
		}
	};

}


void SetGetAlleles(one_allele& allele, CFlock::pred_iterator pred, int type)
{

}

EvolvePN::EvolvePN()
	: DefaultStatistic()
{
	Reset();
}


void EvolvePN::Reset()
{
	Generation_ = 0;
	alleles_.clear();
	allPandS_.clear();
	allPandSPrey_.clear();
	namesParameters_.clear();
	save_data_.clear();
	names_.clear();
	ValuesParameters_.clear();
	StringParameters_.clear();


	if (Sim.experiments.empty())
	{
		Sim.expNumb = 0;
	}
	else
	{
		Sim.expNumb++;
        if (Sim.expNumb > Sim.experiments.size()) AppWindow.PostMessage(WM_CLOSE); 
		CFlock::pred_iterator firstPred(GFLOCKNC.predator_begin());
		CFlock::pred_iterator lastPred(GFLOCKNC.predator_end());
		CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());
		CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());
		
		Param::Params p = Sim.experiments[Sim.expNumb-1].param;
		Sim.SetParams(p);
		for (; firstPred != lastPred; ++firstPred)
		{
			firstPred->SetPredParams(Sim.experiments[Sim.expNumb - 1].pred);
			firstPred->SetBirdParams(Sim.experiments[Sim.expNumb - 1].predBird);
		}
		for (; firstPrey != lastPrey; ++firstPrey)
		{
			firstPrey->SetPreyParams(Sim.experiments[Sim.expNumb - 1].prey);
			firstPrey->SetBirdParams(Sim.experiments[Sim.expNumb - 1].preyBird);
		}
		std::cout << "\n Experiment number: " << Sim.expNumb;
	}
	

	
}



void EvolvePN::apply()
{
	if (Sim.expNumb == 0) Reset();
	if (alleles_.empty())
	{
		alleles_.emplace_back();
		allPandS_.emplace_back();
		allPandSPrey_.emplace_back();
	}	
		Shuffle();

}

void EvolvePN::loadPositions(const char* fname) const
{
	std::vector <glm::vec4> data;
	std::ifstream infile(fname);
	CFlock::prey_iterator first = GFLOCKNC.prey_begin();
	first->externalPos.clear();
	while (!infile.eof())
	{
		glm::vec4 vector;
		for (int i = 0; i < 4; i++)
		{

			infile >> vector[i];
			std::cout<< "\n" << vector[i];
		}
		
		data.push_back(vector);
		first->externalPos.push_back(vector);
	}
	
	
}



void EvolvePN::save(const char* fname, bool append) const
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d-%m-%Y", &tstruct);
	

	std::string bufS("D:/ownCloud/2013-2014/phd\ hunting/dataStarDisplay/");
	bufS.append(buf);
	bufS.append("/");
	std::string luaName("experiment.lua");
	std::string fnameTrunc(std::string(fname).substr(0, std::string(fname).find(".txt")));


	CreateDirectory(bufS.c_str(), NULL);
	CopyFile("../../experiments.lua", (bufS + fnameTrunc + luaName).c_str(), TRUE);
	CopyFile((Sim.experiments[Sim.expNumb - 1].param.birds.csv_file_prey_predator_settings).c_str(), (bufS + fnameTrunc + "pred_prey.csv").c_str(), TRUE);
	CopyFile((Sim.experiments[Sim.expNumb - 1].param.birds.csv_file_species).c_str(), (bufS + fnameTrunc + "species.csv").c_str(), TRUE);


	bufS.append(fname);
	
	std::ofstream os(bufS.c_str(), std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	os.precision(7);
	const char* fname2 = "trajectoryPredator.txt";
	const char* fname3 = "trajectoryPrey.txt";
	std::ofstream os2(fname2, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	os2.precision(7);
	std::ofstream os3(fname3, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	CFlock::pred_iterator first(GFLOCKNC.predator_begin());

	os << "#" << Sim.Params().evolution.title << "\n";
	os << "#" << Sim.Params().evolution.description << "\n";
	os << "#";
	for (size_t i = 0; i < namesParameters_.size(); ++i)
	{
		os << namesParameters_[i] << " ";
	}
	os << "\n#";
	for (size_t i = 0; i < ValuesParameters_.size(); ++i)
	{
		os << ValuesParameters_[i] << " ";
	}
	for (size_t i = 0; i < StringParameters_.size(); ++i)
	{
		os << StringParameters_[i] << " ";
	}
	os << "\n";
	for (size_t i = 0; i < names_.size(); ++i)
	{
		os << names_[i] << " ";
	}

	os << "\n";
	for (size_t i = 0; i<save_data_.size(); ++i)
	{

		for (size_t ii = 0; ii < save_data_[i].size(); ++ii)
		{
			for (size_t iii = 0; iii < save_data_[i][ii].size(); ++iii)
			{
				os << save_data_[i][ii][iii] << " ";
			}
			os << "\n";
		}
	}

	for (size_t i = 0; i < allPandS_.size(); ++i)
	{
	
		std::string buf(" ");
		buf.append(std::to_string(i));
		buf.append(" \n");

		std::ostream_iterator<glm::vec4> oit2(os2, buf.c_str());
		std::copy(allPandS_[i].begin(), allPandS_[i].end(), oit2);

		std::ostream_iterator<glm::vec4> oit3(os3, buf.c_str());
		std::copy(allPandSPrey_[i].begin(), allPandSPrey_[i].end(), oit3);

	}

	//if (!Sim.experiments.empty()) std::cout << "\n and is it working here..?  " << Sim.experiments[0].param.evolution.fileName;
}

void EvolvePN::PrepareSave()
{
	
}

void EvolvePN::Shuffle()
{
	//next generation starts here
	++Generation_;
	PrepareSave();
	// just a vector of type vec4 having three deflection parameters and the min distance
	allele_type allele;
	//looping over the predators and placing all 
	float meanN = 0;
	float meanStartAltitude = 0;
	float meanXDist = 0;

	
	Param::Evolution evol;
	
	CFlock::pred_iterator thefirst(GFLOCKNC.predator_begin());
	CFlock::pred_iterator thelast(GFLOCKNC.predator_end());
	for (; thefirst != thelast; ++thefirst)
	{	
		one_allele anAllele;
		SetGetAlleles(anAllele, thefirst, 1);
		allele.push_back(anAllele);
	};


	//resort on having the minimum distance
	std::sort(allele.begin(), allele.end(), cmp_min_dist());
	// placing all alleles into the total bunch of alleles over time
	alleles_.emplace_back(allele);
	
	//how big is the total amount of alleles in the population?
	unsigned N = static_cast<unsigned>(allele.size());

	// 50% overwritten + mutation
	std::copy(allele.begin(), allele.begin() + (N >> 1), allele.begin() + (N >> 1));
	//std::uniform_real_distribution<float> rnd(-0.5f, 0.5f);
	// or alternatively
	//std::cauchy_distribution<float> cauchy_dist(a, b);
	std::normal_distribution<float> norm_dist(0, 1);

	//each generation the mutation becomes less..
	for (unsigned i = (N >> 1); i < N; ++i)
	{

		for (int ii = 0; ii < allele[i].size(); ii++)
		{
			//allele[i][ii] += (1.0f / Generation_) * rnd(rnd_eng()) * 5 + rnd(rnd_eng()) *allele[i][ii]/10.0f;
			allele[i][ii] += (1.0f / Generation_) * norm_dist(rnd_eng()) * 5 + norm_dist(rnd_eng()) *allele[i][ii] / 10.0f;
		}
	}
	CFlock::pred_iterator first(GFLOCKNC.predator_begin());
	CFlock::pred_iterator last(GFLOCKNC.predator_end());

	CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());


	allPandS_.emplace_back(first->positionsAndSpeed);
	first->positionsAndSpeed.clear();

	allPandSPrey_.emplace_back(firstPrey->positionsAndSpeed);
	firstPrey->positionsAndSpeed.clear();

	//change all of the settings of the predators after mutation
	for (unsigned i = 0; first != last; ++first, ++i)
	{
		SetGetAlleles(allele[i], first,2);
	}
	// Average of top 1%
	unsigned n = unsigned(double(allele.size()) * 0.01);
	one_allele top1;
	
	for (int ii = 0; ii < allele[0].size(); ++ii)
	{

		top1.push_back(0.0f);
	}
	for (unsigned i = 0; i<n; ++i)
	{
		for (int ii = 0; ii < top1.size(); ++ii)
		{
			top1[ii] += allele[i][ii];
		}
	}
	for (int ii = 0; ii < top1.size(); ++ii)
	{
		top1[ii] /= n;
	}


	first = GFLOCKNC.predator_end() - n;
	//set the the last n to the average of the top 1%, This is possibly wrong, you want the sorted vector to be adapted
	//for (; first != last; ++first)
	//{
	//	SetGetAlleles(top1, first, 2);
	//}
	const float R = PROOST.Radius;

	first = GFLOCKNC.predator_begin();
	//std::cout << "\n LALALALALALA";
	for (; first != last; ++first)
	{
		
	
	};
	if (Sim.Params().evolution.externalPrey)
	{
			loadPositions(Sim.Params().evolution.externalPreyFile.c_str());
	}

	
	CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());

	for (; firstPrey != lastPrey; ++firstPrey)
	{
		firstPrey->position_ = glm::vec3(0, 120, 0);
		firstPrey->B_[0] = glmutils::vec3_in_sphere(rnd_eng());
		// reset the couunter to compute the averages
		firstPrey->velocity_ = 20.0f * firstPrey->B_[0];
		firstPrey->SetSpeed(20.0f);
	};

	GFLOCKNC.meanN = meanN;
	GFLOCKNC.meanStartAltitude = meanStartAltitude;
	GFLOCKNC.meanXDist = meanXDist;

	if (Generation_ >= Sim.Params().evolution.terminationGeneration) Reset();
}


