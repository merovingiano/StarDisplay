#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include "Params.hpp"
#include "libParam.hpp"
#include "Bird.hpp"
#include "Prey.hpp"
#include "Predator.hpp"
#include "Flock.hpp"
#include "features.hpp"
#include "visitors.hpp"
#include "Globals.hpp"


using namespace Param;
using namespace libParam;
using namespace luabind;


namespace libBirds {

  float luaVisit(const CBird& bird, int fm)
  {
    if (fm < 0 || fm >= FeatureMap::MaxFeatureMapping__)
    {
      return qNan_val;
    }
    return select_feature(FeatureMap::FMappings(fm))(bird);
  }


  float luaVisit(const CPrey& bird, int fm)
  {
    if (fm < 0 || fm >= FeatureMap::MaxFeatureMapping__)
    {
      return qNan_val;
    }
    return select_feature(FeatureMap::FMappings(fm))(bird);
  }


  float luaVisit(const CPredator& bird, int fm)
  {
    if (fm < 0 || fm >= FeatureMap::MaxFeatureMapping__)
    {
      return qNan_val;
    }
    return select_feature(FeatureMap::FMappings(fm))(bird);
  }


  void SetForward(CBird& bird, const glm::vec3& newVal)
  {
    bird.B_[0] = glmutils::save_normalize(newVal, bird.B_[0]);
    bird.velocity_ = bird.speed() * bird.B_[0];
    bird.B_[1] = glm::vec3(0,1,0);
    bird.B_[2] - glm::cross(bird.B_[0], bird.B_[1]);
  }


  const cluster_entry* GetFlockCluster(const CPrey& prey)
  {
    return GFLOCK.cluster(prey.getFlockId());
  }


}


void luaopen_libBirds(lua_State* L)
{
  module(L)[
    class_<neighborInfo>("__neighborinfo")
      .def_readonly("id", &neighborInfo::id)
      .def_readonly("interacting", &neighborInfo::interacting)
      .def_readonly("position", &neighborInfo::position)
      .def_readonly("forward", &neighborInfo::forward)
      .def_readonly("side", &neighborInfo::side)    
      .def_readonly("direction", &neighborInfo::direction)
      .def_readonly("distance", &neighborInfo::distance)
      .def_readonly("cosAngle", &neighborInfo::cosAngle)
      .def_readonly("azimuth", &neighborInfo::azimuth)
      .def_readonly("speed", &neighborInfo::speed)     
      .def_readonly("predatorReaction", &neighborInfo::predatorReaction)
      .def_readonly("panicOnset", &neighborInfo::panicOnset),

    class_<CPredator::hunt>("__predhunt")
      .def_readonly("sequences", &CPredator::hunt::sequences)
      .def_readonly("locks", &CPredator::hunt::locks)
      .def_readonly("catches", &CPredator::hunt::success)
      .def_readonly("minDist", &CPredator::hunt::minDist)
	  .def_readonly("velocityMinDist", &CPredator::hunt::velocityMinDist)
      .def_readonly("minDistLockedOn", &CPredator::hunt::minDistLockedOn)
      .def_readonly("seqTime", &CPredator::hunt::seqTime)
      .def_readonly("lockTime", &CPredator::hunt::lookTime),
    


    class_<CBird>("__bird")
		  .def("isPrey", &CBird::isPrey)
      .property("BirdParams", (Param::Bird& (CBird::*)()) &CBird::GetBirdParams, &CBird::SetBirdParams)
      .def_readwrite("position", &CBird::position_)
      .property("forward", &CBird::forward, &libBirds::SetForward)
      .property("up", &CBird::up)
      .property("side", &CBird::side)
      .property("B", &CBird::B)
      .property("H", &CBird::H)
      .property("velocity", &CBird::velocity_, &CBird::SetVelocity)
      .property("speed", &CBird::speed, &CBird::SetSpeed)
      .def_readonly("force", &CBird::force_)
      .def_readonly("acceleration", &CBird::accel_)
      .def_readwrite("cohesion", &CBird::cohesion_)
      .def_readwrite("gyro", &CBird::gyro_)
      .def_readwrite("boundary", &CBird::boundary_)
      .def_readwrite("steering", &CBird::steering_)
      .def_readwrite("separation", &CBird::separation_)
      .def_readwrite("alignment", &CBird::alignment_)
      .def_readwrite("cohesion", &CBird::cohesion_)
      .property("ColorTex", &CBird::getCurrentColorTex, &CBird::setCurrentColorTex)
      .property("id", (int (CBird::*)()) &CBird::id)
      .def("SetTrail", (void (CBird::*)(bool)) &CBird::setTrail)
      .def("HasTrail", (bool (CBird::*)()) &CBird::hasTrail)
      .def("visit", (float (*)(const CBird&, int)) &libBirds::luaVisit)
      .def("interactionNeighbors", &CBird::interactionNeighbors)
      .def("NNInfo", (neighborInfo* (CBird::*)()) &CBird::nearestNeighborInfo)
      .def("reactionTime", &CBird::reactionTime)
      .def("reactionInterval", &CBird::reactionInterval),
    
    class_<CPrey, CBird>("__prey")
      .property("PreyParams", (Param::Prey& (CPrey::*)()) &CPrey::GetPreyParams, &CPrey::SetPreyParams)
      .def_readonly("circularity", &CPrey::circularity_)
      .def_readonly("circularityVec", &CPrey::circularityVec_)
      .def_readwrite("predatorReaction", &CPrey::predatorReaction_)
      .def_readwrite("predatorForce", &CPrey::predatorForce_)
      .def_readonly("predatorDist", &CPrey::predatorDist_)
      .def_readonly("detectedPredator", &CPrey::detectedPredator_)
      .def_readonly("panicRelaxation", &CPrey::alertnessRelaxation_)
      .def_readwrite("panicOnset", &CPrey::panicOnset_)
      .def_readwrite("panicCopy", &CPrey::panicCopy_)
      .def("visit", (float (*)(const CPrey&, int)) &libBirds::luaVisit)
      .def("GetFlockId", (int (CPrey::*)()) &CPrey::getFlockId)
      .def("GetFlockSize", (unsigned (CPrey::*)()) &CPrey::getFlockSize)
      .def("GetFlockCluster", (const cluster_entry* (*)(const CPrey&)) &libBirds::GetFlockCluster),
    
    class_<CPredator, CBird>("__predator")
      .property("PredParams", (Param::Predator& (CPredator::*)()) &CPredator::GetPredParams, &CPredator::SetPredParams)
      .def("GetTargetPrey", (CPrey* (CPredator::*)()) &CPredator::GetTargetPrey)
      .def("SetTargetPrey", (void (CPredator::*)(const CPrey*)) &CPredator::SetTargetPrey)
      .def("GetHuntStat", (CPredator::hunt* (CPredator::*)()) &CPredator::hunts)
      .def("ResetHunt", (void (CPredator::*)()) &CPredator::ResetHunt)
      .def("EndHunt", &CPredator::EndHunt)
      .def("StartAttack", (void (CPredator::*)()) &CPredator::BeginHunt)
      .def("is_attacking", &CPredator::is_attacking)
      .def("visit", (float (*)(const CPredator&, int)) &libBirds::luaVisit),

    class_<cluster_entry>("__cluster_entry")
      .def_readonly("bbox", &cluster_entry::bbox)
      .def_readonly("size", &cluster_entry::size)
      .def_readonly("loudest", &cluster_entry::loudest)
      .def_readonly("velocity", &cluster_entry::velocity),

    class_<CFlock>("__flock")
      .def("FindPrey", (const CPrey* (CFlock::*)(int)) &CFlock::FindPrey)
      .def("num_prey", (unsigned (CFlock::*)()) &CFlock::num_prey)
      .def("num_prdators", (unsigned (CFlock::*)()) &CFlock::num_pred)
      .def("num_clusters", (unsigned (CFlock::*)()) &CFlock::num_clusters)
      .def("nextID", (int (CFlock::*)())&CFlock::nextID)
      .def("cluster", &CFlock::cluster)
  ];
}

