//! \file features.hpp Collection of feature retrievers from a \ref CBird.
//! \ingroup Analysis

#ifndef FEATURES_HPP_INCLUDED
#define FEATURES_HPP_INCLUDED

#include "Params.hpp"


typedef float (*fpFeature) (const class CBird&);


fpFeature select_feature(Param::FeatureMap::FMappings mapping);    //!< Returns color mapping \c mapping.


#endif

