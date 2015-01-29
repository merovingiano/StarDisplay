#include <luabind/luabind.hpp>
#include "libGlm.hpp"
#include "libParam.hpp"
#include "ICamera.hpp"
#include "Bird.hpp"
#include "Prey.hpp"
#include "Predator.hpp"


using namespace luabind;


namespace libCamera {


  CPrey* GetFocalPrey(ICamera* self)
  {
    return const_cast<CPrey*>(self->GetFocalPrey());
  }


  CPredator* GetFocalPredator(ICamera* self)
  {
    return const_cast<CPredator*>(self->GetFocalPredator());
  }

}


void luaopen_libCamera(lua_State* L)
{
  module(L, "Camera")[

  class_<TargetInfo>("__pickedbirdinf")
    .def_readonly("pos", &TargetInfo::pos)
    .def_readonly("forward", &TargetInfo::forward)
    .def_readonly("up", &TargetInfo::up)
    .def_readonly("forwardH", &TargetInfo::forwardH)
    .def_readonly("upH", &TargetInfo::upH),


  class_<ICamera>("__camera")
    .def("SetRotationMode", &ICamera::SetRotationMode)
    .property("eye", &ICamera::getEye, &ICamera::setEye)
    .property("center", &ICamera::getCenter, &ICamera::setCenter)
    .property("up", &ICamera::getUp, &ICamera::setUp)
    .property("side", &ICamera::getSide)
    .property("fovy", &ICamera::getFovy, &ICamera::setFovy)
    .property("lerp", &ICamera::getLerp, &ICamera::setLerp)

    .def("GetViewport", &ICamera::GetViewport)
    .def("WindowViewport", &ICamera::WindowViewport)

    .def("shift", &ICamera::shift)
    .def("zoom", &ICamera::zoom)
    .def("moveForward", &ICamera::moveForward)
    .def("moveForwardXZ", &ICamera::moveForwardXZ)
    .def("moveSidewardXZ", &ICamera::moveSidewardXZ)
    .def("moveUpDown", &ICamera::moveUpDown)
    .def("rotateUpDown", &ICamera::rotateUpDown)
    .def("rotateRightLeft", &ICamera::rotateRightLeft)
    .def("tilt", &ICamera::tilt)

    .def("SelectFocalBird", &ICamera::SelectFocalBird)
    .def("SetFocalBird", &ICamera::SetFocalBird)
    .def("SetFocalPrey", &ICamera::SetFocalPrey)
    .def("SetFocalPredator", &ICamera::SetFocalPredator)
    .def("GetFocalBird", &ICamera::GetFocalBird)
    .def("GetFocalPrey", &libCamera::GetFocalPrey)
    .def("GetFocalPredator", &libCamera::GetFocalPredator)
    .def("GetTargetInfo", &ICamera::GetTargetInfo)
    .def("HideFocal", &ICamera::SetHideFocal)
    .def("flushLerp", &ICamera::flushLerp)
  ];
}


