#ifndef LUA_CHECK_HPP_INCLUDED
#define LUA_CHECK_HPP_INCLUDED

#include <exception>
#include <string>
#include <luautils/luautils.hpp>
#include "Parameter.hpp"
#include "initial.hpp"


namespace param {


	inline bool check_parameter_minmax(const char* name, lua_Number x, lua_Number x0, lua_Number x1)
  {
    if (x < x0 || x > x1) 
    {
      lua_pushfstring(LUA, "%s = %f out of bounds [%f, %f]", name, x, x0, x1);
			return false;
    }
		return true;
  }

  inline bool check_parameter_min(const char* name, lua_Number x, lua_Number x0)
  {
    if (x < x0) 
    {
      lua_pushfstring(LUA, "%s = %f out of bounds [%f, ...]", name, x, x0);
      return false;
    }
		return true;
  }

  inline bool check_parameter_normdist(const char* name, const vec4& x)
  {
    if (x.x > x.y || x.z < x.y)
    {
      lua_pushfstring(LUA, "Normal distribution settings illegal in %s", name);
      return false;
    }
		return true;
  }

  template <typename Seq, typename T>
  inline bool lua_check(Seq&, const T&)
  {
		return true;
  }

  template <>
  inline bool lua_check(LuaParam&, const Ruler& val)
  {
    return check_parameter_min("Ruler.MinFlockSize", val.MinFlockSize, 5u);
  }

  template <>
  inline bool lua_check(LuaParam&, const Initial& val)
  {
    return check_parameter_minmax("Initial.Predators", val.Predators, 0, CParam::MAX_PREDATORS);
  }

  template <>
  inline bool lua_check(LuaParam&, const InitialDist& val)
  {
    if (val.Pdf != "const" && val.Pdf != "geometric") 
    {
      lua_pushstring(LUA, "Initial.Pdf must me \"const\" or \"geometric\"");
      return false;
    }
    init_initializer(val);
		return true;
  }

  template <>
  inline bool lua_check(LuaParam&, const Boid& val)
  {
    return check_parameter_normdist("Boid.reactionTime", val.reactionTime) &&
					 check_parameter_normdist("Boid.bodyMass", val.bodyMass) &&
					 check_parameter_normdist("Boid.separationRadius", val.separationRadius);
  }

}

#endif
