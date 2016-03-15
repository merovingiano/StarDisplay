#include <luabind/luabind.hpp>
#include <luabind/adopt_policy.hpp>
#include "libParam.hpp"
#include "libGLM.hpp"
#include <iostream>

namespace libParam
{
  using namespace Param;
	using namespace luabind;


   template <> Roost FromLua<Roost>(const object& luaobj)
  {
    Roost cobj;
    cobj.numPrey = object_cast<unsigned>(luaobj["numPrey"]);
    cobj.numPredators = object_cast<unsigned>(luaobj["numPredators"]);
    cobj.Radius = object_cast<float>(luaobj["Radius"]);
    cobj.minRadius = object_cast<float>(luaobj["minRadius"]);
    cobj.maxRadius = object_cast<float>(luaobj["maxRadius"]);
    return cobj;
  }

  template <> Panic LUA_API FromLua<Panic>(const luabind::object& luaobj)
  {
    Panic cobj;
    cobj.type = object_cast<int>(luaobj["type"]);
    if (cobj.type == Prey::Custom)
    {
      lua_State* L = luaobj.interpreter();
      object hook = luaobj["hook"];
      hook.push(L);
      if (lua_isfunction(L, -1))
      {
        cobj.hook = hook;
      }
      else
      {
        cobj = Panic();   // render it useless
      }
      lua_pop(L, 1);
    }
    else
    {
      cobj.weight = object_cast<float>(luaobj["weight"]);
      cobj.edges = object_cast<glm::vec4>(luaobj["edges"]);
    }
    return cobj;
  }

  template <> Pursuit LUA_API FromLua<Pursuit>(const luabind::object& luaobj)
  {
    Pursuit cobj;
    cobj.type = object_cast<int>(luaobj["type"]);
    if (cobj.type == Predator::CUSTOM)
    {
      lua_State* L = luaobj.interpreter();
      object hook = luaobj["hook"];
      hook.push(L);
      if (lua_isfunction(L, -1))
      {
        cobj.hook = hook;
      }
      else
      {
        cobj = Pursuit();   // render it useless
      }
      lua_pop(L, 1);
    }
    //else
    //{
    //  cobj.deflection = object_cast<glm::vec3>(luaobj["deflection"]);
    //}
    return cobj;
  }

  template <> GPWS LUA_API FromLua<GPWS>(const luabind::object& luaobj)
  {
    GPWS cobj;
    cobj.type = object_cast<int>(luaobj["type"]);
    cobj.threshold = object_cast<float>(luaobj["threshold"]);
    if (cobj.type == 1)
    {
      lua_State* L = luaobj.interpreter();
      object hook = luaobj["hook"];
      hook.push(L);
      if (lua_isfunction(L, -1))
      {
        cobj.hook = hook;
      }
      else
      {
        cobj = GPWS();   // render it useless
      }
      lua_pop(L, 1);
    }
    else
    {
      cobj.tti = object_cast<float>(luaobj["tti"]);
      cobj.lift = object_cast<float>(luaobj["lift"]);
    }
    return cobj;
  }

  template <> ModelLod FromLua<ModelLod>(const object& luaobj)
  {
    ModelLod cobj;
    cobj.acFile = object_cast<std::string>(luaobj["acFile"]);
    cobj.pxCoverage = object_cast<float>(luaobj["pxCoverage"]);
    return cobj;
  }

  template <> ModelDef FromLua<ModelDef>(const object& luaobj)
  {
    ModelDef cobj;
    cobj.name = object_cast<std::string>(luaobj["name"]);
    object lods = luaobj["LOD"];
    int i = 0;
    for (iterator it(lods), end; it != end; ++it, ++i)
    {
      if (i > 3)
      {
        lua_pushstring(luaobj.interpreter(), "Models3D: max. supported numbers of LODs is 4");
        lua_error(luaobj.interpreter());
      }
      cobj.LOD.push_back(FromLua<ModelLod>(*it));
    }
    cobj.Scale = object_cast<float>(luaobj["Scale"]);
    cobj.texMix = object_cast<float>(luaobj["texMix"]);
    return cobj;
  }

  template <> FeatureMap::hist LUA_API FromLua<FeatureMap::hist>(const luabind::object& luaobj)
  {
    FeatureMap::hist cobj;
    cobj.min = object_cast<float>(luaobj[1]);
    cobj.max = object_cast<float>(luaobj[2]);
    cobj.bins = object_cast<int>(luaobj[3]);
    return cobj;
  }

  template <> FeatureMap::Entry LUA_API FromLua<FeatureMap::Entry>(const luabind::object& luaobj)
  {
    FeatureMap::Entry cobj;
    cobj.enable = object_cast<bool>(luaobj["enable"]);
    cobj.dt = object_cast<double>(luaobj["dt"]);         
    cobj.hist = FromLua<FeatureMap::hist>(luaobj["hist"]);
    cobj.title = object_cast<std::string>(luaobj["title"]);      
    iterator it(luaobj["p"]), end;
    for (int i=0; (it != end) && (i < 5); ++i, ++it)
    {
      cobj.p[i] = object_cast<float>(*it);
    }
    cobj.colored = object_cast<bool>(luaobj["colored"]);
    return cobj;
  }

  template <> FeatureMap LUA_API FromLua<FeatureMap>(const luabind::object& luaobj)
  {
    FeatureMap cobj;
    cobj.histKeepPercent = object_cast<double>(luaobj["histKeepPercent"]);
    cobj.current = object_cast<FeatureMap::FMappings>(luaobj["current"]);
    for (iterator it(luaobj["Entries"]), end; it != end; ++it)
    {
      cobj.Entries.push_back(FromLua<FeatureMap::Entry>(*it));
    }
    return cobj;
  }

  template <> Skybox LUA_API FromLua<Skybox>(const luabind::object& luaobj)
  {
    Skybox cobj;
    cobj.name = object_cast<std::string>(luaobj["name"]);
    cobj.ColorCorr = object_cast<glm::vec3>(luaobj["ColorCorr"]);
    cobj.ColorCorrAlt = object_cast<glm::vec3>(luaobj["ColorCorrAlt"]);
    cobj.fovy = object_cast<float>(luaobj["fovy"]);
    return cobj;
  }

  template <> RenderFlags LUA_API FromLua<RenderFlags>(const luabind::object& luaobj)
  {
    RenderFlags cobj;
    cobj.show_local = object_cast<bool>(luaobj["show_local"]);
    cobj.show_head = object_cast<bool>(luaobj["show_head"]);
    cobj.show_search = object_cast<bool>(luaobj["show_search"]);
    cobj.show_forces = object_cast<bool>(luaobj["show_forces"]);
    cobj.show_neighbors = object_cast<bool>(luaobj["show_neighbors"]);
    cobj.show_pred = object_cast<bool>(luaobj["show_pred"]);
    cobj.show_circ = object_cast<bool>(luaobj["show_circ"]);
    cobj.show_trails = object_cast<bool>(luaobj["show_trails"]);
    cobj.show_world = object_cast<bool>(luaobj["show_world"]);
    cobj.show_rulers = object_cast<bool>(luaobj["show_rulers"]);
    cobj.show_annotation = object_cast<bool>(luaobj["show_annotation"]);
    cobj.show_hist = object_cast<bool>(luaobj["show_hist"]);
    cobj.show_numbers = object_cast<bool>(luaobj["show_numbers"]);
    cobj.show_header = object_cast<bool>(luaobj["show_header"]);
    cobj.show_fps = object_cast<bool>(luaobj["show_fps"]);
    cobj.wireFrame = object_cast<bool>(luaobj["wireFrame"]);
    cobj.altBackground = object_cast<bool>(luaobj["altBackground"]);
    cobj.alphaMasking = object_cast<bool>(luaobj["alphaMasking"]);
    cobj.slowMotion = object_cast<bool>(luaobj["slowMotion"]);
    cobj.rtreeLevel = object_cast<int>(luaobj["rtreeLevel"]);
    cobj.helpMsg = object_cast<const char*>(luaobj["helpMsg"]);
    return cobj;
  }


  template <> Params LUA_API FromLua<Params>(const luabind::object& luaobj)
  {
    Params cobj;
    luabind::object Sim = luabind::globals(luaobj.interpreter())["Simulation"];
    cobj.DebugLevel = object_cast<int>(Sim["DebugLevel"]);
    cobj.DebugLogOnce = object_cast<bool>(Sim["DebugLogOnce"]);
    cobj.DebugLogStackLevel = object_cast<int>(Sim["DebugLogStackLevel"]);
    object tab = Sim["FSAA"];
    cobj.FSAA[0] = object_cast<int>(tab[1]);
    cobj.FSAA[1] = object_cast<int>(tab[2]);
    cobj.swap_control = object_cast<int>(Sim["swap_control"]);
    cobj.maxPrey = object_cast<unsigned>(Sim["maxPrey"]);
    cobj.maxPredators = object_cast<unsigned>(Sim["maxPredators"]);
    cobj.maxTopologicalRange = object_cast<unsigned>(Sim["maxTopologicalRange"]);
    cobj.roost = FromLua<Roost>(luaobj["Roost"]);
    cobj.IntegrationTimeStep = object_cast<double>(Sim["IntegrationTimeStep"]);
    cobj.slowMotion = object_cast<int>(Sim["slowMotion"]);
    cobj.pausedSleep = object_cast<int>(Sim["pausedSleep"]);
    cobj.realTime = object_cast<bool>(Sim["realTime"]);
    cobj.maxSkippedFrames = object_cast<int>(Sim["maxSkippedFrames"]);
    tab = luaobj["Clustering"];
    cobj.ClusterDetectionTime = object_cast<double>(tab["DetectionTime"]);
    cobj.ClusterDistance1D = object_cast<float>(tab["Distance1D"]);
    cobj.ClusterDistance3D = object_cast<float>(tab["Distance3D"]);
    cobj.featureMap = FromLua<FeatureMap>(luaobj["FeatureMap"]);
    tab = Sim["Fonts"];
    cobj.TextColor = object_cast<glm::dvec3>(tab["TextColor"]);
    cobj.TextColorAlt = object_cast<glm::dvec3>(tab["TextColorAlt"]);
    for (iterator it(tab["Faces"]), end; it != end; ++it)
    {
      cobj.Fonts.push_back(std::make_pair(object_cast<const char*>(it.key()), object_cast<const char*>(*it)));
    }
    tab = luaobj["Ruler"];
    cobj.RulerTick = object_cast<float>(tab["Tick"]);
    cobj.RulerMinFlockSize = object_cast<unsigned>(tab["MinFlockSize"]);
    cobj.skybox = FromLua<Skybox>(Sim["Skybox"]);
    tab = luaobj["Trail"];
    cobj.TrailLength = object_cast<double>(tab["Length"]);
    cobj.TrailWidth = object_cast<float>(tab["Width"]);
    cobj.TrailTickInterval = object_cast<float>(tab["TickInterval"]);
    cobj.TrailTickWidth = object_cast<float>(tab["TickWidth"]);
    cobj.TrailSkip = object_cast<int  >(tab["Skip"]);
    for (iterator it(Sim["ModelSet"]), end; it != end; ++it)
    {
      cobj.ModelSet.push_back(FromLua<ModelDef>(*it));
    }
    cobj.renderFlags = FromLua<RenderFlags>(luaobj["RenderFlags"]);
	cobj.evolution.durationGeneration = object_cast<float  >(luaobj["evolution"]["durationGeneration"]);
	cobj.evolution.startGen = object_cast<int >(luaobj["evolution"]["startGen"]);
	cobj.evolution.load = object_cast<bool>(luaobj["evolution"]["load"]);
	cobj.evolution.loadFolder = object_cast<std::string>(luaobj["evolution"]["loadFolder"]);
	cobj.evolution.evolveAlt = object_cast<bool  >(luaobj["evolution"]["evolveAlt"]);
	cobj.evolution.type = object_cast<std::string  >(luaobj["evolution"]["type"]);
	cobj.evolution.fileName = object_cast<std::string  >(luaobj["evolution"]["fileName"]);
	cobj.evolution.evolvePN = object_cast<bool  >(luaobj["evolution"]["evolvePN"]);
	cobj.evolution.evolveDPAdjParam = object_cast<bool  >(luaobj["evolution"]["evolveDPAdjParam"]);
	cobj.evolution.evolveX = object_cast<bool  >(luaobj["evolution"]["evolveX"]);
	cobj.evolution.evolveZ = object_cast<bool  >(luaobj["evolution"]["evolveZ"]);
	cobj.evolution.evolveCL = object_cast<bool  >(luaobj["evolution"]["evolveCL"]);
	cobj.evolution.evolvewingAspectRatio = object_cast<bool  >(luaobj["evolution"]["evolvewingAspectRatio"]);
	cobj.evolution.evolvewingBeatFreq = object_cast<bool  >(luaobj["evolution"]["evolvewingBeatFreq"]);
	cobj.evolution.evolvetheta = object_cast<bool  >(luaobj["evolution"]["evolvetheta"]);
	cobj.evolution.evolvewingLength = object_cast<bool  >(luaobj["evolution"]["evolvewingLength"]);
	cobj.evolution.evolvebodyArea = object_cast<bool  >(luaobj["evolution"]["evolvebodyArea"]);
	cobj.evolution.evolvecBody = object_cast<bool  >(luaobj["evolution"]["evolvecBody"]);
	cobj.evolution.evolvecFriction = object_cast<bool  >(luaobj["evolution"]["evolvecFriction"]);
	cobj.evolution.evolvewingSpan = object_cast<bool  >(luaobj["evolution"]["evolvewingSpan"]);
	cobj.evolution.evolvemaxForce = object_cast<bool  >(luaobj["evolution"]["evolvemaxForce"]);
	cobj.evolution.evolvebodyMass = object_cast<bool  >(luaobj["evolution"]["evolvebodyMass"]);
	cobj.evolution.evolvewingMass = object_cast<bool  >(luaobj["evolution"]["evolvewingMass"]);
	cobj.evolution.evolveInertiaWing = object_cast<bool  >(luaobj["evolution"]["evolveInertiaWing"]);
	cobj.evolution.evolveInertiaBody = object_cast<bool  >(luaobj["evolution"]["evolveInertiaBody"]);
	cobj.evolution.evolveJ = object_cast<bool  >(luaobj["evolution"]["evolveJ"]);
	cobj.evolution.evolvecontrolCL = object_cast<bool  >(luaobj["evolution"]["evolvecontrolCL"]);
	cobj.evolution.evolvecruiseSpeed = object_cast<bool  >(luaobj["evolution"]["evolvecruiseSpeed"]);
	cobj.evolution.evolvemaxLift = object_cast<bool  >(luaobj["evolution"]["evolvemaxLift"]);
	cobj.evolution.evolvemaxSpeed = object_cast<bool  >(luaobj["evolution"]["evolvemaxSpeed"]);
	cobj.evolution.evolverollRate = object_cast<bool  >(luaobj["evolution"]["evolverollRate"]);
	cobj.evolution.evolveminSpeed = object_cast<bool  >(luaobj["evolution"]["evolveminSpeed"]);
	cobj.evolution.evolvereactionTime = object_cast<bool  >(luaobj["evolution"]["evolvereactionTime"]);
	cobj.evolution.evolvealignmentWeight = object_cast<bool  >(luaobj["evolution"]["evolvealignmentWeight"]);
	cobj.evolution.evolvecohesionWeight = object_cast<bool  >(luaobj["evolution"]["evolvecohesionWeight"]);
	cobj.evolution.evolveHandleTime = object_cast<bool  >(luaobj["evolution"]["evolveHandleTime"]);
	cobj.evolution.evolveLockDistance = object_cast<bool  >(luaobj["evolution"]["evolveLockDistance"]);
	cobj.evolution.TrajectoryBestPredator = object_cast<bool  >(luaobj["evolution"]["TrajectoryBestPredator"]);
	cobj.evolution.TrajectoryPrey = object_cast<bool  >(luaobj["evolution"]["TrajectoryPrey"]);
	cobj.evolution.terminationGeneration = object_cast<int  >(luaobj["evolution"]["terminationGeneration"]);
	cobj.evolution.externalPreyFile = object_cast<std::string  >(luaobj["evolution"]["externalPreyFile"]);
	cobj.evolution.externalPrey = object_cast<bool>(luaobj["evolution"]["externalPrey"]);
	cobj.evolution.title = object_cast<std::string>(luaobj["evolution"]["title"]);
	cobj.evolution.description = object_cast<std::string >(luaobj["evolution"]["description"]);

	cobj.birds.csv_file_species = object_cast<std::string  >(luaobj["Birds"]["csv_file_species"]);
	cobj.birds.csv_file_prey_predator_settings = object_cast<std::string  >(luaobj["Birds"]["csv_file_prey_predator_settings"]);


	return cobj;
  }


  template <> luabind::object ToLua<Roost>(lua_State* L, const Roost& cobj)
  {
    object luaobj = newtable(L);
    luaobj["numPrey"] = cobj.numPrey;
    luaobj["numPredators"] = cobj.numPredators;
    luaobj["Radius"] = cobj.Radius;
    luaobj["minRadius"] = cobj.minRadius;
    luaobj["maxRadius"] = cobj.maxRadius;
    return luaobj;
  }

  template <> luabind::object ToLua<vaderJacob>(lua_State* L, const vaderJacob& cobj)
  {
	  object luaobj = newtable(L);
	  luaobj["kuchDan"] = cobj.Eitje;

	  return luaobj;
  }


  template <> luabind::object LUA_API ToLua<Panic>(lua_State* L, const Panic& cobj)
  {
    object luaobj = newtable(L);
    luaobj["type"] = cobj.type;
    if (cobj.type == Prey::Custom)
    {
      luaobj["hook"] = cobj.hook;
    }
    else 
    {
      luaobj["weight"] = cobj.weight;
      luaobj["edges"] = cobj.edges;
    }
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<Pursuit>(lua_State* L, const Pursuit& cobj)
  {
    object luaobj = newtable(L);
    luaobj["type"] = cobj.type;
    if (cobj.type == Predator::CUSTOM)
    {
      luaobj["hook"] = cobj.hook;
    }
    else 
    {
      luaobj["deflection"] = cobj.deflection;
    }
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<GPWS>(lua_State* L, const GPWS& cobj)
  {
    object luaobj = newtable(L);
    luaobj["threshold"] = cobj.threshold;
    luaobj["type"] = cobj.type;
    if (cobj.type == Predator::CUSTOM)
    {
      luaobj["hook"] = cobj.hook;
    }
    else 
    {
      luaobj["tti"] = cobj.tti;
      luaobj["lift"] = cobj.lift;
    }
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<FeatureMap::hist>(lua_State* L, const FeatureMap::hist& cobj)
  {
    object luaobj = newtable(L);
    luaobj[1] = cobj.min;
    luaobj[2] = cobj.max;
    luaobj[3] = cobj.bins;
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<FeatureMap::Entry>(lua_State* L, const FeatureMap::Entry& cobj)
  {
    object luaobj = newtable(L);
    luaobj["enable"] = cobj.enable;
    luaobj["dt"] = cobj.dt;         
    luaobj["hist"] = ToLua<FeatureMap::hist>(L, cobj.hist);        
    luaobj["title"] = cobj.title;      
    object p = newtable(L);
    for (int i=0; i < 5; ++i)
    {
      p[i+1] = cobj.p[i];
    }
    luaobj["p"] = p;
    luaobj["colored"] = cobj.colored;
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<FeatureMap>(lua_State* L, const FeatureMap& cobj)
  {
    object luaobj = newtable(L);
    luaobj["histKeepPercent"] = cobj.histKeepPercent;
    luaobj["current"] = cobj.current;
    object luaEntries = newtable(L);
    for (int i=0; i < static_cast<int>(cobj.Entries.size()); ++i)
    {
      luaEntries[i+1] = ToLua<FeatureMap::Entry>(L, cobj.Entries[i]);
    }
    luaobj["Entries"] = luaEntries;
    return luaobj;
  }


  template <> luabind::object LUA_API ToLua<Skybox>(lua_State* L, const Skybox& cobj)
  {
    object luaobj = newtable(L);
    luaobj["name"] = cobj.name;
    luaobj["ColorCorr"] = cobj.ColorCorr;
    luaobj["ColorCorrAlt"] = cobj.ColorCorrAlt;
    luaobj["fovy"] = cobj.fovy;
    return luaobj;
  }

  
  template <> luabind::object LUA_API ToLua<RenderFlags>(lua_State* L, const RenderFlags& cobj)
  {
    object luaobj = newtable(L);
    luaobj["show_local"] = cobj.show_local;
    luaobj["show_head"] = cobj.show_head;
    luaobj["show_search"] = cobj.show_search;
    luaobj["show_forces"] = cobj.show_forces;
    luaobj["show_neighbors"] = cobj.show_neighbors;
    luaobj["show_pred"] = cobj.show_pred;
    luaobj["show_circ"] = cobj.show_circ;
    luaobj["show_trails"] = cobj.show_trails;
    luaobj["show_world"] = cobj.show_world;
    luaobj["show_rulers"] = cobj.show_rulers;
    luaobj["show_annotation"] = cobj.show_annotation;
    luaobj["show_hist"] = cobj.show_hist;
    luaobj["show_numbers"] = cobj.show_numbers;
    luaobj["show_header"] = cobj.show_header;
    luaobj["show_fps"] = cobj.show_fps;
    luaobj["wireFrame"] = cobj.wireFrame;
    luaobj["altBackground"] = cobj.altBackground;
    luaobj["alphaMasking"] = cobj.alphaMasking;
    luaobj["slowMotion"] = cobj.slowMotion;
    luaobj["rtreeLevel"] = cobj.rtreeLevel;
    luaobj["helpMsg"] = cobj.helpMsg;
    return luaobj;
  }

  void SetEvasionStrategy(Prey* prey, const object& luaobj)
  {
    Panic p = FromLua<Panic>(luaobj);
    prey->EvasionStrategy[int(p.type)] = p;
  }

  object GetEvasionStrategy(Prey* prey, const object& type)
  {
    return ToLua<Panic>(type.interpreter(), prey->EvasionStrategy[object_cast<int>(type)]);
  }

  void SetPursuitStrategy(Predator* pred, const object& luaobj)
  {
    Pursuit p = FromLua<Pursuit>(luaobj);
    pred->pursuit = p;
  }

  object GetPursuitStrategy(Predator* pred, const object& type)
  {
    return ToLua<Pursuit>(type.interpreter(), pred->pursuit);
  }


  double GetBodyMass(Bird& bird)
  {
    return bird.bodyMass;
  }

  double GetwingMass(Bird& bird)
  {
	  return bird.wingMass;
  }

  void SetBodyMass(Bird& bird, double newVal)
  {
    bird.bodyMass = float(newVal);
    bird.bodyWeight = float(9.81 * newVal);
  }

  void SetwingMass(Bird& bird, double newVal)
  {
	  bird.wingMass = float(newVal);
  }

  void SetGPWS(Bird* bird, const object& luaobj)
  {
    GPWS gpws = FromLua<GPWS>(luaobj);
    bird->gpws = gpws;
  }

  object GetGPWS(Bird* bird, const object& type)
  {
    return ToLua<GPWS>(type.interpreter(), bird->gpws);
  }

}


void luaopen_libParam(lua_State* L)
{
  using namespace luabind;
  using namespace Param;

  module(L, "Params")[
    class_<Bird>("Bird")
      .def(constructor<>())
      .def_readwrite("reactionTime", &Bird::reactionTime)
      .def_readwrite("reactionStochastic", &Bird::reactionStochastic)
      .def_readwrite("skipLeftHemisphere", &Bird::skipLeftHemisphere)
      .def_readwrite("skipRightHemisphere", &Bird::skipRightHemisphere)
      .property("bodyMass", &libParam::GetBodyMass, &libParam::SetBodyMass)
	  .property("wingMass", &libParam::GetwingMass, &libParam::SetwingMass)
      .property("GPWS", &libParam::GetGPWS, &libParam::SetGPWS)
      .def_readwrite("rho", &Bird::rho)
	  .def_readwrite("bodyWeight", &Bird::bodyWeight)
      .def_readwrite("wingSpan", &Bird::wingSpan)
	  .def_readwrite("InertiaWing", &Bird::InertiaWing)
	  .def_readwrite("InertiaBody", &Bird::InertiaBody)
	  .def_readwrite("J", &Bird::J)
      .def_readwrite("wingAspectRatio", &Bird::wingAspectRatio)
	  .def_readwrite("wingBeatFreq", &Bird::wingBeatFreq)
	  .def_readwrite("birdName", &Bird::birdName)
	  .def_readwrite("theta", &Bird::theta)
	  .def_readwrite("wingLength", &Bird::wingLength)
	  .def_readwrite("bodyArea", &Bird::bodyArea)
	  .def_readwrite("cBody", &Bird::cBody)
	  .def_readwrite("cBody", &Bird::cBody)
	  .def_readwrite("cFriction", &Bird::cFriction)
	  .def_readwrite("wingRetractionSpeed", &Bird::wingRetractionSpeed)
	  .def_readwrite("maneuver", &Bird::maneuver)
	  .def_readwrite("controlCL", &Bird::controlCL)
	  .def_readwrite("CDCL", &Bird::CDCL)
      .def_readwrite("wingArea", &Bird::wingArea)
      .def_readwrite("CL", &Bird::CL)
      .def_readwrite("maxForce", &Bird::maxForce)
	  .def_readwrite("houjebek", &Bird::houjebek)
      .def_readwrite("maxLift", &Bird::maxLift)
      .def_readwrite("cruiseSpeed", &Bird::cruiseSpeed)
      .def_readwrite("speedControl", &Bird::speedControl)
      .def_readwrite("maxSpeed", &Bird::maxSpeed)
	  .def_readwrite("rollRate", &Bird::rollRate)
      .def_readwrite("minSpeed", &Bird::minSpeed)
      .def_readwrite("wBetaOut", &Bird::wBetaOut)
      .def_readwrite("wBetaIn", &Bird::wBetaIn)
      .def_readwrite("maxRadius", &Bird::maxRadius)
      .def_readwrite("neighborLerp", &Bird::neighborLerp)
      .def_readwrite("topologicalRange", &Bird::topologicalRange)
      .def_readwrite("circularityInc", &Bird::circularityInc)
      .def_readwrite("binocularOverlap", &Bird::binocularOverlap)
      .def_readwrite("blindAngle", &Bird::blindAngle)
      .def_readwrite("maxSeparationTopo", &Bird::maxSeparationTopo)
      .def_readwrite("separationStep", &Bird::separationStep)
      .def_readwrite("separationWeight", &Bird::separationWeight)
      .def_readwrite("alignmentWeight", &Bird::alignmentWeight)
      .def_readwrite("cohesionWeight", &Bird::cohesionWeight)
      .def_readwrite("randomWeight", &Bird::randomWeight)
      .def_readwrite("boundaryWeight", &Bird::boundaryWeight)
      .def_readwrite("boundaryReflectAngle", &Bird::boundaryReflectAngle)
      .def_readwrite("outerBoundary", &Bird::outerBoundary)
      .def_readwrite("innerBoundary", &Bird::innerBoundary)
      .def_readwrite("altitude", &Bird::altitude),

    class_<Prey>("Prey")
      .def(constructor<>())
      .property("EvasionStrategy", &libParam::GetEvasionStrategy, &libParam::SetEvasionStrategy)
	  .def_readwrite("EvasionStrategyTEMP", &Prey::EvasionStrategyTEMP)
      .def_readwrite("DetectCruising", &Prey::DetectCruising)
      .def_readwrite("DetectionDistance", &Prey::DetectionDistance)
      .def_readwrite("DetectionSurfaceProb", &Prey::DetectionSurfaceProb)
      .def_readwrite("DetectionHemisphereFOV", &Prey::DetectionHemisphereFOV)
      .def_readwrite("IncurNeighborPanic", &Prey::IncurNeighborPanic)
      .def_readwrite("IncurLatency", &Prey::IncurLatency)
      .def_readwrite("AlertnessRelexation", &Prey::AlertnessRelexation)
      .def_readwrite("AlertedReactionTimeFactor", &Prey::AlertedReactionTimeFactor)
      .def_readwrite("AlertedWBetaIn", &Prey::AlertedWBetaIn)
      .def_readwrite("AlertedWBetaOut", &Prey::AlertedWBetaOut)
      .def_readwrite("AlertedTopo", &Prey::AlertedTopo)
      .def_readwrite("AlertedAlignmentWeight", &Prey::AlertedAlignmentWeight)
      .def_readwrite("Return2Flock", &Prey::Return2Flock)
      .def_readwrite("ReturnRelaxation", &Prey::ReturnRelaxation)
      .def_readwrite("ReturnWeight", &Prey::ReturnWeight)
      .def_readwrite("ReturnThreshold", &Prey::ReturnThreshold),

    class_<Predator>("Predator")
      .def(constructor<>())
      //.property("PursuitStrategy", &libParam::GetPursuitStrategy, &libParam::SetPursuitStrategy)
	  .def_readwrite("PursuitStrategy", &Predator::PursuitStrategy)
      .def_readwrite("StartAttack", &Predator::StartAttack)
      .def_readwrite("PreySelection", &Predator::PreySelection)
      .def_readwrite("AttackWBetaIn", &Predator::AttackWBetaIn)
      .def_readwrite("AttackWBetaOut", &Predator::AttackWBetaOut)
      .def_readwrite("CatchDistance", &Predator::CatchDistance)
      .def_readwrite("AttackSpan", &Predator::AttackSpan)
      .def_readwrite("Dogfight", &Predator::Dogfight)
      .def_readwrite("HoldLock", &Predator::HoldLock)
      .def_readwrite("LockBlindAngle", &Predator::LockBlindAngle)
      .def_readwrite("LockDistance", &Predator::LockDistance)
      .def_readwrite("maxLocks", &Predator::maxLocks)
      .def_readwrite("ExposureThreshold", &Predator::ExposureThreshold)
      .def_readwrite("AttractMix", &Predator::AttractMix)
      .def_readwrite("HandleTime", &Predator::HandleTime)
	  .def_readwrite("VisualError", &Predator::VisualError)
	  .def_readwrite("VisualBias", &Predator::VisualBias)
  ];
}

