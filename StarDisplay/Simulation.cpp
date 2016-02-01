#include <exception>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "filesystem.hpp"
#include "GLWin.hpp"
#include "Bird.hpp"
#include "Predator.hpp"
#include "Flock.hpp"
#include "Clock.hpp"
#include "Simulation.hpp"
#include "Flock.hpp"
#include "Camera.hpp"
#include "GLSLState.hpp"
#include "HistOverlay.hpp"
#include "DefaultStatistic.hpp"
#include "HistogramStatistic.hpp"
#include "VoxelVolumeStatistic.hpp"
#include "TimeSeriesStatistic.hpp"
#include "ClusterStatistic.hpp"
#include "NNStatistic.hpp"
#include "QmStatistic.hpp"
#include "GlobalDevStatistic.hpp"
#include "FCorrelationStatistic.hpp"
#include "PredatorStatistic.hpp"
#include "EvolveDeflection.hpp"
#include "EvolvePN.hpp"
#include "KeyState.hpp"
#include "libLua.hpp"
#include "debug.hpp"
#include "random.hpp"
#include "Mmsystem.h"
#include "libParam.hpp"


using namespace Param;


static const char header_fmt[] =
  "\n[F1] Help\n[F2] Prey: %d + %d\n[F3] Boundary radius: %d\n[F4] HRTree level: %d\nh.hildenbrandt@rug.nl\n";

static const char footer_fmt[] =
  "Sim. time: %02.0f:%02.0f:%02.0f\nupdate %.4f s\nfps: %d";


namespace {

  void printExceptionMsg(const char* prefix)
  {
    std::cerr << (prefix ? prefix : "Unknown error") << ": ";
    const char* luaErr = Lua.ErrMsg();
    std::cerr << (luaErr ? luaErr : "") << '\n';
    debug::StackDump(PARAMS.DebugLogStackLevel);
    std::cerr << "\nBailing out\n";
  }

}


Simulation::Simulation()
: SimulationTime_(0.0),
  UpdateTime_(0.0),
  FrameTime_(0.1),
  statisticsPaused_(false),
  paused_(false),
  track_alpha_(false),
  lastStatTime_(0.0),
  lastClusterTime_(0.0),
  camera_(0)
{
}


Simulation::~Simulation()
{
  if (CustomStatistic_) CustomStatistic_->finalize();
}



void Simulation::SetParams(const Param::Params& param)
{
	params_ = param;
}

void Simulation::SetInitialParameter(const Param::Params& param)
{
  params_ = param;
  params_.vaderJacob.Eitje = 2;
  /*SimulationTime_ = 0.0;	/*changed by Robin*/
	gl_.reset( new GLSLState() );
	gl_->Init(AppWindow.GetDC());
  flock_.reset( new CFlock(params_.roost.numPrey) );
  trails_.reset( new trail_buffer_pool() );
  CustomStatistic_.reset( new DefaultStatistic() );
  SetPFeatureMap(param.featureMap);
}


void Simulation::GetExperimentSettings(const luabind::object& obj)
{

	luabind::object experiments = obj;

	for (luabind::iterator i(experiments), end; i != end; ++i)
	{
		CFlock::prey_iterator Prey = GFLOCKNC.prey_begin();
		CFlock::pred_iterator Pred = GFLOCKNC.predator_begin();

		std::cout << "\n number of pred is: " << GFLOCKNC.num_pred();
		Param::Experiment experiment;
		if (Sim.experiments.empty())
		{
			experiment.pred = Pred->GetPredParams();
			experiment.prey = Prey->GetPreyParams();
			experiment.predBird = Pred->GetBirdParams();
			experiment.preyBird = Prey->GetBirdParams();
			experiment.param = params_;
		}
		else
		{
			experiment.pred = Sim.experiments[0].pred;
			experiment.prey = Sim.experiments[0].prey;
			experiment.predBird = Sim.experiments[0].predBird;
			experiment.preyBird = Sim.experiments[0].preyBird;
			experiment.param = Sim.experiments[0].param;
		}
		int intKey = luabind::object_cast<int>(i.key());
		luabind::object expnumb = obj[intKey];
		for (luabind::iterator ii(expnumb), end; ii != end; ++ii)
		{
			std::string strKey2 = luabind::object_cast<std::string>(ii.key());
			if (strKey2 == "Param")
			{

				luabind::object params = obj[intKey][strKey2];
				for (luabind::iterator iii(params), end; iii != end; ++iii)
				{
					std::string strKey3 = luabind::object_cast<std::string>(iii.key());
					std::cout << "\n" << intKey;
					std::cout << "\n" << strKey2;
					std::cout << "\n" << strKey3;

					if (strKey3 == "evolution")

					{
						luabind::object evolution = obj[intKey][strKey2][strKey3];
						for (luabind::iterator iiii(evolution), end; iiii != end; ++iiii)
						{
							std::string strKey4 = luabind::object_cast<std::string>(iiii.key());
							if (strKey4 == "type") experiment.param.evolution.type = luabind::object_cast<std::string>(evolution["type"]);
							if (strKey4 == "fileName") experiment.param.evolution.fileName = luabind::object_cast<std::string>(evolution["fileName"]);
							if (strKey4 == "durationGeneration") experiment.param.evolution.durationGeneration = luabind::object_cast<float>(evolution["durationGeneration"]);
							if (strKey4 == "startGen") experiment.param.evolution.startGen = luabind::object_cast<int>(evolution["startGen"]);
							if (strKey4 == "load") experiment.param.evolution.load = luabind::object_cast<bool>(evolution["load"]);
							if (strKey4 == "loadFolder") experiment.param.evolution.loadFolder = luabind::object_cast<std::string>(evolution["loadFolder"]);
							if (strKey4 == "evolvePN") experiment.param.evolution.evolvePN = luabind::object_cast<bool>(evolution["evolvePN"]);
							if (strKey4 == "evolveDPAdjParam") experiment.param.evolution.evolveDPAdjParam = luabind::object_cast<bool>(evolution["evolveDPAdjParam"]);
							if (strKey4 == "evolveAlt") experiment.param.evolution.evolveAlt = luabind::object_cast<bool>(evolution["evolveAlt"]);
							if (strKey4 == "evolveZ") experiment.param.evolution.evolveZ = luabind::object_cast<bool>(evolution["evolveZ"]);
							if (strKey4 == "evolveCL") experiment.param.evolution.evolveCL = luabind::object_cast<bool>(evolution["evolveCL"]);
							if (strKey4 == "evolvewingAspectRatio") experiment.param.evolution.evolvewingAspectRatio = luabind::object_cast<bool>(evolution["evolvewingAspectRatio"]);
							if (strKey4 == "evolvewingBeatFreq") experiment.param.evolution.evolvewingBeatFreq = luabind::object_cast<bool>(evolution["evolvewingBeatFreq"]);
							if (strKey4 == "evolvetheta") experiment.param.evolution.evolvetheta = luabind::object_cast<bool>(evolution["evolvetheta"]);
							if (strKey4 == "evolvewingLength") experiment.param.evolution.evolvewingLength = luabind::object_cast<bool>(evolution["evolvewingLength"]);
							if (strKey4 == "evolvebodyArea") experiment.param.evolution.evolvebodyArea = luabind::object_cast<bool>(evolution["evolvebodyArea"]);
							if (strKey4 == "evolvecBody") experiment.param.evolution.evolvecBody = luabind::object_cast<bool>(evolution["evolvecBody"]);
							if (strKey4 == "evolvecFriction") experiment.param.evolution.evolvecFriction = luabind::object_cast<bool>(evolution["evolvecFriction"]);
							if (strKey4 == "evolvemaxForce") experiment.param.evolution.evolvemaxForce = luabind::object_cast<bool>(evolution["evolvemaxForce"]);
							if (strKey4 == "evolvewingSpan") experiment.param.evolution.evolvewingSpan = luabind::object_cast<bool>(evolution["evolvewingSpan"]);
							if (strKey4 == "evolvebodyMass") experiment.param.evolution.evolvebodyMass = luabind::object_cast<bool>(evolution["evolvebodyMass"]);
							if (strKey4 == "evolvecontrolCL") experiment.param.evolution.evolvecontrolCL = luabind::object_cast<bool>(evolution["evolvecontrolCL"]);
							if (strKey4 == "evolvecruiseSpeed") experiment.param.evolution.evolvecruiseSpeed = luabind::object_cast<bool>(evolution["evolvecruiseSpeed"]);
							if (strKey4 == "evolvemaxLift") experiment.param.evolution.evolvemaxLift = luabind::object_cast<bool>(evolution["evolvemaxLift"]);
							if (strKey4 == "evolvemaxSpeed") experiment.param.evolution.evolvemaxSpeed = luabind::object_cast<bool>(evolution["evolvemaxSpeed"]);
							if (strKey4 == "evolverollRate") experiment.param.evolution.evolverollRate = luabind::object_cast<bool>(evolution["evolverollRate"]);
							if (strKey4 == "evolveminSpeed") experiment.param.evolution.evolveminSpeed = luabind::object_cast<bool>(evolution["evolveminSpeed"]);
							if (strKey4 == "evolvereactionTime") experiment.param.evolution.evolvereactionTime = luabind::object_cast<bool>(evolution["evolvereactionTime"]);
							if (strKey4 == "evolvealignmentWeight") experiment.param.evolution.evolvealignmentWeight = luabind::object_cast<bool>(evolution["evolvealignmentWeight"]);
							if (strKey4 == "evolvecohesionWeight") experiment.param.evolution.evolvecohesionWeight = luabind::object_cast<bool>(evolution["evolvecohesionWeight"]);
							if (strKey4 == "evolveHandleTime") experiment.param.evolution.evolveHandleTime = luabind::object_cast<bool>(evolution["evolveHandleTime"]);
							if (strKey4 == "evolveLockDistance") experiment.param.evolution.evolveLockDistance = luabind::object_cast<bool>(evolution["evolveLockDistance"]);
							if (strKey4 == "TrajectoryBestPredator") experiment.param.evolution.TrajectoryBestPredator = luabind::object_cast<bool>(evolution["TrajectoryBestPredator"]);
							if (strKey4 == "TrajectoryPrey") experiment.param.evolution.TrajectoryPrey = luabind::object_cast<bool>(evolution["TrajectoryPrey"]);
							if (strKey4 == "externalPrey") experiment.param.evolution.externalPrey = luabind::object_cast<bool>(evolution["externalPrey"]);
							if (strKey4 == "externalPreyFile") experiment.param.evolution.externalPreyFile = luabind::object_cast<std::string>(evolution["externalPreyFile"]);
							if (strKey4 == "title") experiment.param.evolution.title = luabind::object_cast<std::string>(evolution["title"]);
							if (strKey4 == "description") experiment.param.evolution.description = luabind::object_cast<std::string>(evolution["description"]);
							if (strKey4 == "terminationGeneration") experiment.param.evolution.terminationGeneration = luabind::object_cast<int>(evolution["terminationGeneration"]);



						}


					}

					if (strKey3 == "roost")

					{
						luabind::object roost = obj[intKey][strKey2][strKey3];
						for (luabind::iterator iiii(roost), end; iiii != end; ++iiii)
						{
							std::string strKey4 = luabind::object_cast<std::string>(iiii.key());
							if (strKey4 == "numPrey") experiment.param.roost.numPrey = luabind::object_cast<unsigned>(roost["numPrey"]);
							if (strKey4 == "numPredators") experiment.param.roost.numPredators = luabind::object_cast<unsigned>(roost["numPredators"]);
							if (strKey4 == "Radius") experiment.param.roost.Radius = luabind::object_cast<float>(roost["Radius"]);
							if (strKey4 == "minRadius") experiment.param.roost.numPrey = luabind::object_cast<float>(roost["minRadius"]);
							if (strKey4 == "maxRadius") experiment.param.roost.numPrey = luabind::object_cast<float>(roost["maxRadius"]);

						}

					}
				}
			}
			if (strKey2 == "preyBird" || strKey2 == "predBird")
			{

				luabind::object Bird = obj[intKey][strKey2];
				if (strKey2 == "preyBird") copyBirdParam(Bird, experiment.preyBird);
				if (strKey2 == "predBird") copyBirdParam(Bird, experiment.predBird);
			}
			
			if (strKey2 == "prey")
			{

				luabind::object prey = obj[intKey][strKey2];
				for (luabind::iterator iii(prey), end; iii != end; ++iii)
				{
					std::string strKey3 = luabind::object_cast<std::string>(iii.key());
					if (strKey3 == "DetectCruising") experiment.prey.DetectCruising = luabind::object_cast<bool>(prey["DetectCruising"]);
					if (strKey3 == "DetectionDistance") experiment.prey.DetectionDistance = luabind::object_cast<float>(prey["DetectionDistance"]);
					if (strKey3 == "DetectionSurfaceProb") experiment.prey.DetectionSurfaceProb = luabind::object_cast<float>(prey["DetectionSurfaceProb"]);
					if (strKey3 == "DetectionHemisphereFOV") experiment.prey.DetectionHemisphereFOV = luabind::object_cast<float>(prey["DetectionHemisphereFOV"]);
					if (strKey3 == "IncurNeighborPanic") experiment.prey.IncurNeighborPanic = luabind::object_cast<int>(prey["IncurNeighborPanic"]);
					if (strKey3 == "IncurLatency") experiment.prey.IncurLatency = luabind::object_cast<double>(prey["IncurLatency"]);
					if (strKey3 == "AlertnessRelexation") experiment.prey.AlertnessRelexation = luabind::object_cast<glm::vec2>(prey["AlertnessRelexation"]);
					if (strKey3 == "AlertedReactionTimeFactor") experiment.prey.AlertedReactionTimeFactor = luabind::object_cast<float>(prey["AlertedReactionTimeFactor"]);
					if (strKey3 == "AlertedWBetaIn") experiment.prey.AlertedWBetaIn = luabind::object_cast<glm::vec3>(prey["AlertedWBetaIn"]);
					if (strKey3 == "AlertedWBetaOut") experiment.prey.AlertedWBetaOut = luabind::object_cast<glm::vec3>(prey["DetectCruising"]);
					if (strKey3 == "AlertedTopo") experiment.prey.AlertedTopo = luabind::object_cast<float>(prey["AlertedTopo"]);
					if (strKey3 == "AlertedAlignmentWeight") experiment.prey.AlertedAlignmentWeight = luabind::object_cast<glm::vec2>(prey["AlertedAlignmentWeight"]);
					if (strKey3 == "Return2Flock") experiment.prey.Return2Flock = luabind::object_cast<bool>(prey["Return2Flock"]);
					if (strKey3 == "ReturnRelaxation") experiment.prey.ReturnRelaxation = luabind::object_cast<float>(prey["ReturnRelaxation"]);
					if (strKey3 == "ReturnWeight") experiment.prey.ReturnWeight = luabind::object_cast<glm::vec3>(prey["ReturnWeight"]);
					if (strKey3 == "ReturnThreshold") experiment.prey.ReturnThreshold = luabind::object_cast<glm::vec2>(prey["ReturnThreshold"]);
					if (strKey3 == "EvasionStrategy")
					{
						Panic p = libParam::FromLua<Panic>(prey["EvasionStrategy"]);
						experiment.prey.EvasionStrategy[p.type] = p;
					}


				}
			}
			if (strKey2 == "pred")
			{

				luabind::object pred = obj[intKey][strKey2];
				for (luabind::iterator iii(pred), end; iii != end; ++iii)
				{
					std::string strKey3 = luabind::object_cast<std::string>(iii.key());
					if (strKey3 == "StartAttack") {
						int x = luabind::object_cast<int>(pred["StartAttack"]);
						experiment.pred.StartAttack = static_cast<Param::Predator::StartAttacks>(x);
					}
					if (strKey3 == "PreySelection") {
						int x = luabind::object_cast<int>(pred["PreySelection"]);
						experiment.pred.PreySelection = static_cast<Param::Predator::PreySelections>(x);
					}
					if (strKey3 == "PursuitStrategy") 
					{
						Pursuit p = libParam::FromLua<Pursuit>(pred["PursuitStrategy"]);
						experiment.pred.pursuit = p;
						std::cout << "\n the pursuit is:   " << p.type;
					}
					
					if (strKey3 == "AttackWBetaIn") experiment.pred.AttackWBetaIn = luabind::object_cast<glm::vec3>(pred["AttackWBetaIn"]);
					if (strKey3 == "AttackWBetaOut") experiment.pred.AttackWBetaOut = luabind::object_cast<glm::vec3>(pred["AttackWBetaOut"]);
					if (strKey3 == "CatchDistance") experiment.pred.CatchDistance = luabind::object_cast<float>(pred["CatchDistance"]);
					if (strKey3 == "AttackSpan") experiment.pred.AttackSpan = luabind::object_cast<float>(pred["AttackSpan"]);
					if (strKey3 == "Dogfight") experiment.pred.Dogfight = luabind::object_cast<float>(pred["Dogfight"]);
					if (strKey3 == "HoldLock") experiment.pred.HoldLock = luabind::object_cast<bool>(pred["HoldLock"]);
					if (strKey3 == "LockBlindAngle") experiment.pred.LockBlindAngle = luabind::object_cast<float>(pred["LockBlindAngle"]);
					if (strKey3 == "LockDistance") experiment.pred.LockDistance = luabind::object_cast<float>(pred["LockDistance"]);
					if (strKey3 == "maxLocks") experiment.pred.maxLocks = luabind::object_cast<int>(pred["maxLocks"]);
					if (strKey3 == "ExposureThreshold") experiment.pred.ExposureThreshold = luabind::object_cast<glm::vec2>(pred["ExposureThreshold"]);
					if (strKey3 == "AttractMix") experiment.pred.AttractMix = luabind::object_cast<float>(pred["AttractMix"]);
					if (strKey3 == "VisualError") experiment.pred.VisualError = luabind::object_cast<float>(pred["VisualError"]);
					if (strKey3 == "VisualBias") experiment.pred.VisualBias = luabind::object_cast<glm::vec2>(pred["VisualBias"]);


				}
			}
		}
		std::cout << "\n and it issss:::  " << experiment.param.evolution.fileName;
		std::cout << "\n and visual error is:::  " << experiment.pred.VisualError;
		Sim.experiments.push_back(experiment);
	}
}

void Simulation::copyBirdParam(luabind::object& obj, Param::Bird& bird)
{
	for (luabind::iterator iii(obj), end; iii != end; ++iii)
	{
		std::string strKey3 = luabind::object_cast<std::string>(iii.key());
		if (strKey3 == "reactionTime") bird.reactionTime = luabind::object_cast<float>(obj["reactionTime"]);
		if (strKey3 == "reactionStochastic") bird.reactionStochastic = luabind::object_cast<float>(obj["reactionStochastic"]);
		if (strKey3 == "skipLeftHemisphere") bird.skipLeftHemisphere = luabind::object_cast<int>(obj["skipLeftHemisphere"]);
		if (strKey3 == "skipRightHemisphere") bird.skipRightHemisphere = luabind::object_cast<int>(obj["skipRightHemisphere"]);
		if (strKey3 == "rho") bird.rho = luabind::object_cast<float>(obj["rho"]);
		if (strKey3 == "bodyMass") bird.bodyMass = luabind::object_cast<float>(obj["bodyMass"]);
		if (strKey3 == "bodyWeight") bird.bodyWeight = luabind::object_cast<float>(obj["bodyWeight"]);
		if (strKey3 == "wingSpan") bird.wingSpan = luabind::object_cast<float>(obj["wingSpan"]);
		if (strKey3 == "wingAspectRatio") bird.wingAspectRatio = luabind::object_cast<float>(obj["wingAspectRatio"]);
		if (strKey3 == "wingBeatFreq") bird.wingBeatFreq = luabind::object_cast<float>(obj["wingBeatFreq"]);
		if (strKey3 == "theta") bird.theta = luabind::object_cast<float>(obj["theta"]);
		if (strKey3 == "wingLength") bird.wingLength = luabind::object_cast<float>(obj["wingLength"]);
		if (strKey3 == "bodyArea") bird.bodyArea = luabind::object_cast<float>(obj["bodyArea"]);
		if (strKey3 == "cBody") bird.cBody = luabind::object_cast<float>(obj["cBody"]);
		if (strKey3 == "cFriction") bird.cFriction = luabind::object_cast<float>(obj["cFriction"]);
		if (strKey3 == "wingArea") bird.wingArea = luabind::object_cast<float>(obj["wingArea"]);
		if (strKey3 == "CL") bird.CL = luabind::object_cast<float>(obj["CL"]);
		if (strKey3 == "maxForce") bird.maxForce = luabind::object_cast<float>(obj["maxForce"]);
		if (strKey3 == "maxLift") bird.maxLift = luabind::object_cast<float>(obj["maxLift"]);
		if (strKey3 == "cruiseSpeed") bird.cruiseSpeed = luabind::object_cast<float>(obj["cruiseSpeed"]);
		if (strKey3 == "speedControl") bird.speedControl = luabind::object_cast<float>(obj["speedControl"]);
		if (strKey3 == "maxSpeed") bird.maxSpeed = luabind::object_cast<float>(obj["maxSpeed"]);
		if (strKey3 == "rollRate") bird.rollRate = luabind::object_cast<float>(obj["rollRate"]);
		if (strKey3 == "minSpeed") bird.minSpeed = luabind::object_cast<float>(obj["minSpeed"]);
		if (strKey3 == "wBetaOut") bird.wBetaOut = luabind::object_cast<glm::vec3>(obj["wBetaOut"]);
		if (strKey3 == "wBetaIn") bird.wBetaIn = luabind::object_cast<glm::vec3>(obj["wBetaIn"]);
		if (strKey3 == "maxRadius") bird.maxRadius = luabind::object_cast<float>(obj["maxRadius"]);
		if (strKey3 == "neighborLerp") bird.neighborLerp = luabind::object_cast<float>(obj["neighborLerp"]);
		if (strKey3 == "topologicalRange") bird.topologicalRange = luabind::object_cast<float>(obj["topologicalRange"]);
		if (strKey3 == "circularityInc") bird.circularityInc = luabind::object_cast<float>(obj["circularityInc"]);
		if (strKey3 == "binocularOverlap") bird.binocularOverlap = luabind::object_cast<float>(obj["binocularOverlap"]);
		if (strKey3 == "blindAngle") bird.blindAngle = luabind::object_cast<float>(obj["blindAngle"]);
		if (strKey3 == "maxSeparationTopo") bird.maxSeparationTopo = luabind::object_cast<int>(obj["maxSeparationTopo"]);
		if (strKey3 == "separationStep") bird.separationStep = luabind::object_cast<glm::vec2>(obj["separationStep"]);
		if (strKey3 == "separationWeight") bird.separationWeight = luabind::object_cast<glm::vec3>(obj["separationWeight"]);
		if (strKey3 == "alignmentWeight") bird.alignmentWeight = luabind::object_cast<glm::vec2>(obj["alignmentWeight"]);
		if (strKey3 == "cohesionWeight") bird.cohesionWeight = luabind::object_cast<glm::vec3>(obj["cohesionWeight"]);
		if (strKey3 == "randomWeight") bird.randomWeight = luabind::object_cast<float>(obj["randomWeight"]);
		if (strKey3 == "boundaryWeight") bird.boundaryWeight = luabind::object_cast<glm::vec3>(obj["boundaryWeight"]);
		if (strKey3 == "boundaryReflectAngle") bird.boundaryReflectAngle = luabind::object_cast<float>(obj["boundaryReflectAngle"]);
		if (strKey3 == "outerBoundary") bird.outerBoundary = luabind::object_cast<float>(obj["outerBoundary"]);
		if (strKey3 == "innerBoundary") bird.innerBoundary = luabind::object_cast<float>(obj["innerBoundary"]);
		if (strKey3 == "altitude") bird.altitude = luabind::object_cast<float>(obj["altitude"]);
		//if (strKey3 == "gpws") bird.gpws = luabind::object_cast<float>(obj["gpws"]);
		if (strKey3 == "bodyDrag") bird.bodyDrag = luabind::object_cast<float>(obj["bodyDrag"]);
		if (strKey3 == "wingRetractionSpeed") bird.bodyDrag = luabind::object_cast<float>(obj["wingRetractionSpeed"]);
		if (strKey3 == "controlCL") bird.controlCL = luabind::object_cast<bool>(obj["controlCL"]);
		if (strKey3 == "CDCL") bird.CDCL = luabind::object_cast<float>(obj["CDCL"]);

	}

}

void Simulation::SetPFeatureMap(const Param::FeatureMap& featureMap)
{
  params_.featureMap = featureMap;
  SelectStatistic(featureMap.current);
}


void Simulation::SetPRoost(const Param::Roost& roost)
{
  params_.roost.maxRadius = roost.maxRadius;
  params_.roost.minRadius = roost.minRadius;
  params_.roost.Radius = roost.Radius;
  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.RoostChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.RoostChanged(); });
}


void Simulation::SetPRenderFlags(const Param::RenderFlags& flags)
{
  params_.renderFlags = flags;
}


void Simulation::RegisterFactories(const luabind::object& PreyFactory, const luabind::object& PredatorFactory)
{
  PreyFactory_ = PreyFactory;
  PredatorFactory_ = PredatorFactory;
}


void Simulation::RegisterCustomStatistic(IStatistic* sb)
{
  CustomStatistic_.reset(sb);
  if (params_.featureMap.current == FeatureMap::Custom)
  {
    statistic_ = CustomStatistic_;
    statistic_->Reset();
  }
}


luabind::object& Simulation::GetActiveCamera()
{ 
  return luaCamera_; 
}


void Simulation::SetActiveCamera(const luabind::object& luaobj)
{ 
  luaCamera_ = luaobj;
  camera_ = luabind::object_cast<ICamera*>(luaCamera_["cc"]);
}


void Simulation::deleteInvisibles()
{
  const CPrey* focalPrey = camera_->GetFocalPrey();
  int focalId = (focalPrey) ? focalPrey->id() : -1;
  flock_->remove_invisibles(camera_->ModelViewMatrix(), camera_->ProjectionMatrix());
  if (0 == (focalPrey = flock_->FindPrey(focalId)))
  {
    // the target individual was deleted.
    focalPrey = flock_->num_prey() ? &*(flock_->prey_begin()) : 0;
  }
  camera_->SetFocalPrey(focalPrey);
  params_.roost.numPrey = flock_->num_prey();
}


void Simulation::SelectStatistic(int selectedId)
{
  if (statistic_ && params_.featureMap.current == (FeatureMap::FMappings)selectedId)
  {
    return;
  }
  params_.featureMap.current = (FeatureMap::FMappings)selectedId;
  CustomStatistic_->finalize();
  params_.renderFlags.alphaMasking = false;

  // Statistic
  switch (params_.featureMap.current)
  {
  case FeatureMap::Default:
    statistic_.reset( new DefaultStatistic() );
    break;
  case FeatureMap::Correlation:
    statistic_.reset( new FCorrelationStatistic() );
    break;
  case FeatureMap::TimeSeries:
    statistic_.reset( new TimeSeriesStatistic() );
    break;
  case FeatureMap::NN:
    statistic_.reset( new NNStatistic() );
    break;
  case FeatureMap::Qm:
    statistic_.reset( new QmStatistic() );
    break;
  case FeatureMap::PredatorStats:
    statistic_.reset( new PredatorStatistic() );
    break;
  case FeatureMap::PredatorVid:
    statistic_.reset( new PredatorVidStatistic() );
    break;
  case FeatureMap::EvolveDeflection:
    statistic_.reset( new EvolveDeflection() );
    break;
  case FeatureMap::Custom:
    std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.setDefaultColorTex(); });
    std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.setDefaultColorTex(); });
    statistic_ = CustomStatistic_;
    statistic_->Reset();
    break;
  default: 
    {
      float fSize = static_cast<float>(flock_->num_prey());
      params_.featureMap.Entries[FeatureMap::Subflock].hist.max = fSize;

      switch (params_.featureMap.current)
      {
      case FeatureMap::SubflockPCA:
        statistic_.reset( new SubflockPCAStatistic() );
        break;
      case FeatureMap::Subflock:
        statistic_.reset( new SubflockStatistic() );
        break;
      case FeatureMap::VoxelVolume:
        statistic_.reset( new VoxelVolumeStatistic() );
        break;
      case FeatureMap::GlobalVelDev:
      case FeatureMap::GlobalSpeedDev:
      case FeatureMap::GlobalPolDev:
      case FeatureMap::GlobalFrobenius:
        statistic_.reset( new GlobalDevStatistic(params_.featureMap.current) );
        break;
      default:
        statistic_.reset( new HistogramStatistic(params_.featureMap.current) );
        break;
      }
    }
  }
  lastStatTime_ = SimulationTime_;
}


void Simulation::DisplayStatistic() const
{
  statistic_->Display();
}


void Simulation::ResetCurrentStatistic()
{
  lastStatTime_ = SimulationTime_;
  statistic_->Reset();
}


void Simulation::ResumeCurrentStatistic()
{
  lastStatTime_ = SimulationTime_;
  statisticsPaused_ = false;
}


void Simulation::PauseCurrentStatistic()
{
  statisticsPaused_ = true;
}


void Simulation::PrintVector(glm::vec3 input, std::string text)
{

	std::cout << "\n" << text << " " << input.x << " " << input.y << " " << input.z;
}

void Simulation::PrintVec2(glm::vec2 input, std::string text)
{

	std::cout << "\n" << text << " " << input.x << " " << input.y << " ";
}

void Simulation::PrintFloat(float input, std::string text)
{

	std::cout << "\n" << text << " " << input;
}


void Simulation::SaveCurrentStatistic(const char* fname, bool append)
{
  statistic_->save(fname, append);
  
}


bool Simulation::HandleKey(unsigned key, unsigned ks)
{
  if (Lua.ProcessKeyboardHooks(key, ks))
  {
    return true;
  }
  bool handled = false;
  switch (key) 
  {
  case 'Z':
    {
      params_.realTime = !params_.realTime;
    }
  case 'M':  
    if (KEYSTATE_IS_CTRL(ks) || KEYSTATE_IS_SHIFT_CTRL(ks)) 
    {
      handled = true;
      unsigned& current = KEYSTATE_IS_SHIFT_CTRL(ks) ? gl_->currentPredatorModel() : gl_->currentPreyModel();
      if (++current >= params_.ModelSet.size()) current = 0;
      if (KEYSTATE_IS_SHIFT_CTRL(ks)) gl_->currentPredatorModel() = current;
      else gl_->currentPreyModel() = current;
      gl_->setAnnotation("3D model changed");
      gl_->LoadModels();
    } 
    break;
  case VK_UP:
	  //std::cout <<  "\nUp\n";//key up
  case VK_DOWN:
	  //std::cout << "\nUp\n";//key up
  case VK_RIGHT:
	  //std::cout << "\nUp\n";//key up
  case VK_LEFT:
	  //std::cout << "\nUp\n";//key up
  case 'R':
	  //std::cout << "\nUp\n";//key up
    if (KEYSTATE_IS_ALT_CTRL(ks)) 
    {
      handled = true;
      SimulationTime_ = lastStatTime_ = lastClusterTime_ = 0.0;
    } 
    break;
  case VK_OEM_102:
    if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks))
    {
      handled = true;
      gl_->alphaMaskCenter() += (KEYSTATE_IS_SHIFT(ks)) ? 0.005f : -0.005f;
      gl_->alphaMaskCenter() = glm::clamp(gl_->alphaMaskCenter(), 0.0f, 1.0f);
    }
    break;
  case VK_F2:
    if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_SHIFT_CTRL(ks))
    {
      handled = true;
      bool shift = KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_SHIFT_CTRL(ks);
      bool ctrl = KEYSTATE_IS_CTRL(ks) || KEYSTATE_IS_SHIFT_CTRL(ks);
      double n = static_cast<double>(flock_->num_prey());
      n = n + (shift ? +1.0 : -1.0);
      int dn = (ctrl || (n <= 0.0)) ? 1 : static_cast<int>((::pow(10.0, ::floor(::log10(n)))));
      dn = glm::clamp(dn, 1, 1000);
      unsigned newNum = std::max(0, static_cast<int>(flock_->num_prey() + (shift ? +dn : -dn)));
      setNumPrey(newNum);
    }
    break;
  case VK_F11:
    if (KEYSTATE_IS_SHIFT(ks))
    {
      handled = true;
      if (statisticsPaused_) { ResumeCurrentStatistic(); gl_->setAnnotation("Statistics resumed"); }
      else { PauseCurrentStatistic(); gl_->setAnnotation("Statistics paused"); } 
    }
    else if (KEYSTATE_IS_CTRL(ks)) 
    {
      handled = true;
      ResetCurrentStatistic();
      gl_->setAnnotation("Statistic reseted");
    }
    else if (KEYSTATE_IS_BLANK(ks))
    {
      handled = true;
      luabind::object t = Lua("OutputFileName")(params_.featureMap.current);
      SaveCurrentStatistic( luabind::object_cast<const char*>(t["name"]), 
                            luabind::object_cast<bool>(t["append"]) );
      gl_->setAnnotation("Statistic saved");
    }
    break;
  case VK_PAUSE: 
    handled = true;
    paused_ = !paused_; 
    gl_->setAnnotation(paused_ ? "Simulation paused" : ""); 
    break; 
  case VK_DELETE:
    if (KEYSTATE_IS_CTRL(ks)) 
    {
      handled = true;
      deleteInvisibles();
      gl_->setAnnotation("Invisible birds deleted");
    }
    break;
  }
  return handled;
}


const CBird* Simulation::PickNearestBird2Ray(const glm::vec3& ray_position, const glm::vec3& ray_direction)
{
  return flock_->pickNearest2Ray(ray_position, ray_direction);
}


const CBird* Simulation::PickById(int id, bool showTrail)
{
  const CBird* bird = flock_->FindPredator(id);
  if (0 == bird) bird = flock_->FindPrey(id);
  if (bird) SetFocalBird(bird, showTrail);
  return bird;
}


void Simulation::SetPredatorTarget(const CBird* bird)
{
  CPredator* focal = const_cast<CPredator*>(camera_->GetFocalPredator());
  if (focal)
  {
    focal->SetTargetPrey(static_cast<const CPrey*>(bird));
  }
}


void Simulation::SetFocalBird(const CBird* bird, bool showTrail)
{
  if (0 == bird) return;
  camera_->SetFocalBird(bird);
  const_cast<CBird*>(bird)->setTrail(showTrail);
}


void Simulation::setNumPrey(unsigned newNum)
{
  newNum = std::min(params_.maxPrey, newNum);
  const CPrey* focalPrey = camera_->GetFocalPrey();
  int focalId = (focalPrey) ? focalPrey->id() : -1;
  if (newNum > flock_->num_prey())
  {
    while (flock_->num_prey() < newNum)
    {
      CPrey* prey = luabind::object_cast<CPrey*>(PreyFactory_(flock_->nextID()));
      flock_->insert_prey(prey);
    }
  } 
  else 
  {
    while (flock_->num_prey() > newNum)
    {
      flock_->erase_prey();
    }
  }
  params_.roost.numPrey = flock_->num_prey();
  flock_->refresh();
  if (0 == (focalPrey = flock_->FindPrey(focalId)))
  {
    focalPrey = flock_->num_prey() ? &*(flock_->prey_begin()) : 0;
  }
  camera_->SetFocalPrey(focalPrey);

  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.NumPreyChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.NumPreyChanged(); });
}


void Simulation::setNumPredators(unsigned newNum)
{
  newNum = std::min(params_.maxPredators, newNum);
  const CPredator* focalPredator = camera_->GetFocalPredator();
  int focalId = (focalPredator) ? focalPredator->id() : -1;
  if (newNum > flock_->num_pred())
  {
    while (flock_->num_pred() < newNum)
    {
      //CPredator* pred = luabind::object_cast<CPredator*>(PredatorFactory_(flock_->nextID()));
	  // Robin temp
	  CPredator* pred = luabind::object_cast<CPredator*>(PredatorFactory_(flock_->nextID()));
      flock_->insert_pred( pred );
    }
  } 
  else 
  {
    while (flock_->num_pred() > newNum)
    {
      flock_->erase_pred();
    }
  }
  params_.roost.numPredators = flock_->num_pred();
  //flock_->refresh();
  if (0 == (focalPredator = flock_->FindPredator(focalId))) 
  {
     focalPredator = flock_->num_pred() ? &*flock_->predator_begin() : 0;
  }
  camera_->SetFocalPredator(focalPredator);

  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.NumPredChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.NumPredChanged(); });
}


unsigned Simulation::getNumPrey() const
{ 
  return flock_->num_prey(); 
}


unsigned Simulation::getNumPredators() const
{ 
  return flock_->num_pred(); 
}


void Simulation::UpdateBirds(const float sim_dt)
{
  std::exception_ptr eptr;
  const bool setDefaultColorTex = !params_.featureMap.gCFME().colored;
# pragma omp parallel firstprivate(sim_dt)
  {
    try
    {
      const int Nprey = static_cast<int>(flock_->num_prey());
      const CFlock::prey_iterator firstPrey(flock_->prey_begin());
#     pragma omp for
      for (int i = 0; i < Nprey; ++i) 
      {
        (*(firstPrey+i)).updateNeighbors(sim_dt, *flock_);
        if (setDefaultColorTex) (*(firstPrey+i)).setDefaultColorTex();
      }

#     pragma omp for
      for (int i = 0; i < Nprey; ++i) 
      { 
        (*(firstPrey+i)).update(sim_dt, *flock_);
      }

      const int Npred = static_cast<int>(flock_->num_pred());
      const CFlock::pred_iterator firstPred(flock_->predator_begin());
#     pragma omp for nowait
      for (int i = 0; i < Npred; ++i) 
      { 
        (*(firstPred+i)).updateNeighbors(sim_dt, *flock_);
        (*(firstPred+i)).update(sim_dt, *flock_);
        if (setDefaultColorTex) (*(firstPred+i)).setDefaultColorTex();
      }
    }
    catch (...)
    {
      std::lock_guard<std::mutex> lock(omp_exception_mutex_);
      eptr = std::current_exception();
    }
  }
  if (eptr != nullptr) std::rethrow_exception(eptr);
}


void Simulation::UpdateCurrentStatistic(double stat_dt)
{
  statistic_->apply(stat_dt);
}


void Simulation::UpdateSimulation(double sim_dt)
{
  const double stat_dt = SimulationTime_ - lastStatTime_;
  const double cluster_dt = SimulationTime_ - lastClusterTime_;
  const bool needStat = (!statisticsPaused_) && approx_ge(stat_dt, params_.featureMap.gCFME().dt);
  const bool recluster = approx_ge(cluster_dt, params_.ClusterDetectionTime);
  if (!paused_) 
  {
    UpdateBirds(static_cast<float>(sim_dt));
    flock_->update(static_cast<float>(sim_dt), recluster);
    if (recluster) lastClusterTime_ = SimulationTime_;
    if (needStat) 
    {
      UpdateCurrentStatistic(stat_dt);
      lastStatTime_ = SimulationTime_;
    }

	// specifically for evolution setting:
	if (params_.evolution.type == "PN")
	{
		if (timeSinceEvolution > params_.evolution.durationGeneration)
		{
			evolution.apply();
			evolution.save(params_.evolution.fileName.c_str(), 0); 
			std::cout << "\n test";
			timeSinceEvolution = 0.0f;
		}
	}
	// end evolution setting

  }
  luabind::globals(Lua)["CameraUpdateHook"](luaCamera_, sim_dt);
  camera_->Update(sim_dt);
}


void Simulation::OnSize(int height, int width)
{
  glm::ivec4 ClientRect(0, 0, width, height);
  glm::ivec4 Viewport(ClientRect);
  camera_->OnSize(Viewport, ClientRect);
  gl_->Resize();
}


void Simulation::OnLButtonDown(int x, int y, unsigned ks)
{
  if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_CTRL(ks))
  {
    track_alpha_ = gl_->Overlay->HitTestAlphaSlider(x ,y);

    if (!track_alpha_)
    {
			Lua.ProcessMouseHooks(x, y, 0, false, ks);
    }
  }
}


void Simulation::OnLButtonUp(int, int)
{
  track_alpha_ = false;
}


void Simulation::OnLButtonDblClk(int x, int y)
{
	Lua.ProcessMouseHooks(x, y, 0, true, KeyState());
}


void Simulation::OnMouseMove(int dx, int dy, bool LButtonPressed)
{
  if (track_alpha_ && LButtonPressed) 
  {
    gl_->alphaMaskCenter() = gl_->Overlay->MoveAlphaSlider(dx, gl_->alphaMaskCenter(), gl_->alphaMaskWidth());
  }
}


void Simulation::OnContextMenu(int menuEntry)
{
  SelectStatistic(menuEntry);
}


// Main simulation loop
void Simulation::EnterGameLoop()
{
  GlobalTimerRestart();
  const double dt = params_.IntegrationTimeStep;
  SimulationTime_ = 0.0;
  double lastFrameDuration = 0.0;
  double lastRender = 0.0;
  double timeDrift = 0.0;
  bool done = 0;
  bool odd = true;
  int skippedFrame = 0;
  
  auto renderFun = [&] {
    if (odd) gl_->Flush(); else gl_->Render();
    odd = !odd;
    FrameTime_ = glm::mix(FrameTime_, GlobalTimerSinceReplace(lastRender), 0.005);
  };

  double frameBegin = GlobalTimerNow();
  luabind::object LuaTimeTickHook;
  for (;;)
  {
    // Serve message pump
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) 
    {
      if (msg.message == WM_QUIT)
      {
        return;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      LuaTimeTickHook = luabind::globals(Lua)["TimeTickHook"];
    }

    // waste some time
    double udt = dt * (params_.renderFlags.slowMotion ? params_.slowMotion : 1);
    timeDrift = params_.realTime ? glm::mix(timeDrift, udt - lastFrameDuration, 0.1) : 0.0;
    double nowaitUpdate;
    while (timeDrift > GlobalTimerSinceCopy(frameBegin, nowaitUpdate)) 
    {
      if (params_.renderFlags.slowMotion) renderFun();  // super smooth slow motion
      std::this_thread::yield();                        // be nice
    }

    if (LuaTimeTickHook) LuaTimeTickHook(SimulationTime_, timeDrift);
    UpdateSimulation(dt);
    UpdateTime_ = glm::mix(UpdateTime_, GlobalTimerSince(nowaitUpdate), 0.01);

    if (!paused_)
    {
      SimulationTime_ += dt;
	  timeSinceEvolution += dt;
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::microseconds(params_.pausedSleep));
    }

    if ((timeDrift >= 0) || (skippedFrame == params_.maxSkippedFrames))
    {
		//for (int i = 0; i < 10; i++){
			renderFun();
		//}
      
	  //if (int(SimulationTime_) % 15 == 0) {
		//  PlaySound("C:/Users/Robin/Documents/Cpp/starlings/bin/media/wavs/birds003.wav", NULL, SND_FILENAME | SND_ASYNC);
	  //}
      skippedFrame = 0;
    }
    else
    {
      ++skippedFrame;
    }
    lastFrameDuration = GlobalTimerSinceCopy(nowaitUpdate, frameBegin);

	


	}
}


void Simulation::main(int argc, char** argv)
{
  liblua::Open((argc == 2) ? filesystem::path(argv[1]).string().c_str() : 0, "");

  std::string WinTitle("StarDisplay V");
  WinTitle += luabind::object_cast<const char*>(Lua("Version"));
  if (!AppWindow.Create())
  {
    throw std::exception("Creating main window failed");
  }
  filesystem::create_directory(filesystem::path(luabind::object_cast<const char*>(Lua("DataPath"))));

  std::cout << "Starting simulation.\n";
  Lua.DoFile(luabind::object_cast<const char*>(Lua("ConfigFile")));
  boost::optional<bool> quit = luabind::object_cast_nothrow<bool>(luabind::globals(Lua)["InitHook"]());
  if (quit) return;

  luabind::object rnd = luabind::globals(Lua)["random"];
  rnd_seed(luabind::object_cast<unsigned long>(rnd["getSeed"](rnd)));
  setNumPrey(params_.roost.numPrey);
  setNumPredators(params_.roost.numPredators);
  AppWindow.InitContextMenu();
  ResetCurrentStatistic();
  EnterGameLoop();
}


// Entry point
//
int main(int argc, char** argv)
{
  std::locale::global(std::locale("C"));
  std::cout << "StarDisplay\nInitialising simulation.\n";
  try 
  {
    Sim.main(argc, argv);
  }
  catch (const char* w)
  {
    printExceptionMsg(w);
  }
  catch (const luabind::error& e)
  {
    printExceptionMsg(lua_tostring(e.state(), -1));
  }
  catch (const std::exception& e)
  {
    printExceptionMsg(e.what());
  }
  catch (...)
  {
    printExceptionMsg(0);
  }
  if (AppWindow.IsWindow()) AppWindow.DestroyWindow();
  std::cout << "Regards\n";
  return 0;
}
