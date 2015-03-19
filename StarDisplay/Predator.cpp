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
	N_(float(rand()) / (float(RAND_MAX) / 100))
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

	if ((GCAMERA.GetFocalBird())->id() == id_)
	{

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


	}
	if (int(generation_) == 30 && GFLOCKNC.predator_begin()->id() == id_)
	{
			positionsAndSpeed.push_back(glm::vec4(position_, glm::length(velocity_)));
		
			
	}


	steering_ += 10.0f*B_[0];
	if (position_.y < 1) steering_ += 5.0f*B_[1];
	rand_ = float(rand()) / (float(RAND_MAX)*100) +0.8 * rand_;
	//std::cout << "\n" << rand_;
	//std::cout << "\n" << float(rand()) / (float(RAND_MAX));
    noise();    // add some noise

    avx::vec3 force(steering_);
    const float f2 = avx::length2(force);
    if (f2 > pBird_.maxForce * pBird_.maxForce) {
      force /= avx::fast_sqrt(f2);
      force *= pBird_.maxForce;
      //force.store(steering_);
    }
    force.store(force_);

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
	if (distance < hunts_.minDist)
	{
		hunts_.minDist = distance;
		
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
    if (distance <= pPred_.CatchDistance)
    {
      // EndHunt(true);
    }
  }
  
  appendTrail(trail_, position_, B_[2], color_tex_, dt);
}


void CPredator::flightDynamic()
{
  //float xForce = glm::dot(B_[0], steering_);
  //float maxForce = pBird_.maxForce;
  //xForce = std::min(xForce, maxForce);
  //float yForce = sqrt(maxForce*maxForce - xForce *xForce);

  glm::vec3 forceInBody = glm::vec3(glm::dot(B_[0], steering_), glm::dot(B_[1], steering_), 0);
  const float f2 = glm::length(forceInBody);
  if (f2 > pBird_.maxForce) {
	  forceInBody /= f2;
	  forceInBody *= pBird_.maxForce;
  }

  const float pi = glm::pi<float>();
  const float CL = pBird_.CL;
  const float CDCL = 1.0f / ((pi * pBird_.wingAspectRatio) / CL);
  float L = pBird_.bodyWeight * (speed_ * speed_) / (pBird_.cruiseSpeed * pBird_.cruiseSpeed) + forceInBody.y*0;  // Lift
  



  glm::vec3 down = glm::vec3(0, 0, -pBird_.bodyWeight);
  const float desiredLift = desiredLift_;

  const float D = CDCL * L;   // Drag
  liftMax_ = B_[1] * L;
  lift_ = B_[1] * std::min(L, desiredLift);
  
  //std::cout << "\nlift: " << L << ", desired Lift: " << desiredLift;
  flightForce_ = lift_ + B_[0] * (CDCL * pBird_.bodyWeight*0 - D + forceInBody.x*0); // apply clamped lift, drag and default thrust
  //std::cout << "default thrust: " <<CDCL * pBird_.bodyWeight;
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
	  proportionalNavigation(aveHeading, aveVelocity);
    //PursuitCustom(aveHeading, aveVelocity);
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

void CPredator::proportionalNavigation(const glm::vec3& targetHeading, const glm::vec3& targetVelocity)
{

	glm::vec3 r = targetPoint_ - position_;
	glm::vec3 v = velocity_ - targetVelocity;
	glm::vec3 wLOS = glm::cross(v, r) / glm::dot(r, r);
	steering_ += N_ * glm::cross(wLOS, velocity_)*pBird_.bodyMass;

	if (glm::dot(r, r) < 8)
	{
		glm::vec3 pHead = glm::vec3(glm::dot(r, glm::column(H_, 0)), glm::dot(r, glm::column(H_, 1)), glm::dot(r, glm::column(H_, 2)));
		float phi = atan2(pHead.z, pHead.x);
		bool blind = (abs(phi) - 3.14 < 0.8 && pHead.x < 0);
		if (blind) EndHunt(false);
	}
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
	steering2.y += pBird_.bodyWeight;
	avx::vec3 gyro = gyro_.x * forward + gyro_.y * side + gyro_.z * up;
	//steering += gyro;


	//float Ll = glm::dot(lift_, H_[2]);
	glm::vec3 Fl = glm::vec3(0, glm::dot(steering2, B_[1]), glm::dot(steering2, B_[2]));


	//calculate whether there is enough lift for turning towards steering
	// if so, then decrease lift untill it is equal in length as fSteer
	// if not, then cut off the part of Fsteer that is not gravity, so it remains the desired angle of turn, but slower
	float liftLsq = glm::dot(liftMax_, liftMax_);
	desiredLift_ = 500.0f;
	if (liftLsq > glm::dot(steering2, steering2))
	{
			desiredLift_ = glm::length(Fl);
	}
	else
	{
		desiredLift_ = glm::length(Fl);
		glm::vec3 Fl2 = glm::vec3(0, glm::dot(steering_, B_[1]), glm::dot(steering_, B_[2]));
		if (glm::dot(Fl2, Fl2) > 0)
		{
			
			glm::vec3 weight = glm::vec3(0, pBird_.bodyWeight, 0);
			weight = glm::vec3(0, glm::dot(weight, B_[1]), glm::dot(weight, B_[2]));

			glm::vec3 combined = weight + Fl2;
			//std::cout << "\n combined: " << combined.x << "  " << combined.y << "  " << combined.z;
			//std::cout << "\n Fl: " << Fl.x << "  " << Fl.y << "  " << Fl.z;
			//std::cout << "\nlength old steering: " << glm::length(Fl) << "  length of lift: " << glm::length(liftMax_);
			if (glm::dot(weight, weight) < liftLsq)
			{
				float a = glm::dot(Fl2, Fl2);
				float b = 2 * glm::dot(Fl2, weight);
				float c = glm::dot(weight, weight) - liftLsq;

				float ans1 = (-b + sqrt(b *b - 4 * a*c)) / (2 * a);
				float ans2 = (-b - sqrt(b *b - 4 * a*c)) / (2 * a);
				Fl = Fl2 * std::max(ans1, ans2);
				Fl += weight;
				desiredLift_ = glm::length(Fl);
				//std::cout << "\nlength new steering: " << glm::length(Fl) << "  length of lift: " << glm::length(liftMax_);
				//std::cout << "\n ans1: " << ans1 << "  ans2: " << ans2;
			}
		}
	}


	
	glm::vec3 Ll = glm::vec3(0, glm::dot(lift_, B_[1]), glm::dot(lift_, B_[2]));
	float turn = asin(glm::cross(Ll, Fl).x / (glm::length(Ll) * glm::length(Fl)));
	//std::cout << "\nturn: " << turn;
	//std::cout << "\nlift: " << Ll.x << "  " << Ll.y << "  " << Ll.z;
	float beta = std::max(std::min(wBetaIn_.x * (turn)* dt, 0.036f), -0.036f);
	//std::cout << "\nbeta: " << beta;
	avx::vec3 bank = beta * side;

	//float phi = std::max(std::min((wBetaIn_.y * avx::dot(steering, up)), 0.0005f / dt), -0.0005f / dt);
	//std::cout << "\nphi: " << phi;
	//avx::vec3 pitch = (phi * dt) * up;

	//forward = avx::save_normalize(forward + pitch, forward);
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


	//beat cycle
	beatCycle_ += dt*(8 + 3 * glm::length(force_));
	if (Sim.SimulationTime() < 0.1) beatCycle_ += rand_;
}

