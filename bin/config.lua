print("  sourcing config.lua")
--dofile(Simulation.WorkingPath .. "lua/config_functions.lua")
require "config_functions"
--____________________________________________________________________settings___________________________________________________________________
--experiment or free flying?
doExperiments = 1

-- Overwrite default initial parameter if required
gParam.Roost.numPrey = 1
gParam.Roost.numPredators = 2
gParam.Roost.Radius = 500.0
gParam.Roost.minRadius = 150.0
gParam.Roost.maxRadius = 10000.0
gParam.evolution.type = "PN"
gParam.evolution.fileName = "PNmutation.txt"
--gParam.evolution.type = "noEvol"
gParam.evolution.TrajectoryBestPredator = false
gParam.evolution.title = "PN new mutation test" 
gParam.evolution.durationGeneration = 1
gParam.evolution.load = false
gParam.evolution.loadFolder = "D:/ownCloud/2013-2014/phd hunting/dataStarDisplay/continue folder/"
gParam.evolution.description = "Here, I compare the evolution of 4 species, 3 movements, and 2 RTs"














return gParam
