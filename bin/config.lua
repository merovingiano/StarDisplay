print("  sourcing config.lua")

local VK = require "VK"
local Birds = require "Birds"
local Camera = require "Camera"
local gParam = require "Defaults"


require "wave"

dofile(Simulation.WorkingPath .. "attacks.lua")
dofile(Simulation.WorkingPath .. "evasion.lua")

-- Insert custom statistic
Simulation.CustomStatistic = require "RobinStat"

-- Overwrite default initial parameter if required
gParam.Roost = {
  numPrey = 1,
  numPredators = 20,
  Radius = 500.0,
  minRadius = 150.0,
  maxRadius = 10000.0,
}

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
	if predData ~= nil then
		predData[predator.id] = {["position"] = predator.position}
		predData[predator.id]["MeasuredN"] = 0
		predData[predator.id]["MeasuredNcount"] = 0
		predData[predator.id]["preyStrat"] = preyStrat
	end
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

--[[
-- Quick angular acceleration measurment
--

local mddb = 0
local db0 = 0
local T0 = 0

function TimeTickHook ()
  local T = Simulation.SimulationTime()
  local dt = T - T0
  T0 = T
  for p in Simulation.Prey() do
    local db = math.rad( p:visit(FMappings.RollRate) )
    local ddb = (db0 - db) / dt
    mddb = math.max( mddb , math.abs(ddb) )
    db0 = db
    print(mddb)
    break
  end
end
--]]
function TimeTickHook ()
	ResetSimulation()
	Simulation.EmulateKeyDown(VK.R, false, true, true)
	print("Run: " .. run)
	TimeTickHook = CustomTimeTickHook   -- rehook
end


function CustomTimeTickHook ()

	

  --if RESET == true or Simulation.SimulationTime() > 60 then 
  --  RESET = false
	--if run == 100 then 
	--	Simulation.CustomStatistic:save()
	--end
	--ResetSimulation()
	--Simulation.EmulateKeyDown(VK.R, false, true, true)
	--print("Run: " .. run)
 -- end
  --for p in Simulation.Predators() do
	--	for prey in Simulation.Prey() do
	--		dist = glm.length(p.position - prey.position)
			--if dist < 1 then  print(glm.length(p.position - prey.position)) end
	--	end
	--p:StartAttack()
	--local hunt = p:GetHuntStat()
  --end
  
  local roost = Simulation.GetVaderJacob()

  
end


function ResetSimulation ()
	run = run + 1 
	Simulation.RegisterFactories( PreyFactory(), PredatorFactory() )
	SetParameters()
	
	Simulation.ResetCurrentStatistic()
	
  --parametersetting = setting    
	--ResetFlockPositioning ()    
    --Simulation.SetNumPrey(0)      --remove the existing prey
	--Simulation.SetNumPrey(1)   --introduce the new prey
    
	--Simulation.EmulateKeyDown(VK.R, false, true, true)  -- Restart Simulation time 
	--Simulation.Quit()
end


function SetParameters ()
  
  --Simulation.SetNumPrey(0)
  --Simulation.SetNumPrey(100)     -- numPrey
 
  --Simulation.SetNumPredators(0)
  --Simulation.SetNumPredators(0)
  for prey in Simulation.Prey() do
	prey.position = glm.vec3(0,
								120,
								0) 
	Init_flockForward = glm.normalize(glm.vec3(1,0,0))
	
	if run < 3 then
		preyStrat = "none"
	else
	    --Simulation.SetNumPrey(0)
		
		--Simulation.SetNumPrey(1) 
		prey.EvasionStrategy = { type = EvasionStrategies.Drop, weight = 5.0, edges = glm.vec4(0, 2, 2, 2) }
		preyStrat = "drop"
	end
  end
	  local position = random:vec_in_sphere() 
	  position = position / glm.length(position) * 100
      position.y = position.y  + 120 --math.abs(position.y)
	  local forward = random:vec_in_sphere() 
  --pred = Simulation.NewPredator(run+1, position, forward)
  --Simulation.SetNumPredators(run)
  
  
  for pred in Simulation.Predators() do
	  --print("test")
	  --print(run)
	  --print(pred.position)
	  --print("pred id " .. pred.id)
	  Birds.Falcon(pred)
		pred:SetTrail(true)
		pred:SetTargetPrey(Simulation.GetActiveCamera():GetFocalPrey())
		pred:StartAttack()
		--print (pred:is_attacking())
		if predData ~= nil then
			predData[pred.id] = {["position"] = pred.position}
			predData[pred.id]["MeasuredN"] = 0
			predData[pred.id]["MeasuredNcount"] = 0
			predData[pred.id]["preyStrat"] = preyStrat
		end
		local PB = pred.BirdParams
		--print(ParameterSets.Wr[1] .. " " .. ParameterSets.Wp[1])
		PB.wBetaIn = glm.vec3( ParameterSets.Wr[1], ParameterSets.Wp[1], 0 )
		-- don't forget attack wBetaIn, wBetaOut
		local PP = pred.PredParams
		PP.AttackWBetaIn = PB.wBetaIn
  end
end

ParameterSets = {Wr = {}, Wp = {}, RT = {}}
Wr = {5,2,3,4,5}
Wp = {1,2,3,4,5}
RT = {0.01, 0.02, 0.05}
print(Wr[2])
local counter = 0
for x,wr in pairs(Wr) do

	for y,wp in pairs(Wp) do
		for z,rt in pairs(RT) do
			counter = counter + 1
			ParameterSets.Wr[counter] = wr
			ParameterSets.Wp[counter] = wp
			ParameterSets.RT[counter] = rt
		end
	end
end



return gParam
