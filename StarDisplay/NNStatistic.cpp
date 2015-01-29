#include <fstream>
#include <vector>
#include <glmutils/ostream.hpp>
#include <glmutils/local_space.hpp>
#include <glmutils/avx/vec.hpp>
#include "NNStatistic.hpp"
#include "Flock.hpp"
#include "visitors.hpp"


using namespace Param;
namespace avx = glmutils::avx;


namespace {
  
  //! Tiny proxy for an bird.
  //! Used to find additional neighbors
  struct CBirdProxy
  {
    explicit CBirdProxy(const CBird& bird, size_t maxNeighbors)
      : forward_(bird.forward()), position_(bird.position()), id_(bird.id()), neighbors_(maxNeighbors)
    {
    }

    avx::vec3  forward_;
    avx::vec3  position_;
    int        id_;
    neighbors_vect  neighbors_;
  };


  //! Collects \c neighborInfo for \c CBirdProxy objects 
  struct find_proxy_neighbors_qf
  {
    CBirdProxy*  pivot_;
    float        rr_;    //!< squared search radius

    //! \param pivot The pivot proxy bird
    //! \param radius Search radius
    find_proxy_neighbors_qf(CBirdProxy* pivot, float radius)
    : pivot_(pivot),
      rr_(radius*radius)
    { }

    //! Collects neighbor if distance < search radius
    void operator()(const CPrey& bird) const
    {
      avx::vec3 dir;
      float distance;
      if (test_distance(bird, pivot_->position_, rr_, dir, distance)) 
      {
        pivot_->neighbors_.insert(bird, distance, dir, avx::dot(pivot_->forward_, dir), 0.0f);
      }
    }
  };

}


NNStatistic::NNStatistic()
: DefaultStatistic(), 
  nnd_(static_cast<size_t>(PFME(NN).p[1]), histogram(PFME(NN).hist)),
  acc_(static_cast<size_t>(PFME(NN).p[1]))
{
  Reset();
}


void NNStatistic::Reset()
{
  for (size_t i=0; i<nnd_.size(); ++i)
  {
    nnd_[i].reset(PFME(NN).hist);
    acc_[i].reset();
  }
}


void NNStatistic::apply(double stat_dt)
{
  CFlock::prey_const_iterator first(GFLOCK.prey_begin());
  CFlock::prey_const_iterator last(GFLOCK.prey_end());
  for (; first != last; ++first)
  {
    CBirdProxy proxy(*first, nnd_.size());
    find_proxy_neighbors_qf qf(&proxy, PFME(NN).p[0]);
    GFLOCK.query(qf, avx::cast(proxy.position_), PFME(NN).p[0]);
    proxy.neighbors_.insertion_sort();
    size_t max_n = std::min(nnd_.size(), proxy.neighbors_.size());
    for (size_t i=0; i < max_n; ++i)
    {
      nnd_[i](proxy.neighbors_[i].distance);
      acc_[i](proxy.neighbors_[i].distance);
    }
  }
}


std::string NNStatistic::labelText() const
{
  return PFME(NN).title;
}


void NNStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  os  << "% " << PFME(NN).title << '\n'
    << "% N:     max. nth neighbor\n"
    << "% quart: N quartiles\n"
    << "% var:   variance\n"
    << "% xq:    mean\n"
    << "% H:     N histograms [bin_center counts]\n";
  os << "N = " << nnd_.size() << '\n';
  os << "quart = [";
  for (size_t i=0; i<nnd_.size(); ++i) {
    os << nnd_[i].quartiles() << ';';
  }
  os << "];\n";
  os << "var = [";
  for (size_t i=0; i<nnd_.size(); ++i) {
    os << acc_[i].variance() << ';';
  }
  os << "];\n";
  os << "xq = [";
  for (size_t i=0; i<nnd_.size(); ++i) {
    os << acc_[i].mean() << ';';
  }
  os << "];\nH = {\n";
  for (size_t i=0; i<nnd_.size(); ++i) {
    os << '[';
    for (size_t k=0; k<nnd_[i].num_bins(); ++k) {
      os << nnd_[i][k] << ';';
    }
    os << "];\n";
  }
  os << "};\n";
}
