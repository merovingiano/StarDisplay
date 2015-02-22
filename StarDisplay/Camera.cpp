#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include <glmutils/homogeneous.hpp>
#include "Simulation.hpp"
#include "GLSLState.hpp"
#include "Camera.hpp"
#include "Bird.hpp"
#include "Prey.hpp"
#include "Predator.hpp"
#include "Globals.hpp"


CCamera::CCamera()
: eye_(0, 1.74, 0),
  center_(15, 1.74, 0),
  up_(0.0, 1.0, 0.0),
  actualEye_(0),
  actualCenter_(0),
  actualUp_(0.0, 1.0, 0.0),
  lerp_(1, 1, 1, 1),
  fovy_(50),
  focal_((const CBird**)&focalPrey_),
  focalPrey_(0),
  focalPredator_(0),
  mode_(CameraRotate),
  hideFocal_(false)
{
}
    

glm::ivec2 CCamera::Win2SimViewport(int win_x, int win_y) const
{ 
  glm::ivec2 ret = Win2FullViewport(win_x, win_y); 
  ret.x -= Viewport_.x; 
  ret.y -= Viewport_.y; 
  return ret; 
}


glm::ivec2 CCamera::Win2FullViewport(int win_x, int win_y) const 
{ 
  return glm::ivec2(win_x, WindowViewport_.w - win_y); 
}         


glm::dvec3 CCamera::screenDirection(int win_x, int win_y) const
{
  glm::dvec2 cp( Win2FullViewport(win_x, win_y) ); 
  const glm::dvec3 f = glm::unProject<double,int>(glm::dvec3(cp.x, cp.y, 1.0), MV_, P_, Viewport_);
  const glm::dvec3 n = glm::unProject<double,int>(glm::dvec3(cp.x, cp.y, 0.0001), MV_, P_, Viewport_);
  return glm::normalize(f - n);
}


void CCamera::OnSize(const glm::ivec4& viewport, const glm::ivec4& client)
{
  Viewport_ = viewport;
  WindowViewport_ = client;
}


glm::dvec3 CCamera::getSide() const
{
  const glm::dvec3 f = glm::normalize(center_ - eye_);
  return glm::normalize(glm::cross(up_, f));
}


void CCamera::Update(double sim_dt)
{
  actualFovy_ = glm::mix(actualFovy_, fovy_, lerp_.w * sim_dt);
  actualEye_ = glm::mix(actualEye_, eye_, std::min(1.0, lerp_.x * sim_dt));
  actualCenter_ = glm::mix(actualCenter_, center_, std::min(1.0, lerp_.y * sim_dt));
  actualUp_ = glm::mix(actualUp_, up_, std::min(1.0, lerp_.z * sim_dt));  
  const glm::dvec3 f = glm::normalize(actualEye_ - actualCenter_);
  const glm::dvec3 s = glm::normalize(glm::cross(actualUp_, f));
  const glm::dvec3 u = glm::cross(f, s);
  MV_ = glm::translate(glm::dmat4(s.x, u.x, f.x, 0.0,    
                                  s.y, u.y, f.y, 0.0,
                                  s.z, u.z, f.z, 0.0,
                                  0.0, 0.0, 0.0, 1.0),
                       -actualEye_);
  double aspect = static_cast<double>(Viewport_[2]) / Viewport_[3];
  P_ = glm::perspective(actualFovy_, aspect, 0.1, 1000.0);
  MVP_ = P_*MV_;
}


void CCamera::shift(const glm::dvec3& newCenter)
{
  const glm::dvec3 d(newCenter - center_);
  eye_ += d;
  center_ += d;
}


void CCamera::zoom(double ppd)
{
  fovy_ = glm::clamp(fovy_ + ppd, 2.0, 80.0);
}


void CCamera::moveForward(double ppd)
{
  glm::dvec3 d(center_ - eye_);
  const double dist = glm::length(d);
  double new_dist = dist - ppd;
  if (new_dist < 0.01) new_dist = 0.01;
  d *= (new_dist / dist);
  eye_ = center_ - d;
}


void CCamera::moveForwardXZ(double ppd)
{
  glm::dvec3 d = (center_ - eye_);
  d.y = 0;
  glm::dvec3 step = glmutils::save_normalize(d, glm::dvec3(0)) * ppd;
  eye_ += step;
  center_ += step;
}


void CCamera::moveSidewardXZ(double ppd)
{
  glm::dvec3 d = (center_ - eye_);
  d.y = 0;
  glm::dvec3 side = glm::cross(d, glm::dvec3(0,1,0));
  glm::dvec3 step = glmutils::save_normalize(side, glm::dvec3(0)) * ppd;
  eye_ += step;
  center_ += step;
}


void CCamera::moveUpDown(double ppd)
{
  eye_.y += ppd;
  center_.y += ppd;
}


void CCamera::rotate(const glm::dvec3& axis, double theta)
{
  glm::dmat4 R = glm::rotate<double>(glm::dmat4(1), glm::degrees((mode_ == CameraRotate) ? -theta : theta), axis); 
  switch (mode_)
  {
    case CameraRotate:
      center_ = glmutils::transformVector(R, center_ - eye_) + eye_;
      break;
    case CameraIgnoreRotation:
      return;
    default: // Orbit
      eye_ = glmutils::transformVector(R, eye_ - center_) + center_;
  }
  up_ = glmutils::save_normalize(glmutils::transformVector(R, up_), up_);
}


void CCamera::rotateUpDown(double theta)
{
  const glm::dvec3 lockAt(eye_ - center_);
  const glm::dvec3 side = glm::normalize(glm::cross(up_, lockAt));
  rotate(side, theta);
}


void CCamera::rotateRightLeft(double theta)
{
  rotate(glm::dvec3(0,1,0), theta); //(mode_ == CameraRotate) ? glm::dvec3(0,1,0) : up_, theta);
}


void CCamera::tilt(double theta) 
{ 
  glm::dmat4 R = glm::rotate<double>(glm::dmat4(1), glm::degrees(-theta), glm::normalize(center_ - eye_)); 
  up_ = glmutils::transformVector(R, up_);
}


void CCamera::flushLerp()
{
  actualEye_ = eye_;
  actualCenter_ = center_;
  actualUp_ = up_;
  actualFovy_ = fovy_;
}


void CCamera::SelectFocalBird(bool predator) 
{
  focal_ = predator ? (const CBird**)&focalPredator_ : (const CBird**)&focalPrey_;
}


void CCamera::SetFocalBird(const CBird* bird)
{
  if (bird) 
  {
    if (bird->isPrey()) 
    {
      focalPrey_ = static_cast<const CPrey*>(bird);
      focal_ = (const CBird**)&focalPrey_;
    } 
    else 
    {
      focalPredator_ = static_cast<const CPredator*>(bird);
      focal_ = (const CBird**)&focalPredator_;
    }
  }
  else
  {
    focalPrey_ = 0;
    focalPredator_ = 0;
  }
}


TargetInfo CCamera::GetTargetInfo() const
{
  TargetInfo ti;
  const CBird* pbird = *focal_;
  ti.pos = (pbird) ? glm::dvec3(pbird->position()) : center();
  ti.forward = (pbird) ? glm::dvec3(pbird->forward()) : glm::normalize(center() - eye());
  ti.up = (pbird) ? glm::dvec3(pbird->up()) : up();
  ti.forwardH = (pbird) ? glm::dvec3(pbird->H()[0]) : glm::normalize(center() - eye());
  ti.upH = (pbird) ? glm::dvec3(pbird->H()[1]) : up();
  return ti;
}


