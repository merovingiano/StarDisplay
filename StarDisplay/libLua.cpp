#include <exception>
#include <thread>             // std::thread::hardware_concurrency
#include <hrtree/config.hpp>  // omp, HRTREE_OMP_MAX_THREADS
#include "filesystem.hpp"
#include <luabind/adopt_policy.hpp>
#include <luabind/iterator_policy.hpp>
#include "Clock.hpp"
#include "libParam.hpp"
#include "libLua.hpp"
#include "libGlm.hpp"
#include "Globals.hpp"
#include "Flock.hpp"
#include "GLSLState.hpp"
#include "GLSLImm.hpp"
#include "GLWin.hpp"
#include "ICamera.hpp"
#include "IText.hpp"
#include "Camera.hpp"

//Robin
//#include "Flock.hpp"

extern void luaopen_libBirds(lua_State*);


namespace {


  struct flockmate_iterator
  {
    flockmate_iterator(CFlock::prey_iterator it, int id) : it_(it), id_(id) {}
    CFlock::prey_iterator it_;
    int id_;
  };


  void line(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& color)
  {
    GGl.imm3D->Begin(IMM_LINES);
    GGl.imm3D->Emit(glm::vec3(p0), color32(glm::vec3(color)));
    GGl.imm3D->Emit(glm::vec3(p1), color32(glm::vec3(color)));
    GGl.imm3D->End();
  }
}

namespace luabind { namespace detail {

  template <>
  static int iterator<CFlock::prey_iterator>::next(lua_State* L)
  {
    iterator* self = static_cast<iterator*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (self->first != self->last)
    {
      convert_to_lua(L, static_cast<CPrey*>(&*self->first));
      ++self->first;
    }
    else
    {
      lua_pushnil(L);
    }
    return 1;
  }


  template <>
  static int iterator<CFlock::pred_iterator>::next(lua_State* L)
  {
    iterator* self = static_cast<iterator*>(lua_touserdata(L, lua_upvalueindex(1)));
    if (self->first != self->last)
    {
      convert_to_lua(L, static_cast<CPredator*>(&*self->first));
      ++self->first;
    }
    else
    {
      lua_pushnil(L);
    }
    return 1;
  }


  template <>
  static int iterator<flockmate_iterator>::next(lua_State* L)
  {
    iterator* self = static_cast<iterator*>(lua_touserdata(L, lua_upvalueindex(1)));
    while (self->first.it_ != self->last.it_) 
    {
      if (self->first.it_->getFlockId() == self->first.id_)
      {
        convert_to_lua(L, static_cast<CPrey*>(&*self->first.it_));
        ++self->first.it_;
        return 1;
      }
      ++self->first.it_;
    }
    lua_pushnil(L);
    return 1;
  }

}}


namespace liblua {
  
  using namespace luabind;
	using namespace Param;
  using namespace libParam;

  void SetInitialParameter(const object& obj) 
  { 
    Sim.SetInitialParameter(FromLua<Params>(obj)); 
  }

  void GetExperimentSettings(const object& obj)
  {
	  Sim.GetExperimentSettings(obj);
  }
    
  double SimulationTime() 
  { 
    return Sim.SimulationTime(); 
  }
    
  double RealTime() 
  { 
    return GlobalTimerNow(); 
  }
    
  GLWin& Window() 
  { 
    return AppWindow; 
  }
    
  ICamera* CreateCamera() 
  { 
    return new CCamera(); 
  }
    
  void SetActiveCamera(const object& luaobj)
  {
    Sim.SetActiveCamera(luaobj);
  }

  object GetActiveCamera()
  {
    return Sim.GetActiveCamera();
  }

  void ShowAnnotation(const char* txt, double duration) 
  { 
    GGl.setAnnotation(txt, duration); 
  }
    
  object GetRoost() 
  { 
    return ToLua<Roost>(Lua, PROOST); 
  }

  object GetVaderJacob()
  {
	  return ToLua<vaderJacob>(Lua, VADER);
  }
    
  void SetRoost(const object& luaobj) 
  { 
    Sim.SetPRoost( FromLua<Roost>(luaobj) ); 
  }
    
  object GetRenderFlags() 
  { 
    return ToLua<Param::RenderFlags>(Lua, PRENDERFLAGS); 
  }
    
  void SetRenderFlags(const object& luaobj) 
  { 
    Sim.SetPRenderFlags( FromLua<Param::RenderFlags>(luaobj) ); 
  }
    
  CFlock& GetFlock() 
  { 
    return GFLOCKNC; 
  }

  void RegisterFactories(const object& PreyFactory, const object& PredatorFactory)
  {
    Sim.RegisterFactories(PreyFactory, PredatorFactory);
  }


  void RegisterDataStorage(const object& dataStorage)
  {
	  Sim.RegisterDataStorage(dataStorage);
  }

  void RegisterCustomStatistic(class IStatistic* sp)
  {
    Sim.RegisterCustomStatistic(sp);
  }

  void ResetCurrentStatistic()
  {
    Sim.ResetCurrentStatistic();
  }

  void PauseCurrentStatistic()
  {
    Sim.PauseCurrentStatistic();
  }

  void ResumeCurrentStatistic()
  {
    Sim.ResumeCurrentStatistic();
  }

  void SaveCurrentStatistic(const char* fname, bool append)
  {
    Sim.SaveCurrentStatistic(fname, append);
  }

  CPrey* NewPrey(int id, const glm::vec3& position, const glm::vec3& forward)
  {
    CPrey* ret = new CPrey(id, position, forward);
    return ret;
  }

  CPredator* NewPredator(int id, const glm::vec3& position, const glm::vec3& forward)
  {
    CPredator* ret = new CPredator(id, position, forward);

	//Robin insert, may be wrong
	//Sim.flock_->insert_pred(ret);
    return ret;
  }

  void SetFeatureMap(const object& params)
  {
    Sim.SetPFeatureMap( FromLua<FeatureMap>(params) );
  }

  object GetFeatureMap()
  {
    return ToLua<FeatureMap>(Lua, PFM);
  }

  IText* Text() 
  { 
    return GTEXT.get(); 
  }

	CBird* GetBirdAtScreenPosition(int x, int y)
	{
		const glm::vec3 CSD = glm::vec3(GCAMERA.screenDirection(x, y));
		return GFLOCKNC.pickNearest2Ray(glm::vec3(GCAMERA.eye()), CSD);
	}

  void QuitSimulation()
  {
    AppWindow.PostMessage(WM_CLOSE);
  }

  void DoEmulateKeyDown(unsigned KS)
  {
    AppWindow.PostMessage(WM_USER+0, WPARAM(KS));
  }

  unsigned SetNumPrey(unsigned newNum)
  {
    Sim.setNumPrey(newNum);
    return GFLOCK.num_prey();
  }

  unsigned SetNumPredators(unsigned newNum)
  {
    Sim.setNumPredators(newNum);
    return GFLOCK.num_pred();
  }

  double IntegrationTimeStep()
  {
    return PARAMS.IntegrationTimeStep;
  }

  int PreyIterator(lua_State* L)
  {
    return luabind::detail::make_range(L, GFLOCKNC.prey_begin(), GFLOCKNC.prey_end());
  }


  int PredIterator(lua_State* L)
  {
    return luabind::detail::make_range(L, GFLOCKNC.predator_begin(), GFLOCKNC.predator_end());
  }


  int ClusterIterator(lua_State* L)
  {
    return luabind::detail::make_range(L, GFLOCK.clusters_begin(), GFLOCK.clusters_end());
  }


  int FlockmateIterator(lua_State* L, const CPrey* prey)
  {
    int id = prey->getFlockId();
    return luabind::detail::make_range(L, flockmate_iterator(GFLOCKNC.prey_begin(), id), flockmate_iterator(GFLOCKNC.prey_end(), id));
  }


  int FlockmemberIterator(lua_State* L, int id)
  {
    return luabind::detail::make_range(L, flockmate_iterator(GFLOCKNC.prey_begin(), id), flockmate_iterator(GFLOCKNC.prey_end(), id));
  }


  int NeighborsIterator(lua_State* L, const CBird* bird)
  {
    return luabind::detail::make_range(L, bird->nbegin(), bird->nend());
  }


  void open_libLua()
  {
    module(Lua, "Simulation")[
      def("SetInitialParameter", &SetInitialParameter),
		  def("GetExperimentSettings", &GetExperimentSettings),
      def("Window", &Window),
      def("CreateCamera", &CreateCamera, adopt(result)),
      def("GetActiveCamera", &GetActiveCamera),
      def("SetActiveCamera", &SetActiveCamera),
      def("SimulationTime", &SimulationTime),
      def("IntegrationTimeStep", &IntegrationTimeStep),
      def("RealTime", &RealTime),
      def("ShowAnnotation", &ShowAnnotation),
      def("GetFlock", &GetFlock),
      def("GetRoost", &GetRoost),
	  def("GetVaderJacob", &GetVaderJacob),
      def("SetRoost", &SetRoost),
      def("NewPrey", &NewPrey),
      def("NewPredator", &NewPredator),
      def("RegisterFactories", &RegisterFactories),
	  def("RegisterDataStorage", &RegisterDataStorage),
      def("ResetCurrentStatistic" , &ResetCurrentStatistic),
      def("PauseCurrentStatistic", &PauseCurrentStatistic),
      def("ResumeCurrentStatistic", &ResumeCurrentStatistic),
      def("SaveCurrentStatistic", &SaveCurrentStatistic),
      def("RegisterCustomStatistic", &RegisterCustomStatistic),
      def("GetFeatureMap", &GetFeatureMap),
      def("SetFeatureMap", &SetFeatureMap),
      def("Text", &Text),
			def("GetBirdAtScreenPosition", &GetBirdAtScreenPosition),
      def("GetRenderFlags", &GetRenderFlags),
      def("SetRenderFlags", &SetRenderFlags),
      def("Quit", &QuitSimulation),
      def("DoEmulateKeyDown", &DoEmulateKeyDown),
      def("SetNumPrey", &SetNumPrey),
      def("SetNumPredators", &SetNumPredators),
      def("Prey", &PreyIterator),
      def("Predators", &PredIterator),
      def("Clusters", &ClusterIterator),
      def("Flockmates", &FlockmateIterator),
      def("Flockmembers", &FlockmemberIterator),
      def("Neighbors", &NeighborsIterator),
      def("line", &line)
    ];
  }


  void Open(const char* ConfigFile, const char* ExeFile)
  {
    Lua.Open();
    luaopen_libBirds(Lua);
    open_libLua();
    filesystem::path ExePath = filesystem::path(ExeFile).parent_path();
    if (ExePath.string().empty())
    {
      ExePath /= ".";
    }
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string thePath(buffer);
	thePath = thePath.substr(0, thePath.find("x64\\"));
    Lua.cacheLuaSim();
    Lua()["hardware_concurrency"] = std::min<unsigned>(HRTREE_OMP_MAX_THREADS, std::thread::hardware_concurrency());
    Lua.DoFile((ExePath / "/../../lua/StarDisplay.lua").string().c_str());
    // Optional OpenMP stuff
    unsigned num_threads = luabind::object_cast<unsigned>(Lua("concurrency_num_threads"));
    bool dynamic = luabind::object_cast<bool>(Lua("concurrency_dynamic"));
    num_threads = std::max<unsigned>(1, num_threads);
    omp_set_num_threads(num_threads);
    omp_set_dynamic(dynamic ? num_threads : 0);
    Lua()["concurrency_num_threads"] = num_threads;
	Lua("Initialize")(ExePath.string().c_str(), ConfigFile, thePath);
  }

}
