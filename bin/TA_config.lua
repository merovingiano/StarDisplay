-- TA_config.lua
--

print("  sourcing config.lua")

local VK = require "VK"
local Birds = require "TA_Birds"
local Camera = require "Camera"
local gParam = require "Defaults"
require "wave"

dofile(Simulation.WorkingPath .. "attacks.lua")
dofile(Simulation.WorkingPath .. "evasion.lua")
 
-- Insert custom statistic
local TA = require "TA"
Simulation.CustomStatistic = TA.Statistic
gParam.FeatureMap.current = FMappings.Custom


-- Overwrite default initial parameter if required
gParam.Roost = {
  numPrey = 1,
  numPredators = 1,
  Radius = 150.0,
  minRadius = 10.0,
  maxRadius = 500.0,
}


function InitHook ()
  local sim = Simulation
--  random:seed(os.time())
  random:seed(96)
  sim.CheckVersion("6.3.0.0")

  -- Setup lead camera
  local camera = Camera:new()
  sim.SetActiveCamera(camera)
  camera:flushLerp()
  camera:Use()

  sim.SetInitialParameter(gParam)
  local win = sim.Window()
  win:SetTitle("StarDisplay V" .. sim.Version .. " " .. sim.ConfigFile)
  win:SetClientRect(glm.vec4(0,0,1024,720))
  win:ShowWindow("centered")
  camera:SetMode("Television", false)

  sim.RegisterFactories( PreyFactory(), PredatorFactory() )
  if sim.CustomStatistic ~= nil then
    sim.RegisterCustomStatistic( Statistic.new( sim.CustomStatistic ) )
  end

  -- Add user keyboard hooks
  sim.AddKeyboardHook(VK.RETURN, true, false, false, KBH_ReloadParameter)
  sim.AddKeyboardHook(VK["D"], true, false, false, Debug)
end


-- Opens the debug promt. Type cont to exit.
function Debug ()
  debug.debug()
end


function PreyFactory (Nexp)
  local flockPos = glm.vec3(Simulation.GetRoost().Radius * random:uniform01(),
                            Birds.Starling().altitude,
                            Simulation.GetRoost().Radius * random:uniform01())
  local N = Nexp or Simulation.GetRoost().numPrey

  return function (id, prey)
    if prey == nil then
      local position = glm.vec3(0, Birds.Starling().altitude, 0);
      local forward = glm.vec3(0,0,1)
      prey = Simulation.NewPrey(id, position, forward)
    end
    Birds.Starling(prey)
    return prey
  end
end


function PredatorFactory ()
  local firstPred = gParam.Roost.numPrey    -- id of first predator
  local numPred = gParam.Roost.numPredators            
  local distanceToPrey = 200

  return function (id, predator)
    local predId = id - firstPred
    if predator == nil then
      -- positioning of predators 
      local da, dh = 0, 0
      if predId > numPred * (2/3) then
        da = random:uniform(45, 80)          
      elseif predId > numPred * (1/3) then
        da = random:uniform(-15, 15)
      else
        da = random:uniform(-80, -45)
      end
      da = math.rad(da)
      local xzr = math.cos(da)
      local dh = math.rad(random:uniform(0, 360))
      local position = glm.vec3(xzr * math.sin(dh), math.sin(da), xzr * math.cos(dh))
      position = distanceToPrey * position
      position.y = position.y + Birds.Starling().altitude
      
      local head = random:unit_vec()
      head.y = 0
      head = glm.normalize(head)
      local forward = head
      predator = Simulation.NewPredator(id, position, forward)
    end
    Birds.Falcon(predator)
    predator:SetTrail(true)
    predator:SetTargetPrey(Simulation.GetActiveCamera():GetFocalPrey())
    predator:StartAttack()
    return predator
  end
end


function KBH_ReloadParameter ()
  local sim = Simulation
  dofile(Simulation.WorkingPath .. "attacks.lua")
  dofile(Simulation.WorkingPath .. "evasion.lua")
  package.loaded.gpws = nil;
  package.loaded.pursuits = nil;
  package.loaded.Birds = nil; Birds = require "TA_Birds"       -- reload Birds module
  package.loaded.Defaults = nil; Default = require "Defaults"  -- reload Defaults
  local current = sim.GetFeatureMap().current
  local newFeatureMap = Default.FeatureMap
  newFeatureMap.current = current
  sim.SetFeatureMap(newFeatureMap)
  for p in sim.Prey() do
    Birds.Starling(p)
  end
  for p in sim.Predators() do
    Birds.Falcon(p)
  end
  sim.ShowAnnotation("Parameter reloaded", 2)
end


return gParam
