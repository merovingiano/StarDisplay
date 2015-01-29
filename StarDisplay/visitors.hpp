//! \file visitors.hpp Collection of feature retrievers from a \ref CBird.
//! \ingroup Analysis


#ifndef VISITORS_HPP_INCLUDED
#define VISITORS_HPP_INCLUDED


#include <limits>
#include <cassert>
#include <algorithm>
#include <numeric>
#include "glmfwd.hpp"
#include <glmutils/local_space.hpp>
#include <glmutils/matrix_norm.hpp>
#include "histogram.hpp"
#include "Prey.hpp"
#include "Predator.hpp"
#include "Flock.hpp"
#include "Params.hpp"
#include "accumulators.hpp"
#include "Globals.hpp"


const float true_val = 1.0f;  //!< floating point \c true.
const float false_val = 0.0f;  //!< floating point \c false.


template<Param::FeatureMap::FMappings M>
struct hist_visitor
{
  hist_visitor(const Param::FeatureMap::Entries_type& fme): hist_(fme[M].hist) {}
  void reset() { hist_.reset(); }
  void append(const hist_visitor<M>& h) { hist_.append(h.hist_); }

  double operator()(const CBird& bird) { double x = visit<M>(bird); hist_(x); return x; }
  void operator()(double x) { hist_(x); }

  histogram::quartiles_t quartiles() const { return hist_.quartiles(); }
  histogram hist_;
};



//! \brief Helper function object for SpeedDev.
struct speed_dev_aux 
{
  float as;
  float s;
  speed_dev_aux(const CBird& pivot): as(0.0f), s(pivot.speed()) { }
  void operator()(const neighbors_vect::value_type& n) 
  {
    as += std::abs(s - n.speed);
  }
};


//! \brief Fallback visitors
template <int N> static float visit(const CBird& t) { return qNan_val; }
template <int N> static float visit(const CPrey& t) { return visit<N>(static_cast<const CBird&>(t)); }
template <int N> static float visit(const CPredator& t) { return visit<N>(static_cast<const CBird&>(t)); }



//! \brief Returns the speed [m/s].
template<> inline float visit<Param::FeatureMap::Speed>(const CBird& bird) 
{
  return bird.speed();
}


//! \brief Returns the local density [1/m^3].
template<> inline float visit<Param::FeatureMap::LocalDensity>(const CBird& bird)
{
  const float r = bird.searchRadius();
  const float V = (4.0f/3.0f) * glm::pi<float>() * r*r*r;
  return static_cast<float>(bird.nsize()) / V;
}


//! \brief Returns the nearest neighbor distance [m].
template<> inline float visit<Param::FeatureMap::NND>(const CBird& bird)
{
  const neighborInfo* nni = bird.nearestNeighborInfo();
  return (nni) ? nni->distance : qNan_val;
}


//! \brief Returns the nearest neighbor distance [m] if \bird is in the interior of a flock.
template<> inline float visit<Param::FeatureMap::NNDInterior>(const CPrey& bird)
{
  return ((bird.circularity() < PFME(NNDInterior).p[0]) ? visit<Param::FeatureMap::NND>(bird) : qNan_val);
}


//! \brief Returns the nearest neighbor distance [m] if \bird is at the border of a flock.
template<> inline float visit<Param::FeatureMap::NNDBorder>(const CPrey& bird)
{
  return ((bird.circularity() >= PFME(NNDBorder).p[0]) ? visit<Param::FeatureMap::NND>(bird) : qNan_val);
}


//! \brief Returns the average nearest neighbor distance [m].
template<> inline float visit<Param::FeatureMap::AveNND>(const CBird& bird)
{
  average<float> annd;
  std::for_each(bird.nbegin(), bird.nend(), [&annd] (const neighborInfo& ni) { annd(ni.distance); }); 
  return annd.mean();
}


//! \brief Returns the magnitude of the lateral g-force [g].
template<> inline float visit<Param::FeatureMap::LateralGForce>(const CBird& bird)
{
  return glm::length(bird.H()[2] * glm::dot(bird.accel_, bird.H()[2])) / 9.81f;
}


//! \brief Returns the magnitude of the separation force [N].
template<> inline float visit<Param::FeatureMap::SeparationForce>(const CBird& bird)
{
  return glm::length(bird.separation());
}


//! \brief Returns the magnitude of the cohesion force [N].
template<> inline float visit<Param::FeatureMap::CohesionForce>(const CBird& bird)
{
  return glm::length(bird.cohesion());
}


//! \brief Returns the magnitude of the 'spacing force' [N].
template<> inline float visit<Param::FeatureMap::SpacingForce>(const CBird& bird)
{
  return glm::length(bird.separation() + bird.cohesion());
}


//! \brief Returns the magnitude of the steering force [N].
template<> inline float visit<Param::FeatureMap::SteeringForce>(const CBird& bird)
{
  return glm::length(bird.steering());
}


//! \brief Returns the magnitude of the total force [N].
template<> inline float visit<Param::FeatureMap::TotalForce>(const CBird& bird)
{
  return glm::length(bird.force());
}


//! \brief Returns the magnitude of the total acceleration [g].
template<> inline float visit<Param::FeatureMap::Accel>(const CBird& bird)
{
  return glm::length(bird.accel());
}


//! \brief Returns the polarization [0..1].
template<> inline float visit<Param::FeatureMap::Polarization>(const CBird& bird)
{
  average<glm::vec3> af;
  std::for_each(bird.nbegin(), bird.nend(), [&af] (const neighborInfo& ni) { af(ni.forward); });;
  return glm::dot(bird.forward(), af.mean());
}


//! \brief Returns the abs. speed deviation [m/s]
template<> inline float visit<Param::FeatureMap::SpeedDev>(const CBird& bird)
{
  speed_dev_aux speeddev(bird);
  speeddev = std::for_each(bird.nbegin(), bird.nend(), speeddev);
  return speeddev.as * (bird.nsize() ? 1.0f / static_cast<float>(bird.nsize()) : qNan_val);
}


//! \brief Returns the signed speed deviation [m/s]
template<> inline float visit<Param::FeatureMap::SpeedDevSign>(const CBird& bird)
{
  average<float> as;
  std::for_each(bird.nbegin(), bird.nend(), [&as] (const neighborInfo& ni) { as(ni.speed); });
  return as.mean();
}


//! \brief Returns \c true if the \c bird is not in the interior of its flock, \c false otherwise
template<> inline float visit<Param::FeatureMap::Border>(const CPrey& bird)
{
  return (bird.circularity() >= PFME(Border).p[0]) ? true_val  : false_val;
}


//! \brief Returns the (projected) bearing angle of the nearest neighbor [-180..180][deg]
template<> inline float visit<Param::FeatureMap::Bearing>(const CPrey& bird)
{
  if (bird.circularity() > PFME(Bearing).p[0]) {
    return qNan_val;
  }
  const neighborInfo* nni = bird.nearestNeighborInfo();
  return (nni) ? glm::degrees(nni->azimuth) : qNan_val;
}


//! \brief Returns the elevation angle of the nearest neighbor [-90..90][deg]
template<> 
inline float visit<Param::FeatureMap::Elevation>(const CPrey& bird)
{
  if (bird.circularity() > PFME(Elevation).p[0]) {
    return qNan_val;
  }
  const neighborInfo* nni = bird.nearestNeighborInfo();
  return (nni) ? glmutils::sphericalLocalSpace(nni->direction, bird.forward(), bird.up(), bird.side()).y : qNan_val;
}


//! \brief Returns the 3D bearing angle of the nearest neighbor [-1..1]
template<> 
inline float visit<Param::FeatureMap::Bearing3D>(const CPrey& bird)
{
  if (bird.circularity() > PFME(Bearing3D).p[0]) {
    return qNan_val;
  }
  const neighborInfo* nni = bird.nearestNeighborInfo();
  return (nni) ? nni->cosAngle : qNan_val;
}


//! \brief Returns the drag [N]
template<> inline float visit<Param::FeatureMap::Drag>(const CBird& bird)
{
  return - glm::dot(bird.flightForce(), bird.forward());
}


//! \brief Returns the effective lift [N]
template<> inline float visit<Param::FeatureMap::EffectiveLift>(const CBird& bird)
{
  return bird.flightForce().y;
}


//! \brief Returns the vertical speed [m/s]
template<> inline float visit<Param::FeatureMap::Vario>(const CBird& bird)
{
  return bird.velocity().y;
}


//! \brief Returns altitude [m]
template<> inline float visit<Param::FeatureMap::Altitude>(const CBird& bird)
{
  return bird.position().y;
}


//! \brief Returns the banking angle (right handed) [deg]
template<> inline float visit<Param::FeatureMap::Banking>(const CBird& bird)
{
  // positive rotation angle is cw
  return glm::degrees( ::asinf( - bird.side().y) );
}


//! \brief Returns the absolute banking angle [deg]
template<> inline float visit<Param::FeatureMap::AbsBanking>(const CBird& bird)
{
  return std::abs( visit<Param::FeatureMap::Banking>(bird) );
}


//! \brief Returns the number of neighbors [1]
template<> inline float visit<Param::FeatureMap::Topo>(const CBird& bird)
{
  return static_cast<float>(bird.interactionNeighbors());
}


//! \brief Returns the perception radius [m]
template<> inline float visit<Param::FeatureMap::Perception>(const CBird& bird)
{
  return bird.searchRadius();
}


//! \brief Returns the horizontal distance to (0,0,0) [% of world Radius]
template<> inline float visit<Param::FeatureMap::HorizDist>(const CBird& bird)
{
  glm::vec3 hd(bird.position().x, 0.0f, bird.position().z);
  return glm::length(hd) * 100.0f / PROOST.Radius;
}


//! \brief Returns the roll rate [deg/s]
template<> inline float visit<Param::FeatureMap::RollRate>(const CBird& bird)
{
  return glm::degrees( ::atanf(glm::length(bird.bankingVector())) / static_cast<float>(PARAMS.IntegrationTimeStep) );
}


//! \brief Returns the ration between |cohesion| and |separation|
template<> inline float visit<Param::FeatureMap::CSRatio>(const CBird& bird)
{
  const float ls = glm::length2(bird.separation());
  return (ls > 0.00001f) ? std::sqrt( glm::length2(bird.cohesion()) / ls) : 1.0f;
}


//! \brief Returns the circularity [0..1]
template<> inline float visit<Param::FeatureMap::Circularity>(const CPrey& bird)
{
  return bird.circularity();
}


//! \brief Returns 1 if predator detected.
template<> inline float visit<Param::FeatureMap::PredatorDetection>(const CPrey& bird)
{
  return ( 0 != bird.detectedPredator()) ? 1.0f : 0.0f;
}


//! \brief Returns predator reaction bitset.
template<> inline float visit<Param::FeatureMap::PredatorReaction>(const CPrey& bird)
{
  int reaction = bird.predatorReaction();
  return reaction ? static_cast<float>(reaction) : qNan_val;
}


//! \brief Returns remaining hide relaxation time [s]
template<> inline float visit<Param::FeatureMap::AlertnessRelexation>(const CPrey& bird)
{
  return bird.alertnessRelaxation().x;
}


//! \brief Returns remaining 'return to flock' relaxation time [s]
template<> inline float visit<Param::FeatureMap::ReturnReaxation>(const CPrey& bird)
{
  return bird.returnRelaxation();
}


//! \brief Returns nearest predator distance [m]
template<> inline float visit<Param::FeatureMap::NPredD>(const CPrey& bird)
{
  const CBird* p = bird.detectedPredator();
  if (0 == p) return qNan_val;
  return bird.predatorDist();
}


//! \brief Returns nearest prey distance [m]
template<> inline float visit<Param::FeatureMap::NPreyD>(const CPredator& bird)
{
  neighbors_vect::const_iterator ival = bird.nbegin();
  return (ival != bird.nend()) ? (*ival).distance : qNan_val;
}


//! \brief Returns the body mass [kg]
template<> inline float visit<Param::FeatureMap::BodyMass>(const CBird& bird)
{
  return bird.GetBirdParams().bodyMass;
}


//! \brief Returns the separation radius [m]
template<> inline float visit<Param::FeatureMap::SeparationRadius>(const CPrey& bird)
{
  return bird.separationRadius();
}


//! \brief Returns the flock id
template<> inline float visit<Param::FeatureMap::FlockId>(const CPrey& bird)
{
  return static_cast<float>(bird.getFlockId());
}


//! \param::FeatureMappings::brief local dDistance/dt  [m/s].
template<> inline float visit<Param::FeatureMap::DDistanceDt>(const CBird& bird)
{
  neighbors_vect::const_iterator it = bird.nbegin();
  if (it != bird.nend())
  {
    float dda = 0.0f;
    glm::vec3 p1 = bird.position() + bird.velocity();    // pos(t+1)
    float dt0 = it->distance;
    float dt1 = glm::length((it->position + it->speed * it->forward) - p1);
    dda += (dt1 - (dt0 * dt0));
    return dda;
  }
  return 0.0f;
}


//! \param::FeatureMappings::brief Reaction time .
template<> inline float visit<Param::FeatureMap::ReactionTime>(const CBird& bird)
{
  return bird.reactionInterval();
}


//! \brief Frobenius distance to nearest neighbor.
template<> inline float visit<Param::FeatureMap::Frobenius>(const CBird& bird)
{
  const neighborInfo* nni = bird.nearestNeighborInfo();
  if (0 == nni) return qNan_val;
  glm::mat3 A( bird.forward(), bird.side(), bird.up() ); 
  glm::mat3 B( nni->forward, nni->side, glm::cross( nni->side, nni->forward ) ); 
  return glmutils::frobenius_distance(A, B);
}


//! \param::FeatureMappings::brief Id.
template<> inline float visit<Param::FeatureMap::Id>(const CBird& bird)
{
  return float(bird.id());
}


//! \param::FeatureMappings::brief Volatile experimental .
template<> inline float visit<Param::FeatureMap::Debug>(const CBird& bird)
{
  return glm::dot(bird.steering(), bird.side());
}



#endif
