#ifndef LIBGLM_LIBGLM_HPP_INCLUDED
#define LIBGLM_LIBGLM_HPP_INCLUDED

#include <glm/glm.hpp>
#include <glmutils/bbox.hpp>
#include <luabind/luabind.hpp>


#ifndef NDEBUG
  #include <typeinfo>

  void LUA_API logGLMConversions(lua_State*, const char* A, const char* B);


  template <typename A, typename B>
  inline void logGLMConversions(lua_State* L) 
  {
    logGLMConversions(L, typeid(A).name(), typeid(B).name());
  }
#else
  template <typename A, typename B>
  inline void logGLMConversions(lua_State*) 
  {
  }
#endif


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
      logGLMConversions<B,A>(L);
      return A(luabind::object_cast<B>(luabind::object(luabind::from_stack(L, index))));
    }

    void to(lua_State* L, A const& value)
    {
      logGLMConversions<A,B>(L);
      luabind::detail::convert_to_lua(L, B(value));
    }
  };

}


namespace luabind {

  template <> struct default_converter<glm::dvec2> : luaglm::converter<glm::dvec2, glm::vec2> {};
  template <> struct default_converter<glm::dvec2 const> : luaglm::converter<glm::dvec2, glm::vec2> {};
  template <> struct default_converter<glm::dvec2 const&> : luaglm::converter<glm::dvec2, glm::vec2> {};

  template <> struct default_converter<glm::dvec3> : luaglm::converter<glm::dvec3, glm::vec3> {};
  template <> struct default_converter<glm::dvec3 const> : luaglm::converter<glm::dvec3, glm::vec3> {};
  template <> struct default_converter<glm::dvec3 const&> : luaglm::converter<glm::dvec3, glm::vec3> {};

  template <> struct default_converter<glm::dvec4> : luaglm::converter<glm::dvec4, glm::vec4> {};
  template <> struct default_converter<glm::dvec4 const> : luaglm::converter<glm::dvec4, glm::vec4> {};
  template <> struct default_converter<glm::dvec4 const&> : luaglm::converter<glm::dvec4, glm::vec4> {};

  template <> struct default_converter<glm::ivec2> : luaglm::converter<glm::ivec2, glm::vec2> {};
  template <> struct default_converter<glm::ivec2 const> : luaglm::converter<glm::ivec2, glm::vec2> {};
  template <> struct default_converter<glm::ivec2 const&> : luaglm::converter<glm::ivec2, glm::vec2> {};

  template <> struct default_converter<glm::ivec3> : luaglm::converter<glm::ivec3, glm::vec3> {};
  template <> struct default_converter<glm::ivec3 const> : luaglm::converter<glm::ivec3, glm::vec3> {};
  template <> struct default_converter<glm::ivec3 const&> : luaglm::converter<glm::ivec3, glm::vec3> {};

  template <> struct default_converter<glm::ivec4> : luaglm::converter<glm::ivec4, glm::vec4> {};
  template <> struct default_converter<glm::ivec4 const> : luaglm::converter<glm::ivec4, glm::vec4> {};
  template <> struct default_converter<glm::ivec4 const&> : luaglm::converter<glm::ivec4, glm::vec4> {};

  template <> struct default_converter<glm::dmat3> : luaglm::converter<glm::dmat3, glm::mat3> {};
  template <> struct default_converter<glm::dmat3 const> : luaglm::converter<glm::dmat3, glm::mat3> {};
  template <> struct default_converter<glm::dmat3 const&> : luaglm::converter<glm::dmat3, glm::mat3> {};

  template <> struct default_converter<glm::dmat4> : luaglm::converter<glm::dmat4, glm::mat4> {};
  template <> struct default_converter<glm::dmat4 const> : luaglm::converter<glm::dmat4, glm::mat4> {};
  template <> struct default_converter<glm::dmat4 const&> : luaglm::converter<glm::dmat4, glm::mat4> {};
}

#endif
