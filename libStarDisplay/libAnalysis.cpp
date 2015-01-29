#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/return_reference_to_policy.hpp>
#include "libGlm.hpp"
#include "accumulators.hpp"
#include "histogram.hpp"
#include "bounding_box.hpp"
#include "voxel_volume.hpp"


using namespace luabind;

typedef kahan<double> kahanD;
typedef kahan<glm::dvec3> kahanV;
typedef accumulator<double> accumulatorD;
typedef accumulator<glm::dvec3> accumulatorV;
typedef average<double> averageD;
typedef average<glm::dvec3> averageV;
typedef average<glm::dmat3> averageM;
typedef min_max_mean<double> min_max_meanD;
typedef min_max_mean<glm::dvec3> min_max_meanV;


namespace libAnalysis
{
  double quantile(const histogram& h, double q)
  {
    return h.quantile(q);
  }


  glm::dvec3 quartiles(const histogram& h)
  {
    return h.quartiles();
  }


  glm::dvec2 histBin(const histogram& h, int i)
  {
    return h[i];
  }
}


void luaopen_libAnalysis(lua_State* L)
{
  module(L, "Analysis")
  [
    class_<kahanD>("kahan_N")
      .def(constructor<>())
      .def(constructor<double>())
      .def("value", &kahanD::value)
      .def(self(double())),


    class_<kahanV>("kahan_V")
      .def(constructor<>())
      .def(constructor<glm::dvec3>())
      .def("value", &kahanV::value)
      .def(self(glm::dvec3())),


    class_<accumulatorD>("accumulator_N")
      .def(constructor<>())
      .def("reset", &accumulatorD::reset)
      .def("sum", &accumulatorD::sum)
      .def("min", &accumulatorD::min)
      .def("max", &accumulatorD::max)
      .def("mean", &accumulatorD::mean)
      .def("variance", &accumulatorD::variance)
      .def("data", &accumulatorD::data, return_stl_iterator)
      .def(self(double())),


    class_<accumulatorV>("accumulator_V")
      .def(constructor<>())
      .def("reset", &accumulatorV::reset)
      .def("sum", &accumulatorV::sum)
      .def("min", &accumulatorV::min)
      .def("max", &accumulatorV::max)
      .def("mean", &accumulatorV::mean)
      .def("variance", &accumulatorV::variance)
      .def("data", &accumulatorV::data, return_stl_iterator)
      .def(self(glm::dvec3())),


    class_<averageD>("average_N")
      .def(constructor<>())
      .def("reset", &averageD::reset)
      .def("count", &averageD::count)
      .def("sum", &averageD::sum)
      .def("mean", &averageD::mean)
      .def("append", &averageD::append)
      .def(self(double())),


    class_<averageV>("average_V")
      .def(constructor<>())
      .def("reset", &averageV::reset)
      .def("count", &averageV::count)
      .def("sum", &averageV::sum)
      .def("mean", &averageV::mean)
      .def("append", &averageV::append)
      .def(self(glm::dvec3())),


    class_<averageM>("average_M")
      .def(constructor<>())
      .def("reset", &averageM::reset)
      .def("count", &averageM::count)
      .def("sum", &averageM::sum)
      .def("mean", &averageM::mean)
      .def("append", &averageM::append)
      .def(self(glm::dmat3())),


    class_<min_max_meanD>("min_max_mean_N")
      .def(constructor<>())
      .def("reset", &min_max_meanD::reset)
      .def("count", &min_max_meanD::count)
      .def("min", &min_max_meanD::min)
      .def("max", &min_max_meanD::max)
      .def("mean", &min_max_meanD::mean)
      .def(self(double())),


    class_<min_max_meanV>("min_max_mean_V")
      .def(constructor<>())
      .def("reset", &min_max_meanV::reset)
      .def("count", &min_max_meanV::count)
      .def("min", &min_max_meanV::min)
      .def("max", &min_max_meanV::max)
      .def("mean", &min_max_meanV::mean)
      .def(self(glm::dvec3())),


    class_<histogram>("histogram")
      .def(constructor<double, double, int>())
      .def("reset", (void (histogram::*)()) &histogram::reset)
      .def("reset", (void (histogram::*)(double, double, int)) &histogram::reset)
      .def("reduce", &histogram::reduce)
      .def("append", &histogram::append)
      .def(self(double()))
      .def(self(double(), int()))
      .def("max_count", &histogram::max_count)
      .def("num_bins", &histogram::num_bins)
      .def("quantile", libAnalysis::quantile)
      .def("quartiles", libAnalysis::quartiles)
      .def("bins", &histogram::bins, return_stl_iterator)
      .def("bin", libAnalysis::histBin),


    class_<bounding_box>("bounding_box")
      .def(constructor<>())
      .def("reset", &bounding_box::reset)
      .def("velocity", &bounding_box::velocity)
      .def("lw_H", &bounding_box::lw_H)
      .def("lw_extent", &bounding_box::lw_extent)
      .def("lw_geoCenter", &bounding_box::lw_geoCenter)
      .def("lw_balanceShift", &bounding_box::lw_balanceShift)
      .def("pca_H", &bounding_box::pca_H)
      .def("pca_EV3", &bounding_box::pca_EV3)
      .def("pca_I123", &bounding_box::pca_I123)
      .def("pca_geoCenter", &bounding_box::pca_geoCenter)
      .def("pca_balanceShift", &bounding_box::pca_balanceShift)
      .def(self(other<const CBird&>())),


    class_<voxel_volume>("voxel_volume")
      .def(constructor<float>())
      .def("reset", (void (voxel_volume::*)()) &voxel_volume::reset)
      .def(self(other<const glm::vec3&>()))
      .def("volume", &voxel_volume::volume)
  ];
}

