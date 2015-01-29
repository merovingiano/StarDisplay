#ifndef LIBPARAM_HPP_INCLUDED
#define LIBPARAM_HPP_INCLUDED

#include <luabind\luabind.hpp>
#include "Params.hpp"


namespace libParam {

  using namespace Param;

	// FromLua ToLua
	template <typename T>
	T LUA_API FromLua(const luabind::object& luaobj);

  template <typename T>
  luabind::object LUA_API ToLua(lua_State* L, const T& cobj);

  template <> Roost       LUA_API FromLua<Roost>(const luabind::object& luaobj);
  template <> FeatureMap  LUA_API FromLua<FeatureMap>(const luabind::object& luaobj);
  template <> RenderFlags LUA_API FromLua<RenderFlags>(const luabind::object& luaobj);
  template <> Pursuit     LUA_API FromLua<Pursuit>(const luabind::object& luaobj);
  template <> GPWS        LUA_API FromLua<GPWS>(const luabind::object& luaobj);
  template <> Params      LUA_API FromLua<Params>(const luabind::object& luaobj);
  
  template <> luabind::object LUA_API ToLua<Roost>(lua_State* L, const Roost& cobj);
  template <> luabind::object LUA_API ToLua<vaderJacob>(lua_State* L, const vaderJacob& cobj);
  template <> luabind::object LUA_API ToLua<RenderFlags>(lua_State* L, const RenderFlags& cobj);
}


#endif
