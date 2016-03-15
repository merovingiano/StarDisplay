print("  sourcing Birds.lua")
-- Birds.lua
--
-- A function that loads the birds excel sheets and chooses the appropriate bird settings for prey or predator

local random = require "Random"
local pursuits = require "pursuits"
local gpws = require "gpws"
require "helper_functions"


local Birds = {
}



function Birds.newBird (p, file,settingsFile, name, isPredator)
		  local bird = Params.Bird()
		  local file = io.open(file, "r")
		  local names = ParseCSVLine(file:read(),",")
		  local bird_info = {}
		  local tmp = {}
		  local prey_pred = 0
		  local bird_spec = 0
		  local line = ParseCSVLine(file:read(),",")
		  local predator = Params.Predator()
		  local prey = Params.Prey()
		  while line[2] ~= "" do  
			if(line[2] == name) then
				tmp = line
				for k,v in pairs(tmp) do		  
				  bird_info[names[k]] = tmp[k]
				end
			end
			line = ParseCSVLine(file:read(),",")
		  end
		  io.close(file)
		  local keyset={}
		  local n=0
		  local settings = io.open(settingsFile, "r")
		  line = ParseCSVLine(settings:read(),",")
		  while line[1] ~= "end" do  
			if line[1] == "Prey" then prey_pred = 1 end
			if line[1] == "Predator" then prey_pred = 2 end
			if line[1] == "Bird Properties" then bird_spec = 1 end
			if line[1] == "Prey Properties" then bird_spec = 2 end
			if line[1] == "Predator Properties" then bird_spec = 3 end
			if isPredator == 1 and prey_pred == 2  and bird_spec == 1 and line[1] ~= "Predator Properties" and line[1] ~= "" then
			  bird[line[1]] = process_CSV_cell(line[2], line[3])
			end
			if isPredator == 1 and prey_pred == 2  and bird_spec == 3 and line[1] ~= "Bird Properties" and line[1] ~= "" then
			  predator[line[1]] = process_CSV_cell(line[2], line[3])
			end
			if isPredator == 0 and prey_pred == 1  and bird_spec == 1 and line[1] ~= "Bird Properties" and line[1] ~= "" then
			  bird[line[1]] = process_CSV_cell(line[2], line[3])
			end
			if isPredator == 0 and prey_pred == 1  and bird_spec == 2 and line[1] ~= "Prey Properties" and line[1] ~= "" then
			  if  line[1] ~= "EvasionStrategy" then 
				prey[line[1]] = process_CSV_cell(line[2], line[3]) 
			  end
			end
			line = ParseCSVLine(settings:read(),",")
		  end
		  io.close(settings)
		  --prey.EvasionStrategy = { type = EvasionStrategies.None, weight = 1.0, edges = glm.vec4(0, 2, 2, 2) }

		  --used: physical
		  bird.rho = 1.2
		  bird.bodyMass = tonumber(bird_info["body mass male"]) / 1000     -- [kg]
		  bird.wingMass = tonumber(bird_info["wing mass male"])   
		  bird.InertiaWing = tonumber(bird_info["moment of inertia extended wing male"])
		  bird.InertiaBody = tonumber(bird_info["moment of inertia body male"])
		  bird.J = tonumber(bird_info["J wing male"])
		  bird.wingSpan = tonumber(bird_info["Wingspan male"] / 100  )      -- [m]
		  bird.wingAspectRatio = tonumber(bird_info["Aspect ratio male"])
		  bird.wingBeatFreq = tonumber(bird_info["Wingbeat frequency"])
		  bird.birdName = bird_info["Species(eng)"]
		  bird.theta =  tonumber(bird_info["Span Angle Down to Upstroke"])
		  bird.wingLength = tonumber(bird_info["Wing length male"]) / 100
		  bird.bodyArea = tonumber(bird_info["Body area male"] )
		  bird.cBody =  tonumber(bird_info["Body drag coefficient"] )
		  bird.cFriction = tonumber(bird_info["Friction drag wing"])
		  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2] 
		  bird.cruiseSpeed = tonumber(bird_info["Cruise speed male"])

		  -- currently unused, to be deleted or may we useful later
		  bird.CL = CL(bird)
		  bird.CDCL= CDCL(bird)
		  bird.controlCL = false
		  bird.maneuver = 1
		  bird.maxLift = 5   
		  bird.minSpeed = 1
		  bird.maxSpeed = 40
		  bird.maxForce = 3       -- max steering force [N]
		  bird.rollRate= 5  -- has become redundent. Roll rate is now calculated with roll acceleration and inertia

		  -- currently unused, to be deleted or may we useful later
		  bird.wBetaIn = glm.vec3( 4, 1, 0 )    -- roll, pitch, yaw
		  bird.wBetaOut = glm.vec3( 0, 0, 0 )     -- roll, pitch, yaw 
		  prey.AlertedWBetaIn = bird.wBetaIn
		  prey.AlertedWBetaOut = bird.wBetaOut

		  if isPredator == 0 then
			 if p ~= nil then
				 p.BirdParams = bird
				 p.PreyParams = prey
			 end
			 return bird, prey
		  else
			 if p ~= nil then
				 p.BirdParams = bird
				 p.PredParams = predator
			 end
			 return bird, predator
		  end
end

function Birds.getMorphology(birdy)
--This function extracts the relevent  morphological data from an userData to a lua table
		  bird = {}
		  bird.bodyMass =birdy.bodyMass 
		  bird.wingMass =birdy.wingMass
		  bird.InertiaWing = birdy.InertiaWing
		  bird.InertiaBody =  birdy.InertiaBody
		  bird.J =birdy.J
		  bird.wingSpan = birdy.wingSpan     -- [m]
		  bird.wingAspectRatio =birdy.wingAspectRatio 
		  bird.wingBeatFreq = birdy.wingBeatFreq
		  bird.birdName = tostring (birdy.birdName)
		  bird.theta =  birdy.theta
		  bird.wingLength = birdy.wingLength
		  bird.bodyArea = birdy.bodyArea
		  bird.cBody =  birdy.cBody
		  bird.cFriction = birdy.cFriction 
		  bird.wingArea = birdy.wingArea -- [m^2] 
		  bird.cruiseSpeed = birdy.cruiseSpeed
		  return bird
end


return Birds

