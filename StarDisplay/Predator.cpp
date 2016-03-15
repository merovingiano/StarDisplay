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
#include "ICamera.hpp"
#include <glm/gtc/matrix_access.hpp>


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
  if (minDist < h.minDist) velocityMinDist = h.velocityMinDist;
  minDist = std::min(minDist, h.minDist);
  lastMinDist = h.minDist;
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
	locks_(0),
	N_(0)
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
		  std::cout << "JAJAJAJAJAJA";
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
        //handleBoundary(pBird_.altitude, position_.y); 
      }
    }
    else 
    {
      //wBetaIn_ = pBird_.wBetaIn;
      //wBetaOut_ = pBird_.wBetaOut;
      //handleBoundary(pBird_.altitude, position_.y); 
    }
    //handleGPWS();
    //steering_ += boundary_;
    //steering_ += speedControl() * B_[0];
	//! Controlling the bird. 
	if ((GCAMERA.GetFocalBird())->id() == id_)
	{
		//std::cout << pPred_.VisualError << "\n";
		//std::cout << pPred_.VisualBias.x << "  " << pPred_.VisualBias.y << "\n";


		if (!GetAsyncKeyState(VK_F11) && keyState_ != 0)
		{
			keyState_ = 0;
		}
		if (GetAsyncKeyState(VK_F11) && keyState_ == 0)
		{
			testSettings();

		}

		if (GetAsyncKeyState(VK_DOWN)) steering_ += 5.0f*glm::vec3(0, 1, 0);
		if (GetAsyncKeyState(VK_NUMPAD2)) steering_ += glm::length(liftMax_)*glm::vec3(0, 1, 0);
		if (GetAsyncKeyState(VK_UP)) steering_ -= 5.0f*glm::vec3(0, 1, 0);;
		if (GetAsyncKeyState(VK_NUMPAD8)) steering_ -= glm::length(liftMax_)*glm::vec3(0, 1, 0);
		if (GetAsyncKeyState(VK_RIGHT)) steering_ += 5.0f*H_[2];
		if (GetAsyncKeyState(VK_NUMPAD6)) steering_ += glm::length(liftMax_)*H_[2];
		if (GetAsyncKeyState(VK_LEFT)) steering_ -= 5.0f*H_[2];
		if (GetAsyncKeyState(VK_NUMPAD4)) steering_ -= glm::length(liftMax_)*H_[2];
		if (GetAsyncKeyState(VK_NUMPAD5)) steering_ += 3.0f*H_[0];
		if (GetAsyncKeyState(VK_NUMPAD0)) steering_ += 1.0f*H_[0];

		if (GetAsyncKeyState(VK_F7))
		{
			position_.y = 1200.0f;
			position_.x = 1.0f;
			position_.z = 1.0f;
			B_[0] = glm::normalize(-position_);

			velocity_ = 80.0f *B_[0];
			N_ = 2.0f;
		}
		if (GetAsyncKeyState(VK_NUMPAD5)) N_ = 5.0f;

		//if (GetAsyncKeyState(VK_NUMPAD8)) std::cout << "\n Yes"; else std::cout << "\n No";

	}
	//! saving the position of some predator in a genetic algorithm (will be more general code when I come back)
	if (GFLOCKNC.predator_begin()->id() == id_  && Sim.Params().evolution.TrajectoryBestPredator)
	{
			positionsAndSpeed.push_back(glm::vec4(position_, glm::length(velocity_)));
		
			
	}

	//! handle acceleration
	//Accelerate();
	//!Quick hack for StarDisplay problem in steering when too low to the ground
	//if (position_.y < 1) steering_ += 5.0f*B_[1];
	//! used to mimick wind effects in graphics
	rand_ = float(rand()) / (float(RAND_MAX)*100) +0.8 * rand_;
    //noise();    // add some noise
	//! Clamped force no longer used. Just for graphics
    avx::vec3 force(steering_);


    // calculate time of next reaction
    nextReactionTime();
  }

  // Physics works in real time...
  handleTime_ -= dt;
  flightDynamic();
  predatorIntegration(dt);
  predatorRegenerateLocalSpace(dt);

  if (closestPrey_)
  {
    // Check for collisions in real-time
    const float distance = glm::distance(position_, closestPrey_->position());
	checkEndHunt(closestPrey_->velocity(), closestPrey_->forward());
	if (distance < hunts_.minDist)
	{
		hunts_.minDist = distance;
		hunts_.velocityMinDist = glm::length(velocity_);
		
	}
	if (distance < hunts_.lastMinDist)
	{
		hunts_.lastMinDist = distance;
	}
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
	checkEndHunt(lockedOn_->velocity(), lockedOn_->forward());
    if (distance <= pPred_.CatchDistance)
    {
      // EndHunt(true);
    }
  }
  
  appendTrail(trail_, position_, B_[2], color_tex_, dt);

  //std::cout << "\n x: " << B_[0].x << " y: " <<  B_[0].y << " z: " <<B_[0].z;	
  //testSettings();
}


void CPredator::Accelerate()
{
	//! There is no acceleration or deceleration in the model currently; it always accelerates as much as possible
	steering_ += 0.0f*B_[0];
}


void CPredator::flightDynamic()
{

	const float pi = glm::pi<float>();
	// often used combination of variables:
	const float dynamic = 0.5*pBird_.rho*speed_* speed_;
	// subtract the part of the body of total wing area
	float area = pBird_.wingArea * (pBird_.wingLength * 2) / pBird_.wingSpan;
	// Now, first calculate the maximum lift, given torque constraints, the wing span is:
	float bmax = pBird_.wingSpan;
	float bmin = pBird_.wingSpan - 2 * pBird_.wingLength;
	float L0 = 1.7f*pBird_.bodyMass*9.81f;
	float b_maxlift = std::min(bmax, (bmax - bmin) * avx::fast_sqrt(L0 / (1.6f*dynamic*area)) + bmin);
	//Sim.PrintFloat(b_maxlift, "b max lift");
	float liftMax = 1.6f *dynamic*area* (b_maxlift - bmin) / (bmax - bmin);

	// calculate the inertia, bases on b_maxlift
	float phi = (b_maxlift - bmin) / (bmax - bmin);
	//
	float InertiaWingCenter = pBird_.InertiaWing * phi*phi + 1 / 4 * 0.098*0.098 * pow(pBird_.bodyMass, 0.70f) * pBird_.wingMass + 0.098 * pow(pBird_.bodyMass, 0.35f) * pBird_.J*phi;
	float Inertia = 2 * InertiaWingCenter + pBird_.InertiaBody;

	// How big should the cl be?
	float r_desired_lift = std::min(desiredLift_, liftMax);  //VISUAL ERROR, DT 
	float CL = std::min(r_desired_lift / (dynamic*area), 1.6f);
	// unconstrained Tmax
	float Tmax = pi*pi*pi / 16 * pBird_.rho * pBird_.wingAspectRatio * area * (sin(0.5*pBird_.theta)*pBird_.wingBeatFreq)  * (sin(0.5*pBird_.theta)*pBird_.wingBeatFreq) * pBird_.wingLength *  pBird_.wingLength;
	// constrained Tmax
	float Tmax_cons = std::min(Tmax, Tmax * pBird_.cruiseSpeed / speed_);
	// calculate Thrust for flapping. !!IMPORTANT, torque constraint is hacky
	float T = -CL*CL * area * dynamic / (pi * pBird_.wingAspectRatio) + (1 - CL*CL / (1.6f*1.6f))*Tmax_cons;
	// and lift
	float L = CL *dynamic *area;
	//Sim.PrintFloat(L, "actual lift");
	//Sim.PrintFloat(liftMax, "maximum lift");
	lift_ = L*B_[1];
	// what would the maximum lift have been? (for later)
	liftMax_ = liftMax*B_[1];
	// calculate the body drag and friction drag
	float D = pBird_.cBody * dynamic *pBird_.bodyArea + pBird_.cFriction * dynamic * area;

	// Now we calculate thrust lift and drag for gliding and wing retraction

	// calculate the optimal wingspan, to decrease drag, given certain lift
	float b = pow((bmax - bmin) * 4.0f * r_desired_lift / (2.0f * pBird_.cFriction*area*(pBird_.rho*speed_*speed_)*(pBird_.rho*speed_*speed_)), 1.0f / 3.0f);
	// bound the wingspan to min and max
	b = std::min(std::max(b, bmin), bmax);
	// calculate CL given b
	float CL2 = std::min(1.6f, r_desired_lift / (dynamic * area* (b - bmin) / (bmax - bmin)));
	// make sure with the new CL, the new b is also still smaller than maximum wing span
	b = std::min(r_desired_lift / (dynamic * area* CL2 / (bmax - bmin)) + bmin, bmax);
	// calculate new area
	float areaNew = area * (b - bmin) / (bmax - bmin);


	//calculate roll acceleration with inertia and b maxlift
	//HACK TIMES 2
	angular_acc_ = liftMax * (b_maxlift / 4) / (Inertia) / 2 ;

	// calculate new aspect ratio
	float AR2 = b*b / areaNew;
	// calculate new drag 
	float D2 = pBird_.cBody * dynamic *pBird_.bodyArea + pBird_.cFriction * dynamic * areaNew + CL2*CL2 * areaNew*dynamic / (pi * AR2);
	// flap or glide?
	if ((-D + T) > -D2) glide_ = false; else glide_ = true;
	float forwardAccel = std::max(-D + T, -D2);
	flightForce_ = lift_ + B_[0] * (forwardAccel);
	flightForce_.y -= pBird_.bodyMass * 9.81;        // apply gravity
	if (glide_ == true) span_ = 1.0f - (b - bmin) / (bmax - bmin); else span_ = 0.0f;                     
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
	  //! Proportional navigation function in c++
	  if (pPred_.PursuitStrategy == 1) proportionalNavigation(aveHeading, aveVelocity);
	  if (pPred_.PursuitStrategy == 2) DirectPursuit(aveHeading, aveVelocity);
	  if (pPred_.PursuitStrategy == 3) DirectPursuit2(aveHeading, aveVelocity);
	  if (pPred_.PursuitStrategy == 4) PNDP(aveHeading, aveVelocity);
    //PursuitCustom(aveHeading, aveVelocity);
    //cohesion_ = H_ * (cohesion_ * H_);
    //steering_ += cohesion_;
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
  hunts_.lastMinDist = 999.99f;
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

void CPredator::DirectPursuit(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{

	glm::vec3 r = targetPoint_ - position_;
	glm::vec3 rp = r;

	if(Sim.Params().evolution.evolveDPAdjParam)	rp += DPAdjParam_ * targetVelocity; 
	glm::vec3 v = velocity_;
	glm::vec3 omega = glm::cross(v, rp) / glm::length(glm::cross(v, rp));
	float angle = asin(glm::length(glm::cross(v, rp)) / (glm::length(v)*glm::length(rp)));
	omega *= angle;
	omega *= glm::length(v);
	omega = glm::cross(omega, v);
	steering_ += omega * N_;
}

void CPredator::checkEndHunt(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{
	glm::vec3 r = targetPoint_ - position_;

	if (hunts_.minDist< 5)
	{
		glm::vec3 pHead = glm::vec3(glm::dot(r, glm::column(H_, 0)), glm::dot(r, glm::column(H_, 1)), glm::dot(r, glm::column(H_, 2)));
		float phi = atan2(pHead.z, pHead.x);
		bool blind = (abs(phi) - 3.14 < 0.8 && pHead.x < 0);
		if (glm::length(r) > 10) {
			EndHunt(false);
			//std::cout << "\n" << "should end";
		}
		if (blind) {
			EndHunt(false);
			//std::cout << "\n" <<glm::length(r);
		}
	}


}

void CPredator::DirectPursuit2(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{

	glm::vec3 r = targetPoint_ - position_;
	glm::vec3 rp = r;
	if (Sim.Params().evolution.evolveDPAdjParam)	rp += DPAdjParam_ * targetVelocity;
	steering_ += glm::normalize(rp) * N_ * 100.0f;

}

void CPredator::PNDP(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{
	glm::vec3 r = targetPoint_ - position_;
	if (glm::length(r) > 300) DirectPursuit2(targetHeading, targetVelocity);
	else proportionalNavigation(targetHeading, targetVelocity);
}


//! Prop Nav function
void CPredator::proportionalNavigation(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{
	//Hack
	//N_ = 8.0f;

	glm::vec3 r = targetPoint_ - position_;

	//add visual error!! translate angular velocity to position. This is hacky: actually computations should be done on FOV pred (cant do bias like this)
	//if ((GCAMERA.GetFocalBird())->id() == id_) Sim.PrintVector(r, "before");

	//float LOS = avx::fast_sqrt(glm::dot(r, r));
	//if ((GCAMERA.GetFocalBird())->id() == id_) Sim.PrintFloat(LOS, "LOS");
	//r.x += (pPred_.VisualError * (float(rand()) / (float(RAND_MAX))) ) * LOS;
	//r.y += (pPred_.VisualError * (float(rand()) / (float(RAND_MAX))) ) * LOS;
	//r.z += (pPred_.VisualError * (float(rand()) / (float(RAND_MAX)))) * LOS;

	//if ((GCAMERA.GetFocalBird())->id() == id_) Sim.PrintFloat((float(rand()) / (float(RAND_MAX))), "random");
	//if ((GCAMERA.GetFocalBird())->id() == id_) Sim.PrintVector(r, "after");

	glm::vec3 v = velocity_ - targetVelocity;
	glm::vec3 wLOS = glm::cross(v, r) / glm::dot(r, r);
	steering_ += N_ * glm::cross(wLOS, velocity_)*pBird_.bodyMass;
	//!stopping attack when in blind angle and close to prey
	//if (glm::length(steering_) > 50) std::cout << "\n r " << glm::length(r) << "  v: " << glm::length(v) << "  wLOS " << glm::length(wLOS) << " mass " << pBird_.bodyMass << " N " << N_ << " steering " << glm::length(steering_);
}

void CPredator::predatorIntegration(float dt)
{
	const float hdt = 0.5f * dt;
	const float rBM = 1.0f / pBird_.bodyMass;
	avx::vec3 accel(accel_);
	avx::vec3 velocity(velocity_);


	//std::cout << "\n yforce: " << force.get_y();
	avx::vec3 position(position_);
	avx::vec3 flightForce(flightForce_);
	avx::vec3 forward(B_[0]);

	velocity += accel * hdt;                 // v(t + dt/2) = v(t) + a(t) dt/2
	position += velocity * dt;               // r(t + dt) = r(t) + v(t + dt/2)
	accel = (flightForce) * rBM;     // a(t + dt) = F(t + dt)/m
	position.store(position_);
	velocity += accel * hdt;                 // v(t) = v(t + dt/2) + a(t + dt) dt/2
	accel.store(accel_);

	// clip speed
	speed_ = avx::length(velocity);
	//! don't clamp speed: no stoop possible otherwise
	//speed_ = avx::clamp(speed_, pBird_.minSpeed, pBird_.maxSpeed);
	forward = velocity / speed_;


	//EndHunt(false);
	// interesting: This keeps it always aligned with the forward velocity. This has great impact on the turning behavior.
	forward.store(B_[0]);
}



void CPredator::predatorRegenerateLocalSpace(float dt)
{
	avx::vec3 forward = B_[0];
	avx::vec3 up = B_[1];
	avx::vec3 side = B_[2];
	avx::vec3 steering = steering_;

	glm::vec3 steering2 = steering_;
	steering2.y += pBird_.bodyMass * 9.81;
	avx::vec3 gyro = gyro_.x * forward + gyro_.y * side + gyro_.z * up;
	//steering += gyro;


	//!New steering method as discussed
	glm::vec3 Fl = glm::vec3(0, glm::dot(steering2, B_[1]), glm::dot(steering2, B_[2]));

	   
	desiredLift_ = glm::length(Fl);



	//Stuff to adapt here!! not finished
	//also, make sure not to have a roll acceleration of too high near the desired bank angle, so that it won't oscillate due to finite model steps
	glm::vec3 Ll = glm::vec3(0, glm::dot(lift_, B_[1]), glm::dot(lift_, B_[2]));
	//! determine the size of the turn towards desired angle (positive or negative)
	float turn = asin(glm::cross(Ll, Fl).x / (glm::length(Ll) * glm::length(Fl)));


	// calculate adaptation roll here. If the bird is able to have a roll rate of 0 at the desired bank angle, given maximal acceleration, then accelerate maximally. If this is not possible, decelerate maximally (bang-bang control)
	// calculate timepoints at desired bank angle

	float p = turn;
	float a = angular_acc_;
	float c = roll_rate_;
	// first set the roll acceleration in the direction of the desired bank angle
	if (p < 0) a *= -1;
	// I calculate whether, if the bird would decelerate maximally from here, it would still reach the desired bank angle (it has a solution). If so, it means that the bird has to decelerate from now on in order to have a rollrate of zero 
	// at the desired bank angle. I also check whether the rollrate is in the direction of desired bank angle, else always accelerate in the direction of the bank angle.
	if (c*c > 2 * a*p & p*c > 0) a *= -1;

	roll_rate_ = roll_rate_ + a*dt;
	float rollrate = roll_rate_;

	float beta = rollrate* dt;
	
	avx::vec3 bank = beta * side;


	forward = avx::normalize(forward);
	up += bank;
	side = avx::normalize(avx::cross(forward, up));
	up = avx::cross(side, forward);

	forward.store(B_[0]);
	up.store(B_[1]);
	side.store(B_[2]);
	bank.store(bank_);
	(forward * speed_).store(velocity_);

	// Head system
	up = avx::vec3(0, 1, 0);            // tmp. Head-up
	side = avx::normalize(avx::cross(forward, up));   // Head-side
	up = avx::cross(side, forward);   // Head-up
	forward.store(H_[0]);
	up.store(H_[1]);
	side.store(H_[2]);

	//! Doing the beat cycle for graphics
	//beat cycle
	beatCycle_ += dt*(pBird_.wingBeatFreq * 2*3.14);
	if (glide_) beatCycle_ = 0.0f;
	if (Sim.SimulationTime() < 0.1) beatCycle_ += rand_;
}


void CPredator::testSettings()
{
	keyState_ = 1;
	Sim.PrintFloat(pBird_.wingMass, "wing mass");
	Sim.PrintFloat(pBird_.InertiaBody, "InertiaBody");
	Sim.PrintFloat(pBird_.J, "J");
	Sim.PrintFloat(pBird_.bodyMass, "bodymass");
	Sim.PrintFloat(pBird_.bodyArea, "bodyArea");
	Sim.PrintFloat(pBird_.cBody, "cBody");
	Sim.PrintFloat(pBird_.cFriction, "cFriction");
	Sim.PrintFloat(pBird_.cruiseSpeed, "cruiseSpeed");
	Sim.PrintFloat(pBird_.wingSpan, "wingSpan");
	Sim.PrintFloat(pBird_.wingBeatFreq, "wingBeatFreq");
	Sim.PrintFloat(pBird_.rollRate, "rollRate");
	Sim.PrintFloat(pBird_.bodyWeight, "bodyWeight");
	Sim.PrintFloat(pBird_.rho, "rho");
	Sim.PrintFloat(pBird_.InertiaWing, "Inertia wing");
	Sim.PrintFloat(pBird_.maxForce, "maxForce");
	Sim.PrintFloat(pBird_.minSpeed, "minSpeed");
	Sim.PrintFloat(pBird_.maxSpeed, "maxSpeed");
	Sim.PrintFloat(pBird_.speedControl, "speedControl");
	Sim.PrintFloat(pBird_.innerBoundary, "innerBoundary");
	Sim.PrintFloat(pBird_.outerBoundary, "outerBoundary");
	Sim.PrintVector(pBird_.boundaryWeight, "boundaryWeight");
	Sim.PrintFloat(pBird_.randomWeight, "randomweight");
	Sim.PrintFloat(pPred_.VisualError, "VisualError");
	Sim.PrintVector(B_[0], "body x");
	Sim.PrintVector(B_[1], "body y");
	Sim.PrintVector(B_[2], "body z");
}