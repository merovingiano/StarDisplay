#include "libStarDisplay.hpp"
#include "libGlm.hpp"
#include "libRnd.hpp"
#include "luabind/class_info.hpp"

using namespace luabind;


extern void luaopen_libGlm(lua_State* L);
extern void luaopen_libWin(lua_State* L);
extern void luaopen_libParam(lua_State* L);
extern void luaopen_libCamera(lua_State* L);
extern void luaopen_libIText(lua_State* L);
extern void luaopen_libAnalysis(lua_State* L);
extern void luaopen_libStatistic(lua_State* L);
extern "C" LUALIB_API int luaopen_bit(lua_State* L);


namespace libIText
{
  extern class ITextBox* CreateTextBoxCpp(class IText* self, const char* name, const glm::ivec4& box, const class ICamera* camera);
}


static int pcall_error_callback(lua_State* L)
{
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  const char* msg = lua_tostring(L, -1);
  lua_getfield(L, LUA_REGISTRYINDEX, "__LuaStarDisplayThisPtr");
  LuaStarDisplay* lsd = (LuaStarDisplay*)lua_touserdata(L, -1);
  lsd->setLuaErrMsg(msg);
  return LUA_ERRRUN;
}


LuaStarDisplay::LuaStarDisplay() : L_(0), errMsg_(0)
{
  L_ = luaL_newstate();
}


void LuaStarDisplay::Open()
{ 
  lua_pushlightuserdata(L_, (void*)(this));
  lua_setfield(L_, LUA_REGISTRYINDEX, "__LuaStarDisplayThisPtr");
  set_pcall_callback(&pcall_error_callback);
  luaL_openlibs(L_); 
  luabind::open(L_);
  luabind::bind_class_info(L_);
  luaopen_libGlm(L_);
  luaopen_libRnd(L_);
  luaopen_libWin(L_);
	luaopen_libParam(L_);
  luaopen_libCamera(L_);
  luaopen_libIText(L_);
  luaopen_libAnalysis(L_);
  luaopen_libStatistic(L_);
  luaopen_bit(L_);
}


void LuaStarDisplay::cacheLuaSim()
{
  LuaSim_ = luabind::globals(L_)["Simulation"];
}


void LuaStarDisplay::Close()
{ 
  if (LuaSim_.is_valid()) LuaSim_ = luabind::object();
  if (L_) lua_close(L_); 
  delete [] errMsg_;
  L_ = 0; 
  errMsg_ = 0;
}


LuaStarDisplay::~LuaStarDisplay()
{ 
  Close(); 
}


object LuaStarDisplay::operator()()
{
  return LuaSim_;
}


object LuaStarDisplay::operator()(const char* key)
{
  return LuaSim_[key];
}


void LuaStarDisplay::DoFile(const char* filename)
{
  if ( luaL_dofile(L_, filename) )
  {
    setLuaErrMsg("");
    throw error(L_);
  }
}


bool LuaStarDisplay::ProcessKeyboardHooks(unsigned key, unsigned kbState)
{
	object pkh = globals(L_)["__ProcessKeyboardHooks"];
	return object_cast<bool>(pkh((key << 8) | kbState));
}


bool LuaStarDisplay::ProcessMouseHooks(int x, int y, unsigned button, bool dbl, unsigned kbState)
{
	object pmh = globals(L_)["__ProcessMouseHooks"];
	if (dbl) kbState |= 8;
	return object_cast<bool>(pmh((button << 8) | kbState, x, y));
}


class ITextBox* LuaStarDisplay::CreateTextBox(class IText* self, const char* name, const glm::ivec4& box, const class ICamera* camera)
{
  return libIText::CreateTextBoxCpp(self, name, box, camera);
}


void LuaStarDisplay::setLuaErrMsg(const char* msg)
{
  delete [] errMsg_;
  errMsg_ = new char[strlen(msg) + 1];
  strcpy(errMsg_, msg);
}


