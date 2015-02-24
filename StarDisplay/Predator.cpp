#include <cassert>
#include <glmutils/avx/vec.hpp>
#include <glmutils/clip_length.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/random.hpp>
#include "random.hpp"
#include "glmfwd.hpp"
#include "Predator.hpp"
#include "Prey.hpp"
#include "Flock.hpp"
#include "Params.hpp"
#include "visitors.hpp"
#include "Globals.hpp"
#include "Mmsystem.h"
#include <stdlib.h> 
#include <iostream>


using namespace Param;
namespace avx = glmutils::avx;


struct find_prey_qf
{
  avx::vec3               position_;
  avx::vec3               forward_;
  CPredator*              pivot_;
  float                   rr_;    // squared search radius
  float                   cosBlindAngle_;
  float                   cosLockBlindAngle_;
  float                   attract_;
  float                   minDist_;
  const CPrey*            lockedOn_;
  const CPrey*            closestPrey_;
  const CPrey*            currentLock_;
  const Param::Predator&  param_;

  find_prey_qf(const Param::Predator& param) : param_(param) {}
  void Init(CPredator* pivot, float radius)
  { 
    position_ = pivot->position();
    forward_ = pivot->forward();
    pivot_ = pivot; 
    rr_ = radius*radius;
    cosBlindAngle_ = BlindCosine(pivot->GetBirdParams().blindAngle);
    cosLockBlindAngle_ = BlindCosine(pivot->GetPredParams().LockBlindAngle);
    attract_ = 0.0f;
    minDist_ = 10000000.0f;
    lockedOn_ = 0;
    closestPrey_ = 0;
    currentLock_ = pivot->GetLockedOn();
    if (currentLock_)
    {
      // check if lock is lost due to circularity
      if (currentLock_->circularity() < param_.ExposureThreshold[1])
      {
        currentLock_ = 0;
      }
    }
  }

  void operator()(const CPrey& prey)
  {
    avx::vec3 dir;
    float distance;
    if (test_distance(prey, position_, rr_, dir, distance))
    {
      const float cangle = avx::dot(forward_, dir);
      if (cangle >= cosBlindAngle_)
      {
        if ((cangle >= cosLockBlindAngle_) && (distance < param_.LockDistance))
        {
          float attract = 0.0f;
          const float Ci = prey.circularity();
          if (param_.HoldLock && (&prey == currentLock_))
          {
            // The prey in view was already locked - hold the lock.
            attract = 100000.0f;
          }
          else 
          {
            const float expT = param_.ExposureThreshold[0];
            const float distFact = 1.0f - distance / param_.LockDistance;
            const float exposureFactor = (Ci < expT) ? 0.0f : ((Ci - expT) / (1.0000001f - expT));
            attract = param_.AttractMix * distFact + (1.0f - param_.AttractMix) * exposureFactor;
          }
          if (attract > attract_)
          {
            attract_ = attract;
            lockedOn_ = &prey;
          }
        }
        pivot_->neighbors_.insert(prey, distance, dir, cangle, 0.0f);
      }
      if (distance < minDist_)
      {
        minDist_ = distance;
        closestPrey_ = &prey;
      }
    }
  }
};


CPredator::handleAttackMPTR CPredator::startAttack[Param::Predator::MaxAttackStrategy__] = {
  &CPredator::ManualStartAttack,
  &CPredator::AutoStartAttack,
  &CPredator::EvolveStartAttack
};


CPredator::selectPreyMPTR CPredator::selectPrey[Param::Predator::MaxPreySelection__] = {
  &CPredator::SelectAuto,
  &CPredator::SelectPicked,
  &CPredator::SelectPickedTopo
};


CPredator::hunt& CPredator::hunt::operator+=(const CPredator::hunt& h)
{
  sequences += h.sequences;
  locks += h.locks;
  success += h.success;
  minDist = std::min(minDist, h.minDist);
  minDistLockedOn = std::min(minDistLockedOn, h.minDistLockedOn);
  seqTime += h.seqTime;
  lookTime += h.lookTime;
  std::copy(h.attackSize.begin(), h.attackSize.end(), std::back_inserter(attackSize));
  std::copy(h.catchSize.begin(), h.catchSize.end(), std::back_inserter(catchSize));
  return *this;
}


CPredator::CPredator(int ID, const glm::vec3& position, const glm::vec3& forward)
: CBird(ID, position, forward),
  attackTime_(0),
  handleTime_(0),
  dogFight_(0),
  lockedOn_(0),
  closestPrey_(0),
  targetPrey_(0),
  targetPoint_(0),
  locks_(0)
{
}


CPredator::~CPredator()
{
}


void CPredator::SetPredParams(const Param::Predator& pPred)
{
  pPred_ = pPred;
}


void CPredator::NumPreyChanged()
{
  neighbors_.clear();
  lockedOn_ = 0;
  closestPrey_ = 0;
  targetPrey_ = 0;
}


void CPredator::NumPredChanged()
{
}


void CPredator::updateNeighbors(float dt, const CFlock& flock)
{
  //
  // Warning: this function is called from inside a parallel section!
  //
  if ((reactionTime_ += dt) >= reactionInterval_) 
  {
    neighbors_.clear();
    if (is_attacking()) 
    {
      hunts_.seqTime += reactionTime_;
      find_prey_qf fnqf(pPred_);
      (this->*selectPrey[pPred_.PreySelection])(fnqf);
      if ((0 != fnqf.lockedOn_) && (lockedOn_ != fnqf.lockedOn_)) 
      {
        // locked prey has switched
        if (0.0f == dogFight_) dogFight_ = 0.0001f;   // first lock in sequence
        ++hunts_.locks;
        ++locks_;
        const cluster_entry* pce = flock.cluster(fnqf.lockedOn_->getFlockId());
        hunts_.attackSize.push_back((0 == pce) ? 1 : pce->size);
      }
      lockedOn_ = fnqf.lockedOn_;
      closestPrey_ = fnqf.closestPrey_;
      if (lockedOn_) 
      {
        hunts_.lookTime += reactionTime_;
      }
      else
      {
        if (dogFight_ > 0.0f) dogFight_ += dt;
      }
      if ((dogFight_ > pPred_.Dogfight) || (locks_ > pPred_.maxLocks))
      {
        EndHunt(false);
      }
    } 
    else 
    {
      lockedOn_ = 0;
      closestPrey_ = 0;
    }
  }
}


void CPredator::update(float dt, const CFlock&)
{
  //
  // Warning: this function is called from inside a parallel section!
  //
  if (reactionTime_ >= reactionInterval_)
  {
    steering_ = cohesion_ = boundary_ = gyro_ = glm::vec3(0);
    if (! is_attacking()) 
    {
      (this->*startAttack[pPred_.StartAttack]);
    }
    if (is_attacking()) 
    {
      wBetaIn_ = pPred_.AttackWBetaIn;
      wBetaOut_ = pPred_.AttackWBetaOut;
      steerToFlock();
      if (interaction_neighbors_ == 0) 
      {
        handleBoundary(pBird_.altitude, position_.y); 
      }
    }
    else 
    {
      wBetaIn_ = pBird_.wBetaIn;
      wBetaOut_ = pBird_.wBetaOut;
      handleBoundary(pBird_.altitude, position_.y); 
    }
    handleGPWS();
    steering_ += boundary_;
    steering_ += speedControl() * B_[0];
	if (GetAsyncKeyState(VK_DOWN)) steering_ +=  B_[1];
	if (GetAsyncKeyState(VK_NUMPAD2)) steering_ += 5.0f*B_[1];
	if (GetAsyncKeyState(VK_UP)) steering_ -= B_[1];
	if (GetAsyncKeyState(VK_NUMPAD8)) steering_ -= 5.0f*B_[1];
	if (GetAsyncKeyState(VK_RIGHT)) steering_ += B_[2];
	if (GetAsyncKeyState(VK_NUMPAD6)) steering_ += 5.0f*B_[2];
	if (GetAsyncKeyState(VK_LEFT)) steering_ -= B_[2];
	if (GetAsyncKeyState(VK_NUMPAD4)) steering_ -= 5.0f*B_[2];
	if (GetAsyncKeyState(VK_NUMPAD5)) steering_ += 3.0f*B_[0];
	if (GetAsyncKeyState(VK_NUMPAD0)) steering_ += 10.0f*B_[0];
	rand_ = float(rand()) / (float(RAND_MAX)*100) +0.8 * rand_;
	std::cout << "\n" << rand_;
	std::cout << "\n" << float(rand()) / (float(RAND_MAX));
    noise();    // add some noise

    avx::vec3 force(steering_);
    const float f2 = avx::length2(force);
    if (f2 > pBird_.maxForce * pBird_.maxForce) {
      force /= avx::fast_sqrt(f2);
      force *= pBird_.maxForce;
      force.store(steering_);
    }
    force.store(force_);

    // calculate time of next reaction
    nextReactionTime();
  }

  // Physics works in real time...
  handleTime_ -= dt;
  flightDynamic();
  integration(dt);
  regenerateLocalSpace(dt);

  if (closestPrey_)
  {
    // Check for collisions in real-time
    const float distance = glm::distance(position_, closestPrey_->position());
    if (distance < hunts_.minDist) hunts_.minDist = distance;
  }
  if (0 == lockedOn_)
  {
    bool attack = is_attacking();
    attackTime_ -= dt;
    if (attack && (attackTime_ < 0.0)) EndHunt(false);
  }
  else
  {
    // Check for catches in real-time
    const float distance = glm::distance(position_, lockedOn_->position());
    if (distance < hunts_.minDistLockedOn) hunts_.minDistLockedOn = distance;
    if (distance <= pPred_.CatchDistance)
    {
      // EndHunt(true);
    }
  }
  
  appendTrail(trail_, position_, B_[2], color_tex_, dt);
}


void CPredator::flightDynamic()
{
  const float pi = glm::pi<float>();
  const float CL = pBird_.CL;
  const float CDCL = 1.0f / ((pi * pBird_.wingAspectRatio) / CL);
  const float L = pBird_.bodyWeight * (speed_ * speed_) / (pBird_.cruiseSpeed * pBird_.cruiseSpeed);  // Lift
  const float D = CDCL * L;   // Drag
  lift_ = B_[1] * std::min(L, pBird_.maxLift);
  flightForce_ = lift_ + B_[0] * (CDCL * pBird_.bodyWeight - D); // apply clamped lift, drag and default thrust
  flightForce_.y -= pBird_.bodyWeight;        // apply gravity
}


// basic flocking
void CPredator::steerToFlock()
{
  // determine each of the three behaviors of flocking
  neighbors_vect::iterator first(neighbors_.begin());
  neighbors_vect::iterator last(neighbors_.end());

  steering_ = targetPoint_ = cohesion_ = gyro_ = glm::vec3(0);
  glm::vec3 aveVelocity(0);
  glm::vec3 aveHeading(0);
  if (0 == lockedOn_)
  {
    cohesion_neighbors_ = 0;
    for (; first != last; ++first) 
    {
      first->interacting = true;
      aveVelocity += (*first).speed * (*first).forward;
      aveHeading += (*first).forward;
      targetPoint_ += (*first).position;
      ++cohesion_neighbors_;
    }
    if (cohesion_neighbors_ > 0) 
    {
      first = neighbors_.begin();
      aveVelocity /= cohesion_neighbors_;
      aveHeading = glmutils::save_normalize(aveHeading, (*first).forward);
      targetPoint_ /= cohesion_neighbors_;
    }
  }
  else
  {
    aveVelocity = lockedOn_->velocity();
    aveHeading = lockedOn_->forward();
    targetPoint_ = lockedOn_->position();
    cohesion_neighbors_ = 1 + static_cast<int>(pBird_.topologicalRange);    // fake: no need to increase search radius
  }
  // Pursuit
  if (cohesion_neighbors_)
  {
    PursuitCustom(aveHeading, aveVelocity);
    //cohesion_ = H_ * (cohesion_ * H_);
    steering_ += cohesion_;
  }
  interaction_neighbors_ = cohesion_neighbors_;
}


void CPredator::ManualStartAttack()
{
}


void CPredator::AutoStartAttack()
{
  if ((position_.y > pBird_.altitude) && (handleTime_ < 0.0f))
  {
    BeginHunt();
  } 
}


void CPredator::EvolveStartAttack()
{
  if (handleTime_ > 0.0f)
  {
    position_ = PROOST.Radius * glmutils::vec3_in_sphere(rnd_eng());
    position_.y = pBird_.altitude;
    setTrail(false);
    BeginHunt();
    setTrail(true);
  } 
}


void CPredator::BeginHunt()
{
  searchRadius_ = pBird_.maxRadius;
  ++hunts_.sequences;
  handleTime_ = 0;
  attackTime_ = pPred_.AttackSpan;
}


void CPredator::EndHunt(bool success)
{
  if (success) ++hunts_.success;
  lockedOn_ = 0;
  closestPrey_ = 0;
  attackTime_ = 0;
  dogFight_ = 0;
  locks_ = 0;
  handleTime_ = pPred_.HandleTime;
}


void CPredator::ResetHunt()
{
  EndHunt(false);
  hunts_ = hunt();
}


void CPredator::SetTargetPrey(const CPrey* prey)
{
  targetPrey_ = prey;
}


void CPredator::SelectAuto(find_prey_qf& fnqf)
{
  targetPrey_ = 0;
  const float nl = pBird_.topologicalRange / std::max<int>(interaction_neighbors_, 1);
  const float r = searchRadius_ * std::pow(nl, 1.0f/3.0f);
  searchRadius_ = glm::mix(searchRadius_, r, pBird_.neighborLerp);
  searchRadius_ = std::min(searchRadius_, pBird_.maxRadius);
  fnqf.Init(this, searchRadius_);
  GFLOCK.query(fnqf, position_, searchRadius_);
}


void CPredator::SelectPicked(find_prey_qf& fnqf)
{
  fnqf.Init(this, pBird_.maxRadius);
  if (targetPrey_)
  {
    fnqf(*targetPrey_);
  }
}


void CPredator::SelectPickedTopo(find_prey_qf& fnqf)
{
  fnqf.Init(this, pBird_.maxRadius);
  if (targetPrey_)
  {
    GFLOCK.query(fnqf, targetPrey_->position(), targetPrey_->searchRadius());
  }
}


void CPredator::setDefaultColorTex() const
{
  color_tex_ = GetLockedOn() ? 1.0f : (is_attacking() ? 0.85f : 0.5f);
}


void CPredator::PursuitCustom(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{
  if (pPred_.pursuit.hook.is_valid())
  {
    auto lock = Lua.LuaLock();
    this->pPred_.pursuit.hook(this, Sim.SimulationTime(), targetPoint_, targetHeading, targetVelocity);
  }
}

