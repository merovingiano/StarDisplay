-- meta_config.lua
--
-- Config file to run several independent simulation runs

local gParam = require "config"
local VK = require "VK"
local Birds = require "Birds"
require "attacks"

local timeStamp = os.date("_%d-%m-%y_%H-%M-%S")
local DataFileName = Simulation.DataPath .. "EgbertStat" .. timeStamp .. ".dat"
print("Using meta_config.lua. Output to:")
print("  " .. DataFileName)

gParam.FeatureMap.current = FMappings.Custom
Simulation.CustomStatistic = require "egbert_statistic"

local Run = 1
local MaxRun = 40             -- must equal number parameter sets
local Repetition = 1
local MaxRepetition = 30      -- Repetitions per parameter set


local ParameterSets = {
  AttackStrategy = { 
    AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, 
    AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,
    AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest, AttackHighest,
    AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal,  AttackDorsal},
  wBetaInOutFactor = {
    1, 1, 1, 1, 1, 1, 1.25, 1.5, 0.75, 0.5,
    1, 1, 1, 1, 1, 1, 1.25, 1.5, 0.75, 0.5,
    1, 1, 1, 1, 1, 1, 1.25, 1.5, 0.75, 0.5,
    1, 1, 1, 1, 1, 1, 1.25, 1.5, 0.75, 0.5},
  numPrey = { 
    5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000,  
    5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000,  
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  predMaxSpeed = { 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1},
  predCruiseSpeed = { 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1, 
    1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1},
  EvasionStrategy= { 
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) }, --1
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) }, --10
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) },
    { type = EvasionStrategies.TurnInward, weight = 0.5, edges = glm.vec4(0, 0, 10, 15) }, --20
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)}, --30
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)} --40
  }
  
  -- add more (don't forget to adjust SetParameters())
}


local function SetParameters ()
  Simulation.CustomStatistic:setT1(0)
  Simulation.SetNumPrey(0)
  Simulation.SetNumPrey(ParameterSets.numPrey[Run])     -- numPrey
  Simulation.SetNumPredators(0)
  Simulation.SetNumPredators(1)
  for prey in Simulation.Prey() do
    local PreyB = prey.PreyParams()
    PreyB:SetEvasionStrategy(ParameterSets.EvasionStrategy[Run])
  end
  for pred in Simulation.Predators() do
    local PB = pred.BirdParams
    PB.wBetaIn = ParameterSets.wBetaInOutFactor[Run] * PB.wBetaIn
    PB.wBetaOut = ParameterSets.wBetaInOutFactor[Run] * PB.wBetaOut
    PB.cruiseSpeed = ParameterSets.predCruiseSpeed[Run] * PB.cruiseSpeed
	  PB.maxSpeed = ParameterSets.predMaxSpeed[Run] * PB.maxSpeed
    -- don't forget attack wBetaIn, wBetaOut
    local PP = pred.PredParams
    PP.AttackWBetaIn = ParameterSets.wBetaInOutFactor[Run] * PP.AttackWBetaIn
    PP.AttackWBetaOut = ParameterSets.wBetaInOutFactor[Run] * PP.AttackWBetaOut
  end
end


local function CustomTimeTickHook (simTime, paused)
  if paused then return end
  local eps = 0.0000001
  local first_attack = 30 --time flock has to fly around until the predator attacks

  if math.abs(simTime - first_attack)  < eps then
    print("Run: " .. Run, "Repetition: " .. Repetition, "First attack")
    Simulation.CustomStatistic:setT1(0)
    ParameterSets.AttackStrategy[Run]()
  elseif Simulation.CustomStatistic:Done() then
    Simulation.CustomStatistic:save(DataFileName, true, Run, Repetition)
    lastAttack = false
    Repetition = Repetition + 1
    if Repetition > MaxRepetition then
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
  TimeTickHook = CustomTimeTickHook   -- rehook
end


function ResetSimulation ()
  Simulation.RegisterFactories( PreyFactory(ParameterSets.numPrey[Run]), PredatorFactory() )
  SetParameters()
  Simulation.ResetCurrentStatistic()
  Simulation.EmulateKeyDown(VK.R, false, true, true)  -- Restart Simulation time 
end


