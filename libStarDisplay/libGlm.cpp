#include <limits>
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glmutils/bbox.hpp>
#include <glmutils/ostream.hpp>
#include <glmutils/local_space.hpp>
#include <glmutils/homogeneous.hpp>
#include <glmutils/ray.hpp>
#include <glmutils/fast_eigen.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <set>


void LUA_API logGLMConversions(lua_State* L, const char* A, const char* B)
{
  static std::set<std::string> memoize;
  lua_Debug ar;
  lua_getstack(L, 1, &ar);
  lua_getinfo(L, "nSl", &ar);
  std::stringstream ss;
  ss << "warning: " << ar.short_src << ", line " << ar.currentline;
  ss << ":\nconversion form: " << B << " to: " << A << '\n';
  if (memoize.insert(ss.str()).second) {
    std::cerr << ss.str();
  }
}


template <typename T>
struct helpers 
{
  using vec2 = glm::detail::tvec2<T>;
  using vec3 = glm::detail::tvec3<T>;
  using vec4 = glm::detail::tvec4<T>;
  using mat3 = glm::detail::tmat3x3<T>;
  using mat4 = glm::detail::tmat4x4<T>;
  

  static T smoothstep(vec2& e, T x)
  {
    return glm::smoothstep(e.x, e.y, x);
  }

  static T smoothhump(T e0, T e1, T e2, T e3, T x)
  {
    return glm::smoothstep(e0, e1, x) * (T(1) - glm::smoothstep(e2, e3, x));
  }

  static T smoothhump(const vec4& e, T x)
  {
    return smoothhump(e.x, e.y, e.z, e.w, x);
  }

  static vec3 left_mul3(vec3 const& v, mat3 const& M)
  {
    return v * M;
  }

  static vec4 left_mul4(vec4 const& v, mat4 const& M)
  {
    return v * M;
  }

  static T intersectRayPlane(vec3 const& orig, vec3 const& vel, vec4 const& plane)
  {
    T t;
    bool hit = glmutils::intersectRayPlane(orig, vel, plane, t);
    return hit ? t : T(-1000000.0);
  }

  static T trace(mat3 const& A)
  {
    return A[0][0] + A[1][1] + A[2][2];
  }

  static vec3 eigen_values(mat3 const& A)
  {
    return glmutils::fast_eig(A);
  }

  static mat3 eigen_vectors(mat3 const& A)
  {
    mat3 EV;
    glmutils::fast_eig(A,EV);
    return EV;
  }
};


template <typename T>
void luaopen_libGlm_typed(lua_State* L, const std::string Suffix)
{
  using namespace luabind;
  using vec2 = glm::detail::tvec2<T>;
  using vec3 = glm::detail::tvec3<T>;
  using vec4 = glm::detail::tvec4<T>;
  using mat3 = glm::detail::tmat3x3<T>;
  using mat4 = glm::detail::tmat4x4<T>;
  using bbox2 = glmutils::tbbox<vec2>;
  using bbox3 = glmutils::tbbox<vec3>;
  using helper = helpers<T>;

  module(L, (Suffix + "glm").c_str())
  [
    def("isnan", (bool(*)(T const &)) &glm::isnan),

    def("abs", (T(*)(T const &)) &glm::abs),
    def("sign", (T(*)(T const &)) &glm::sign),
    def("ceil", (T(*)(T const &)) &glm::ceil),
    def("clamp", (T(*)(T const &, T const &, T const &)) &glm::clamp),

    def("length", (T(*)(T const &)) &glm::length),
    def("length2", (T(*)(T const &)) &glm::length2),
    def("distance", (T(*)(T const &, T const &)) &glm::distance),
    def("distance2", (T(*)(T const &, T const &)) &glm::distance2),

    class_<vec2>((Suffix + "vec2").c_str())
      .def(constructor<const vec2&>())
      .def(constructor<const T&>())
      .def(constructor<const T&, const T&>())
      .def_readwrite("x", &vec2::x)
      .def_readwrite("y", &vec2::y)
      .def_readwrite("r", &vec2::x)
      .def_readwrite("g", &vec2::y)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - T())
      .def(const_self - vec2())
      .def(const_self + T())
      .def(const_self + vec2())
      .def(T() * const_self)
      .def(const_self * T())
      .def(const_self * vec2())
      .def(T() / const_self)
      .def(const_self / T())
      .def(const_self / vec2()),

    def("abs", (vec2(*)(vec2 const &)) &glm::abs),
    def("sign", (vec2(*)(vec2 const &)) &glm::sign),
    def("ceil", (vec2(*)(vec2 const &)) &glm::ceil),
    def("clamp", (vec2(*)(vec2 const &, vec2 const &, vec2 const &)) &glm::clamp),

    def("length", (T(*)(vec2 const &)) &glm::length),
    def("length2", (T(*)(vec2 const &)) &glm::length2),
    def("dot", (T(*)(vec2 const &, vec2 const &)) &glm::dot),
    def("normalize", (vec2(*)(vec2 const &)) &glm::normalize),
    def("distance", (T(*)(vec2 const &, vec2 const &)) &glm::distance),
    def("distance2", (T(*)(vec2 const &, vec2 const &)) &glm::distance2),

    class_<vec3>((Suffix + "vec3").c_str())
      .def(constructor<const vec3&>())
      .def(constructor<const T&>())
      .def(constructor<const T&, const T&, const T&>())
      .def_readwrite("x", &vec3::x)
      .def_readwrite("y", &vec3::y)
      .def_readwrite("z", &vec3::z)
      .def_readwrite("r", &vec3::x)
      .def_readwrite("g", &vec3::y)
      .def_readwrite("b", &vec3::z)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - T())
      .def(const_self - vec3())
      .def(const_self + T())
      .def(const_self + vec3())
      .def(T() * const_self)
      .def(const_self * T())
      .def(const_self * vec3())
      .def(T() / const_self)
      .def(const_self / T())
      .def(const_self / vec3()),
      
    def("abs", (vec3(*)(vec3 const &)) &glm::abs),
    def("sign", (vec3(*)(vec3 const &)) &glm::sign),
    def("ceil", (vec3(*)(vec3 const &)) &glm::ceil),
    def("clamp", (vec3(*)(vec3 const &, vec3 const &, vec3 const &)) &glm::clamp),

    def("length", (T(*)(vec3 const &)) &glm::length),
    def("length2", (T(*)(vec3 const &)) &glm::length2),
    def("dot", (T(*)(vec3 const &, vec3 const &)) &glm::dot),
    def("cross", (vec3(*)(vec3 const &, vec3 const &)) &glm::cross),
    def("normalize", (vec3(*)(vec3 const &)) &glm::normalize),
    def("distance", (T(*)(vec3 const &, vec3 const &)) &glm::distance),
    def("distance2", (T(*)(vec3 const &, vec3 const &)) &glm::distance2),

    class_<vec4>((Suffix + "vec4").c_str())
      .def(constructor<const vec4&>())
      .def(constructor<const T&>())
      .def(constructor<const vec3&, const T&>())
      .def(constructor<const T&, const T&, const T&, const T&>())
      .def_readwrite("x", &vec4::x)
      .def_readwrite("y", &vec4::y)
      .def_readwrite("z", &vec4::z)
      .def_readwrite("w", &vec4::w)
      .def_readwrite("r", &vec4::x)
      .def_readwrite("g", &vec4::y)
      .def_readwrite("b", &vec4::z)
      .def_readwrite("a", &vec4::w)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - T())
      .def(const_self - vec4())
      .def(const_self + T())
      .def(const_self + vec4())
      .def(T() * const_self)
      .def(const_self * T())
      .def(const_self * vec4())
      .def(T() / const_self)
      .def(const_self / T())
      .def(const_self / vec4()),

    def("abs", (vec4(*)(vec4 const &)) &glm::abs),
    def("sign", (vec4(*)(vec4 const &)) &glm::sign),
    def("ceil", (vec4(*)(vec4 const &)) &glm::ceil),
    def("clamp", (vec4(*)(vec4 const &, vec4 const &, vec4 const &)) &glm::clamp),

    def("length", (T(*)(vec4 const &)) &glm::length),
    def("length2", (T(*)(vec4 const &)) &glm::length2),
    def("dot", (T(*)(vec4 const &, vec4 const &)) &glm::dot),
    def("normalize", (vec4(*)(vec4 const &)) &glm::normalize),
    def("distance", (T(*)(vec4 const &, vec4 const &)) &glm::distance),
    def("distance2", (T(*)(vec4 const &, vec4 const &)) &glm::distance2),

    def("smoothstep", (T(*)(const vec2&, T)) &helper::smoothstep),
    def("smoothstep", (T(*)(const T&, const T&, const T&)) &glm::smoothstep<T>),
    def("smoothhump", (T(*)(const vec4&, T)) &helper::smoothhump),
    def("intersectRayPlane", (T(*)(vec3 const&, vec3 const&, vec4 const&)) &helper::intersectRayPlane),

    class_<mat3>((Suffix + "mat3").c_str())
      .def(constructor<>())
      .def(constructor<T>())
      .def(constructor<const vec3&, const vec3&, const vec3&>())
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == other<mat3>())
      .def(const_self + T())
      .def(T() + const_self)
      .def(const_self + other<mat3>())
      .def(const_self - T())
      .def(T() - const_self)
      .def(const_self - other<mat3>())
      .def(const_self * other<mat3>())
      .def(const_self * T())
      .def(T() * const_self)
      .def(const_self * vec3())
      .def(const_self / T())
      .def(T() / const_self)
      .def(const_self / vec3()),

    def("row", (vec3(*)(mat3 const &, int)) &glm::row),
    def("column", (vec3(*)(mat3 const &, int)) &glm::column),
    def("transpose", (mat3(*)(mat3 const&)) &glm::transpose),
    def("left_mul3", &helper::left_mul3),
    def("trace", (T(*)(mat3 const&)) &helper::trace),
    def("eigen_values", (vec3(*)(mat3 const&)) &helper::eigen_values),
    def("eigen_vectors", (mat3(*)(mat3 const&)) &helper::eigen_vectors),
    def("outerProduct", (mat3(*)(vec3 const &, vec3 const &)) &glm::outerProduct),
    def("localSpace", (mat4(*)(vec3 const&, vec3 const&, vec3 const&, vec3 const&)) &glmutils::localSpace<T>),
    def("transformPoint", (vec3(*)(mat4 const&, vec3 const&)) &glmutils::transformPoint<T>),
    def("transformVector", (vec3(*)(mat4 const&, vec3 const&)) &glmutils::transformVector<T>),

    class_<mat4>((Suffix + "mat4").c_str())
      .def(constructor<>())
      .def(constructor<T>())
      .def(constructor<const mat3&>())
      .def(constructor<const vec4&, const vec4&, const vec4&, const vec4&>())
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self + T())
      .def(T() + const_self)
      .def(const_self + other<mat4>())
      .def(const_self - T())
      .def(T() - const_self)
      .def(const_self - other<mat4>())
      .def(const_self * other<mat4>())
      .def(const_self * T())
      .def(T() * const_self)
      .def(const_self * vec4())
      .def(const_self / T())
      .def(T() / const_self)
      .def(const_self / vec4()),

    def("row", (vec4(*)(mat4 const &, int)) &glm::row),
    def("column", (vec4(*)(mat4 const &, int)) &glm::column),
    def("transpose", (mat4(*)(mat4 const&)) &glm::transpose),
    def("translate", (mat4(*)(mat4 const &, vec3 const&)) &glm::translate),
    def("rotate", (mat4(*)(mat4 const &, T const &, vec3 const&)) &glm::rotate),
    def("scale", (mat4(*)(mat4 const &, vec3 const&)) &glm::scale),
    def("left_mul4", &helper::left_mul4),

    class_<bbox2>((Suffix + "bbox2").c_str())
      .def(constructor<>())
      .def(constructor<const vec2&>())
      .def(constructor<const vec2&, const vec2&>())
      .property("p0", (const vec2& (bbox2::*)() const) &bbox2::p0)
      .property("p1", (const vec2& (bbox2::*)() const) &bbox2::p1),

      def("center", (vec2(*)(bbox2 const &)) &glmutils::center),
      def("volume", (T(*)(bbox2 const &)) &glmutils::volume),
      def("extent", (vec2(*)(bbox2 const &)) &glmutils::extent),
      def("inflate", (void(*)(bbox2& b, vec2 const&)) &glmutils::inflate),
      def("include", (void(*)(bbox2& b, vec2 const&)) &glmutils::include),
      def("include", (void(*)(bbox2& b, bbox2 const&)) &glmutils::include),

    class_<bbox3>((Suffix + "bbox3").c_str())
      .def(constructor<>())
      .def(constructor<const vec3&>())
      .def(constructor<const vec3&, const vec3&>())
      .property("p0", (const vec3& (bbox3::*)() const) &bbox3::p0)
      .property("p1", (const vec3& (bbox3::*)() const) &bbox3::p1),

      def("center", (vec3(*)(bbox3 const &)) &glmutils::center),
      def("volume", (T(*)(bbox3 const &)) &glmutils::volume),
      def("extent", (vec3(*)(bbox3 const &)) &glmutils::extent),
      def("inflate", (void(*)(bbox3& b, vec3 const&)) &glmutils::inflate),
      def("include", (void(*)(bbox3& b, vec3 const&)) &glmutils::include),
      def("include", (void(*)(bbox3& b, bbox3 const&)) &glmutils::include)
  ];
}


void luaopen_libGlm(lua_State* L)
{
  luaopen_libGlm_typed<float>(L, "");
}
