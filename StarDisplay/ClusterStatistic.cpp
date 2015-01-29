#include <fstream>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/ostream.hpp>
#include <glmutils/plane.hpp>
#include <glmutils/homogeneous.hpp>
#include "GLSLImm.hpp"
#include "GLSLState.hpp"
#include "Bird.hpp"
#include "bounding_box.hpp"
#include "voxel_volume.hpp"
#include "ClusterStatistic.hpp"
#include "HistOverlay.hpp"
#include "Clock.hpp"
#include "Flock.hpp"
#include "Simulation.hpp"
#include "Globals.hpp"


using namespace Param;


namespace {

  template <typename N, typename T>
  struct plus_size
  {
    N operator ()(N n, const T& t) const { return n + t.size; }
  };

  struct common_logger
  {
    common_logger(const Param::FeatureMap::Entries_type& fme)
    : nnd_(fme),
      nndI_(fme),
      nndB_(fme),
      localD_(fme),
      polarization_(fme),
      speed_(fme),
      banking_(fme),
      hide_(fme),
      voxel_volume_(fme[FeatureMap::VoxelVolume].p[0])
    {
    }

    void reset() 
    {
      nnd_.reset();
      nndI_.reset();
      nndB_.reset();
      localD_.reset();
      speed_.reset();
      polarization_.reset();
      banking_.reset();
      bounding_box_.reset();
      voxel_volume_.reset();
      flockSize_ = 0;
    }

    void operator()(const CBird& bird)
    {
      ++flockSize_;
      bounding_box_(bird);
      voxel_volume_(bird.position());
      double nnd = nnd_(bird);
      (true_val == visit<FeatureMap::Border>(bird)) ? nndB_(nnd) : nndI_(nnd);
      localD_(bird);
      polarization_(bird);
      speed_(bird);
      banking_(bird);
      hide_(bird);
    }

    hist_visitor<FeatureMap::NND> nnd_;
    hist_visitor<FeatureMap::NND> nndI_;
    hist_visitor<FeatureMap::NND> nndB_;
    hist_visitor<FeatureMap::LocalDensity> localD_;
    hist_visitor<FeatureMap::Polarization> polarization_;
    hist_visitor<FeatureMap::Speed> speed_;
    hist_visitor<FeatureMap::Banking> banking_;
    hist_visitor<FeatureMap::AlertnessRelexation> hide_;
    bounding_box bounding_box_;
    voxel_volume voxel_volume_;
    size_t flockSize_;
  };

}


SubflockStatistic::SubflockStatistic()
:  HistogramStatistic(FeatureMap::Subflock)
{
  Reset();
};


void SubflockStatistic::apply(double stat_dt)
{
  HistogramStatistic::apply(stat_dt);   // default color
  std::for_each(GFLOCK.clusters_begin(), GFLOCK.clusters_end(), [this] (const cluster_entry& ce) {
    log_(float(ce.size), ce.size);
  });
}


std::string SubflockStatistic::labelText() const
{
  char buf[128];
  _snprintf_s(buf, 127, "%s (%u)", fme_.title.c_str(), GFLOCK.num_clusters());
  return std::string(buf);
}


void SubflockStatistic::Display() const
{
  HistogramStatistic::Display();
  val2TexCoord v2tex(fme_.hist.min, fme_.hist.max);
  std::for_each(GFLOCK.clusters_begin(), GFLOCK.clusters_end(), [&v2tex, this] (const cluster_entry & ce) 
  {
    float ctex = v2tex(float(ce.size));
    GGl.imm3D->Box(ce.bbox, ctex);
    const cluster_entry* pce = GFLOCK.cluster(ce.loudest);
    if (pce)
    {
      glm::vec3 lPos = glmutils::center(pce->bbox);
      glm::vec3 pos = glmutils::center(ce.bbox);
      GGl.imm3D->Begin(IMM_LINES);
        GGl.imm3D->Emit(pos, color32( 0.98f, 0.52f, 0.18f ));
        GGl.imm3D->Emit(lPos, color32( 0.98f, 0.52f, 0.18f ));
      GGl.imm3D->End();
    }
  });
}


SubflockPCAStatistic::SubflockPCAStatistic()
:  HistogramStatistic(FeatureMap::SubflockPCA)
{
  Reset();
}


void SubflockPCAStatistic::Reset()
{
  storage_.clear();
  last_display_ = Sim.SimulationTime();
}


void SubflockPCAStatistic::apply(double stat_dt)
{
  HistogramStatistic::apply(stat_dt);   // default color
  LWM_.clear();
  LWH_.clear();
  PCAM_.clear();
  Velocity_.clear();
  I123_.clear();
  common_logger cl(PFM.Entries);
  for (unsigned i=0; i<GFLOCK.num_clusters(); ++i)
  {
    cl.reset();
    GFLOCK.query(cl, GFLOCK.cluster(i)->bbox);
    if (cl.flockSize_ > PFME(SubflockPCA).p[0]) 
    {
      LWM_.emplace_back( cl.bounding_box_.lw_H() );           // Display needs this
      LWH_.emplace_back( cl.bounding_box_.lw_extent() );      // Display needs this
      PCAM_.emplace_back( cl.bounding_box_.pca_H() );         // Display needs this
      Velocity_.emplace_back( cl.bounding_box_.velocity() );  // Display needs this
      I123_.emplace_back( cl.bounding_box_.pca_I123() );      // Display needs this
      storage_type::iterator it = storage_.find(cl.flockSize_);
      if (it == storage_.end())
      {
        it = storage_.insert(storage_type::value_type(cl.flockSize_, storage_entry(PFM.Entries, cl.flockSize_))).first;
      }
      (*it).second.samples_ += 1;
      (*it).second.nnd_.append(cl.nnd_);
      (*it).second.nndI_.append(cl.nndI_);
      (*it).second.nndB_.append(cl.nndB_);
      (*it).second.localD_.append(cl.localD_);
      (*it).second.polarization_.append(cl.polarization_);
      (*it).second.speed_.append(cl.speed_);
      (*it).second.banking_.append(cl.banking_);
      (*it).second.volume_(cl.voxel_volume_.volume());
      (*it).second.LW_(cl.bounding_box_.lw_extent().x / cl.bounding_box_.lw_extent().z);
      (*it).second.I1_(cl.bounding_box_.pca_I123().x);
      (*it).second.I3I1_(cl.bounding_box_.pca_I123().z / cl.bounding_box_.pca_I123().x);
      (*it).second.I3I2_(cl.bounding_box_.pca_I123().z / cl.bounding_box_.pca_I123().y);
      glm::dvec3 pI3 = glmutils::projectToPlane(cl.bounding_box_.pca_EV3(), glm::dvec3(0,1,0));
      glm::dvec3 pF = glmutils::projectToPlane( glm::normalize(cl.bounding_box_.velocity()), glm::dvec3(0,1,0) );
      double dotFI3 = glm::degrees(std::acos(std::abs( glm::dot(glm::normalize(pF), glm::normalize(pI3)) )));
      (*it).second.angleFI3_(dotFI3);
      (*it).second.balance_(cl.bounding_box_.pca_balanceShift());
      (*it).second.hide_.append(cl.hide_);
      log_(static_cast<float>(dotFI3));
    }
  }
}


void SubflockPCAStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  os << "% Cluster statistic\n"
    << "% Indices of X:\n"
    << "iN = 1;                 % subflock size\n"
    << "isamples = 2;           % samples\n"
    << "innd = 3:5;             % nnd average [quartiles]\n"
    << "inndI = 6:8;            % nnd internal [quartiles]\n"
    << "inndB = 9:11;           % nnd boundary [quartiles]\n"
    << "ilocalD = 12:14;        % local density [quartiles]\n"
    << "ipol = 15:17;           % polarization [quartiles]\n"
    << "ispeed = 18:20;         % speed [quartiles]\n"
    << "ibanking = 21:23;       % Banking angle [quartiles]\n"
    << "ivol = 24:25;           % Volume [mean, variance]\n"
    << "ibalance = 26:28;       % balance shift [quartiles]\n"
    << "iLW = 29:31;            % Length-width ratio [quartiles]\n"
    << "iI1 = 32:34;            % Thicknes [quartiles]\n"
    << "iI3I1 = 35:37;          % I3/I1 [quartiles]\n"
    << "iI3I2 = 38:40;          % I3/I2 [quartiles]\n"
    << "iangleFI3 = 41:43;      % cos angle F,I3 [quartiles]\n"
    << "iHide = 44:46;          % Hide relaxation time [quartiles]\n";
  os << "X = [\n";
  std::for_each(storage_.begin(), storage_.end(), [&os] (const storage_type::value_type & se)
  {
    os << se.second.flockSize_ << ' '
      << se.second.samples_ << ' '
      << se.second.nnd_.quartiles() << ' '
      << se.second.nndI_.quartiles() << ' '
      << se.second.nndB_.quartiles() << ' '
      << se.second.localD_.quartiles() << ' '
      << se.second.polarization_.quartiles() << ' '
      << se.second.speed_.quartiles() << ' '
      << se.second.banking_.quartiles() << ' '
      << se.second.volume_.mean() << ' ' << se.second.volume_.variance() << ' '
      << se.second.banking_.quartiles() << ' '
      << se.second.LW_.quartiles() << ' '
      << se.second.I1_.quartiles() << ' '
      << se.second.I3I1_.quartiles() << ' '
      << se.second.I3I2_.quartiles() << ' '
      << se.second.angleFI3_.quartiles() << ";\n";
  });
  os << "];\n";
  os.close();
  HistogramStatistic::save(fname, true);
}


void SubflockPCAStatistic::Display() const
{
  HistogramStatistic::Display();
  float dt = float(Sim.SimulationTime() - last_display_);
  for (size_t i=0; i<PCAM_.size(); ++i)
  {
    const_cast<glm::vec4&>(PCAM_[i][3]) += glm::vec4(Velocity_[i] * dt, 0.0f);
    GGl.imm3D->Box(PCAM_[i], glm::vec3(-0.5f*I123_[i]), glm::vec3(0.5f*I123_[i]), 1.0f);
  }
  const_cast<double&>(last_display_) = Sim.SimulationTime();
}
