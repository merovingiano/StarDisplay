//! \file Predator.h The Predator
//! \ingroup Model

#ifndef PREDATOR_HPP_INCLUDED
#define PREDATOR_HPP_INCLUDED


#include "Bird.hpp"


class CFlock;
class CPrey;


//! Predator class
//! \ingroup Simulation
class CPredator : public CBird
{
  CPredator();
  CPredator(const CPredator&);
  CPredator& operator=(const CPredator&);
  CPredator(CPredator&&);
  CPredator& operator=(CPredator&&);

public:
  struct hunt
  {
    hunt() : sequences(0), locks(0), success(0),
             minDist(999.99f), minDistLockedOn(999.99f), 
			 seqTime(0), lookTime(0), lastMinDist(999.99f)
    {}
    hunt& operator+=(const hunt& h);
    int sequences;
    int locks;
    int success;
    float minDist;
	float velocityMinDist;
    float minDistLockedOn;
    double seqTime;
    double lookTime;
    std::vector<unsigned> attackSize;
    std::vector<unsigned> catchSize;
	//! add the miss distance of last attack too
    float lastMinDist;

  };

public:
  CPredator(int ID, const glm::vec3& position, const glm::vec3& forward);
  virtual ~CPredator();

  virtual bool isPrey() const { return false; }
  virtual bool isPredator() const { return true; }
  
  virtual void NumPreyChanged();
  virtual void NumPredChanged();

  const Param::Predator& GetPredParams() const { return pPred_; }
  Param::Predator& GetPredParams() { return pPred_; }
  void SetPredParams(const Param::Predator& pPred);

  void updateNeighbors(float dt, const CFlock& flock);
  void update(float dt, const CFlock&);

  bool is_attacking() const { return attackTime_ > 0.0; }
  const CPrey* GetLockedOn() const { return lockedOn_; }
  const CPrey* GetTargetPrey() const { return targetPrey_; }
  //! bunch of functions to set and get variables
  float get_N() const { return N_; }
  float getDPAdjParam() const { return DPAdjParam_; }
  float getStartAltitude() const { return startAltitude_; }
  float getStartXDist() const { return XDist_; }
  float getStartZDist() const { return ZDist_; }
  float getGeneration() const { return generation_; }
  void set_N(float N) {N_ = N; }
  void setDPAdjParam(float DPAdjParam) { DPAdjParam_ = DPAdjParam; }
  void setStartAltitude(float start) { startAltitude_ = start; }
  void setStartXDist(float XDist) { XDist_ = XDist; }
  void setStartZDist(float ZDist) { ZDist_ = ZDist; }
  void setGeneration(float generation) { generation_ = generation; }
  void SetTargetPrey(const CPrey*);
  const glm::vec3& TargetPoint() const { return targetPoint_; }
  const hunt& hunts() const { return hunts_; }
  void BeginHunt();
  void testSettings();
  void EndHunt(bool success);
  void ResetHunt();
  
  typedef std::vector<glm::vec4>   Storage;
  Storage positionsAndSpeed;

  void setDefaultColorTex() const;

  // Evolving deflection needs this
  const glm::vec3& getDeflection() const { return pPred_.pursuit.deflection; }
  void setDeflection(const glm::vec3& deflection) { pPred_.pursuit.deflection = deflection; }

private:
  friend struct find_prey_qf;

  void Accelerate();
  void flightDynamic();
  void steerToFlock();
  void ignoreFlock();
  void predatorIntegration(float dt);
  void predatorRegenerateLocalSpace(float dt);
  void ManualStartAttack();
  void AutoStartAttack();
  void EvolveStartAttack();

  void SelectAuto(struct find_prey_qf&);
  void SelectPicked(struct find_prey_qf&);
  void SelectPickedTopo(struct find_prey_qf&);

  void PursuitCustom(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);
  void proportionalNavigation(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);
  void PNDP(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);
  void DirectPursuit(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);
  void DirectPursuit2(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);
  void checkEndHunt(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);

  Param::Predator  pPred_;
  float            attackTime_;

  float            handleTime_;
  float            dogFight_;
  const CPrey*     lockedOn_;
  const CPrey*     closestPrey_;
  const CPrey*     targetPrey_;
  glm::vec3        targetPoint_;
  hunt             hunts_;
  int              locks_;        // locks in current attack
  //! some new parameters, probably will move them to params
  float            N_;
  float			   DPAdjParam_;
  float            startAltitude_;
  float            XDist_;
  float            ZDist_;
  float            generation_;

  
  typedef void (CPredator::*handleAttackMPTR)();
  typedef void (CPredator::*selectPreyMPTR)(struct find_prey_qf&);
  static handleAttackMPTR startAttack[Param::Predator::MaxAttackStrategy__];
  static selectPreyMPTR selectPrey[Param::Predator::MaxPreySelection__];
};



#endif
