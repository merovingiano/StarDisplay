print("  sourcing Rolf.lua")
local random = require "Random" 
local Now = Simulation.SimulationTime
local Neighbors = Simulation.Neighbors


local Rolf = {}

-- Fixed parameter
local alpha = 0.5					-- Weight for social facilitation
local beta = 1 						-- Weight for individuals flying upwards and downwards regarding social social facilitation
local gamma = 1						-- Weight for individuals flying staying regarding social social facilitation
local average_P = 0.2				-- Average roosting preference
local stdev_P = 0.01 				-- Standard deviation in roosting preference
local K = 0.01 				        -- Average increase of average_P per second [1/s]
local preferred_height_max = nil 	-- Maximal preferred height, will be initialised with bird.altitude [m]
local preferred_height_min = 40 	-- Minimal preferred height [m]
local gpws_lower_border = 5
local gpws_highest_high_border = 60 -- Ground avoidance for motivation = 0 [m]
local gpws_lowest_high_border = 40 	-- Ground avoidance for motivation = 1 [m]
local V_upwards_Threshold = 0.5 	-- relative speed difference threshold in the vertical direction for flying upwards [m/s]
local V_downwards_Threshold = -0.5 	-- relative speed difference threshold in the vertical direction for flying downwards [m/s]
local altitudeControl = 0.01       	-- Weight for reaching P_height
local maxlift = 1

 -- linear function relationship roosting preference and preferred height
 local LinearPheigth = function (P, height_range)
  return preferred_height_max - P * height_range
 end

 -- linear function relationship roosting preference and ground avoidance boundary altitude
 local LinearPGroundavoidance = function(P, groundavoid_range)
	return gpws_highest_high_border - P * groundavoid_range
 end
 
 -- step function function relationship roosting preference and preferred height
 local StepPheigth = function (P)
	if P > 0.5 then
	  return preferred_height_min
	else 
	  return preferred_height_max
	end
 end

 -- step function relationship roosting preference and ground avoidance boundary altitude
 local StepPGroundavoidance = function(P)
	if P > 0.5 then
	  return gpws_lowest_high_border
	else
	  return gpws_highest_high_border
	end
 end

 -- smooth step function function relationship roosting preference and preferred height
 local SmoothStepPheigth = function (P, height_range)
  return preferred_height_max - glm.smoothstep(0, 1, P) * height_range
 end
 
 -- smooth step function relationship roosting preference and ground avoidance boundary altitude
 local SmoothStepPGroundavoidance = function (P, groundavoid_range)
  return gpws_highest_high_border - glm.smoothstep(0, 1, P) * groundavoid_range
 end
 
 
 -- relationships used
 local CalculatePheight = LinearPheigth
 local CalculatePgroundavoidance = LinearPGroundavoidance
 
 print("hoi")
function Rolf.HelloWorld (altitude)
  
  preferred_height_max = altitude
  local height_range = preferred_height_max - preferred_height_min
  local groundavoid_range = gpws_highest_high_border - gpws_lowest_high_border
  local P = random:normal_min_max( average_P , stdev_P, 0, 1)
  local T0 = 0
 
  print("hoi")
  return function (bird)
    local T1 = Now()
    local du = T1 - T0
	T0 = T1
	
	
	-- loop over neighbors
	local Nup, Ndown, Nnotup, Nnotdown = 0, 0, 0, 0
	local vv = bird.velocity.y
	for ni in Neighbors(bird) do
	  print(ni)
	  nivv = ni.speed * ni.forward.y
	  if nivv - vv > V_upwards_Threshold then
	    Nup = Nup + 1
		else Nnotup = Nnotup +1
	  end
	  if nivv - vv < V_downwards_Threshold then
	    Ndown = Ndown + 1
		else Nnotdown = Nnotdown +1
	  end
	  if Nup + Nnotup > 7 then
	    break
	  end
	end
	
	-- increase roosting pref.
	P = math.min(P + K * du, 1)
	
	-- preferred height due to roosting preference calculation
	local P_height = CalculatePheight(P, height_range)
	-- social facilitation calculation
	local S = (Nup^beta/(Nnotup+Nup)^gamma) - (Ndown^beta/(Nnotdown+Ndown)^gamma)
	-- Final preferred height calculation
	local preferred_height = P_height * (1 + alpha * S)
	
	-- ground avoidance boundary altitude due to roosting preference calculation
	local P_groundavoidance = CalculatePgroundavoidance(P, groundavoid_range)
	-- Final boundary ground avoidance altitude calculation
	local groundavoidance = P_groundavoidance * (1 + alpha * S)
	
	
	local flyingheight = bird.position.y
    local pos = bird.position
    local vel = bird.velocity
	local plane = glm.vec4(0,-1,0,0)
	local time_to_impact = glm.intersectRayPlane(pos, vel, plane)
		
	---[=[																										-- Making ground avoidance behaviour solely dependant on altitude
	if flyingheight < gpws_lower_border then
		GroundAvoidSteering = 1
		elseif groundavoidance < flyingheight then
		GroundAvoidSteering = 0
	end	

	if groundavoidance > flyingheight then
		if flyingheight > gpws_lower_border then	
	    ---[[
		GroundAvoidSteering = 1 - (flyingheight - gpws_lower_border)/(groundavoidance - gpws_lower_border) 		-- Linear relationship between the amount of ground avoidance and the altitude
		--]]	
		--[[	
			if flyingheight < 0.5 * groundavoidance then 														-- Threshold relationship between the amount of ground avoidance and the altitude
			GroundAvoidSteering = 1
			else 
			GroundAvoidSteering = 0
			end
		--]]		
		--[[	
		GroundAvoidSteering = 1 - glm.smoothstep(gpws_lower_border, groundavoidance, flyingheight) 				-- Sigmoid relationship between the amount of ground avoidance and the altitude
		--]]	
		end
	end 
	--]=]
	
	
	
	--[=[																										-- Making ground avoidance behaviour dependant on impact time
	if time_to_impact < gpws_lower_border then
		if time_to_impact < 0 then
			GroundAvoidSteering = 0
			else GroundAvoidSteering = 1
		end 
		elseif groundavoidance < time_to_impact then
		GroundAvoidSteering = 0
	end	

	if groundavoidance > time_to_impact then
		if time_to_impact > gpws_lower_border then
	    ---[[
		GroundAvoidSteering = 1 - (time_to_impact - gpws_lower_border)/(groundavoidance - gpws_lower_border) 	-- Linear relationship between the amount of ground avoidance and the altitude
		--]]
		--[[	
			if time_to_impact < 0.5 * groundavoidance then 														-- Threshold relationship between the amount of ground avoidance and the altitude
			GroundAvoidSteering = 1
			else 
			GroundAvoidSteering = 0
			end
		--]]		
		--[[	
		GroundAvoidSteering = 1 - glm.smoothstep(gpws_lower_border, groundavoidance, time_to_impact) 			-- Sigmoid relationship between the amount of ground avoidance and the altitude
		--]]
		end
	end 
	--]=]
	
	if lagg == nil then 
	 lagg = 0
	end
	
	
    local F = bird.boundary
	F.y = math.min(F.y + altitudeControl * (preferred_height - bird.position.y) + maxlift * GroundAvoidSteering + lagg, 1)
	
    --[[
	F.y = F.y + maxlift * (preferred_height - bird.position.y) / preferred_height 
	--]]
	
	
	
	bird.boundary = F
	
	--[[																										-- Add lagg to the reactions of the birds in regards to their ground avoidance and preferred height behaviour
	local lagg = altitudeControl * (preferred_height - bird.position.y) + maxlift * GroundAvoidSteering
	--]]
	
	
	---[[
	if bird.id == 1 then
	  print(P, F.y)
    bird.ColorTex= 1.0
	end
	--]]
  end
  
	
	
end


return Rolf
