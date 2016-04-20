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


namespace {

  double smoothstep(glm::dvec2& e, double x)
  {
    return glm::smoothstep(e.x, e.y, x);
  }

  double smoothhump(double e0, double e1, double e2, double e3, double x)
  {
    return glm::smoothstep(e0, e1, x) * (1.0 - glm::smoothstep(e2, e3, x));
  }

  double smoothhump(const glm::dvec4& e, double x)
  {
    return smoothhump(e.x, e.y, e.z, e.w, x);
  }

	bool isnan(double val)
	{
		return glm::isnan(val);
	}

  glm::dvec3 left_mul3(glm::dvec3 const& v, glm::dmat3 const& M)
  {
    return v * M;
  }

  glm::dvec4 left_mul4(glm::dvec4 const& v, glm::dmat4 const& M)
  {
    return v * M;
  }

  double intersectRayPlane(glm::dvec3 const& orig, glm::dvec3 const& vel, glm::dvec4 const& plane)
  {
    double t;
    bool hit = glmutils::intersectRayPlane(orig, vel, plane, t);
    return hit ? t : -1000000.0;
  }

  double trace(glm::dmat3 const& A)
  {
    return A[0][0] + A[1][1] + A[2][2];
  }

  glm::dvec3 eigen_values(glm::dmat3 const& A)
  {
    return glmutils::fast_eig(A);
  }

  glm::dmat3 eigen_vectors(glm::dmat3 const& A)
  {
    glm::dmat3 EV;
    glmutils::fast_eig(A,EV);
    return EV;
  }
}


void luaopen_libGlm(lua_State* L)
{
  using namespace luabind;
  using glm::dvec2;
  using glm::dvec3;
  using glm::dvec4;
  using glm::dmat3;
  using glm::dmat4;
  using glmutils::dbbox2;
  using glmutils::dbbox3;

  module(L,"glm")
  [
	  def("isnan", &isnan),

    def("abs", (double(*)(double const &)) &glm::abs),
    def("sign", (double(*)(double const &)) &glm::sign),
    def("ceil", (double(*)(double const &)) &glm::ceil),
    def("clamp", (double(*)(double const &, double const &, double const &)) &glm::clamp),

    def("length", (double(*)(double const &)) &glm::length),
    def("length2", (double(*)(double const &)) &glm::length2),
    def("distance", (double(*)(double const &, double const &)) &glm::distance),
    def("distance2", (double(*)(double const &, double const &)) &glm::distance2),

	class_<glm::detail::tvec3<double>>("tvec3")
	.def(constructor<const glm::vec3&>())
	.def(constructor<const double&>())
	.def(constructor<const double&, const double&, const double&>())
	.def_readwrite("x", &glm::detail::tvec3<double>::x)
	.def_readwrite("y", &glm::detail::tvec3<double>::y)
	.def_readwrite("z", &glm::detail::tvec3<double>::z),



    class_<dvec2>("vec2")
      .def(constructor<const dvec2&>())
      .def(constructor<const double&>())
      .def(constructor<const double&, const double&>())
      .def_readwrite("x", &dvec2::x)
      .def_readwrite("y", &dvec2::y)
      .def_readwrite("r", &dvec2::x)
      .def_readwrite("g", &dvec2::y)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - double())
      .def(const_self - dvec2())
      .def(const_self + double())
      .def(const_self + dvec2())
      .def(double() * const_self)
      .def(const_self * double())
      .def(const_self * dvec2())
      .def(double() / const_self)
      .def(const_self / double())
      .def(const_self / dvec2()),

    def("abs", (dvec2(*)(dvec2 const &)) &glm::abs),
    def("sign", (dvec2(*)(dvec2 const &)) &glm::sign),
    def("ceil", (dvec2(*)(dvec2 const &)) &glm::ceil),
    def("clamp", (dvec2(*)(dvec2 const &, dvec2 const &, dvec2 const &)) &glm::clamp),

    def("length", (double(*)(dvec2 const &)) &glm::length),
    def("length2", (double(*)(dvec2 const &)) &glm::length2),
    def("dot", (double(*)(dvec2 const &, dvec2 const &)) &glm::dot),
    def("normalize", (dvec2(*)(dvec2 const &)) &glm::normalize),
    def("distance", (double(*)(dvec2 const &, dvec2 const &)) &glm::distance),
    def("distance2", (double(*)(dvec2 const &, dvec2 const &)) &glm::distance2),

    class_<dvec3>("vec3")
      .def(constructor<const dvec3&>())
      .def(constructor<const double&>())
      .def(constructor<const double&, const double&, const double&>())
      .def_readwrite("x", &dvec3::x)
      .def_readwrite("y", &dvec3::y)
      .def_readwrite("z", &dvec3::z)
      .def_readwrite("r", &dvec3::x)
      .def_readwrite("g", &dvec3::y)
      .def_readwrite("b", &dvec3::z)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - double())
      .def(const_self - dvec3())
      .def(const_self + double())
      .def(const_self + dvec3())
      .def(double() * const_self)
      .def(const_self * double())
      .def(const_self * dvec3())
      .def(double() / const_self)
      .def(const_self / double())
      .def(const_self / dvec3()),
      
    def("abs", (dvec3(*)(dvec3 const &)) &glm::abs),
    def("sign", (dvec3(*)(dvec3 const &)) &glm::sign),
    def("ceil", (dvec3(*)(dvec3 const &)) &glm::ceil),
    def("clamp", (dvec3(*)(dvec3 const &, dvec3 const &, dvec3 const &)) &glm::clamp),

    def("length", (double(*)(dvec3 const &)) &glm::length),
    def("length2", (double(*)(dvec3 const &)) &glm::length2),
    def("dot", (double(*)(dvec3 const &, dvec3 const &)) &glm::dot),
    def("cross", (dvec3(*)(dvec3 const &, dvec3 const &)) &glm::cross),
    def("normalize", (dvec3(*)(dvec3 const &)) &glm::normalize),
    def("distance", (double(*)(dvec3 const &, dvec3 const &)) &glm::distance),
    def("distance2", (double(*)(dvec3 const &, dvec3 const &)) &glm::distance2),
    def("outerProduct", (dmat3(*)(dvec3 const &, dvec3 const &)) &glm::outerProduct),

    def("localSpace", (dmat4(*)(dvec3 const&, dvec3 const&, dvec3 const&, dvec3 const&)) &glmutils::localSpace<double>),
    def("transformPoint", (dvec3(*)(dmat4 const&, dvec3 const&)) &glmutils::transformPoint<double>),
    def("transformVector", (dvec3(*)(dmat4 const&, dvec3 const&)) &glmutils::transformVector<double>),

    class_<dvec4>("vec4")
      .def(constructor<const dvec4&>())
      .def(constructor<const double&>())
      .def(constructor<const dvec3&, const double&>())
      .def(constructor<const double&, const double&, const double&, const double&>())
      .def_readwrite("x", &dvec4::x)
      .def_readwrite("y", &dvec4::y)
      .def_readwrite("z", &dvec4::z)
      .def_readwrite("w", &dvec4::w)
      .def_readwrite("r", &dvec4::x)
      .def_readwrite("g", &dvec4::y)
      .def_readwrite("b", &dvec4::z)
      .def_readwrite("a", &dvec4::w)
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self - double())
      .def(const_self - dvec4())
      .def(const_self + double())
      .def(const_self + dvec4())
      .def(double() * const_self)
      .def(const_self * double())
      .def(const_self * dvec4())
      .def(double() / const_self)
      .def(const_self / double())
      .def(const_self / dvec4()),

    def("abs", (dvec4(*)(dvec4 const &)) &glm::abs),
    def("sign", (dvec4(*)(dvec4 const &)) &glm::sign),
    def("ceil", (dvec4(*)(dvec4 const &)) &glm::ceil),
    def("clamp", (dvec4(*)(dvec4 const &, dvec4 const &, dvec4 const &)) &glm::clamp),

    def("length", (double(*)(dvec4 const &)) &glm::length),
    def("length2", (double(*)(dvec4 const &)) &glm::length2),
    def("dot", (double(*)(dvec4 const &, dvec4 const &)) &glm::dot),
    def("normalize", (dvec4(*)(dvec4 const &)) &glm::normalize),
    def("distance", (double(*)(dvec4 const &, dvec4 const &)) &glm::distance),
    def("distance2", (double(*)(dvec4 const &, dvec4 const &)) &glm::distance2),
    def("outerProduct", (dmat4(*)(dvec4 const &, dvec4 const &)) &glm::outerProduct),

    class_<dmat3>("mat3")
      .def(constructor<>())
      .def(constructor<double>())
      .def(constructor<const dvec3&, const dvec3&, const dvec3&>())
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == other<dmat3>())
      .def(const_self + double())
      .def(double() + const_self)
      .def(const_self + other<dmat3>())
      .def(const_self - double())
      .def(double() - const_self)
      .def(const_self - other<dmat3>())
      .def(const_self * other<dmat3>())
      .def(const_self * double())
      .def(double() * const_self)
      .def(const_self * dvec3())
      .def(const_self / double())
      .def(double() / const_self)
      .def(const_self / dvec3()),

    def("row", (dvec3(*)(dmat3 const &, int)) &glm::row),
    def("column", (dvec3(*)(dmat3 const &, int)) &glm::column),
    def("transpose", (dmat3(*)(dmat3 const&)) &glm::transpose),
    def("left_mul3", &left_mul3),
    def("trace", (double(*)(dmat3 const&)) &trace),
    def("eigen_values", (dvec3(*)(dmat3 const&)) &eigen_values),
    def("eigen_vectors", (dmat3(*)(dmat3 const&)) &eigen_vectors),

    class_<dmat4>("mat4")
      .def(constructor<>())
      .def(constructor<double>())
      .def(constructor<const dmat3&>())
      .def(constructor<const dvec4&, const dvec4&, const dvec4&, const dvec4&>())
      .def(tostring(const_self))
      .def(- const_self)
      .def(const_self == const_self)
      .def(const_self + double())
      .def(double() + const_self)
      .def(const_self + other<dmat4>())
      .def(const_self - double())
      .def(double() - const_self)
      .def(const_self - other<dmat4>())
      .def(const_self * other<dmat4>())
      .def(const_self * double())
      .def(double() * const_self)
      .def(const_self * dvec4())
      .def(const_self / double())
      .def(double() / const_self)
      .def(const_self / dvec4()),

    def("row", (dvec4(*)(dmat4 const &, int)) &glm::row),
    def("column", (dvec4(*)(dmat4 const &, int)) &glm::column),
    def("transpose", (dmat4(*)(dmat4 const&)) &glm::transpose),
    def("translate", (dmat4(*)(dmat4 const &, dvec3 const&)) &glm::translate),
    def("rotate", (dmat4(*)(dmat4 const &, double const &, dvec3 const&)) &glm::rotate),
    def("scale", (dmat4(*)(dmat4 const &, dvec3 const&)) &glm::scale),
    def("left_mul4", &left_mul4),

    class_<dbbox2>("bbox2")
      .def(constructor<>())
      .def(constructor<const dvec2&>())
      .def(constructor<const dvec2&, const dvec2&>())
      .property("p0", (const dvec2& (dbbox2::*)() const) &dbbox2::p0)
      .property("p1", (const dvec2& (dbbox2::*)() const) &dbbox2::p1),

    def("center", (dvec2(*)(dbbox2 const &)) &glmutils::center),
    def("volume", (double(*)(dbbox2 const &)) &glmutils::volume),
    def("extent", (dvec2(*)(dbbox2 const &)) &glmutils::extent),
    def("inflate", (void(*)(dbbox2& b, dvec2 const&)) &glmutils::inflate),
    def("include", (void(*)(dbbox2& b, dvec2 const&)) &glmutils::include),
    def("include", (void(*)(dbbox2& b, dbbox2 const&)) &glmutils::include),

    class_<dbbox3>("bbox3")
      .def(constructor<>())
      .def(constructor<const dvec3&>())
      .def(constructor<const dvec3&, const dvec3&>())
      .property("p0", (const dvec3& (dbbox3::*)() const) &dbbox3::p0)
      .property("p1", (const dvec3& (dbbox3::*)() const) &dbbox3::p1),

    def("center", (dvec3(*)(dbbox3 const &)) &glmutils::center),
    def("volume", (double(*)(dbbox3 const &)) &glmutils::volume),
    def("extent", (dvec3(*)(dbbox3 const &)) &glmutils::extent),
    def("inflate", (void(*)(dbbox3& b, dvec3 const&)) &glmutils::inflate),
    def("include", (void(*)(dbbox3& b, dvec3 const&)) &glmutils::include),
    def("include", (void(*)(dbbox3& b, dbbox3 const&)) &glmutils::include),

    def("smoothstep", (double(*)(const dvec2&, double)) &smoothstep),
    def("smoothstep", (double(*)(const double&, const double&, const double&)) &glm::smoothstep<double>),
    def("smoothhump", (double(*)(const dvec4&, double)) &smoothhump),
    def("intersectRayPlane", (double(*)(dvec3 const&, dvec3 const&, dvec4 const&)) &intersectRayPlane)
  ];
}



