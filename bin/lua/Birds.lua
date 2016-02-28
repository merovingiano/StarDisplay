print("  sourcing Birds.lua")
-- Birds.lua
--
-- A collection of bird parameterizations

local random = require "Random"
local pursuits = require "pursuits"
local gpws = require "gpws"




local rho = 1.2               -- air density [kg/m^3]


-- CL after Van Dijk (1964)
--
local CL = function (bird, alpha)
  alpha = alpha or 0.28
  local AR = bird.wingAspectRatio
  local piAR = math.pi * AR
  local CL = 2*math.pi * alpha /
             (1 + 2/AR + 16*(math.log(piAR) - 9/8) / (piAR * piAR))
  return CL
end


function process_CSV_cell(cell, type)
   sep = ';'

   if type == "list" then
	   list = {}
	   for word in string.gmatch(cell, '([^;]+)') do
	      for k, v in string.gmatch(cell, "(%w+) = (%w+)") do
		     list[k] = tonumber(v)
	      end
	   end
      return list
   end

   if type == "boolean" then
      local value = tonumber(cell)
	   if value == 0 then return false else return true end
   end

   if type == "glm2" or  type == "glm3"  then
      list = {}
	  counter = 0
      for word in string.gmatch(cell, '([^;]+)') do
	     counter = counter + 1
		 list[counter] = tonumber(word)
	  end
	  if counter == 2 then return glm.vec2(list[1],list[2]) end
	  if counter == 3 then return glm.vec3(list[1],list[2], list[3]) end
   end

   --print(cell)
   --print(tonumber(cell))
   return tonumber(cell)

end



function ParseCSVLine (line,sep) 
	local res = {}
	local pos = 1
	sep = sep or ','
	while true do 
		local c = string.sub(line,pos,pos)
		if (c == "") then break end
		if (c == '"') then
			-- quoted value (ignore separator within)
			local txt = ""
			repeat
				local startp,endp = string.find(line,'^%b""',pos)
				txt = txt..string.sub(line,startp+1,endp-1)
				pos = endp + 1
				c = string.sub(line,pos,pos) 
				if (c == '"') then txt = txt..'"' end 
				-- check first char AFTER quoted string, if it is another
				-- quoted string without separator, then append it
				-- this is the way to "escape" the quote char in a quote. example:
				--   value1,"blub""blip""boing",value3  will result in blub"blip"boing  for the middle
			until (c ~= '"')
			table.insert(res,txt)
			assert(c == sep or c == "")
			pos = pos + 1
		else	
			-- no quotes used, just look for the first separator
			local startp,endp = string.find(line,sep,pos)
			if (startp) then 
				table.insert(res,string.sub(line,pos,startp-1))
				pos = endp + 1
			else
				-- no separator found -> use rest of string and terminate
				table.insert(res,string.sub(line,pos))
				break
			end 
		end
	end
	return res
end


-- Allometric scaling with wing load
-- Alerstam et al PLOS Biol 5, 2007
--
local CruiseSpeed = function (bird)
  local wingLoad = bird.bodyMass * 9.81 / bird.wingArea
  local U =  4.8 * math.pow(wingLoad, 0.28)
  return U
end

--drag to lift ratio

local CDCL = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird))
	return cdcl
end

local CDCL2 = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird,1))
	return cdcl
end

local maxForce = function (bird)
	local maxForce = (bird.maxSpeed^2 / bird.cruiseSpeed^2) * bird.bodyMass * 9.81 * (CDCL(bird) + bird.bodyDrag/CL(bird)) -- removed:  - CDCL(bird)* bird.bodyMass * 9.81 
	return maxForce
end 


local skipHemisphere = function (P)
  if random:uniform01() < P then
    return 0, 1
  else
    return 1, 0
  end
end

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
  bird.rho = rho
  bird.bodyMass = tonumber(bird_info["Mass male"] / 1000)       -- [kg]
  bird.InertiaWing = tonumber(bird_info["moment of inertia male"])
  bird.wingSpan = tonumber(bird_info["Wingspan male"] / 100  )      -- [m]
  bird.wingAspectRatio = tonumber(bird_info["Aspect ratio male"])
  bird.wingBeatFreq = tonumber(bird_info["Wingbeat frequency"])
  bird.theta =  tonumber(bird_info["Span Angle Down to Upstroke"])
  bird.wingLength = tonumber(bird_info["Wing length male"]) / 100
  bird.bodyArea = tonumber(bird_info["Body area"] )
  bird.cBody =  tonumber(bird_info["Body drag coefficient"] )
  bird.cFriction = tonumber(bird_info["Friction drag wing"])
  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2] 
  bird.rollRate= tonumber(bird_info["Roll rate"])   -- Important: needs physical theory!

  -- currently unused, to be deleted or may we useful later
  bird.CL = CL(bird)
  bird.CDCL= CDCL(bird)
  bird.controlCL = false
  bird.wingRetractionSpeed = 59
  bird.maxLift = 5   
  bird.cruiseSpeed = 20
  bird.minSpeed = 1
  bird.maxSpeed = 40
  bird.maxForce = 3       -- max steering force [N]



  -- currently unused, to be deleted or may we useful later
  bird.wBetaIn = glm.vec3( 4, 1, 0 )    -- roll, pitch, yaw
  bird.wBetaOut = glm.vec3( 0, 0, 0 )     -- roll, pitch, yaw 
  prey.AlertedWBetaIn = bird.wBetaIn
  prey.AlertedWBetaOut = bird.wBetaOut

--  print(bird.bodyArea)
 -- print(bird.maxSpeed)
 -- print(bird.wingSpan)
 -- print(prey.IncurLatency)
 --print(prey.DetectCruising)
 -- print(prey.AlertedAlignmentWeight)
 -- print(bird.bodyMass)
 -- print(bird.theta)
 -- print(bird.speedControl)
  --while 1 == 1 do

 --end

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


return Birds

