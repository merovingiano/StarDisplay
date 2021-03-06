#include <random>
#include <glmutils/avx/vec.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/random.hpp>
#include <hrtree/memory/aligned_memory.hpp>
#include "random.hpp"
#include "Bird.hpp"
#include "Globals.hpp"
#include <stdlib.h> 
#include <iostream>
#include <math.h>

using namespace Param;
namespace avx = glmutils::avx;


void* CBird::operator new (size_t s) 
{
  void* raw = hrtree::memory::aligned_malloc<64>(s);
  if (raw == nullptr) throw std::bad_alloc();
  return raw;
}


void CBird::operator delete (void* ptr) 
{
  hrtree::memory::aligned_free<64>(ptr);
}


CBird::CBird(int id, const glm::vec3& position, const glm::vec3& forward)
: position_( position ),
  wingSpan_(0.5f),
  velocity_(0),
  speed_(-1),
  accel_(0),
  force_(0),
  gyro_(0),
  steering_(0),
  separation_(0),
  alignment_(0),
  cohesion_(0),
  boundary_(0),
  beatCycle_(0),
  flightForce_(0),
  bank_(0),
  wBetaIn_(0),
  wBetaOut_(0),
  reactionTime_(0),
  reactionInterval_(0),
  searchRadius_(500),
  separation_neighbors_(PARAMS.maxTopologicalRange),
  alignment_neighbors_(PARAMS.maxTopologicalRange),
  cohesion_neighbors_(PARAMS.maxTopologicalRange),
  interaction_neighbors_(PARAMS.maxTopologicalRange),
  color_tex_(0.0f),
  neighbors_(PARAMS.maxTopologicalRange),
  trail_(nullptr),
  effBoundary_(PROOST.Radius),
  id_(id)
{
  B_[0] = forward;
  B_[2] = glm::normalize( glm::cross(B_[0], B_[1]) );
  B_[1] = glm::cross(B_[2], B_[0]);
  H_ = B_;
}


CBird::~CBird()
{
  GTRAILS.destroy(trail_);
}


void CBird::SetBirdParams(const Param::Bird& pBird)
{
  pBird_ = pBird;
  wingSpan_ = pBird.wingSpan;
  reactionTime_ = std::uniform_real_distribution<float>()(rnd_eng()) * pBird.reactionTime;
  if (speed_ < 0.0f) 
  {
    speed_ = pBird_.cruiseSpeed;
    velocity_ = speed_ * B_[0];
  }
}


void CBird::SetSpeed(float x)
{
  speed_ = std::max<float>(x, 0.01f);
  velocity_ = speed_ * B_[0];
}


void CBird::SetVelocity(glm::vec3 const& x)
{
  velocity_ = x;
  speed_ = glm::length(velocity_);
}


void CBird::RoostChanged()
{
  effBoundary_ = PROOST.Radius;
}


void CBird::setTrail(bool show)
{
  if (show && trail_) return;   // done
  if (nullptr == trail_)
  {
    trail_ = GTRAILS.create(id_);
  }
  else
  {
    GTRAILS.destroy(trail_);
    trail_ = nullptr;
  }
}


void CBird::noise()
{
  if (pBird_.randomWeight > 0.0f) {
    steering_ += pBird_.randomWeight * reactionTime_ * glmutils::unit_vec3(rnd_eng());
	  //std::cout << "\n noise " << pBird_.randomWeight * reactionTime_;
  }
}


float CBird::speedControl() const
{
  return pBird_.bodyMass * pBird_.speedControl * (pBird_.cruiseSpeed - speed_);
}


void CBird::handleBoundary(float preferredAltitude, float refAltitude)
{
  avx::vec2 n(position_.x, position_.z);
  const float distance2Center = avx::length(n);
  const float distance2Border = distance2Center - effBoundary_;
  if (distance2Border > 0.0f) 
  {
    effBoundary_ = PROOST.Radius - pBird_.innerBoundary * PROOST.Radius;
    n /= distance2Center;      // boundary normal
    avx::vec2 f(H_[0].x, H_[0].z);
    f = avx::fast_normalize(f);
    const float outwardHeading = 0.5f * (1.0f + avx::dot(f,n));    // rescale to (0..1) 
    if (outwardHeading > ::cos(glm::radians(pBird_.boundaryReflectAngle)))
    {
      if (avx::perpDot(n, f) > 0.0f)
      {
        boundary_ = (+outwardHeading * distance2Border) * H_[2];
      } else {
        boundary_ = (-outwardHeading * distance2Border) * H_[2];
      }
    }
  }
  else if (distance2Center < (PROOST.Radius - pBird_.innerBoundary * PROOST.Radius))
  {
    effBoundary_ = PROOST.Radius;
  }
  // vertical boundary force ~ altitude deviation
  boundary_.y = glm::clamp((preferredAltitude - refAltitude), -10.0f, 10.0f);
  boundary_ *= pBird_.boundaryWeight;
}


void CBird::handleGPWS()
{
  if (position_.y <= pBird_.gpws.threshold)
  {
    if (pBird_.gpws.type == 1) return handleCustomGPWS();
    boundary_.y += pBird_.gpws.lift;
    const glm::vec4 groundPlane(0.0f, -1.0f, 0.0f, 0.0f);
    float t;
    if (glmutils::intersectRayPlane(position_, velocity_, groundPlane, t) && (t <= pBird_.gpws.tti))
    {
      boundary_.y += ((pBird_.gpws.tti - t) / pBird_.gpws.tti) * pBird_.maxLift;
    }
    if (position_.y <= 0.0f)
    {
      // crash: skid
      //position_.y = 0.0f;
      //glm::vec3 f(B_[0]);
     // f.y = 0.0f;
     // B_[0] = glm::normalize(f);
     // B_[2] = glm::normalize(glm::cross(B_[0], B_[1]));
     // B_[1] = glm::cross(B_[2], B_[0]);
     // H_ = B_;
     // velocity_ = speed_ * B_[0];
     // steering_ = glm::vec3(0.0f);
		steering_ += glm::vec3(0, 0.2, 0);
    }
  }
}


void CBird::handleCustomGPWS()
{
  if (pBird_.gpws.hook.is_valid())
  {
    auto lock = Lua.LuaLock();
    this->pBird_.gpws.hook(this);
  }
}


// Velocity Verlet integration
void CBird::integration(float dt)
{

  const float hdt = 0.5f * dt;
  const float rBM = 1.0f / pBird_.bodyMass;
  avx::vec3 accel(accel_);
  avx::vec3 velocity(velocity_);


  //std::cout << "\n yforce: " << force.get_y();
  avx::vec3 position(position_);
  avx::vec3 flightForce(flightForce_);
  avx::vec3 forward(B_[0]);

  //Sim.PrintVector(accel_, "acceleration");
 // Sim.PrintVector(flightForce_, "flightForce_");
  //Sim.PrintFloat(rBM, "rBM");
  //
  velocity += accel * hdt;                 // v(t + dt/2) = v(t) + a(t) dt/2
  position += velocity * dt;               // r(t + dt) = r(t) + v(t + dt/2)
  accel = (flightForce)* rBM;     // a(t + dt) = F(t + dt)/m

  position.store(position_);
  velocity += accel * hdt;                 // v(t) = v(t + dt/2) + a(t + dt) dt/2
  accel.store(accel_);
  //Sim.PrintVector(accel_, "acceleration after integration");
  // clip speed
  speed_ = avx::length(velocity);
  //! don't clamp speed: no stoop possible otherwise
  //speed_ = avx::clamp(speed_, pBird_.minSpeed, pBird_.maxSpeed);
  forward = velocity / speed_;

  // interesting: This keeps it always aligned with the forward velocity. This has great impact on the turning behavior.
  forward.store(B_[0]);
}


void CBird::regenerateLocalSpace(float dt)
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



	desiredLift_ = glm::length(Fl);





	//roll_rate_ = beta;
	avx::vec3 bank = beta * side;

	//! remove pitching (comment out)
	//float phi = std::max(std::min((wBetaIn_.y * avx::dot(steering, up)), 0.0005f / dt), -0.0005f / dt);
	//std::cout << "\nphi: " << phi;
	//avx::vec3 pitch = (phi * dt) * up;

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

	//std::cout << "\n body " << B_[0][0] << " " << B_[0][1] << " " << B_[0][2];
	//std::cout << "\n body " << B_[1][0] << " " << B_[1][1] << " " << B_[1][2];
	//std::cout << "\n body " << B_[2][0] << " " << B_[2][1] << " " << B_[2][2];
	//beat cycle
	beatCycle_ += dt*(pBird_.wingBeatFreq * 2 * 3.14);
	if (glide_) beatCycle_ = 0.0f;
	if (Sim.SimulationTime() < 0.1) beatCycle_ += rand_;
}

void CBird::nextReactionTime()
{
  reactionTime_ = 0.0f;
  float rs = pBird_.reactionStochastic;
  reactionInterval_ = pBird_.reactionTime * (1.0f  + (rs == 0.0f ? 0.0f : std::uniform_real<float>(-rs, +rs)(rnd_eng())));
}
