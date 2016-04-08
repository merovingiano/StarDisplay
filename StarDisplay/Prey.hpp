//! \defgroup Model The starling model implementation
//! \file Prey.hpp The 'prey'
//! \ingroup Model

#ifndef PREY_HPP_INCLUDED
#define PREY_HPP_INCLUDED

#include "Bird.hpp"


class CFlock;
class CPredator;


namespace PredationReactions 
{
  const int None = 0;
  const int Return = 1;
  const int Alerted = 2;
  const int Panic = 4;
  const int Detectable = 8;
}


//! CPrey class
//! \ingroup Simulation
class CPrey : public CBird
{
  CPrey();
  CPrey(const CPrey&);
  CPrey& operator=(const CPrey&);
  CPrey(CPrey&&);
  CPrey& operator=(CPrey&&);

public:
  CPrey(int id, const glm::vec3& position, const glm::vec3& forward);
  virtual ~CPrey();

  virtual bool isPrey() const { return true; }
  virtual void NumPreyChanged();
  virtual void NumPredChanged();

  const Param::Prey& GetPreyParams() const { return pPrey_; }     //!< get prey parameter
  Param::Prey& GetPreyParams() { return pPrey_; }                 //!< get prey parameter
  void SetPreyParams(const Param::Prey& prey);                    //!< Set Prey parameter

  const glm::vec3& predatorForce() const  { return predatorForce_; }   //!< predator induced force

  void setDefaultColorTex() const { color_tex_ = 0.75f; }
  float getCurrentColorTex() const { return color_tex_; }
  void setCurrentColorTex(float val) const { color_tex_ = val; }

  float separationRadius() const { return pBird_.separationStep.y; }

  //! Collect information about neighbors.
  //! Called once per frame.
  void updateNeighbors(float dt, const CFlock& flock);

  //! Integrate forces.
  //! Called once per frame.
  void update(float dt, const CFlock& flock);

  //! \return Remaining alertness relaxation time
  glm::vec2 alertnessRelaxation() const { return alertnessRelaxation_; }

  //! \return nearest detected predator in range.
  const CPredator* detectedPredator() const { return detectedPredator_; }

  //! \return distance to nearest detected predator in range.
  float predatorDist() const { return predatorDist_; }

  float get_average_lat_acceleration() const { return average_lat_acceleration_; }
  float get_max_lat_acceleration() const { return max_lat_acceleration_; }
  float get_average_for_acceleration() const { return average_for_acceleration_; }
  float get_max_for_acceleration() const { return max_for_acceleration_; }
  float get_average_roll_rate() const { return average_roll_rate_; }
  float get_max_roll_rate() const { return max_roll_rate_; }
  long get_counter_acc() const { return counter_acc_; }
  void set_counter_acc(float counter) { counter_acc_ = counter; }
  void handleTrajectoryStorage();
  void handleManeuvers();

  //! \return predator reaction
  int predatorReaction() const { return predatorReaction_; }

  //! \return Remaining 'return to flock' relaxation time
  float returnRelaxation() const { return returnRelaxation_; }

  //! \return scalar circularity (length of circularityVec).
  float circularity() const { return circularity_; }

  //! \return vectorial circularity.
  glm::vec3 circularityVec() const { return circularityVec_; }

  void invalidateFlockId() { flockId_ = -1; flockSize_ = 0; }
  void setFlockId(int id, unsigned size) { flockId_ = id; flockSize_ = size; }
  int getFlockId() const { return flockId_; }
  unsigned getFlockSize() const { return flockSize_; }


  typedef std::vector<glm::vec4>   Storage;
  Storage positionsAndSpeed;

  Storage externalPos;

private:
  friend struct find_neighbors_qf;

  //! Calculate flight forces
  void flightDynamic();
  void flightExternal();
  void maneuver_lat_roll();
  void maneuver_lat_roll2();
  void maneuver_lat();
  void calculateAccelerations();
  //! Handles main social interaction rules
  void steerToFlock(struct fov_filter const& filter);

  //! Set closestPredator_ to closest predator if any in detect range.
  void detectClosestPredator(const CFlock& flock);
  void handleEvasion();
  void predatorPanicMaximizeDist();				//! Try to increase expected closest approach.
  void predatorPanicTurnInward();					//! Turn along circularity vector.
  void predatorPanicTurnAway();						//! Turn in opposite direction to predator.
  void testSettings();
	void predatorPanicDrop();						//! Drop out of sky.
  void predatorPanicMoveCentered();		//! Move towards center.
  void predatorPanicZig();		  //! Left-right evasion.
  void predatorPanicCustom();		      //! Callback Lua.
  void return2Flock(const CFlock& flock);

public:
  glm::vec3 circularityVec_;
  float     circularity_;
  glm::vec3 predatorForce_;
  const     CPredator* detectedPredator_;
  float     predatorDist_;
 
  glm::vec2 alertnessRelaxation_;
  glm::vec3 returnForce_;
  float     returnRelaxation_;
  double    panicOnset_;
  int       panicCopy_;
  int       predatorReaction_;
  


private:
  int         skippedLeftHemisphere_;
  int         skippedRightHemisphere_;
  Param::Prey pPrey_;
  glm::vec2   alignmentWeight_;
  int         flockId_;
  unsigned    flockSize_;
  float     average_lat_acceleration_;
  float     max_lat_acceleration_;
  float     average_for_acceleration_;
  float     max_for_acceleration_;
  float     average_roll_rate_;
  float     max_roll_rate_;
  long      counter_acc_;
  ;

};


#endif
