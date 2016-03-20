local random = require "Random"
local pursuits = require "pursuits"
local gpws = require "gpws"



--________________________________________________________________________________________________________________________________________________
function shallowcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' or  orig_type ==  'userData' then
        copy = {}
        for orig_key, orig_value in pairs(orig) do
            copy[orig_key] = orig_value
        end
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end
--________________________________________________________________________________________________________________________________________________



--________________________________________________________________________________________________________________________________________________
function deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end
--________________________________________________________________________________________________________________________________________________



--________________________________________________________________________________________________________________________________________________
function tableToFile(orig, skip, string, file)
    local orig_type = type(orig)
    if orig_type == 'table' then
        for orig_key, orig_value in next, orig, nil do
		    --print(orig_key)
			newstring = string .. "." .. orig_key
            if not string.match(skip, orig_key) then tableToFile(orig_value, skip, newstring, file ) end
        end
    else -- number, string, boolean, etc 
		if tostring(orig_type) ~= 'nil' then 
		--print( tostring(type(orig)) " " .. tostring(string) .. " " .. tostring(orig) .. "\n")
			file:write( tostring(type(orig)) .. " " .. tostring(string) .. " " .. tostring(orig) .. "\n")
		end
    end
end
--________________________________________________________________________________________________________________________________________________



--________________________________________________________________________________________________________________________________________________
function tableToUserData(orig, bird)
    local orig_type = type(orig)
    if orig_type == 'table' then
        for orig_key, orig_value in next, orig, nil do
			newstring = bird .. "." .. orig_key
			tableToUserData(orig_value, newstring )
        end
    else -- number, string, boolean, etc 
		if tostring(orig_type) ~= 'nil' then 		    
			if tostring(orig_type) == 'string' then orig = "\"" .. orig .. "\"" else orig = tostring(orig) end
			--print(tostring(bird) .. " =  " .. orig)
			loadstring(  tostring(bird) .. " =  " .. orig)()
		end
    end
end
--________________________________________________________________________________________________________________________________________________



--________________________________________________________________________________________________________________________________________________
function userDatatoTable(output_table, bird, tableName)
    local orig_type = type(output_table)
    if orig_type == 'table' then
        for orig_key, orig_value in next, output_table, nil do
		    --print(orig_key)
			newbird = bird .. "." .. orig_key
			newtable = tableName .. "." .. orig_key
            userDatatoTable(orig_value, newbird, newtable)
        end
    else -- number, string, boolean, etc 
		if tostring(orig_type) ~= 'nil' then 
		--print(  tostring(tableName) .. " =  " .. tostring(bird))
			loadstring(  tostring(tableName) .. " =  " .. tostring(bird))()
		end
    end
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
function experimentToTable(expNum)
    exp_table = {}
	for key, value in pairs(experiments[expNum]) do
	   if type(experiments[expNum][key]) == 'table' then exp_table[key] = deepcopy(experiment[key]) end
	end
	userData =experiments[expNum]['predBird']
	userDatatoTable(predBird,"userData", "predBird")
	userData =experiments[expNum]['preyBird']
	userDatatoTable(preyBird,"userData", "preyBird")
	userData =experiments[expNum]['pred']
	userDatatoTable(pred,"userData", "pred")
	userData =experiments[expNum]['prey']
	userDatatoTable(prey,"userData", "prey")
	 exp_table.preyBird = deepcopy(preyBird) 
	 exp_table.prey = deepcopy(prey)
	 exp_table.predBird = deepcopy(predBird) 
	 exp_table.pred = deepcopy(pred)	 
	 return exp_table
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
repeater = function(n, reps, counts)
  return ((n - (n % reps)) / reps ) % counts + 1
end
--________________________________________________________________________________________________________________________________________________





--________________________________________________________________________________________________________________________________________________
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
	  if counter == 2 then return {x = list[1], y = list[2]} end
	  if counter == 3 then return {x = list[1],y = list[2],z = list[3]} end
   end
   if type == "string" then
      return tostring(cell)
   end
   return tonumber(cell)
end
--________________________________________________________________________________________________________________________________________________





--________________________________________________________________________________________________________________________________________________
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
--________________________________________________________________________________________________________________________________________________





-- CL after Van Dijk (1964)
--
--________________________________________________________________________________________________________________________________________________
CL = function (bird, alpha)
  alpha = alpha or 0.28
  local AR = bird.wingAspectRatio
  local piAR = math.pi * AR
  local CL = 2*math.pi * alpha /
             (1 + 2/AR + 16*(math.log(piAR) - 9/8) / (piAR * piAR))
  return CL
end
--________________________________________________________________________________________________________________________________________________




-- Allometric scaling with wing load
-- Alerstam et al PLOS Biol 5, 2007
--________________________________________________________________________________________________________________________________________________
CruiseSpeed = function (bird)
  local wingLoad = bird.bodyMass * 9.81 / bird.wingArea
  local U =  4.8 * math.pow(wingLoad, 0.28)
  return U
end
--________________________________________________________________________________________________________________________________________________





--drag to lift ratio
--________________________________________________________________________________________________________________________________________________
CDCL = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird))
	return cdcl
end
--________________________________________________________________________________________________________________________________________________


--________________________________________________________________________________________________________________________________________________
CDCL2 = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird,1))
	return cdcl
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
maxForce = function (bird)
	local maxForce = (bird.maxSpeed^2 / bird.cruiseSpeed^2) * bird.bodyMass * 9.81 * (CDCL(bird) + bird.bodyDrag/CL(bird)) -- removed:  - CDCL(bird)* bird.bodyMass * 9.81 
	return maxForce
end 
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
skipHemisphere = function (P)
  if random:uniform01() < P then
    return 0, 1
  else
    return 1, 0
  end
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
function script_path() 
    -- remember to strip off the starting @ 
    return debug.getinfo(2, "S").source:sub(2) 
end 
--________________________________________________________________________________________________________________________________________________