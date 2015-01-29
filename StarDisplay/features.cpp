#include "features.hpp"
#include "visitors.hpp"


using namespace Param;


namespace {

  template <typename T, int N>
  struct visit_dispatcher
  {
	static float apply(const CBird& b) { return visit<N>(b); }
  };

  //Robin
  template <int N>
  struct visit_dispatcher<CPrey, N>
  {
	  static float apply(const CBird& b) { return b.isPrey() ? visit<N>(static_cast<const CPrey&>(b)) : 0.0f; }
  };


  template <int N>
  struct visit_dispatcher<CPredator, N>
  {
	  static float apply(const CBird& b) { return b.isPredator() ? visit<N>(static_cast<const CPredator&>(b)) : 0.0f; }
  };
  //Robin
}


const fpFeature VisitorTab[FeatureMap::MaxFeatureMapping__] = 
{
  visit_dispatcher<CBird, -1>::apply,      // Correlation
  visit_dispatcher<CBird, -1>::apply,      // PCA
  visit_dispatcher<CBird, -1>::apply,      // Subflock
  visit_dispatcher<CBird, -1>::apply,      // Voxel Volume
  visit_dispatcher<CBird, -1>::apply,      // TimeSeries
  visit_dispatcher<CBird, -1>::apply,      // NN
  visit_dispatcher<CBird, -1>::apply,      // Qm
  visit_dispatcher<CBird, -1>::apply,      // PredatorVid
  visit_dispatcher<CBird, -1>::apply,      // Evolve deflection
  visit_dispatcher<CBird, -1>::apply,      // Snapshot recording
  visit_dispatcher<CBird, -1>::apply,      // Default
  visit_dispatcher<CBird, -1>::apply,
  visit_dispatcher<CBird, -1>::apply,
  visit_dispatcher<CBird, -1>::apply,
  visit_dispatcher<CBird, -1>::apply,
  visit_dispatcher<CBird, FeatureMap::Speed>::apply,
  visit_dispatcher<CBird, FeatureMap::LocalDensity>::apply,
  visit_dispatcher<CPrey, FeatureMap::NND>::apply,
  visit_dispatcher<CPrey, FeatureMap::NNDInterior>::apply,
  visit_dispatcher<CPrey, FeatureMap::NNDBorder>::apply,
  visit_dispatcher<CBird, FeatureMap::LateralGForce>::apply,
  visit_dispatcher<CBird, FeatureMap::SeparationForce>::apply,
  visit_dispatcher<CBird, FeatureMap::CohesionForce>::apply,
  visit_dispatcher<CBird, FeatureMap::SpacingForce>::apply,
  visit_dispatcher<CBird, FeatureMap::SteeringForce>::apply,
  visit_dispatcher<CBird, FeatureMap::TotalForce>::apply,
  visit_dispatcher<CBird, FeatureMap::Accel>::apply,
  visit_dispatcher<CBird, FeatureMap::Polarization>::apply,
  visit_dispatcher<CBird, FeatureMap::SpeedDev>::apply,
  visit_dispatcher<CBird, FeatureMap::SpeedDevSign>::apply,
  visit_dispatcher<CPrey, FeatureMap::Bearing>::apply,
  visit_dispatcher<CPrey, FeatureMap::Elevation>::apply,
  visit_dispatcher<CPrey, FeatureMap::Border>::apply,
  visit_dispatcher<CPrey, FeatureMap::Bearing3D>::apply,
  visit_dispatcher<CBird, FeatureMap::Drag>::apply,
  visit_dispatcher<CBird, FeatureMap::EffectiveLift>::apply,
  visit_dispatcher<CBird, FeatureMap::Vario>::apply,
  visit_dispatcher<CBird, FeatureMap::Altitude>::apply,
  visit_dispatcher<CBird, FeatureMap::Banking>::apply,
  visit_dispatcher<CBird, FeatureMap::Topo>::apply,
  visit_dispatcher<CBird, FeatureMap::Perception>::apply,
  visit_dispatcher<CBird, FeatureMap::HorizDist>::apply,
  visit_dispatcher<CBird, FeatureMap::AbsBanking>::apply,
  visit_dispatcher<CBird, FeatureMap::RollRate>::apply,
  visit_dispatcher<CBird, FeatureMap::CSRatio>::apply,
  visit_dispatcher<CPrey, FeatureMap::Circularity>::apply,
  visit_dispatcher<CBird, FeatureMap::AveNND>::apply,
  visit_dispatcher<CPrey, FeatureMap::PredatorDetection>::apply,
  visit_dispatcher<CPrey, FeatureMap::PredatorReaction>::apply,
  visit_dispatcher<CPrey, FeatureMap::AlertnessRelexation>::apply,
  visit_dispatcher<CPrey, FeatureMap::ReturnReaxation>::apply,
  visit_dispatcher<CPrey, FeatureMap::NPredD>::apply,
  visit_dispatcher<CPredator, FeatureMap::NPreyD>::apply,
  visit_dispatcher<CBird, FeatureMap::BodyMass>::apply,
  visit_dispatcher<CPrey, FeatureMap::SeparationRadius>::apply,
  visit_dispatcher<CPrey, FeatureMap::FlockId>::apply,
  visit_dispatcher<CBird, -1>::apply,              // Predator Statistics
  visit_dispatcher<CBird, FeatureMap::DDistanceDt>::apply,
  visit_dispatcher<CBird, FeatureMap::ReactionTime>::apply,
  visit_dispatcher<CBird, FeatureMap::Frobenius>::apply,
  visit_dispatcher<CBird, FeatureMap::Id>::apply,
  visit_dispatcher<CPrey, -1>::apply
};


// Select color mapping.
fpFeature select_feature(FeatureMap::FMappings mapping)
{
  return VisitorTab[mapping];
}


