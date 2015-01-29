#ifndef GLOBALS_HPP_INCLUDED
#define GLOBALS_HPP_INCLUDED

#include "libStarDisplay.hpp"
#include "Simulation.hpp"
#include "GLWin.hpp"


extern LuaStarDisplay Lua;
extern GLWin AppWindow;
extern Simulation Sim;


#define PARAMS Sim.Params()
#define PROOST Sim.Params().roost
#define PSKYBOX Sim.Params().skybox
#define PFM Sim.Params().featureMap
#define PFME(E) PFM.Entries[(Param::FeatureMap::##E)]
#define PCFME PFM.Entries[PFM.current]
#define PRENDERFLAGS Sim.Params().renderFlags


#define GGl Sim.gl()
#define GTEXT GGl.Fonts
#define GCAMERA Sim.ccamera()      // const
#define GFLOCK Sim.cflock()        // const
#define GFLOCKNC Sim.flock()       // non const
#define GTRAILS Sim.trails()


#endif
