//! \file Flock.hpp The Flock.
//! \ingroup Model

#ifndef FLOCK_HPP_INCLUDED
#define FLOCK_HPP_INCLUDED


#include <limits>
#include <algorithm>
#include <functional>
#include <vector>
#include <boost/iterator/iterator_adaptor.hpp>
#include <hrtree/rtree.hpp>
#include <hrtree/adapt_mbr.hpp>
#include <hrtree/mbr_build_policy.hpp>
#include <hrtree/mbr_intersect_policy.hpp>
#include <hrtree/isfc/hilbert.hpp>
#include "ClusterDetection.hpp"
#include "glmfwd.hpp"
#include "Prey.hpp"
#include "Predator.hpp"


HRTREE_ADAPT_POINT_FUNCTION(glm::vec3, float, 3, glm::value_ptr);
HRTREE_ADAPT_MBR_MEMBERS(glmutils::bbox3, glm::vec3, p0(), p1());


class CFlock
{
  typedef hrtree::hilbert<3,21>::type key_type;

  class proxy
  {
  private:
    proxy(const proxy&);
    proxy& operator=(const proxy&);

  public:
    proxy() : ptr_(0) {} 
    explicit proxy(CBird* ptr) : ptr_(ptr) {}
    proxy(proxy&& rhs) { *this = std::move(rhs); }
    proxy& operator=(proxy&& rhs)
    {
      key_ = rhs.key_;
      ptr_ = rhs.ptr_; rhs.ptr_ = 0;
      return *this;
    }
    ~proxy() { if (ptr_) delete ptr_; }

    CBird*    ptr_;
    key_type  key_;
  };

  template <typename IT, typename T>
  class proxy_iter : public boost::iterator_adaptor< proxy_iter<IT, T>, IT, T>
  {
  public:
    proxy_iter() : proxy_iter::iterator_adaptor_(IT()) {}
    explicit proxy_iter(const IT& it) : proxy_iter::iterator_adaptor_(it) {}

    template <typename T2>
    proxy_iter(const proxy_iter<IT,T2>& other) : proxy_iter::iterator_adaptor_(other.base()) 
    {
      static_assert(std::is_same<CBird, typename std::remove_const<T>::type>::value, 
        "Only upcasts allowed");
    }

  private:
    friend class boost::iterator_core_access;

    typename T& dereference() const 
    { 
      return static_cast<T&>(*(this->base_reference()->ptr_)); 
    }
  };


private:
  typedef hrtree::rtree<glmutils::bbox3>      rtree_type;
  typedef std::vector<proxy>                  proxy_collection;
  typedef proxy_collection::iterator          proxy_iterator;
  typedef proxy_collection::const_iterator    proxy_const_iterator;
  typedef ClusterDetection::cluster_container cluster_container;

public:
  typedef rtree_type::bv_type                 bbox;
  typedef rtree_type::build_policy            build_policy;    
  typedef hrtree::mbr_intersect_policy<bbox>  intersect_policy;

  typedef proxy_iter<proxy_iterator, CBird>             bird_iterator;
  typedef proxy_iter<proxy_const_iterator, const CBird> bird_const_iterator;

  typedef proxy_iter<proxy_iterator, CPrey>             prey_iterator;
  typedef proxy_iter<proxy_const_iterator, const CPrey> prey_const_iterator;

  typedef proxy_iter<proxy_iterator, CPredator>             pred_iterator;
  typedef proxy_iter<proxy_const_iterator, const CPredator> pred_const_iterator;

  typedef cluster_container::const_iterator cluster_const_iterator;
  typedef rtree_type::const_bv_iterator     bv_const_iterator;

public:
  CFlock(unsigned numPrey);
  ~CFlock();
  
  //! Required after number of prey changed
  void refresh();
  void update(float sim_dt, bool recluster);
  float meanN;
  float meanStartAltitude;
  float meanXDist;

  unsigned height() const { return static_cast<unsigned>(rtree_.height()); }
  unsigned num_prey() const { return static_cast<unsigned>(prey_.size()); }
  unsigned num_pred() const { return static_cast<unsigned>(predators_.size()); }
  int nextID() { return nextID_++; }

  void insert_prey(CPrey* prey) { prey_.emplace_back(prey); }
  void insert_pred(CPredator* pred) { predators_.emplace_back(pred); };
  void erase_prey() { prey_.pop_back(); }
  void erase_pred() { predators_.pop_back(); }

  //! Removes all individual outside the viewing frustum
  void remove_invisibles(const glmutils::dmat4& ModelViewMatrix, const glmutils::dmat4& ProjectionMatrix);

  prey_const_iterator prey_begin() const { return prey_const_iterator(prey_.begin()); }
  prey_const_iterator prey_end() const { return prey_const_iterator(prey_.end()); }
  prey_iterator prey_begin() { return prey_iterator(prey_.begin()); }
  prey_iterator prey_end() { return prey_iterator(prey_.end()); }
  
  pred_const_iterator predator_begin() const { return pred_const_iterator(predators_.begin()); }
  pred_const_iterator predator_end() const { return pred_const_iterator(predators_.end()); }
  pred_iterator predator_begin() { return pred_iterator(predators_.begin()); }
  pred_iterator predator_end() { return pred_iterator(predators_.end()); }

  cluster_const_iterator clusters_begin() const { return clusters_.begin(); }
  cluster_const_iterator clusters_end() const { return clusters_.end(); }

  bv_const_iterator level_begin(size_t i) const { return rtree_.level_begin(i); }
  bv_const_iterator level_end(size_t i) const { return rtree_.level_end(i); }

  CBird* pickNearest2Ray(const glm::vec3& ray_position, const glm::vec3& ray_direction);
  
  //! Returns 0 if Prey with id @id doesn't exists. 
  const CPrey* FindPrey(int id) const { return do_find_id(prey_begin(), prey_end(), id); }

  //! Returns 0 if Predator with id @id doesn't exists. 
  const CPredator* FindPredator(int id) const { return do_find_id(predator_begin(), predator_end(), id); }

  template <typename QF>
  void query(QF& qf, const glm::vec3& position, const float radius) const
  {
    intersect_policy ip(bbox(position, radius));
    rtree_.query(prey_begin(), ip, qf);
  }

  template <typename QF>
  void query(QF& qf, const bbox& box) const
  {
    intersect_policy ip(box);
    rtree_.query(prey_begin(), ip, qf);
  }

  unsigned num_clusters() const { return static_cast<unsigned>(clusters_.size()); }    //!< Returns number of subflocks

  const cluster_entry* cluster(int flockId) const               //!< Returns the subflock with id =\c flockId.
  { 
    return ((flockId >= 0) && (flockId < static_cast<int>(num_clusters()))) ? &clusters_[flockId] : 0; 
  }  

private:
  template<typename IT>
  const typename IT::value_type* do_find_id(IT first, IT last, int id) const
  {
    for (; first != last && (*first).id() != id; ++first) {}
    return (first != last) ? &*first : 0;
  }

  void cluster_detection(float sim_dt, bool recluster);    //!< perform sub-flock detection
  void assignFlockId();

  int                 nextID_;
  ClusterDetection    clusterDetection_;
  proxy_collection    prey_, buffer_;    // buffer_ used by radix_sort
  proxy_collection    predators_;
  cluster_container   clusters_;
  rtree_type          rtree_;
};


#endif
