#include <cassert>
#include <limits>
#include <glm/gtx/projection.hpp>
#include <glm/gtx/fast_square_root.hpp>
#include <glm/gtx/transform.hpp>
#include <glmutils/avx/vec.hpp>
#include <glmutils/avx/mat3.hpp>
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
  if ((reactionTime_ += dt) >= reactionInterval_) 
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
  if (reactionTime_ >= reactionInterval_)
  {
    bool alreadyAlerted = (0 != (predatorReaction_ & PredationReactions::Alerted));
    predatorReaction_ = PredationReactions::None;
    predatorForce_ = boundary_ = gyro_ = glm::vec3(0);
    if ((panicOnset_ < 0.0) && (pPrey_.IncurNeighborPanic > 0))
    {
      double now = Sim.SimulationTime();
      double latency = pPrey_.IncurLatency;
      auto cit = std::find_if(neighbors_.begin(), neighbors_.end(), [now, latency] (const neighborInfo& ni) {
        bool detectable = (0 != (ni.predatorReaction & PredationReactions::Detectable));
        return detectable && ((now - ni.panicOnset) >= latency);
      });
      if ((cit != neighbors_.end()) && ((int)std::distance(neighbors_.begin(), cit) < pPrey_.IncurNeighborPanic))
      {
        panicOnset_ = now;
        predatorReaction_ |= (PredationReactions::Panic | PredationReactions::Alerted);
        panicCopy_ = cit->panicCopy + 1;
      }
    }
    handleEvasion();
    if (0 == (predatorReaction_ & PredationReactions::Panic)) 
    {
      return2Flock(flock); 
      handleBoundary(pBird_.altitude, position_.y);
      panicOnset_ = -1;
      panicCopy_ = 0;
    }
    handleGPWS();

    // flocking
    if (skippedLeftHemisphere_ >= pBird_.skipLeftHemisphere) skippedLeftHemisphere_ = 0; else  ++skippedLeftHemisphere_;
    if (skippedRightHemisphere_ >= pBird_.skipRightHemisphere) skippedRightHemisphere_ = 0; else  ++skippedRightHemisphere_;
    float a = -180.0f + 0.5f * pBird_.blindAngle;     // default: both sided view
    float b = 180.0f - 0.5f * pBird_.blindAngle;      // default: both sided view
    if (skippedLeftHemisphere_)
    {  // left side skipped
      a = -0.5f * pBird_.binocularOverlap;
    }
    else if (skippedRightHemisphere_)
    {  // right side skipped
      b = 0.5f * pBird_.binocularOverlap;
    }
    steerToFlock(fov_filter(a, b));

    // Default parameter
    wBetaIn_ = pBird_.wBetaIn;
    wBetaOut_ = pBird_.wBetaOut;
    alignmentWeight_ = pBird_.alignmentWeight;

    // Adjust for the case of alertness
    if (!alreadyAlerted && (predatorReaction_ & PredationReactions::Alerted))
    {
      // the prey became alerted in the current update.
      alertnessRelaxation_ = pPrey_.AlertnessRelexation;
    }
    if (alertnessRelaxation_.x > 0.0f)
    {
      reactionInterval_ *= pPrey_.AlertedReactionTimeFactor;
      wBetaIn_ = pPrey_.AlertedWBetaIn;
      wBetaOut_ = pPrey_.AlertedWBetaOut;
      alignmentWeight_ = pPrey_.AlertedAlignmentWeight;
    }
    else
    {
      // alertness relaxation over.
      predatorReaction_ &= ~PredationReactions::Alerted;
    }

    steering_ += boundary_;
    steering_ += predatorForce_;
    steering_ += speedControl() * B_[0];
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
  alertnessRelaxation_ -= dt;
  returnRelaxation_ -= dt;
  flightDynamic();
  integration(dt);
  regenerateLocalSpace(dt);

  appendTrail(trail_, position_, B_[2], color_tex_, dt);
}


//! \brief Calculates the flight force
//! 
//! \return Flight force
//! \f[    L = \frac{1}{2}p S v^2 C_l                                    \f]
//! \f[    D = \frac{1}{2}p S v^2 C_d                                    \f]
//! \f[    L_0 = \frac{1}{2}p S v_0^2 C_l = mg                           \f]
//! \f[    L = \frac{v^2}{v_0^2} \cdot L_0 = \frac{v^2}{v_0^2} \cdot mg  \f]
//! \f[    D = \frac{C_d}{C_l} \cdot L                                   \f]
//! \f[    D_0 = \frac{C_d}{C_l} \cdot L_0 = T_0                         \f]
//! 
void CPrey::flightDynamic()
{
  const float pi = glm::pi<float>();
  const float CL = pBird_.CL;
  const float CDCL = 1.0f / ((pi * pBird_.wingAspectRatio) / CL);
  const float L = pBird_.bodyWeight * (speed_ * speed_) / (pBird_.cruiseSpeed * pBird_.cruiseSpeed);  // Lift
  const float D = CDCL * L;                                           // Drag
  lift_ = B_[1] * std::min(L, pBird_.maxLift);
  flightForce_ = B_[1] * lift_ + B_[0] * (CDCL * pBird_.bodyWeight - D);  // apply lift, drag and default thrust
  flightForce_.y -= pBird_.bodyWeight;                                // apply gravity
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

  steering_ = separation_;
  steering_ += alignment_;
  steering_ += cohesion_;
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
      returnForce_ += H_ * ((force * H_) * pPrey_.ReturnWeight);
      predatorReaction_ |= PredationReactions::Return;
    }
  }
  predatorForce_ += returnForce_;
}

