#include <fstream>
#include <glmutils/ostream.hpp>
#include <glmutils/homogeneous.hpp>
#include "PredatorStatistic.hpp"
#include "Flock.hpp"
#include "HistOverlay.hpp"
#include "ICamera.hpp"
#include "IText.hpp"
#include "GLSLState.hpp"
#include "visitors.hpp"
#include "Simulation.hpp"
#include "Globals.hpp"


using namespace Param;


PredatorStatistic::PredatorStatistic()
  : DefaultStatistic()
{
}


void PredatorStatistic::Reset()
{
  std::for_each(GFLOCKNC.predator_begin(), GFLOCKNC.predator_end(), [] (CPredator& pred)
  {
    pred.ResetHunt();
  });
}


void PredatorStatistic::Display() const
{
  CPredator::hunt hunts;
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&hunts] (const CPredator& pred) 
  {
    hunts += pred.hunts();
  });
  char buf[512];
  _snprintf_s(buf, 511,
    "Predators:  %d   \n"
    "Sequences:  %d   \n"
    "Locks:      %d   \n"
    "Catches:    %d   \n"
    "Seq. time:  %d s \n"
    "Look time:  %d s \n"
    "min. Dist:  %.2f m ",
    GFLOCK.num_pred(), hunts.sequences, hunts.locks, hunts.success, static_cast<int>(hunts.seqTime), static_cast<int>(hunts.lookTime), hunts.minDistLockedOn);
  GTEXT->print("\\monoface{}");
  glm::ivec4 r(GCAMERA.GetViewport());
  glm::ivec2 ext = GTEXT->extent(buf);
  GTEXT->set_color(GGl.textColor());
  GTEXT->set_orig(glm::ivec2(r.z - ext.x, r.w - ext.y + GTEXT->base() - 4));
  GTEXT->print(buf);
}


void PredatorStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  os << "% " << PFME(PredatorStats).title << "\n\n";
  CPredator::hunt hunts;
  std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [&hunts] (const CPredator& pred) 
  {
    hunts += pred.hunts();
  });
  char buf[512];
  _snprintf_s(buf, 511, "SimTime = %d;\nPredators = %d;\nSequences = %d;\nLocks = %d;\nCatches = %d;\nSeqTime = %d;\nLookTime = %d;\n",
    static_cast<int>(Sim.SimulationTime()), GFLOCK.num_pred(), hunts.sequences, hunts.locks, hunts.success, static_cast<int>(hunts.seqTime), static_cast<int>(hunts.lookTime));
  os << buf;
  std::ostream_iterator<unsigned> osit(os, " ");
  os << "attackSize = [ ";
  std::copy(hunts.attackSize.begin(), hunts.attackSize.end(), osit);
  os << " ];\n";
  os << "catchSize = [ ";
  std::copy(hunts.catchSize.begin(), hunts.catchSize.end(), osit);
  os << " ];\n";
  os << std::endl;
}



PredatorVidStatistic::PredatorVidStatistic()
  : DefaultStatistic()
{
  Reset();
}


void PredatorVidStatistic::Reset()
{
  catched = -1;
  time.clear();
  predPos.clear();
  preyPos.clear();
  project.clear();
  const unsigned N = GFLOCK.num_prey();
  for (unsigned i=0; i<N; ++i)
  {
    preyPos.emplace_back();
    project.emplace_back();
  }
  lockedOn.clear();
  banking.clear();
  speed.clear();
  distance.clear();
}


void PredatorVidStatistic::apply(double stat_dt)
{
  CFlock::pred_const_iterator pit = GFLOCK.predator_begin(); 
  if ((pit == GFLOCK.predator_end()) || (!pit->is_attacking()) ) 
  {
    return;
  }
  const CPredator& pred = *pit;
  time.push_back(Sim.SimulationTime());
  glm::mat4 M(GCAMERA.ModelViewProjectionMatrix());
  CFlock::prey_const_iterator first(GFLOCK.prey_begin());
  CFlock::prey_const_iterator last(GFLOCK.prey_end());
  for (; first != last; ++first)
  {
    const size_t id = static_cast<size_t>((*first).id()) - 1;
    if (id < preyPos.size())
    {
      preyPos[id].push_back( (*first).position() );
      project[id].push_back( glmutils::transformPoint(M, (*first).position()) );
    }
  }
  if (GFLOCK.num_pred())
  {
    const CBird* locked = pred.GetLockedOn();
    lockedOn.push_back((locked) ? locked->id() : -1);
    banking.push_back(visit<FeatureMap::Banking>(pred));
    speed.push_back(visit<FeatureMap::Speed>(pred));
    if (pred.hunts().success != 0)
    {
      catched = 1;
    }
    distance.push_back(pred.nsize() ? pred.nearestNeighborInfo()->distance : 0);
    predPos.push_back( pred.position() );
  }
}


std::string PredatorVidStatistic::labelText() const
{
  return PCFME.title;
}


namespace {

  template<typename C>
  void saveVect(std::ostream& os, const C& coll)
  {
    std::ostream_iterator<typename C::value_type> osit(os, ";");
    os << " = [ "; std::copy(coll.begin(), coll.end(), osit); os << " ];\n";
  }

}


void PredatorVidStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  os << "P = {}; R = {};\n";
  os << "T"; saveVect(os, time);
  os << "bank"; saveVect(os, banking);
  os << "lock"; saveVect(os, lockedOn);
  os << "speed"; saveVect(os, speed);
  os << "dist"; saveVect(os, distance);
  for (size_t i = 0; i<project.size(); ++i)
  {
    os << "P{" << i+1 << '}'; saveVect(os, project[i]);
    os << "R{" << i+1 << '}'; saveVect(os, preyPos[i]);
  }
  os << "Rp"; saveVect(os, predPos);
}

