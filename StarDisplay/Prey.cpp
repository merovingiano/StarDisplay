#include <cassert>
#include <limits>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/transform.hpp>
#include <glmutils/avx/vec.hpp>
#include <glmutils/avx/mat3.hpp>
#include <glmutils/random.hpp>
#include <glmutils/homogeneous.hpp>
#include <glmutils/clip_length.hpp>
#include <glmutils/plane.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/matrix_norm.hpp>
#include <glmutils/smootherstep.hpp>
#include "glmfwd.hpp"
#include "Prey.hpp"
#include "Flock.hpp"
#include "Globals.hpp"
#include <iostream>
#include <string>
#include "random.hpp"

using namespace Param;
namespace avx = glmutils::avx;


struct fov_filter
{
  fov_filter(float a, float b) : a_(glm::radians(a)), b_(glm::radians(b)) {}
  bool operator()(neighborInfo const& ni) const
  {
    return (ni.azimuth >= a_) && (ni.azimuth <= b_);
  }
  const float a_, b_;
};


struct find_neighbors_qf
{
  find_neighbors_qf(CPrey& pivot, float radius)
  : position_(pivot.position()),
    H_(pivot.H()),
    circ_vec_(0),
    rr_(0.0f, radius * radius, 0.0f),
    pivot_(pivot)
  {
  }

  void operator()(const CPrey& neighbor)
  {
    if (&pivot_ != &neighbor) 
    {
      const avx::vec3 ofs = avx::vec3(neighbor.position()) - position_;
      const __m128 dp = avx::detail::dot<glm::vec3>(ofs.m128, ofs.m128);
      if (_mm_movemask_ps(_mm_cmple_ps(dp, rr_.m128)) == 0x2)  // dp > 0.0f && dp <= rr_
      {
        __m128 y0 = _mm_rsqrt_ps(dp);
        float distance = _mm_cvtss_f32(_mm_mul_ss(dp, y0));
        if (auto pni = pivot_.neighbors_.try_insert(distance))
        {
          avx::vec3 dir(_mm_mul_ps(ofs.m128, y0), false); 
          circ_vec_ += dir;
          const avx::vec3 posH = dir * H_;
          const float azimuth = std::atan2(posH.get_z(), posH.get_x());
          new (pni) neighborInfo(neighbor, distance, dir, posH.get_x(), azimuth);
        }
      }
    }
  }

  avx::vec3   position_;
  avx::mat3   H_;
  avx::vec3   circ_vec_;
  avx::vec3   rr_;        // squared search radius
  CPrey&      pivot_;
};


CPrey::CPrey(int ID, const glm::vec3& position, const glm::vec3& forward)
: CBird(ID, position, forward ),
  circularity_(0.0f),
  circularityVec_(0.0f),
  predatorForce_(0),
  detectedPredator_(0),
  predatorDist_(100.0f),
  alertnessRelaxation_(0.0f),
  returnForce_(0.0f),
  returnRelaxation_(0.0f),
  panicOnset_(-1.0),
  panicCopy_(0),
  predatorReaction_(PredationReactions::None),
  alignmentWeight_(0),
  flockId_(-1),                // 'invalid'
  flockSize_(0)
{
	random_orientation_ = glm::vec3(1.0f,0.0f,0.0f);
}


CPrey::~CPrey()
{
}


void CPrey::SetPreyParams(const Param::Prey& prey)
{
  pPrey_ = prey;
  skippedLeftHemisphere_ = skippedRightHemisphere_ = std::numeric_limits<int>::max();
}


void CPrey::NumPreyChanged()
{
  neighbors_.clear();
}


void CPrey::NumPredChanged()
{
  detectedPredator_ = 0;
}


void CPrey::updateNeighbors(float dt, const CFlock& flock)
{
  //
  // Warning: this function is called from inside a parallel section!
  //
  if ((reactionTime_ ) >= reactionInterval_) 
  {
    float topo = (alertnessRelaxation_.x > 0.0f) ? pPrey_.AlertedTopo : pBird_.topologicalRange;
    const float r = searchRadius_ * std::pow((topo / (interaction_neighbors_ + 1)), 1.0f/3.0f);
    searchRadius_ = glm::mix(searchRadius_, r, pBird_.neighborLerp);
    searchRadius_ = std::min(searchRadius_, pBird_.maxRadius);

    // find all flock mates within perception volume
    find_neighbors_qf fnqf(*this, searchRadius_);
    neighbors_.clear();
    flock.query(fnqf, position_, searchRadius_);
    circularity_ = 1.0f;    // solitaire 
    avx::vec3 circ_vec = fnqf.circ_vec_;
    const size_t n = neighbors_.size();
    if (n)
    {
      circ_vec /= static_cast<float>(n);
      circularity_ = avx::fast_length(circ_vec);
      neighbors_.insertion_sort();
    }
    circ_vec.store(circularityVec_);
    detectClosestPredator(flock);
  }
}


void CPrey::update(float dt, const CFlock& flock)
{
  //
  // Warning: this function is called from inside a parallel section!
  //

	calculateAccelerations();
	handleTrajectoryStorage();
	if ((reactionTime_ += dt )>= reactionInterval_)
  {
	  predatorForce_ = boundary_ = gyro_ = steering_ = glm::vec3(0);
      if (pPrey_.EvasionStrategyTEMP != 0 ) handleEvasion();
	  handleManeuvers(); 
	  handleBoundary(pBird_.altitude, position_.y);
      // calculate time of next reaction
      nextReactionTime();
  }


  if (Sim.Params().evolution.externalPrey)
  {
	  flightExternal();
	 
  }
  else
  {
		flightDynamic();
		integration(dt);
  }
  regenerateLocalSpace(dt);
  appendTrail(trail_, position_, B_[2], color_tex_, dt);
  testSettings();
}



void CPrey::handleTrajectoryStorage()
{
	if (GFLOCKNC.prey_begin()->id() == id_ && Sim.Params().evolution.TrajectoryPrey)
	{
		positionsAndSpeed.push_back(glm::vec4(position_, glm::length(velocity_)));
	}
}

void CPrey::handleManeuvers()
{
	if (pBird_.maneuver == 1) steering_ = glm::vec3(1, 0, 0) * 20.0f;
	if (pBird_.maneuver == 2) { maneuver_lat_roll(); };
	if (pBird_.maneuver == 3){ maneuver_lat(); };
	if (pBird_.maneuver == 4){ maneuver_lat_roll2(); };
}


void CPrey::flightDynamic()
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
	float Inertia = 2* InertiaWingCenter + pBird_.InertiaBody;

	// How big should the cl be?
	float r_desired_lift = std::min(desiredLift_, liftMax);
	float CL = std::min(r_desired_lift / (dynamic*area), 1.6f);
	// unconstrained Tmax
	float Tmax = pi*pi*pi / 16 * pBird_.rho * pBird_.wingAspectRatio * area * (sin(0.5*pBird_.theta)*pBird_.wingBeatFreq)  * (sin(0.5*pBird_.theta)*pBird_.wingBeatFreq) * pBird_.wingLength *  pBird_.wingLength;
	// constrained Tmax
	float Tmax_cons = std::min(Tmax, Tmax * pBird_.cruiseSpeed / speed_);
	// calculate Thrust for flapping. !!IMPORTANT, I don't yet have a cap on the maximum thrust due to torque constraints. How to do this?
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
	//HACK TIMES 2 (no negative lift)
	angular_acc_ = liftMax * (b_maxlift / 4) / (Inertia) / 2;


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


void CPrey::maneuver_lat_roll()
{
	float max = glm::length(liftMax_) - pBird_.bodyMass*9.81 / 2.0f;
	//if (fmod(Sim.SimulationTime(), 0.05) < 0.025)
	//HACK
	if ((float(rand()) / float(RAND_MAX)) > (1.0f - reactionInterval_*10.0f))
	{
		random_orientation_ = (1.0f * glmutils::unit_vec3(rnd_eng()) + 0.0f * random_orientation_);
		random_orientation_.y = 0.0f ;
		random_orientation_ = glm::normalize(random_orientation_);
		random_orientation_.y = float(rand()) / float(RAND_MAX) * 0.5f -0.25f;
		random_orientation_ = glm::normalize(random_orientation_);
		//random_orientation_ = float(rand()) / float(RAND_MAX) * B_
		//Sim.PrintVector(random_orientation_, "random vec");
	};
	steering_ += random_orientation_ * max;
	
}


void CPrey::maneuver_lat_roll2()
{
	float max = glm::length(liftMax_);
	//if (fmod(Sim.SimulationTime(), 0.05) < 0.025)
	//HACK
	if ((float(rand()) / float(RAND_MAX)) > (1.0f - reactionInterval_*20.0f))
	{
		random_orientation_ = (1.0f * glmutils::unit_vec3(rnd_eng()) + 0.0f * random_orientation_);
		random_orientation_.y = 0.0f;
		random_orientation_ = glm::normalize(random_orientation_);
		random_orientation_.y = float(rand()) / float(RAND_MAX) * 0.5f - 0.25f;
		random_orientation_ = glm::normalize(random_orientation_);
		//random_orientation_ = float(rand()) / float(RAND_MAX) * B_
		//Sim.PrintVector(random_orientation_, "random vec");
	};
	steering_ += random_orientation_ * max;

}


void CPrey::maneuver_lat()
{
	float max = glm::sqrt(glm::length(liftMax_)*glm::length(liftMax_) - pBird_.bodyMass * pBird_.bodyMass);
	steering_ += (H_[2]) *float((0.5*sin(Sim.SimulationTime()) + 0.5) * max);
	// if (B_[0].y < 0)  steering_ += glm::vec3(0, 0.2, 0);
}

void CPrey::calculateAccelerations()
{
	// counter is reset to 0 with each new experiment in EvolvePN
	counter_acc_++;

	if (counter_acc_ == 1)
	{
		max_for_acceleration_ = 0.0f;
		max_lat_acceleration_ = 0.0f;
		max_roll_rate_ = 0.0f;
	}

	average_for_acceleration_ = abs(glm::dot(B_[0], accel_)) / float(counter_acc_) + ((float(counter_acc_) - 1.0f) / float(counter_acc_)) * average_for_acceleration_;
	max_for_acceleration_ = std::max(abs(glm::dot(B_[0], accel_)), max_for_acceleration_);
	average_lat_acceleration_ = abs(glm::dot(B_[1], accel_)) / float(counter_acc_) + (float(counter_acc_) - 1.0f) / float(counter_acc_) * average_lat_acceleration_; 
	max_lat_acceleration_ = std::max(abs(glm::dot(B_[1], accel_)), max_lat_acceleration_);
	average_roll_rate_ = abs(roll_rate_) + ((float(counter_acc_) - 1.0f) / float(counter_acc_)) * average_roll_rate_;;
	max_roll_rate_ = std::max(abs(roll_rate_), max_for_acceleration_);
	

}

void CPrey::flightExternal()
{
	if ((externalPos.size() > 1))
	{
		if (externalPos[1][3] < fmod(Sim.SimulationTime(), Sim.Params().evolution.durationGeneration))
		{
			externalPos.erase(externalPos.begin());
		}
		float time1 = externalPos[0][3];
		float time2 = externalPos[1][3];
		float timeNow = fmod(Sim.SimulationTime(), Sim.Params().evolution.durationGeneration);
		float weight = (timeNow - time1) / (time2 - time1);
		position_ = (1.0f - weight) * glm::vec3(externalPos[0].x, externalPos[0].z, externalPos[0].y) + weight *glm::vec3(externalPos[1].x, externalPos[1].z, externalPos[1].y);
		position_.y += 120; 
		//std::cout << "\nPosition: " << position_.x << " " << position_.y << " " << position_.z << " ";
	}
}


inline void CPrey::steerToFlock(fov_filter const& filter)
{
  separation_ = cohesion_ = alignment_ = glm::vec3(0);
  separation_neighbors_ = cohesion_neighbors_ = alignment_neighbors_ = 0;
  float bankAlignment = 0.0f;
  const float circ2 = circularity_ * circularity_;

  // determine each of the three behaviors of flocking
  neighbors_vect::iterator first(neighbors_.begin());
  neighbors_vect::iterator last(neighbors_.end());
  for (; first != last; ++first) 
  {
    if (filter(*first))
    {
      first->interacting = true;
      const float distance = (*first).distance;

      // Separation
      const float sstep = glmutils::smootherstep(pBird_.separationStep.x, pBird_.separationStep.y, distance);
      if ((separation_neighbors_ < pBird_.maxSeparationTopo) && (distance < pBird_.separationStep.y))
      {
        separation_ += (1.0f - sstep) * (*first).direction;
        ++separation_neighbors_;
      }

      // Alignment
      alignment_ += (*first).forward;  // sum up headings
      bankAlignment += glm::dot((*first).side, B_[1]);
      ++alignment_neighbors_;

      // Cohesion
      cohesion_ += sstep * ((*first).direction);
      ++cohesion_neighbors_;
    }
  }
  if (separation_neighbors_ > 0)
  {
    separation_ *= -1.0f / separation_neighbors_;
  }
  if (alignment_neighbors_ > 0)
  {
    bankAlignment /= (alignment_neighbors_);
    alignment_ /= static_cast<float>(alignment_neighbors_);    // mean
    alignment_ -= B_[0];
  }
  if (cohesion_neighbors_ > 0) 
  {
    cohesion_ *= circ2 / cohesion_neighbors_;
  }
  interaction_neighbors_ = std::max(std::max(alignment_neighbors_, separation_neighbors_), cohesion_neighbors_);

  // apply weights to components
  separation_ = H_ * ((separation_ * H_) * pBird_.separationWeight);
  cohesion_ = H_ * ((cohesion_ * H_) * pBird_.cohesionWeight);
  alignment_ *= alignmentWeight_.x;
  bankAlignment *= alignmentWeight_.y;
  alignment_ -= bankAlignment * B_[2];

  //steering_ = separation_;
  //steering_ += alignment_;
  //steering_ += cohesion_;
}


void CPrey::detectClosestPredator(const CFlock& flock)
{
  detectedPredator_ = 0;
  if (circularity_ > pPrey_.DetectionSurfaceProb)
  {
    const glm::vec3 invCircularityDir( circularityVec_ / - std::max(circularity_, 0.00001f) );
    predatorDist_ = pPrey_.DetectionDistance;
    float dist = 0.0f;
    CFlock::pred_const_iterator first(flock.predator_begin());
    CFlock::pred_const_iterator last(flock.predator_end());
    for (; first != last; ++first)
    {
      if (pPrey_.DetectCruising || first->is_attacking())
      {
        glm::vec3 dir((*first).position() - position_);
        dist = std::max(glm::length(dir), 0.000001f);
        if (dist < predatorDist_) 
        {
          dir /= dist;
          if (glm::dot(dir, invCircularityDir) >= FOVCosine(pPrey_.DetectionHemisphereFOV))
          {
            predatorDist_ = dist;
            detectedPredator_ = &*first;
          }
        }
      }
    }
  }
}


void CPrey::handleEvasion()
{
  if (detectedPredator_)
  {
    predatorReaction_ |= PredationReactions::Alerted;
    if (pPrey_.EvasionStrategy[Prey::MaximizeDist].weight) predatorPanicMaximizeDist();
    if (pPrey_.EvasionStrategy[Prey::TurnInward].weight) predatorPanicTurnInward();
    if (pPrey_.EvasionStrategy[Prey::TurnAway].weight) predatorPanicTurnAway();
    if (pPrey_.EvasionStrategy[Prey::Drop].weight) predatorPanicDrop();
    if (pPrey_.EvasionStrategy[Prey::MoveCentered].weight) predatorPanicMoveCentered();
    if (pPrey_.EvasionStrategy[Prey::Zig].weight) predatorPanicZig();
  }
  predatorPanicCustom();
}


void CPrey::testSettings()
{
	if (!GetAsyncKeyState(VK_F11) && keyState_ != 0)
	{
		keyState_ = 0;
	}
	if (GetAsyncKeyState(VK_F11) && keyState_ == 0)
	{
		keyState_ = 1;
		Sim.PrintString(Sim.Params().evolution.fileName);
		Sim.PrintFloat(Sim.SimulationTime(), "Prey settings. Simulation Time");
		Sim.PrintString(pBird_.birdName);
		Sim.PrintVector(glm::vec3(pBird_.separationWeight), "separation weight ");
		Sim.PrintFloat(pBird_.maneuver, "maneuver");
		Sim.PrintFloat(pPrey_.AlertedTopo, "alertedTopo");
		Sim.PrintFloat(pBird_.wingMass, "wing mass");
		Sim.PrintFloat(pBird_.InertiaBody, "InertiaBody");
		Sim.PrintFloat(pBird_.J, "J");
		Sim.PrintFloat(pPrey_.AlertedTopo, "alertedTopo");
		Sim.PrintFloat(pPrey_.ReturnRelaxation, "ReturnRelaxation");
		Sim.PrintFloat(pPrey_.IncurLatency, "IncurLatency");
		Sim.PrintFloat(pPrey_.DetectCruising, "DetectCruising");
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
		Sim.PrintFloat(pBird_.InertiaWing, "Inertia");
		Sim.PrintFloat(pBird_.maxForce, "maxForce");
		Sim.PrintFloat(pBird_.minSpeed, "minSpeed");
		Sim.PrintFloat(pBird_.maxSpeed, "maxSpeed");
		Sim.PrintFloat(pBird_.speedControl, "speedControl");
		Sim.PrintFloat(pBird_.innerBoundary, "innerBoundary");
		Sim.PrintFloat(pBird_.outerBoundary, "outerBoundary");
		Sim.PrintVector(glm::vec3(pBird_.boundaryWeight), "boundaryWeight");
		Sim.PrintVector(B_[0], "body x");
		Sim.PrintVector(B_[1], "body y");
		Sim.PrintVector(B_[2], "body z");

		glm::dvec3 haha;
		glm::dvec3 hihi;

		haha.x = 6;
		glm::normalize(haha);
		glm::dot(haha, hihi);
	}
		
}



void CPrey::predatorPanicMaximizeDist()
{
  const Param::Panic& p = pPrey_.EvasionStrategy[Prey::MaximizeDist];
  const float ss = p.edges.x;
  if (ss > 0.0f)
  {
    float s,t;
    if (glmutils::intersectRayRay(position_, velocity_, detectedPredator_->position(), detectedPredator_->velocity(), s, t))
    {
      if (s > 0.0f)
      {
        if (s < ss)
        {
          glm::vec3 P0 = position_ + s * velocity_;
          glm::vec3 P1 = detectedPredator_->position() + t * detectedPredator_->velocity();
          glm::vec3 evasionDir(P0 - P1);
          predatorForce_ += p.weight * glmutils::save_normalize(evasionDir, glm::vec3(0));
          predatorReaction_ |= PredationReactions::Panic;
        }
      }
    }
  }
}


void CPrey::predatorPanicTurnInward()
{
  const Param::Panic& p = pPrey_.EvasionStrategy[Prey::TurnInward];
  const float ss = smoothhump(p.edges, predatorDist_);
  if (ss > 0.0f) 
  {
    glm::vec3 evasion = circularityVec_ / std::max(circularity_, 0.000001f); 
    predatorForce_ += ((p.weight * ss) * evasion);
    predatorReaction_ |= PredationReactions::Panic;
  }
}


void CPrey::predatorPanicTurnAway()
{
  const Param::Panic& p = pPrey_.EvasionStrategy[Prey::TurnAway];
  const float ss = smoothhump(p.edges, predatorDist_);
  if (ss > 0.0f) 
  {
    const glm::vec3 dir(position_ - detectedPredator_->position()); 
    glm::vec3 evasion = dir / std::max(predatorDist_, 0.000001f); 
    predatorForce_ += ((p.weight * ss) * evasion);
    predatorReaction_ |= PredationReactions::Panic;
  }
}


void CPrey::predatorPanicDrop()
{
  const Param::Panic& p = pPrey_.EvasionStrategy[Prey::Drop];
  const float ss = smoothhump(p.edges, predatorDist_);
  if (ss > 0.0f) 
  {
    glm::vec3 evasion = glm::vec3(0, -1, 0);
    predatorForce_ += ((p.weight * ss) * evasion);
    predatorReaction_ |= PredationReactions::Panic;
  }
}


void CPrey::predatorPanicMoveCentered()
{
  const Param::Panic& p = pPrey_.EvasionStrategy[Prey::MoveCentered];
  const float ss = smoothhump(p.edges, predatorDist_);
  if (ss > 0.0f) 
	{
		const cluster_entry* ce = GFLOCK.cluster(flockId_);
		if (ce)
		{
			glm::vec3 evasion = glmutils::save_normalize(glmutils::center(ce->bbox) - position_, glm::vec3(0));
			predatorForce_ += ((p.weight * ss) * evasion);
			predatorReaction_ |= PredationReactions::Panic;
		}
	}
}


// Called with state combinations as follows:
// panicOnset_ < 0 and pred ~= 0    : no panic reaction yet but predator detected
// panicOnset_ >= 0 and pred ~= nil : ongoing panic reaction, predator detected
// panicOnset_ >= 0 and pred == nil : ongoing panic reaction, no predator
//
// Parameter edge is reinterpreted as (TirgDist, t_left, t_right, t_handle)
//
void CPrey::predatorPanicZig()
{
  if (panicOnset_ >= 0.0)
  {
    const Param::Panic& p = pPrey_.EvasionStrategy[Prey::Zig];
    const double now = Sim.SimulationTime();
    if (panicOnset_ < 0.0)
    {
      if (predatorDist_ > p.edges.x) 
      {
          return;
      }
      panicOnset_ = now;    // new individual panic reaction
    }
    int reaction = predatorReaction_ | PredationReactions::Panic;
    const float dt = float(now - panicOnset_);
    float evasion = 0.0f;
    if (dt < p.edges.y)
    {
      evasion = -1.0f;
    }
    else if (dt < p.edges.z)
    {
      evasion = 1.0f;
    }
    else if (dt < p.edges.w)
    {
      predatorReaction_ = reaction;
      return;
    }
    else
    {
      return;     // done, don't set prey.predatorReaction
    }
    predatorReaction_ = reaction;
    // predatorForce_ += p.weight * evasion;
    gyro_ = p.weight * glm::vec3(0, evasion, 0);
  }
}


void CPrey::predatorPanicCustom()
{
  if (detectedPredator_ || (panicOnset_ >= 0.0))
  {
    if (pPrey_.EvasionStrategy[Prey::Custom].hook.is_valid())
    {
      auto lock = Lua.LuaLock();
      if (luabind::object_cast_nothrow<bool>(this->pPrey_.EvasionStrategy[Prey::Custom].hook(this, this->detectedPredator_)))
      {
        this->predatorReaction_ |= PredationReactions::Panic;
      }
    }
  }
}


void CPrey::return2Flock(const CFlock& flock)
{
  returnForce_ = glm::vec3(0);
  if (!pPrey_.Return2Flock)
  {
    returnRelaxation_ = 0.0f;
    return;
  }
  if (returnRelaxation_ < 0.0f) 
  {
    if ((flockSize_ < pPrey_.ReturnThreshold.y) || ((0 != detectedPredator_) && (circularity() > pPrey_.DetectionSurfaceProb)))
    {
      returnRelaxation_ = pPrey_.ReturnRelaxation;
    }
  }
  if (returnRelaxation_ > 0.0f)
  {
    const cluster_entry* pce = flock.cluster(flockId_);
    if (0 == pce) return;
    int f = pce->loudest;
    pce = flock.cluster(f);
    if (pce && (f != flockId_) && (flockSize_ < pce->size) && (flockSize_ < pPrey_.ReturnThreshold.y)) 
    {
      glm::vec3 force = glmutils::save_normalize(glmutils::center(pce->bbox) - position_, glm::vec3(0));
      returnForce_ += H_ * ((force * H_) * glm::vec3(pPrey_.ReturnWeight));
      predatorReaction_ |= PredationReactions::Return;
    }
  }
  predatorForce_ += returnForce_;
}

