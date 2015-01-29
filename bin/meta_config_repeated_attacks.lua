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
local MaxRun = 10             -- must equal number parameter sets
local Repetition = 1
local MaxRepetition = 2      -- Repetitions per parameter set


local ParameterSets = {
  --AttackStrategy = { AttackHighest, AttackHighest,AttackHighest,AttackDorsal, AttackDorsal,AttackDorsal }, -- <-- this is set in CustomTimeTickHook()
  wBetaInOutFactor = { 1, 1, 1, 1, 1, 1, 1.25, 1.5, 0.75, 0.5},
  numPrey = { 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000 },
  predMaxSpeed = { 1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1 },
  predCruiseSpeed = { 1, 1.25, 1.5, 0.75, 0.5, 1, 1, 1, 1, 1 },
  EvasionStrategy= { 
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)},
    { type = EvasionStrategies.LeftRight, weight = 1, edges = glm.vec4(10, 0.25, 0.55, 3.0)}
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
    local PreyB = prey.PreyParams
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


local lastAttack = false;
local attack_number = 1
local attack_timer = false
local start_time = 9999999999999

local function CustomTimeTickHook (simTime, paused) 
  if paused then return end
  local eps = 0.0000001
  local first_attack = 5 --time flock has to fly around until the predator attacks
  local next_attack = 5 --time between [end of previous attack] and [start of new one]
  local number_of_attacks = 5
  local pred = Simulation.GetActiveCamera():GetFocalPredator()
  
  -- set timer between attacks to allow a bit of time between them
  if(not pred:is_attacking() and attack_timer==false and attack_number > 1) then
    start_time = simTime
    attack_timer=true
  end
  
  -- do attacks
  if (math.abs(simTime - first_attack)  < eps and  attack_number==1) then
    print("Run: " .. Run, "Repetition: " .. Repetition, "First attack")
    pred:SelectPreySelection(PreySelections.PickedTopo)
    AttackDorsal() --first attack, predator will be placed at designated position (attacks.lua)
    attack_number = attack_number + 1  
  elseif (not pred:is_attacking() and simTime > (start_time+next_attack)) then
    if (attack_number==2) then
      print("Run: " .. Run, "Repetition: " .. Repetition, "Second attack")
      pred:SelectPreySelection(PreySelections.Auto)
      pred:StartAttack ()
    elseif (attack_number==3) then
      print("Run: " .. Run, "Repetition: " .. Repetition, "Third attack")
      pred:StartAttack ()
    elseif (attack_number==4) then
      print("Run: " .. Run, "Repetition: " .. Repetition, "Fourth attack")
      pred:StartAttack ()
    elseif (attack_number==5) then
      print("Run: " .. Run, "Repetition: " .. Repetition, "Fifth attack")
      pred:StartAttack ()
      lastAttack = true --used to move on to next iteration
    end
    attack_timer = false
    attack_number = attack_number + 1
  elseif not pred:is_attacking() and lastAttack and Simulation.CustomStatistic:Done() then
    Simulation.CustomStatistic:save(DataFileName, true, Run, Repetition)
    lastAttack = false
    attack_number=1
    Repetition = Repetition + 1
    if Repetition > MaxRepetition then
      Run = Run + 1
      Repetition = 1
    end
    if Run > MaxRun then
      Simulation.Quit()
    else
      print("Starting next run")
      ResetSimulation()
    end
    attack_timer = false
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




  -- if (math.abs(simTime - first_attack)  < eps and  attack_number=1) then
    -- print("Run: " .. Run, "Repetition: " .. Repetition, "First attack")
    -- pred:SelectPreySelection(PreySelections.PickedTopo)
	-- AttackDorsal()
  -- elseif math.abs(simTime - (first_attack+2*next_attack)) < eps then
    -- print("Run: " .. Run, "Repetition: " .. Repetition, "Second attack")
    -- AttackDorsalRepeated()
  -- elseif math.abs(simTime - (first_attack+3*next_attack)) < eps then
    -- print("Run: " .. Run, "Repetition: " .. Repetition, "Third attack")
	  -- AttackDorsalRepeated()
  -- elseif math.abs(simTime - (first_attack+4*next_attack)) < eps then
    -- print("Run: " .. Run, "Repetition: " .. Repetition, "Fourth attack")
	  -- AttackDorsalRepeated()
  -- elseif math.abs(simTime - (first_attack+5*next_attack)) < eps then
    -- print("Run: " .. Run, "Repetition: " .. Repetition, "Fifth attack")
    -- Simulation.CustomStatistic:setT1(5)
	  -- AttackDorsalRepeated()
    -- lastAttack = true
