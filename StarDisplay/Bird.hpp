//! \defgroup Model The starling model implementation
//! \file Bird.hpp The 'bird'
//! \ingroup Model

#ifndef BIRD_HPP_INCLUDED
#define BIRD_HPP_INCLUDED

#include <algorithm>
#include <hrtree/config.hpp>
#include "neighbors_vect.hpp"
#include "trail_buffer.hpp"
#include "Params.hpp"



//! bird class
//! \ingroup Simulation
HRTREE_ALIGN_CACHELINE class CBird
{
public:
  void* operator new (size_t);
  void operator delete (void*);

public:
  CBird(const CBird&) = delete;
  CBird& operator=(const CBird&) = delete;
  CBird(int id, const glm::vec3& position, const glm::vec3& forward);
  virtual ~CBird();

  virtual bool isPrey() const { return false; }
  virtual bool isPredator() const { return false; }
  virtual void NumPreyChanged() = 0;
  virtual void NumPredChanged() = 0;
  void RoostChanged();
  
  int id() const { return id_; }                                      //!< Returns unique id
  const Param::Bird& GetBirdParams() const { return pBird_; }         //!< Returns bird parameter
  Param::Bird& GetBirdParams() { return pBird_; }                     //!< Returns bird parameter
  void SetBirdParams(const Param::Bird& pBird);

  const glm::mat3& B() const {return B_; }                            //!< Body system
  const glm::mat3& H() const {return H_; }                            //!< Head system
  const glm::vec3& position() const { return position_; }             //!< position
  const glm::vec3& forward() const { return B_[0]; }                  //!< forward direction
  const glm::vec3& up() const { return B_[1]; }                       //!< up direction
  const glm::vec3& side() const { return B_[2]; }                     //!< side direction
  const glm::vec3& velocity() const { return velocity_; }             //!< velocity
  float speed() const { return speed_; }                              //!< speed
  float beatCycle() const { return beatCycle_; }       
  float getRand() const { return rand_; }
  float getSpan() const { return span_; }
  void SetSpeed(float x);                                             //!< write speed
  void SetVelocity(glm::vec3 const& x);                               //!< write velocity

  float searchRadius() const { return searchRadius_; }                  //!< current search radius
  int cohesionNeighbors() const { return cohesion_neighbors_; }         //!< current number of cohesion neighbors 
  int separationNeighbors() const { return separation_neighbors_; }     //!< current number of separation neighbors
  int alignmentNeighbors() const { return alignment_neighbors_; }       //!< current number of alignment neighbors
  int interactionNeighbors() const { return interaction_neighbors_; }   //!< current number of interaction neighbors

  float wingSpan() const { return wingSpan_; }
  const glm::vec3& accel() const { return accel_; }                   //!< acceleration dv/dt
  const glm::vec3& force() const { return force_; }                   //!< tot. steering forces
  const glm::vec3& steering() const { return steering_; }             //!< steering force (social + boundary)
  const glm::vec3& separation() const { return separation_; }         //!< separation force
  const glm::vec3& alignment() const { return alignment_; }           //!< alignment force
  const glm::vec3& cohesion() const { return cohesion_; }             //!< cohesion force
  const glm::vec3& boundary() const { return boundary_; }             //!< boundary force
  const glm::vec3& flightForce() const  { return flightForce_; }      //!< aerodynamic force
  const glm::vec3& bankingVector() const { return bank_; }            //!< banking vector

  //! \return Current reaction time
  float reactionTime() const { return reactionTime_; }

  //! \return Current reaction interval
  float reactionInterval() const { return reactionInterval_; }

  float getCurrentColorTex() const { return color_tex_; }
  void setCurrentColorTex(float val) const { color_tex_ = val; }

  bool hasTrail() const { return (nullptr != trail_); }
  void setTrail(bool show);

  //! \return Iterator referencing first element of the \c neighborInfo vector. 
  neighbors_vect::const_iterator nbegin() const { return neighbors_.begin(); }

  //! \return Iterator referencing one behind last element of the \c neighborInfo vector. 
  neighbors_vect::const_iterator nend() const { return neighbors_.end(); }

  //! \return Number of neighbors in search volume
  neighbors_vect::size_type nsize() const { return neighbors_.size(); }
  
  //! \return pointer to nearest neighbor info (could be zero)
  const neighborInfo* nearestNeighborInfo() const { return nsize() ? &*neighbors_.begin() : 0; }

  //! \return Mean reaction time
  float ReactionTime() const { return reactionTime_; }

protected:
  void noise();
  float speedControl() const;                                         //!< Speed control
  void handleBoundary(float preferredAltitude, float refAltitude);    //!< Handles cylindric boundary
  void handleGPWS();                                                  //!< Handles GPWS
  void handleCustomGPWS();                                            //!< Handles custom GPWS
  void integration(float dt);
  void regenerateLocalSpace(float dt);
  void nextReactionTime();

public:
  HRTREE_ALIGN(16) glm::vec3 position_;
  float                      wingSpan_;
  float   roll_rate_;
  HRTREE_ALIGN(16) glm::mat3 B_;          
  HRTREE_ALIGN(16) glm::mat3 H_; 
  HRTREE_ALIGN(16) glm::vec3 liftMax_;
  HRTREE_ALIGN(16) glm::vec3 velocity_;
  HRTREE_ALIGN(16) glm::vec3 accel_;
  float angular_acc_;
  HRTREE_ALIGN(16) glm::vec3 force_;
  HRTREE_ALIGN(16) glm::vec3 gyro_;
  HRTREE_ALIGN(16) glm::vec3 boundary_;
  HRTREE_ALIGN(16) glm::vec3 steering_;
  HRTREE_ALIGN(16) glm::vec3 separation_;
  HRTREE_ALIGN(16) glm::vec3 alignment_;
  HRTREE_ALIGN(16) glm::vec3 cohesion_;
  HRTREE_ALIGN(16) glm::vec3 random_orientation_;

  Param::Bird pBird_;

protected:
  HRTREE_ALIGN(16) glm::vec3 lift_;
  
  HRTREE_ALIGN(16) glm::vec3 flightForce_;
  HRTREE_ALIGN(16) glm::vec3 bank_;

  glm::vec3 wBetaIn_;
  glm::vec3 wBetaOut_;

  float   beatCycle_;
  float   rand_;
  float   desiredLift_;
  float   speed_;
  
  bool			   glide_;
  float		span_;
  int     keyState_;
  float   reactionTime_;
  float   reactionInterval_;
  float   StoreTrajectory_;
  float   searchRadius_;
  int     separation_neighbors_;
  int     alignment_neighbors_;
  int     cohesion_neighbors_;
  int     interaction_neighbors_;

  float effBoundary_;
  
  mutable float   color_tex_;
  neighbors_vect  neighbors_;
  trail_buffer*   trail_;

  
  int         id_;
};


#endif
