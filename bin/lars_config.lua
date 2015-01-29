print("  sourcing lars_config.lua")
--
-- Config file to run several independent simulation runs
--

local gParam = require "config"
local VK = require "VK"
local Birds = require "Birds"

local Run = 1
local Repetition = 1
local MaxRun = 16              -- must equal number parameter sets
local MaxRepetition = 30      -- Repetitions per parameter set


local ParameterSets = {
  numPrey = { 2000, 500, 1000, 4000, 8000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 },
  reactionTime = {
    { mean = 0.1, sd = 0.01 },
    { mean = 0.1, sd = 0.01 },
    { mean = 0.1, sd = 0.01 },
  	{ mean = 0.1, sd = 0.01 },
    { mean = 0.1, sd = 0.01 },
    { mean = 0.01, sd = 0.01 },
	  { mean = 0.05, sd = 0.01 },
	  { mean = 0.15, sd = 0.01 },
	  { mean = 0.2, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
	  { mean = 0.1, sd = 0.01 },
  },
  IncurNeighborPanic = { 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 2, 3, 4, 13, 6.5, 6.5, 6.5 },
  topologicalRange = { 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 6.5, 13, 6.5, 6.5, 6.5 },
  separationRadius = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 1.9, 1.5 },
}


local function SetParameters ()
  Simulation.SetNumPrey(0)
  Simulation.SetNumPrey(ParameterSets.numPrey[Run])     -- numPrey
  -- set parameters for all prey
  local reactionTime = ParameterSets.reactionTime[Run]
  for prey in Simulation.Prey() do
  	local P = prey:GetBirdParams()
    local rtm = reactionTime.mean
    local rsd = reactionTime.sd
    local rmin = math.max(0, rtm-4*rts)
    local rmax = rtm+4*rts
	  P.reactionTime = random:normal_min_max(rtm, sd, rmin, rmax)
	  P.separationRadius = ParameterSets.separationRadius[Run]
	  P.topologicalRange = ParameterSets.topologicalRange[Run]
	  prey:SetBirdParams(P)
	  P = prey:GetPreyParams()
	  P.IncurNeighborPanic = ParameterSets.IncurNeighborPanic[Run]
	  prey:SetPreyParams(P)
  end
end


local function CustomTimeTickHook (simTime, paused)
  if paused then return end
  local eps = 0.0000001
  if math.abs(simTime - 30) < eps then          
    local roost = Simulation.GetRoost()
	  roost.Radius = 10000
	  Simulation.SetRoost(roost)
  elseif math.abs(simTime - 50) < eps then      
    io.write("Wave triggered for Run " .. Run .. ", Repetition " .. Repetition .. "\n")
    WaveTrigger(Run, Repetition)
  elseif math.abs(simTime - 70) < eps then      
    Repetition = Repetition + 1
    if Repetition > MaxRepetition then
      Run = Run + 1
      Repetition = 1
    end
	print("hoi---------------------------")
    if Run > MaxRun then
      Simulation.Quit()
    else
      ResetSimulation()
    end
  end
end


-- This is called only once at the very beginning.
-- Used to make sure that the simulation parameters
-- reflects those in ParameterSets for the first run.
function TimeTickHook ()
  ResetSimulation()
  ResetSimulation()
  Simulation.GetActiveCamera():SetMode("Television", false)
  TimeTickHook = CustomTimeTickHook   -- rehook
end


function ResetSimulation ()
  local roost = Simulation.GetRoost()
  roost.Radius = 200
  Simulation.SetRoost(roost)
  Simulation.RegisterFactories( PreyFactory(ParameterSets.numPrey[Run]), PredatorFactory() )
  SetParameters()
  Simulation.EmulateKeyDown(VK.R, false, true, true)    -- Restart Simulation time 
end


