#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <glmutils/ostream.hpp>
#include <glmutils/random.hpp>
#include "EvolvePN.hpp"
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

using namespace Param;


namespace {

	struct cmp_min_dist
	{
		bool operator () (const glm::vec4& a, const glm::vec4& b) const
		{
			return a.w < b.w;
		}
	};

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
}


void EvolvePN::apply()
{
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
	std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	const char* fname2 = "trajectoryPredator.txt";
	const char* fname3 = "trajectoryPrey.txt";
	std::ofstream os2(fname2, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	std::ofstream os3(fname3, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
	CFlock::pred_iterator first(GFLOCKNC.predator_begin());

	for (size_t i = 0; i<alleles_.size(); ++i)
	{
		std::string buf(" ");
		buf.append(std::to_string(i));
		buf.append(" \n");
		std::ostream_iterator<glm::vec4> oit(os, buf.c_str());
		std::copy(alleles_[i].begin(), alleles_[i].end(), oit);
		
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

	os << '\n';
	os2 << '\n';

}



void EvolvePN::Shuffle()
{
	//next generation starts here
	++Generation_;
	// just a vector of type vec4 having three deflection parameters and the min distance
	allele_type allele;
	//looping over the predators and placing all 
	float meanN = 0;
	float meanStartAltitude = 0;
	float meanXDist = 0;
	std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&allele](const CPredator& pred)
	{
		float N = pred.get_N();	
		allele.push_back(glm::vec4(N, pred.getStartAltitude(), pred.getStartXDist(), pred.hunts().minDist));
	});
	//resort on having the minimum distance
	std::sort(allele.begin(), allele.end(), cmp_min_dist());
	// placing all alleles into the total bunch of alleles over time
	alleles_.emplace_back(allele);
	
	//how big is the total amount of alleles in the population?
	unsigned N = static_cast<unsigned>(allele.size());

	// 50% overwritten + mutation
	std::copy(allele.begin(), allele.begin() + (N >> 1), allele.begin() + (N >> 1));
	std::uniform_real_distribution<float> rnd(-0.5f, 0.5f);
	//each generation the mutation becomes less..
	for (unsigned i = (N >> 1); i < N; ++i)
	{
		allele[i].x += (1.0f / Generation_) * rnd(rnd_eng());
		
		allele[i].y += (1.0f / Generation_) * rnd(rnd_eng())*10; // more variation in x and y
		allele[i].z += (1.0f / Generation_) * rnd(rnd_eng())*10;
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
		first->set_N(allele[i][0]);
		first->setStartAltitude(allele[i][1]);
		first->setStartXDist(allele[i][2]);
		first->setGeneration(Generation_);

		
	}
	// Average of top 1%
	unsigned n = unsigned(double(allele.size()) * 0.01);
	glm::vec4 top1(0);
	for (unsigned i = 0; i<n; ++i)
	{
		top1 += allele[i];
	}
	top1 /= n;

	first = GFLOCKNC.predator_end() - n;
	//set the the last n to the average of the top 1%, This is possibly wrong, you want the sorted vector to be adapted
	for (; first != last; ++first)
	{
		first->set_N(top1[0]);
		first->setStartAltitude(top1[1]);
		first->setStartXDist(top1[2]);
	}
	const float R = PROOST.Radius;

	first = GFLOCKNC.predator_begin();
	//std::cout << "\n LALALALALALA";
	for (; first != last; ++first)
	{
		if (Generation_ == 1)
		{
			first->set_N((float(rand()) / (float(RAND_MAX) / 200.0f) ) - 100.0f);
			first->setStartAltitude(float(rand()) / (float(RAND_MAX) / (100.0f * 6.0f)));
			first->setStartXDist(float(rand()) / (float(RAND_MAX) / (100.0f * 6.0f)));
			first->setGeneration(Generation_);

			//temporary code to get the positions:
			//first->set_N(3.4f);
			//first->setStartAltitude(377.0f);
			//first->setStartXDist(150.0f);
		}

		first->ResetHunt();
		
		first->setTrail(false);
		first->position_ = 100.0f*glmutils::vec3_in_sphere(rnd_eng());
		first->position_.y += 120;
		if (Sim.Params().evolution.evolveAlt && !Sim.Params().evolution.evolveX)
		{
			first->position_.y = first->getStartAltitude();
			first->position_.x = rand();
			first->position_.z = rand();
			float length = glm::length(glm::vec2(first->position_.x, first->position_.z));
			first->position_.x /= length / 100.0f;
			first->position_.z /= length / 100.0f;
		}
		if (Sim.Params().evolution.evolveAlt && Sim.Params().evolution.evolveX)
		{
			first->position_.y = first->getStartAltitude();
			first->position_.x = first->getStartXDist();

		}
		if (!Sim.Params().evolution.evolveAlt && Sim.Params().evolution.evolveX)
		{
			first->position_.y = rand();
			first->position_.x = first->getStartXDist();;
			first->position_.z = rand();
			float length = glm::length(glm::vec2(first->position_.y, first->position_.z));
			first->position_.y /= length / 100.0f;
			first->position_.z /= length / 100.0f;
		}
		first->B_[0] = glmutils::vec3_in_sphere(rnd_eng());
		first->B_[0] /= glm::length(first->B_[0]);
		first->velocity_ = 20.0f * first->B_[0];




		first->setTrail(true);
		first->BeginHunt(); 
		//std::cout << "\npred num " << first->id();
		std::cout << "\nN :  " << first->get_N() << " startAltitude :  " << first->getStartAltitude() << " startXDist :  " << first->getStartXDist() << " Generation: " << first->getGeneration();
		//std::cout << "\n" << rnd(rnd_eng());
		meanN += first->get_N() * 1 / N;
		meanStartAltitude += first->getStartAltitude() * 1 / N;
		meanXDist += first->getStartXDist() * 1 / N;

	
	};
	if (Sim.Params().evolution.externalPrey)
	{
			loadPositions(Sim.Params().evolution.externalPreyFile.c_str());
	}

	GFLOCKNC.meanN = meanN;
	GFLOCKNC.meanStartAltitude = meanStartAltitude;
	GFLOCKNC.meanXDist = meanXDist;
}
