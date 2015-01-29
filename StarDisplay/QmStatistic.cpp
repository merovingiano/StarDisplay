#include <fstream>
#include <algorithm>
#include "QmStatistic.hpp"
#include "Flock.hpp"
#include "Globals.hpp"


using namespace Param;
namespace avx = glmutils::avx;


namespace {
  
  //! Tiny proxy for an bird.
  //! Used to find additional neighbors
  struct CBirdProxy
  {
    explicit CBirdProxy(const CBird& bird, size_t maxNeighbors)
      : position_(bird.position()), neighbors_(maxNeighbors)
    {
    }

    glm::vec3  position_;
    neighbors_vect  neighbors_;
  };


  //! Collects \c neighborInfo for \c CBirdProxy objects 
  struct find_proxy_neighbors_qf
  {
    CBirdProxy* pivot_;
    const float rr_;    //!< squared search radius

    //! \param pivot The pivot proxy bird
    //! \param radius Search radius
    find_proxy_neighbors_qf(CBirdProxy* pivot, float radius)
    : pivot_(pivot),
      rr_(radius*radius)
    { }

    //! Collects neighbor if distance < search radius
    void operator()(const CPrey& bird) const
    {
      const float dist2 = glm::distance2(pivot_->position_, bird.position());
      if (dist2 < rr_ && dist2 > 0.0000001f)
      {
        pivot_->neighbors_.insert(bird, dist2, glm::vec3(0), 0.0f, 0.0f);
      }
    }
  };


  std::vector<int> assessM(CBird const& bird)
  {
    size_t maxN = (size_t)*std::max_element(Qmp, Qmp + Qmpn);
    CBirdProxy proxy(bird, maxN);
    find_proxy_neighbors_qf qf(&proxy, 1000.0f);
    GFLOCK.query(qf, proxy.position_, 1000.0f);
    proxy.neighbors_.insertion_sort();
    size_t n = std::min(maxN, proxy.neighbors_.size());
    std::vector<int> tmp;
    for (int i=0; i<n; ++i)
    {
      tmp.push_back( proxy.neighbors_[i].id );
    }
    return tmp;
  }

}


QmStatistic::QmStatistic()
: DefaultStatistic() 
{
  for (int i=0; i < Qmpn; ++i) Qm_[i].clear();
  Reset();
}


void QmStatistic::Reset()
{
  M0_.clear();
  for (int i=0; i < Qmpn; ++i) Qm_[i].clear();
  T_.clear();
  CFlock::prey_const_iterator first(GFLOCK.prey_begin());
  CFlock::prey_const_iterator last(GFLOCK.prey_end());
  for (; first != last; ++first)
  {
    M0_[first->id()] = assessM(*first);
  }
}


void QmStatistic::apply(double stat_dt)
{
  double t = (T_.empty() ? 0.0 : T_.back()) + stat_dt;
  if (t > 1.0) return;
  T_.push_back(t);

  std::map<int, std::vector<int> > M;
  CFlock::prey_const_iterator first(GFLOCK.prey_begin());
  CFlock::prey_const_iterator last(GFLOCK.prey_end());
  for (; first != last; ++first)
  {
    M[first->id()] = assessM(*first);
  }
  for (int im=0; im < Qmpn; ++im)
  {
    size_t m = Qmp[im];
    if (m == 0) break;
    std::vector<int> S;
    double qm = 0.0;
    first = GFLOCK.prey_begin();
    for (; first != last; ++first)
    {
      S.clear();
      auto sit = std::back_inserter(S);
      size_t mm = std::min<size_t>(std::min<size_t>(M[first->id()].size(), M0_[first->id()].size()), m);
      std::vector<int> rMi(M[first->id()].begin(), M[first->id()].begin() + mm);
      std::vector<int> rM0(M0_[first->id()].begin(), M0_[first->id()].begin() + mm);
      std::sort(rMi.begin(), rMi.end());
      std::sort(rM0.begin(), rM0.end());
      std::set_intersection(rMi.begin(), rMi.end(), rM0.begin(), rM0.end(), sit);
      qm += double(S.size()) / mm;
    }
    Qm_[im].push_back( qm / GFLOCK.num_prey() );
  }
}


std::string QmStatistic::labelText() const
{
  return PFME(Qm).title;
}


void QmStatistic::save(const char* fname, bool append) const
{
  std::ofstream os(fname, std::ios_base::out | (append ? std::ios_base::app : std::ios_base::trunc));
  for (size_t t=0; t < T_.size(); ++t)
  {
    os << T_[t] << ' ';
    for (int i=0; i<Qmpn; ++i)
    {
      if (Qm_[i].empty()) break;
      os << Qm_[i][t] << ' ';
    }
    os << '\n';
  }
}


