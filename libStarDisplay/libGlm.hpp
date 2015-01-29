#ifndef LIBGLM_LIBGLM_HPP_INCLUDED
#define LIBGLM_LIBGLM_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glmutils/bbox.hpp>
#include <glmutils/avx/vec.hpp>
#include <glmutils/avx/mat3.hpp>
#include <glmutils/avx/mat4.hpp>
#include <luabind/luabind.hpp>


namespace luaglm {

  template <typename A, typename B>
  struct converter : luabind::native_converter_base<A>
  {
    static int compute_score(lua_State* L, int index)
    {
      boost::optional<B*> p = luabind::object_cast_nothrow<B*>(luabind::object(luabind::from_stack(L, index)));
      return p ? 0 : -1;
    }

    A from(lua_State* L, int index)
    {
      return A(luabind::object_cast<B>(luabind::object(luabind::from_stack(L, index))));
    }

    void to(lua_State* L, A const& value)
    {
      luabind::detail::convert_to_lua(L, B(value));
    }
  };

}


namespace luaavx {

  template <typename A, typename B>
  struct converter : luabind::native_converter_base<A>
  {
    static int compute_score(lua_State* L, int index)
    {
      boost::optional<B*> p = luabind::object_cast_nothrow<B*>(luabind::object(luabind::from_stack(L, index)));
      return p ? 0 : -1;
    }

    A from(lua_State* L, int index)
    {
      return A(luabind::object_cast<typename A::glm_type>(luabind::object(luabind::from_stack(L, index))));
    }

    void to(lua_State* L, A const& value)
    {
      luabind::detail::convert_to_lua(L, B(avx::cast(value)));
    }
  };

}


namespace luabind {

  namespace avx = glmutils::avx;

  template <> struct default_converter<glm::vec2> : luaglm::converter<glm::vec2, glm::dvec2> {};
  template <> struct default_converter<glm::vec2 const> : luaglm::converter<glm::vec2, glm::dvec2> {};
  template <> struct default_converter<glm::vec2 const&> : luaglm::converter<glm::vec2, glm::dvec2> {};

  template <> struct default_converter<glm::vec3> : luaglm::converter<glm::vec3, glm::dvec3> {};
  template <> struct default_converter<glm::vec3 const> : luaglm::converter<glm::vec3, glm::dvec3> {};
  template <> struct default_converter<glm::vec3 const&> : luaglm::converter<glm::vec3, glm::dvec3> {};

  template <> struct default_converter<glm::vec4> : luaglm::converter<glm::vec4, glm::dvec4> {};
  template <> struct default_converter<glm::vec4 const> : luaglm::converter<glm::vec4, glm::dvec4> {};
  template <> struct default_converter<glm::vec4 const&> : luaglm::converter<glm::vec4, glm::dvec4> {};

  template <> struct default_converter<glm::ivec2> : luaglm::converter<glm::ivec2, glm::dvec2> {};
  template <> struct default_converter<glm::ivec2 const> : luaglm::converter<glm::ivec2, glm::dvec2> {};
  template <> struct default_converter<glm::ivec2 const&> : luaglm::converter<glm::ivec2, glm::dvec2> {};

  template <> struct default_converter<glm::ivec3> : luaglm::converter<glm::ivec3, glm::dvec3> {};
  template <> struct default_converter<glm::ivec3 const> : luaglm::converter<glm::ivec3, glm::dvec3> {};
  template <> struct default_converter<glm::ivec3 const&> : luaglm::converter<glm::ivec3, glm::dvec3> {};

  template <> struct default_converter<glm::ivec4> : luaglm::converter<glm::ivec4, glm::dvec4> {};
  template <> struct default_converter<glm::ivec4 const> : luaglm::converter<glm::ivec4, glm::dvec4> {};
  template <> struct default_converter<glm::ivec4 const&> : luaglm::converter<glm::ivec4, glm::dvec4> {};

  template <> struct default_converter<glm::mat3> : luaglm::converter<glm::mat3, glm::dmat3> {};
  template <> struct default_converter<glm::mat3 const> : luaglm::converter<glm::mat3, glm::dmat3> {};
  template <> struct default_converter<glm::mat3 const&> : luaglm::converter<glm::mat3, glm::dmat3> {};

  template <> struct default_converter<glm::mat4> : luaglm::converter<glm::mat4, glm::dmat4> {};
  template <> struct default_converter<glm::mat4 const> : luaglm::converter<glm::mat4, glm::dmat4> {};
  template <> struct default_converter<glm::mat4 const&> : luaglm::converter<glm::mat4, glm::dmat4> {};

  template <> struct default_converter<glmutils::bbox2> : luaglm::converter<glmutils::bbox2, glmutils::dbbox2> {};
  template <> struct default_converter<glmutils::bbox2 const> : luaglm::converter<glmutils::bbox2, glmutils::dbbox2> {};
  template <> struct default_converter<glmutils::bbox2 const&> : luaglm::converter<glmutils::bbox2, glmutils::dbbox2> {};

  template <> struct default_converter<glmutils::bbox3> : luaglm::converter<glmutils::bbox3, glmutils::dbbox3> {};
  template <> struct default_converter<glmutils::bbox3 const> : luaglm::converter<glmutils::bbox3, glmutils::dbbox3> {};
  template <> struct default_converter<glmutils::bbox3 const&> : luaglm::converter<glmutils::bbox3, glmutils::dbbox3> {};

  template <> struct default_converter<avx::vec3> : luaavx::converter<avx::vec3, glm::dvec3> {};
  template <> struct default_converter<avx::vec3 const> : luaavx::converter<avx::vec3, glm::dvec3> {};
  template <> struct default_converter<avx::vec3 const&> : luaavx::converter<avx::vec3, glm::dvec3> {};

  template <> struct default_converter<avx::vec4> : luaavx::converter<avx::vec4, glm::dvec4> {};
  template <> struct default_converter<avx::vec4 const> : luaavx::converter<avx::vec4, glm::dvec4> {};
  template <> struct default_converter<avx::vec4 const&> : luaavx::converter<avx::vec4, glm::dvec4> {};

  template <> struct default_converter<avx::mat3> : luaavx::converter<avx::mat3, glm::dmat3> {};
  template <> struct default_converter<avx::mat3 const> : luaavx::converter<avx::mat3, glm::dmat3> {};
  template <> struct default_converter<avx::mat3 const&> : luaavx::converter<avx::mat3, glm::dmat3> {};

  template <> struct default_converter<avx::mat4> : luaavx::converter<avx::mat4, glm::dmat4> {};
  template <> struct default_converter<avx::mat4 const> : luaavx::converter<avx::mat4, glm::dmat4> {};
  template <> struct default_converter<avx::mat4 const&> : luaavx::converter<avx::mat4, glm::dmat4> {};

}


#endif
