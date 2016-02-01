print("  sourcing config.lua")

local VK = require "VK"
local Birds = require "Birds"
local Camera = require "Camera"
local gParam = require "Defaults"


require "wave"

dofile(Simulation.WorkingPath .. "attacks.lua")
dofile(Simulation.WorkingPath .. "evasion.lua")
dofile(Simulation.WorkingPath .. "experiments.lua")

-- Insert custom statistic

doExperiments = 1
-- Overwrite default initial parameter if required
gParam.Roost = {
  numPrey = 1,
  numPredators = 1000,
  Radius = 500.0,
  minRadius = 150.0,
  maxRadius = 10000.0,
}
gParam.evolution.type = "PN"
gParam.evolution.fileName = "PNmutation.txt"
--gParam.evolution.type = "noEvol"
gParam.evolution.TrajectoryBestPredator = false
gParam.evolution.title = "PN new mutation test" 
gParam.evolution.durationGeneration = 100
gParam.evolution.load = false
gParam.evolution.loadFolder = "D:/ownCloud/2013-2014/phd hunting/dataStarDisplay/continue folder/"
gParam.evolution.description = "I've added a new extra mutation, allowing it to vary more. I want to see the long term effects of this. "

function InitHook ()
  local sim = Simulation
  random:seed(os.time())
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
  sim.AddKeyboardHook(VK["D"], true, false, false, KBH_Debug)

  
end


-- Opens the debug promt. Type cont to exit.
function KBH_Debug ()
  debug.debug()
end


function ResetFlockPositioning ()
	Init_Roost = gParam.Roost
	Init_flockPos = glm.vec3(0,
								120,
								0) 
	Init_flockForward = glm.normalize(glm.vec3(1,0,0))
end

function flock_placement()
	return Init_flockPos + random:vec_in_sphere() * ( 1.5 * math.pow(Init_Roost.numPrey, 1/3))
end

PreyFactory = PreyFactory or function ()
  ResetFlockPositioning ()

  return function (id, prey)
    if prey == nil then
      local position = Init_flockPos
      position.y = 120
      prey = Simulation.NewPrey(id, position, Init_flockForward)
    end
    Birds.Starling(prey)
    return prey
  end
end


PredatorFactory = PredatorFactory or function ()
  local Roost = gParam.Roost

  return function (id, predator)
    if predator == nil then
      local position = random:vec_in_sphere() 
	  position = position / glm.length(position) * 100
      position.y = position.y  + 120 --math.abs(position.y)
      local forward = random:vec_in_sphere() 
	  forward = forward / glm.length(forward)
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
  package.loaded.Birds = nil; Birds = require "Birds"              -- reload Birds module
  package.loaded.Defaults = nil; Default = require "Defaults"      -- reload Defaults
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


function TimeTickHook ()
    if doExperiments == 1 and gParam.evolution.load == false then
		Simulation.GetExperimentSettings(experiments)
		doExperiments = 0
	end
end














return gParam
