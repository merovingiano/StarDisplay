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

	Param::Predator predParam = pred->GetPredParams();
	Param::Bird bird = pred->GetBirdParams();
	int index = 0;
	if (type == 2) pred->setGeneration(Sim.evolution.getGeneration()); 
	if (type == 1) allele.push_back(pred->get_N()); else if (Sim.Params().evolution.evolvePN) pred->set_N(allele[index]); index++; 
	if (type == 1) allele.push_back(pred->getDPAdjParam()); else if (Sim.Params().evolution.evolveDPAdjParam) pred->setDPAdjParam(allele[index]); index++;
	if (type == 1) allele.push_back(pred->getStartAltitude()); else if (Sim.Params().evolution.evolveAlt) pred->setStartAltitude(allele[index]); index++; 
	if (type == 1) allele.push_back(pred->getStartXDist()); else if (Sim.Params().evolution.evolveX) pred->setStartXDist(allele[index]); index++; 
	if (type == 1) allele.push_back(pred->getStartZDist()); else if (Sim.Params().evolution.evolveZ) pred->setStartZDist(allele[index]); index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().CL); else if (Sim.Params().evolution.evolveCL) bird.CL = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().wingAspectRatio); else if (Sim.Params().evolution.evolvewingAspectRatio) bird.wingAspectRatio = allele[index]; index++;
	if (type == 1) allele.push_back(pred->GetBirdParams().maxForce); else if (Sim.Params().evolution.evolvemaxForce) bird.maxForce = allele[index]; index++;
	if (type == 1) allele.push_back(pred->GetBirdParams().wingSpan); else if (Sim.Params().evolution.evolvewingSpan) bird.wingSpan = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().bodyMass); else if (Sim.Params().evolution.evolvebodyMass) bird.bodyMass = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().controlCL); else if (Sim.Params().evolution.evolvecontrolCL) bird.controlCL = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().cruiseSpeed); else if (Sim.Params().evolution.evolvecruiseSpeed) bird.cruiseSpeed = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().maxLift); else if (Sim.Params().evolution.evolvemaxLift) bird.maxLift = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().maxSpeed); else 	if (Sim.Params().evolution.evolvemaxSpeed) bird.maxSpeed = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().rollRate); else 	if (Sim.Params().evolution.evolverollRate) bird.rollRate = allele[index]; index++;
	if (type == 1) allele.push_back(pred->GetBirdParams().minSpeed); else 	if (Sim.Params().evolution.evolveminSpeed) bird.minSpeed = allele[index]; index++;
	if (type == 1) allele.push_back(pred->GetBirdParams().reactionTime); else if (Sim.Params().evolution.evolvereactionTime) bird.reactionTime = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().alignmentWeight.x); else if (Sim.Params().evolution.evolvealignmentWeight) bird.alignmentWeight.x = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().alignmentWeight.y); else if (Sim.Params().evolution.evolvecohesionWeight) bird.alignmentWeight.y = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().cohesionWeight.x); else if (Sim.Params().evolution.evolvecohesionWeight) bird.cohesionWeight.x = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().cohesionWeight.y); else if (Sim.Params().evolution.evolvecohesionWeight) bird.cohesionWeight.y = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetBirdParams().cohesionWeight.z); else if (Sim.Params().evolution.evolvecohesionWeight) bird.cohesionWeight.z = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetPredParams().HandleTime); else if (Sim.Params().evolution.evolveHandleTime) predParam.HandleTime = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->GetPredParams().LockDistance); else if (Sim.Params().evolution.evolveLockDistance) predParam.LockDistance = allele[index]; index++; 
	if (type == 1) allele.push_back(pred->hunts().minDist); else
	{
		pred->SetPredParams(predParam);
		pred->SetBirdParams(bird);
	}
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
		CFlock::pred_iterator firstPred(GFLOCKNC.predator_begin());
		CFlock::pred_iterator lastPred(GFLOCKNC.predator_end());
		CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());
		CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());
		std::cout << "\n Starting Simulation " << Sim.expNumb;
		if (Sim.expNumb > Sim.experiments.size()) AppWindow.PostMessage(WM_CLOSE);
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
		CFlock::prey_iterator testfirstPrey(GFLOCKNC.prey_begin());
		std::cout << "\n testing alterness relaxation: " << testfirstPrey->GetPreyParams().AlertnessRelexation.x << " " << testfirstPrey->GetPreyParams().AlertnessRelexation.y;


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
	CreateDirectory(bufS.c_str(), NULL);
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

	os << '\n';
	os2 << '\n';
	std::cout << "\n done__________________________________________";
	//if (!Sim.experiments.empty()) std::cout << "\n and is it working here..?  " << Sim.experiments[0].param.evolution.fileName;
}

void EvolvePN::PrepareSave()
{
	CFlock::pred_iterator first(GFLOCKNC.predator_begin());
	CFlock::pred_iterator last(GFLOCKNC.predator_end());
	CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());
	data_all_pred data_all_pred;

	for (; first != last; ++first)
	{
		data_per_pred data;

		
		data.push_back(first->get_N());
		data.push_back(first->getDPAdjParam());
		data.push_back(first->getStartAltitude());
		data.push_back(first->getStartXDist()); 
		data.push_back(first->hunts().minDist);
		data.push_back(first->hunts().velocityMinDist);
		data.push_back(first->GetBirdParams().maxForce);
		data.push_back(first->getGeneration());
		data.push_back(first->hunts().seqTime);
		data.push_back(first->GetBirdParams().reactionTime);
		data.push_back(first->GetBirdParams().cruiseSpeed);
		data.push_back(float(first->GetPredParams().pursuit.type));
		data_all_pred.push_back(data);

	}
	
	save_data_.push_back(data_all_pred);

	if (names_.empty())
	{
		names_.push_back("N");
		names_.push_back("DPAdjParam");
		names_.push_back("StartAltitude");
		names_.push_back("StartXDist");
		names_.push_back("minDist");
		names_.push_back("velocityMinDist");
		names_.push_back("maxForce");
		names_.push_back("Generation");
		names_.push_back("seqTime");
		names_.push_back("reactionTime");
		names_.push_back("cruiseSpeed");
		names_.push_back("PursuitType1pn2dp");
	}
	first = GFLOCKNC.predator_begin();
	if (namesParameters_.empty())
	{
		namesParameters_.push_back("predAR"); ValuesParameters_.push_back(first->GetBirdParams().wingAspectRatio);
		namesParameters_.push_back("predBodyDrag");  ValuesParameters_.push_back(first->GetBirdParams().bodyDrag);
		namesParameters_.push_back("predControlCL");  ValuesParameters_.push_back(first->GetBirdParams().controlCL);
		namesParameters_.push_back("predCDCL");  ValuesParameters_.push_back(first->GetBirdParams().CDCL);
		namesParameters_.push_back("predMinSpeed"); ValuesParameters_.push_back(first->GetBirdParams().minSpeed);
		namesParameters_.push_back("predCruiseSpeed"); ValuesParameters_.push_back(first->GetBirdParams().cruiseSpeed);
		namesParameters_.push_back("predMaxSpeed"); ValuesParameters_.push_back(first->GetBirdParams().maxSpeed);
		namesParameters_.push_back("predRollRate"); ValuesParameters_.push_back(first->GetBirdParams().rollRate);
		namesParameters_.push_back("predMaxLift"); ValuesParameters_.push_back(first->GetBirdParams().maxLift);
		namesParameters_.push_back("predCL"); ValuesParameters_.push_back(first->GetBirdParams().CL);
		namesParameters_.push_back("predWingSpan"); ValuesParameters_.push_back(first->GetBirdParams().wingSpan);
		namesParameters_.push_back("predMass"); ValuesParameters_.push_back(first->GetBirdParams().bodyMass);
		namesParameters_.push_back("predMaxForce"); ValuesParameters_.push_back(first->GetBirdParams().maxForce);
		namesParameters_.push_back("predRT"); ValuesParameters_.push_back(first->GetBirdParams().reactionTime);
		namesParameters_.push_back("predRTStochastic"); ValuesParameters_.push_back(first->GetBirdParams().reactionStochastic);
		namesParameters_.push_back("predBlindAngle"); ValuesParameters_.push_back(first->GetBirdParams().blindAngle);
		namesParameters_.push_back("predNoiseWeight"); ValuesParameters_.push_back(first->GetBirdParams().randomWeight);
		namesParameters_.push_back("predLockDistance"); ValuesParameters_.push_back(first->GetPredParams().LockDistance);
		namesParameters_.push_back("predBlindAngleLock"); ValuesParameters_.push_back(first->GetPredParams().LockDistance);
		namesParameters_.push_back("predskipRightHemisphere"); ValuesParameters_.push_back(first->GetBirdParams().skipRightHemisphere);
		namesParameters_.push_back("predrho"); ValuesParameters_.push_back(first->GetBirdParams().rho);
		namesParameters_.push_back("predspeedControl"); ValuesParameters_.push_back(first->GetBirdParams().speedControl);
		namesParameters_.push_back("predmaxRadius"); ValuesParameters_.push_back(first->GetBirdParams().maxRadius);
		namesParameters_.push_back("predneighborLerp"); ValuesParameters_.push_back(first->GetBirdParams().neighborLerp);
		namesParameters_.push_back("predtopologicalRange"); ValuesParameters_.push_back(first->GetBirdParams().topologicalRange);
		namesParameters_.push_back("predcircularityInc"); ValuesParameters_.push_back(first->GetBirdParams().circularityInc);
		namesParameters_.push_back("predbinocularOverlap"); ValuesParameters_.push_back(first->GetBirdParams().binocularOverlap);
		namesParameters_.push_back("predseparationStepx"); ValuesParameters_.push_back(first->GetBirdParams().separationStep.x);
		namesParameters_.push_back("predseparationStepy"); ValuesParameters_.push_back(first->GetBirdParams().separationStep.y);
		namesParameters_.push_back("predseparationWeightx"); ValuesParameters_.push_back(first->GetBirdParams().separationWeight.x);
		namesParameters_.push_back("predseparationWeighty"); ValuesParameters_.push_back(first->GetBirdParams().separationWeight.y);
		namesParameters_.push_back("predseparationWeightz"); ValuesParameters_.push_back(first->GetBirdParams().separationWeight.z);
		namesParameters_.push_back("predalignmentWeightx"); ValuesParameters_.push_back(first->GetBirdParams().alignmentWeight.x);
		namesParameters_.push_back("predalignmentWeighty"); ValuesParameters_.push_back(first->GetBirdParams().alignmentWeight.y);
		namesParameters_.push_back("predcohesionWeightx"); ValuesParameters_.push_back(first->GetBirdParams().cohesionWeight.x);
		namesParameters_.push_back("predcohesionWeighty"); ValuesParameters_.push_back(first->GetBirdParams().cohesionWeight.y);
		namesParameters_.push_back("predcohesionWeightz"); ValuesParameters_.push_back(first->GetBirdParams().cohesionWeight.z);
		namesParameters_.push_back("predAltitude"); ValuesParameters_.push_back(first->GetBirdParams().altitude);
		namesParameters_.push_back("predrandomWeight"); ValuesParameters_.push_back(first->GetBirdParams().randomWeight);

		namesParameters_.push_back("numPrey"); ValuesParameters_.push_back(GFLOCK.num_prey());
		namesParameters_.push_back("dt"); ValuesParameters_.push_back(Sim.Params().IntegrationTimeStep);
		namesParameters_.push_back("EvolDuration"); ValuesParameters_.push_back(Sim.Params().evolution.durationGeneration);
		namesParameters_.push_back("evolX"); ValuesParameters_.push_back(Sim.Params().evolution.evolveX);
		namesParameters_.push_back("evolY"); ValuesParameters_.push_back(Sim.Params().evolution.evolveAlt);
		namesParameters_.push_back("evolZ"); ValuesParameters_.push_back(Sim.Params().evolution.evolveZ);
		namesParameters_.push_back("evolveCL"); ValuesParameters_.push_back(int(Sim.Params().evolution.evolveCL));
		namesParameters_.push_back("evolvewingAspectRatio"); ValuesParameters_.push_back(Sim.Params().evolution.evolvewingAspectRatio);
		namesParameters_.push_back("evolvemaxForce"); ValuesParameters_.push_back(Sim.Params().evolution.evolvemaxForce);

		namesParameters_.push_back("evolvewingSpan"); ValuesParameters_.push_back(Sim.Params().evolution.evolvewingSpan);
		namesParameters_.push_back("evolvebodyMass"); ValuesParameters_.push_back(Sim.Params().evolution.evolvebodyMass);
		namesParameters_.push_back("evolvecontrolCL"); ValuesParameters_.push_back(Sim.Params().evolution.evolvecontrolCL);
		namesParameters_.push_back("evolvecruiseSpeed"); ValuesParameters_.push_back(Sim.Params().evolution.evolvecruiseSpeed);
		namesParameters_.push_back("evolvemaxLift"); ValuesParameters_.push_back(Sim.Params().evolution.evolvemaxLift);
		namesParameters_.push_back("evolvemaxSpeed"); ValuesParameters_.push_back(Sim.Params().evolution.evolvemaxSpeed);
		namesParameters_.push_back("evolverollRate"); ValuesParameters_.push_back(Sim.Params().evolution.evolverollRate);
		namesParameters_.push_back("evolveminSpeed"); ValuesParameters_.push_back(Sim.Params().evolution.evolveminSpeed);
		namesParameters_.push_back("evolvereactionTime"); ValuesParameters_.push_back(Sim.Params().evolution.evolvereactionTime);
		namesParameters_.push_back("evolvealignmentWeight"); ValuesParameters_.push_back(Sim.Params().evolution.evolvealignmentWeight);
		namesParameters_.push_back("evolvecohesionWeight"); ValuesParameters_.push_back(Sim.Params().evolution.evolvecohesionWeight);
		namesParameters_.push_back("evolveHandleTime"); ValuesParameters_.push_back(Sim.Params().evolution.evolveHandleTime);
		namesParameters_.push_back("evolveLockDistance"); ValuesParameters_.push_back(Sim.Params().evolution.evolveLockDistance);
		namesParameters_.push_back("evolPN"); ValuesParameters_.push_back(Sim.Params().evolution.evolvePN);
		namesParameters_.push_back("evolveDPAdjParam"); ValuesParameters_.push_back(Sim.Params().evolution.evolveDPAdjParam);
		namesParameters_.push_back("minRadius"); ValuesParameters_.push_back(Sim.Params().roost.minRadius);
		namesParameters_.push_back("maxRadius"); ValuesParameters_.push_back(Sim.Params().roost.maxRadius);
		namesParameters_.push_back("Radius");  ValuesParameters_.push_back(Sim.Params().roost.Radius);
		namesParameters_.push_back("externalPrey"); ValuesParameters_.push_back(Sim.Params().evolution.externalPrey);
		

		namesParameters_.push_back("preyMaxForce"); ValuesParameters_.push_back(firstPrey->GetBirdParams().maxForce);
		namesParameters_.push_back("preyAR"); ValuesParameters_.push_back(firstPrey->GetBirdParams().wingAspectRatio);
		namesParameters_.push_back("preyCL"); ValuesParameters_.push_back(firstPrey->GetBirdParams().CL);
		namesParameters_.push_back("preyWingSpan"); ValuesParameters_.push_back(firstPrey->GetBirdParams().wingSpan);
		namesParameters_.push_back("preyCruiseSpeed"); ValuesParameters_.push_back(firstPrey->GetBirdParams().cruiseSpeed);
		namesParameters_.push_back("preyMaxLift"); ValuesParameters_.push_back(firstPrey->GetBirdParams().maxLift);
		namesParameters_.push_back("preyMaxSpeed"); ValuesParameters_.push_back(firstPrey->GetBirdParams().maxSpeed);
		namesParameters_.push_back("preyRollRate"); ValuesParameters_.push_back(firstPrey->GetBirdParams().rollRate);
		namesParameters_.push_back("preyMinSpeed"); ValuesParameters_.push_back(firstPrey->GetBirdParams().minSpeed);
		namesParameters_.push_back("preyWBetaInRoll"); ValuesParameters_.push_back(firstPrey->GetBirdParams().wBetaIn.x);
		namesParameters_.push_back("preyWBetaInPitch"); ValuesParameters_.push_back(firstPrey->GetBirdParams().wBetaIn.y);
		namesParameters_.push_back("preyMaxRadius"); ValuesParameters_.push_back(firstPrey->GetBirdParams().maxRadius);
		namesParameters_.push_back("preyMaxSeparationTopo"); ValuesParameters_.push_back(firstPrey->GetBirdParams().maxSeparationTopo);
		namesParameters_.push_back("preyseparationWeightx"); ValuesParameters_.push_back(firstPrey->GetBirdParams().separationWeight.x);
		namesParameters_.push_back("preyseparationWeighty"); ValuesParameters_.push_back(firstPrey->GetBirdParams().separationWeight.y);
		namesParameters_.push_back("preyseparationWeightz"); ValuesParameters_.push_back(firstPrey->GetBirdParams().separationWeight.z);
		namesParameters_.push_back("preyalignmentWeightx"); ValuesParameters_.push_back(firstPrey->GetBirdParams().alignmentWeight.x);
		namesParameters_.push_back("preyalignmentWeighty"); ValuesParameters_.push_back(firstPrey->GetBirdParams().alignmentWeight.y);
		namesParameters_.push_back("preycohesionWeightx"); ValuesParameters_.push_back(firstPrey->GetBirdParams().cohesionWeight.x);
		namesParameters_.push_back("preycohesionWeighty"); ValuesParameters_.push_back(firstPrey->GetBirdParams().cohesionWeight.y);
		namesParameters_.push_back("preycohesionWeightz"); ValuesParameters_.push_back(firstPrey->GetBirdParams().cohesionWeight.z);
		namesParameters_.push_back("preyrandomWeight"); ValuesParameters_.push_back(firstPrey->GetBirdParams().randomWeight);
		namesParameters_.push_back("preyBoundaryWeightx"); ValuesParameters_.push_back(firstPrey->GetBirdParams().boundaryWeight.x);
		namesParameters_.push_back("preyBoundaryWeighty"); ValuesParameters_.push_back(firstPrey->GetBirdParams().boundaryWeight.y);
		namesParameters_.push_back("preyBoundaryWeightz"); ValuesParameters_.push_back(firstPrey->GetBirdParams().boundaryWeight.z);
		namesParameters_.push_back("preyOuterBoundary"); ValuesParameters_.push_back(firstPrey->GetBirdParams().outerBoundary);
		namesParameters_.push_back("preyInnerBoundary"); ValuesParameters_.push_back(firstPrey->GetBirdParams().innerBoundary);
		namesParameters_.push_back("preyAltitude"); ValuesParameters_.push_back(firstPrey->GetBirdParams().altitude);
		namesParameters_.push_back("preyDetectCruising"); ValuesParameters_.push_back(firstPrey->GetPreyParams().DetectCruising);
		namesParameters_.push_back("preyDetectionDistance"); ValuesParameters_.push_back(firstPrey->GetPreyParams().DetectionDistance);
		namesParameters_.push_back("preyDetectionSurfaceProb"); ValuesParameters_.push_back(firstPrey->GetPreyParams().DetectionSurfaceProb);
		namesParameters_.push_back("preyDetectionHemisphereFOV"); ValuesParameters_.push_back(firstPrey->GetPreyParams().DetectionHemisphereFOV);
		namesParameters_.push_back("preyIncurNeighborPanic"); ValuesParameters_.push_back(firstPrey->GetPreyParams().IncurNeighborPanic);
		namesParameters_.push_back("preyIncurLatency"); ValuesParameters_.push_back(firstPrey->GetPreyParams().IncurLatency);
		namesParameters_.push_back("preyAlertnessRelexationx"); ValuesParameters_.push_back(firstPrey->GetPreyParams().AlertnessRelexation.x);
		namesParameters_.push_back("preyAlertnessRelexationy"); ValuesParameters_.push_back(firstPrey->GetPreyParams().AlertnessRelexation.y);
		namesParameters_.push_back("preyAlertedReactionTimeFactor"); ValuesParameters_.push_back(firstPrey->GetPreyParams().AlertedReactionTimeFactor);
		namesParameters_.push_back("preyReturnRelaxation"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnRelaxation);
		namesParameters_.push_back("preyReturnWeightx"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnWeight.x);
		namesParameters_.push_back("preyReturnWeighty"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnWeight.y);
		namesParameters_.push_back("preyReturnWeightz"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnWeight.z);
		namesParameters_.push_back("preyReturnThresholdx"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnThreshold.x);
		namesParameters_.push_back("preyReturnThresholdy"); ValuesParameters_.push_back(firstPrey->GetPreyParams().ReturnThreshold.y);
		namesParameters_.push_back("preyRT"); ValuesParameters_.push_back(firstPrey->GetBirdParams().reactionTime);
		namesParameters_.push_back("preyreactionStochastic"); ValuesParameters_.push_back(firstPrey->GetBirdParams().reactionStochastic);
		namesParameters_.push_back("preyBlindAngle"); ValuesParameters_.push_back(firstPrey->GetBirdParams().blindAngle);
		namesParameters_.push_back("preyBodyDrag"); ValuesParameters_.push_back(firstPrey->GetBirdParams().bodyDrag);
		namesParameters_.push_back("preyControlCL"); ValuesParameters_.push_back(firstPrey->GetBirdParams().controlCL);
		namesParameters_.push_back("preyCDCL"); ValuesParameters_.push_back(firstPrey->GetBirdParams().CDCL);
		namesParameters_.push_back("preyMass"); ValuesParameters_.push_back(firstPrey->GetBirdParams().bodyMass);
		namesParameters_.push_back("preyEvMaximizeDist"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[0].weight);
		namesParameters_.push_back("preyEvTurnInward"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[1].weight);
		namesParameters_.push_back("preyEvTurnAway"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[2].weight);
		namesParameters_.push_back("preyEvDrop"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[3].weight);
		namesParameters_.push_back("preyEvMoveCentered"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[4].weight);
		namesParameters_.push_back("preyEvZig"); ValuesParameters_.push_back(firstPrey->GetPreyParams().EvasionStrategy[5].weight);

		std::string EvasionNames[8] = {
			"MaximizeDist",     // maximize distance of closest approach
			"TurnInward",       // turn along circularity vector
			"TurnAway",         // Turn in opposite direction (Chris)
			"Drop",             // Drop out of sky
			"MoveCentered",			// Move towards center
			"Zig",              // Left-Right evasion. Parameter edge is reinterpreted as (TirgDist, t_left, t_right, t_handle)
			"Custom",	  			  // Lua
			"MaxEvasionStrategy__"
		};

		std::string GuidanceNames[5] = {
			"Custom",    
			"ProportionalNavigation",       
			"DirectPursuit",      
			"DirectPursuit2",
			"PNDP",
		};

		namesParameters_.push_back("predGuidance"); StringParameters_.push_back(GuidanceNames[first->GetPredParams().pursuit.type]);

		

	}
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
	std::uniform_real_distribution<float> rnd(-0.5f, 0.5f);
	//each generation the mutation becomes less..
	for (unsigned i = (N >> 1); i < N; ++i)
	{

		for (int ii = 0; ii < allele[i].size(); ii++)
		{
			allele[i][ii] += (1.0f / Generation_) * rnd(rnd_eng()) * 5 + 1.0f * rnd(rnd_eng());
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
	for (; first != last; ++first)
	{
		SetGetAlleles(top1, first, 2);
	}
	const float R = PROOST.Radius;

	first = GFLOCKNC.predator_begin();
	//std::cout << "\n LALALALALALA";
	for (; first != last; ++first)
	{
		if (Generation_ == 1)
		{
			if (Sim.Params().evolution.evolvePN) first->set_N((float(rand()) / (float(RAND_MAX) / 200.0f) ) - 100.0f);
			if (Sim.Params().evolution.evolveDPAdjParam) first->setDPAdjParam((float(rand()) / (float(RAND_MAX) / 200.0f)) - 100.0f);
			if (Sim.Params().evolution.evolvemaxForce) first->GetBirdParams().maxForce = ((float(rand()) / (float(RAND_MAX) / 100.0f)));
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
		std::cout << "\nN :  " << first->get_N() << " startAltitude :  " << first->getStartAltitude() << " startXDist :  " << first->getStartXDist() << " maxForce :  " << first->GetBirdParams().maxForce << " DPadjParam :  " << first->getDPAdjParam() << " Generation: " << first->getGeneration();
		//std::cout << "\n" << rnd(rnd_eng());

		meanN += first->get_N() * 1 / N;
		meanStartAltitude += first->getStartAltitude() * 1 / N;
		meanXDist += first->getStartXDist() * 1 / N;

	
	};
	if (Sim.Params().evolution.externalPrey)
	{
			loadPositions(Sim.Params().evolution.externalPreyFile.c_str());
	}

	
	CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());

	for (; firstPrey != lastPrey; ++firstPrey)
	{
		firstPrey->position_ = glm::vec3(0, 120, 0);
	};

	GFLOCKNC.meanN = meanN;
	GFLOCKNC.meanStartAltitude = meanStartAltitude;
	GFLOCKNC.meanXDist = meanXDist;

	if (Generation_ >= Sim.Params().evolution.terminationGeneration) Reset();
}


