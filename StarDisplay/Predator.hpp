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
             seqTime(0), lookTime(0) 
    {}
    hunt& operator+=(const hunt& h);
    int sequences;
    int locks;
    int success;
    float minDist;
    float minDistLockedOn;
    double seqTime;
    double lookTime;
    std::vector<unsigned> attackSize;
    std::vector<unsigned> catchSize;
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
  void SetTargetPrey(const CPrey*);
  const glm::vec3& TargetPoint() const { return targetPoint_; }
  const hunt& hunts() const { return hunts_; }
  void BeginHunt();
  void EndHunt(bool success);
  void ResetHunt();

  void setDefaultColorTex() const;

  // Evolving deflection needs this
  const glm::vec3& getDeflection() const { return pPred_.pursuit.deflection; }
  void setDeflection(const glm::vec3& deflection) { pPred_.pursuit.deflection = deflection; }

private:
  friend struct find_prey_qf;

  void flightDynamic();
  void steerToFlock();
  void ignoreFlock();

  void ManualStartAttack();
  void AutoStartAttack();
  void EvolveStartAttack();

  void SelectAuto(struct find_prey_qf&);
  void SelectPicked(struct find_prey_qf&);
  void SelectPickedTopo(struct find_prey_qf&);

  void PursuitCustom(const glm::vec3& targetHeading, const glm::vec3& targetVelocity);

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
  
  typedef void (CPredator::*handleAttackMPTR)();
  typedef void (CPredator::*selectPreyMPTR)(struct find_prey_qf&);
  static handleAttackMPTR startAttack[Param::Predator::MaxAttackStrategy__];
  static selectPreyMPTR selectPrey[Param::Predator::MaxPreySelection__];
};



#endif
