#ifndef ICAMERA_HPP_INCLUDED
#define ICAMERA_HPP_INCLUDED

#include <glm/glm.hpp>


enum CameraRotationMode {
  CameraRotate,
  CameraOrbit,
  CameraIgnoreRotation
};


struct TargetInfo
{
  glm::dvec3 pos;
  glm::dvec3 forward;
  glm::dvec3 up;
  glm::dvec3 forwardH;
  glm::dvec3 upH;
};


class __declspec(novtable) ICamera
{
public: 
  virtual ~ICamera() {}
  virtual const glm::dvec3& eye() const = 0;
  virtual const glm::dvec3& center() const = 0;
  virtual const glm::dvec3& up() const = 0;
  virtual double Fovy() const = 0;

  int depthFieldCounter;

  virtual glm::dvec3 getEye() const = 0;
  virtual glm::dvec3 getCenter() const = 0;
  virtual glm::dvec3 getUp() const = 0;
  virtual glm::dvec3 getSide() const = 0;
  virtual double getFovy() const = 0;
  virtual glm::dvec4 getLerp() const = 0;
  virtual void setEye(const glm::dvec3& newEye) = 0;
  virtual void setCenter(const glm::dvec3& newCenter) = 0;
  virtual void setUp(const glm::dvec3& newUp) = 0;
  virtual void setLerp(const glm::dvec4& newLerp) = 0;
  virtual void setFovy(double newFovy) = 0;
  virtual void SetRotationMode(CameraRotationMode newMode) = 0;

  virtual const glm::dmat4& ModelViewMatrix() const = 0;
  virtual const glm::dmat4& ProjectionMatrix() const = 0;
  virtual const glm::dmat4& ModelViewProjectionMatrix() const = 0;
  virtual const glm::ivec4& GetViewport() const = 0;
  virtual const glm::ivec4& WindowViewport() const = 0;
  virtual glm::dvec3 screenDirection(int win_x, int win_y) const = 0;

  virtual void shift(const glm::dvec3& newCenter) = 0;       
  virtual void zoom(double ppd) = 0;
  virtual void moveForward(double ppd) = 0;             
  virtual void moveForwardXZ(double ppd) = 0;
  virtual void moveSidewardXZ(double ppd) = 0;
  virtual void moveUpDown(double ppd) = 0;
  virtual void rotate(const glm::dvec3& axis, double theta) = 0;
  virtual void rotateUpDown(double theta) = 0;
  virtual void rotateRightLeft(double theta) = 0;
  virtual void tilt(double theta) = 0;
  virtual void flushLerp() = 0;

  virtual void SelectFocalBird(bool predator) = 0;
  virtual void SetFocalBird(const class CBird* bird) = 0;
  virtual void SetFocalPrey(const class CPrey* bird) = 0;
  virtual void SetFocalPredator(const class CPredator* bird) = 0;
  virtual const class CBird* GetFocalBird() const = 0;
  virtual const class CPrey* GetFocalPrey() const = 0;
  virtual const class CPredator* GetFocalPredator() const = 0;
  virtual TargetInfo GetTargetInfo() const = 0;
  virtual bool HideFocal() const = 0;
  virtual void SetHideFocal(bool ignore) = 0;

  virtual void OnSize(const glm::ivec4& viewport, const glm::ivec4& win) = 0;
  virtual void Update(double sim_dt) = 0;
};


#endif
