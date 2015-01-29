#include <random>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <glm/glm.hpp>
#include <glmutils/random.hpp>


using namespace luabind;


struct lua_rnd
{
  void seed(int s) { eng_.seed(static_cast<std::mt19937::result_type>(seed_ = s)); }
  int getSeed() const { return seed_; }
  double uniform01() { return uniform01_(eng_); }
  double uniform(double min, double max) { return std::uniform_real_distribution<>(min, max)(eng_); }
  double normal(double mean, double sigma) { return std::normal_distribution<>(mean, sigma)(eng_); }
  double geometric(double p) { return std::geometric_distribution<>(p)(eng_); }
  glm::dvec3 unit_vec() { return glmutils::unit_dvec3(eng_); }
  glm::dvec3 vec_in_sphere() { return glmutils::dvec3_in_sphere(eng_); }
  std::mt19937 eng_;
  int seed_;
  std::uniform_real_distribution<> uniform01_;
};


void luaopen_libRnd(lua_State* L)
{
  module(L)
  [
    class_<lua_rnd>("random")
    .def("seed", &lua_rnd::seed)
    .def("uniform01", (double (lua_rnd::*)())&lua_rnd::uniform01)
    .def("uniform", (double (lua_rnd::*)(double, double))&lua_rnd::uniform)
    .def("normal", (double (lua_rnd::*)(double, double))&lua_rnd::normal)
    .def("geometric", (double (lua_rnd::*)(double))&lua_rnd::geometric)
    .def("unit_vec", &lua_rnd::unit_vec)
    .def("vec_in_sphere", &lua_rnd::vec_in_sphere)
    .def("getSeed", &lua_rnd::getSeed)
  ];

  globals(L)["random"] = lua_rnd();
}
