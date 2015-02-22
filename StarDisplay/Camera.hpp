#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "ICamera.hpp"


class CCamera : public ICamera
{
public:
  CCamera();
  ~CCamera() {}

  const glm::dvec3& eye() const { return actualEye_; }
  const glm::dvec3& center() const { return actualCenter_; }
  const glm::dvec3& up() const { return actualUp_; }
  double Fovy() const { return actualFovy_; }
  
  void OnSize(const glm::ivec4& viewport, const glm::ivec4& win);
  void Update(double sim_dt);

  const glm::dmat4& ModelViewMatrix() const { return MV_; }              // valid after Update()
  const glm::dmat4& ProjectionMatrix() const { return P_; }              // valid after Update()
  const glm::dmat4& ModelViewProjectionMatrix() const { return MVP_; }   // valid after Update()
  const glm::ivec4& GetViewport() const { return Viewport_; }            // valid after Update()
  const glm::ivec4& WindowViewport() const { return WindowViewport_; }   // valid after Update()

  glm::ivec2 Win2SimViewport(int win_x, int win_y) const;
  glm::ivec2 Win2FullViewport(int win_x, int win_y) const;
  glm::dvec3 screenDirection(int win_x, int win_y) const;
 
  glm::dvec3 getEye() const { return eye_; }
  glm::dvec3 getCenter() const { return center_; }
  glm::dvec3 getUp() const { return up_; }
  glm::dvec3 getSide() const;
  double getFovy() const { return fovy_; }
  glm::dvec4 getLerp() const { return lerp_; }
  void setEye(const glm::dvec3& newEye) { eye_ = newEye; }
  void setCenter(const glm::dvec3& newCenter) { center_ = newCenter; }
  void setUp(const glm::dvec3& newUp) { up_ = newUp; }
  void setFovy(double newFovy) { fovy_ = newFovy; }
  void setLerp(const glm::dvec4& newLerp) { lerp_ = newLerp; }
  void SetRotationMode(CameraRotationMode newMode) { mode_ = newMode; }

  void shift(const glm::dvec3& newCenter);       //!< affects eye and center
  void zoom(double ppd);                    //!< Adapt FOVY (+/-)
  void moveForward(double ppd);             //!< Move towards center (+/-)
  void moveForwardXZ(double ppd);           //!< Move forward in plane (+/-)
  void moveSidewardXZ(double ppd);          //!< Move to the side (+/-)
  void moveUpDown(double ppd);              //!< Move up (+/-)
  void rotate(const glm::dvec3& axis, double theta);
  void rotateUpDown(double theta);
  void rotateRightLeft(double theta);
  void tilt(double theta);
  void flushLerp();
 
  

  void SelectFocalBird(bool predator);
  void SetFocalBird(const class CBird* bird);
  void SetFocalPrey(const class CPrey* bird) { focalPrey_ = bird; }
  void SetFocalPredator(const class CPredator* bird) { focalPredator_ = bird; }
  const class CBird* GetFocalBird() const { return *focal_; }
  const class CPrey* GetFocalPrey() const { return focalPrey_; }
  const class CPredator* GetFocalPredator() const { return focalPredator_; }
  TargetInfo GetTargetInfo() const;
  bool HideFocal() const { return hideFocal_; }
  void SetHideFocal(bool hide) { hideFocal_ = hide; } 

private:
  glm::dvec3          eye_;
  glm::dvec3          up_;
  glm::dvec3          center_;
  glm::dvec4          lerp_;               // eye -> actualEye, center -> actualCenter, up -> actualUp, fovy -> actualFovy
  double              fovy_;

  glm::dvec3          actualEye_;
  glm::dvec3          actualUp_;
  glm::dvec3          actualCenter_;
  double              actualFovy_;

  glm::dmat4          MV_;                 // viewing matrix, valid after update()
  glm::dmat4          P_;                  // projection matrix, valid after setProjection() / update()
  glm::dmat4          MVP_;                // view projection matrix, valid after setProjection() / update()
  glm::dmat4          Ortho_;              // Orthogonal projection, valid after setOrtho() / update()
  glm::ivec4          Viewport_;           // Viewport, valid after setProjection() / update()
  glm::ivec4          WindowViewport_;     // Windows client rect, valid after setProjection() / update()
  CameraRotationMode  mode_;

  const class CBird**     focal_;            // points to focalPrey_ or focalPredator_
  const class CPrey*      focalPrey_;
  const class CPredator*  focalPredator_;

  bool hideFocal_;
};



#endif
