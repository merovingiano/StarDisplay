--loading necessary files
 VK = require "VK"
 Birds = require "Birds"
 Camera = require "Camera"
 gParam = require "Defaults"
 storeData = require "StoreData"
require "wave"
dofile(Simulation.WorkingPath .. "attacks.lua")
dofile(Simulation.WorkingPath .. "evasion.lua")
dofile(Simulation.WorkingPath .. "experiments.lua")


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

  folder = sim.exeFolder
  --automatically convert the excel sheet to csv
  os.execute( gParam.Birds.csv_convertor  .. " "  .. folder .. gParam.Birds.csv_file_species_xlsm .. " " .. folder .. "lua\\bird_properties.csv")

  local win = sim.Window()
  win:SetTitle("StarDisplay V" .. sim.Version .. " " .. sim.ConfigFile)
  win:SetClientRect(glm.vec4(0,0,1024,720))
  win:ShowWindow("centered")
  camera:SetMode("Television", false)

  sim.RegisterFactories( PreyFactory(), PredatorFactory() )

  --set data storage (just a test for now)
  sim.RegisterDataStorage(storeData.DataStorage())
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
	prey:SetTrail(true)
    Birds.newBird(prey, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)

	
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
    Birds.newBird(predator, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Peregrine falcon", 1)
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


