#ifndef LIBSTARDISPLAY_HPP_INCLUDED
#define LIBSTARDISPLAY_HPP_INCLUDED

#include <mutex>
#include <exception>
#include <functional>
#include <lua/lua.hpp>
#include <luabind/luabind.hpp>
#include "libGlm.hpp"


class LUA_API LuaStarDisplay
{
  LuaStarDisplay(const LuaStarDisplay&);
  LuaStarDisplay& operator=(const LuaStarDisplay&);

public:
  typedef std::unique_lock<std::mutex> unique_lock;

  LuaStarDisplay();
  ~LuaStarDisplay();

  void Open();
  void Close();
  void cacheLuaSim();
  const char* ErrMsg() { return errMsg_; }

  operator lua_State* () { return L_; }
  luabind::object operator()();
  luabind::object operator()(const char* key);
   
  void DoFile(const char* filename);
  bool ProcessKeyboardHooks(unsigned key, unsigned kbstate);
	bool ProcessMouseHooks(int x, int y, unsigned button, bool dbl, unsigned kbState);

  class ITextBox* CreateTextBox(class IText* self, const char* name, const glm::ivec4& box, const class ICamera* camera);

  unique_lock LuaLock() const { return unique_lock(mutex_); }

private:
  void SetCurrentException();

  lua_State* L_;
  luabind::object LuaSim_;
  char* errMsg_;
  void setLuaErrMsg(const char* msg);
  friend int pcall_error_callback(lua_State*);
  mutable std::mutex mutex_;
};


#endif
