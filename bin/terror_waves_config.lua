print("  sourcing lars_config.lua")
--
-- Config file to run several independent simulation runs
--

local gParam = require "config"
local VK = require "VK"
local Birds = require "Birds"

local Run = 14
local Repetition = 1
local MaxRun = 7 + 7                  -- must equal number parameter sets
local MaxRepetition = 10              -- Repetitions per parameter set


--[[ Flock size
local ParameterSets = {
  numPrey = { 
    500, 1000, 2000, 4000, 8000,
  },
  IncurNeighborPanic = { 
    6, 6, 6, 6, 6, 6, 6,      -- Density
  },
  IncurLatency = { 
    0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
  },
  separationRadius = { 
    2, 2, 2, 2, 2, 2,
  },
}
--]]


-- Density & Anticipation range 
local ParameterSets = {
  numPrey = { 
    2000, 2000, 2000, 2000, 2000, 2000, 2000,      -- Density
    2000, 2000, 2000, 2000, 2000, 2000, 2000,      -- Anticipation range
  },
  IncurNeighborPanic = { 
    6, 6, 6, 6, 6, 6, 6,      -- Density
    1, 2, 3, 4, 5, 6, 7,      -- Anticipation range
  },
  IncurLatency = { 
    0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05,
    0.05, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05,
  },
  separationRadius = { 
    1, 1+1/3, 1+2/3, 2, 2+1/3, 2+2/3, 3,      -- Density
    2, 2, 2, 2, 2, 2, 2,                      -- Anticipation range
  },
}


--[[ Incur Latency 
local ParameterSets = {
  numPrey = { 
    2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000,
  },
  IncurNeighborPanic = { 
    7, 7, 7, 7, 7, 7, 7, 7,
  },
  IncurLatency = { 
    0.001, 0.005, 0.01, 0.05, 0.01, 0.1, 0.15, 0.2, 
  },
  separationRadius = { 
    2, 2, 2, 2, 2, 2, 2, 2,
  },
}
--]]

local function SetParameters ()
  Simulation.SetNumPrey(0)
  Simulation.SetNumPrey(ParameterSets.numPrey[Run])     -- numPrey
  -- set parameters for all prey
  for prey in Simulation.Prey() do
	  local P = prey.BirdParams
	  P.separationStep = glm.vec2(0.4, ParameterSets.separationRadius[Run])
	  prey.BirdParams = P
	  P = prey.PreyParams
	  P.IncurNeighborPanic = ParameterSets.IncurNeighborPanic[Run]
	  P.IncurLatency = ParameterSets.IncurLatency[Run]
	  prey.PreyParams = P
  end
end


local function CustomTimeTickHook (simTime, paused)
  --if paused then return end
  local eps = 0.0000001
  if math.abs(simTime - 30) < eps then          
    local roost = Simulation.GetRoost()
	  roost.Radius = 10000
	  Simulation.SetRoost(roost)
    print(roost.Radius)
  elseif math.abs(simTime - 50) < eps then
    if Simulation.GetFlock():num_clusters() > 1 then
      print("Split")
      ResetSimulation()
      return
    end      
    io.write("Wave triggered for Run " .. Run .. ", Repetition " .. Repetition .. "\n")
    WaveTrigger(Run, Repetition, ParameterSets)
  elseif math.abs(simTime - 70) < eps then      
    Repetition = Repetition + 1
    WaveRepetitionSummary()
    if Repetition > MaxRepetition then
      WaveRunSummary()
      Run = Run + 1
      Repetition = 1
    end
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
  print("Reset simulation")
  local roost = Simulation.GetRoost()
  roost.Radius = 200
  Simulation.SetRoost(roost)
  Simulation.RegisterFactories( PreyFactory(ParameterSets.numPrey[Run]), PredatorFactory() )
  SetParameters()
  Simulation.EmulateKeyDown(VK.R, false, true, true)    -- Restart Simulation time 
end


