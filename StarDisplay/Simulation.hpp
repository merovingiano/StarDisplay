#ifndef SIMULATION_HPP_INCLUDED
#define SIMULATION_HPP_INCLUDED

#include <mutex>
#include <string>
#include <boost/noncopyable.hpp>
#include <luabind/luabind.hpp>
#include "glmfwd.hpp"
#include "Params.hpp"
#include "evolvePN.hpp"
#include <vector>
#include <iostream>

class Simulation
{
public:
  Simulation();
  ~Simulation();

  //Robin
  EvolvePN evolution;
  bool done_;
  double timeSinceEvolution;
  //
  void SetInitialParameter(const Param::Params&);
  void SetParams(const Param::Params&);
  void GetExperimentSettings(const luabind::object& obj);
  void SetPFeatureMap(const Param::FeatureMap&);
  void SetPRoost(const Param::Roost&);
  void SetPRenderFlags(const Param::RenderFlags&);
  void RegisterFactories(const luabind::object&, const luabind::object&);
  void RegisterDataStorage(const luabind::object&);
  void RegisterCustomStatistic(class IStatistic* sb);
  luabind::object& GetActiveCamera();
  void SetActiveCamera(const luabind::object& luaobj);
  const Param::Params& Params() const { return params_; }

  void SelectStatistic(int selectedId);
  void ResetCurrentStatistic();
  void PauseCurrentStatistic();
  void PrintVector(glm::vec3 input, std::string text);
  void PrintVec2(glm::vec2 input, std::string text);
  void PrintFloat(float input, std::string text);
  void PrintString(std::string text);
  void ResumeCurrentStatistic();
  void SaveCurrentStatistic(const char* fname, bool append);
  void SetFocalBird(const class CBird* bird, bool showTrail);
  void SetPredatorTarget(const class CBird*);
  bool HandleKey(unsigned key, unsigned keystate);
  void EnableReload(bool enable);
  const class CBird* PickById(int id, bool showTrail);
  void setNumPrey(unsigned newNum);
  void setNumPredators(unsigned newNum);
  unsigned getNumPrey() const;
  unsigned getNumPredators() const;
  double SimulationTime() const { return SimulationTime_; }   //!< elapsed simulation time since program started (seconds)
  double UpdateTime() const { return UpdateTime_; }           //!< smoothed update time
  double FrameTime() const { return FrameTime_; }             //!< smoothed last frame time
  void DisplayStatistic() const;
  void main(int argc, char** argv);

public:
  Simulation&                sim() { return *this; }
  class CFlock&              flock() { return *(flock_); }
  class CFlock const&        cflock() const { return *(flock_); }
  class GLSLState&           gl() const { return *(gl_); }
  class ICamera const&       ccamera() const { return *camera_; }
  class trail_buffer_pool&   trails() const { return *(trails_); }

  //experiments
  std::vector <Param::Experiment> experiments;
  int expNumb;
  
  //data storage
  luabind::object StorageData_;
private:
  const class CBird* PickNearestBird2Ray(const glm::vec3& ray_position, const glm::vec3& ray_direction);
  void UpdateBirds(const float sim_dt);
  void UpdateCurrentStatistic(double stat_dt);
  void UpdateSimulation(double sim_dt);
  void deleteInvisibles();

private:
	// Callbacks Window class
  void OnSize(int height, int width);
  void OnLButtonDown(int x, int y, unsigned ks);
  void OnLButtonUp(int x, int y);
  void OnLButtonDblClk(int x, int y);
  void OnMouseMove(int dx, int dy, bool LButtonPressed);
  void OnContextMenu(int menuEntry);
  void copyBirdParam(luabind::object& obj, Param::Bird& bird);
private:
  void EnterGameLoop();
  void EnterGameLoopNoGraphicsNoLua();
private:
  double  SimulationTime_;
  double  UpdateTime_;
  double  FrameTime_;

  std::unique_ptr<class trail_buffer_pool>  trails_;
  std::unique_ptr<class CFlock>             flock_;
  std::shared_ptr<class IStatistic>         statistic_;
  std::unique_ptr<class GLSLState>          gl_;
  class ICamera*                            camera_;
  luabind::object                           luaCamera_;

  bool  statisticsPaused_;
  bool  paused_;
  bool  track_alpha_;

  double lastStatTime_;
  double lastClusterTime_;

  // Parameter records
  Param::Params params_;

 

  // Registered callbacks
  luabind::object PreyFactory_;
  luabind::object PredatorFactory_;
  

  std::shared_ptr<IStatistic> CustomStatistic_;

  mutable std::mutex omp_exception_mutex_;
    
  friend class GLWin;
  friend class LuaStarDisplay;
};


#endif
