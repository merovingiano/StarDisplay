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


function sort_by(unsorted_table, by_var)
table.sort(by_var,function (k1, k2) return k1[1] < k2[1] end )   
	   local sorted_table = {}
       for i,n in ipairs(by_var) do
			sorted_table[i] = unsorted_table[n[2]]
		end
		return sorted_table,by_var
end


function copy_half_and_mutate(the_table,bird_prey_or_pred, expNum)
	n = table.getn(the_table)
	global_table_copy = the_table
	for loop = 1,(0.5*n),1 do
	    for key,val in pairs(experiments[expNum].Param.evolution.evolving_parameters) do
			if string.find(val.name, bird_prey_or_pred) then
				local variab = string.gsub(val.name, bird_prey_or_pred, "")
				mutation = 0
				loadstring("mutation = random:" .. val.type .. "(" .. val.a .. "," .. val.b .. ")")()
				if (val.scale == true) then loadstring("mutation = mutation * global_table_copy[" .. loop .. "]" ..  variab)() end
				loadstring("global_table_copy[" .. loop + 0.5*n .. "]" ..  variab .. " = " .. "global_table_copy[" .. loop .. "]" ..  variab .. " + mutation")()
			end
	    end
	end
end


function randomize_random_variables(the_table, bird_prey_or_pred, expNum)
   n = table.getn(the_table)
   global_table_copy = the_table
	for loop = 1,(n),1 do
	    for key,val in pairs(experiments[expNum].Param.evolution.random_variables) do
			if string.find(val.name, bird_prey_or_pred) then
				local variab = string.gsub(val.name, bird_prey_or_pred, "")
			    if val.a ~= nil then loadstring("global_table_copy[" .. loop .. "]" ..  variab .. " =  random:" .. val.type  .. "(" .. val.a .. "," .. val.b .. ")")() end
				if val.a == nil then loadstring("global_table_copy[" .. loop .. "]" ..  variab .. " =  random:" .. val.type  .. "()")() end
			end
	    end
	end
end


function initialize_evolving_parameters(the_table, bird_prey_or_pred, expNum)
   n = table.getn(the_table)
   global_table_copy = the_table
	for loop = 1,(n),1 do
	    for key,val in pairs(experiments[expNum].Param.evolution.evolving_parameters) do
			if string.find(val.name, bird_prey_or_pred) then
				local variab = string.gsub(val.name, bird_prey_or_pred, "")
				loadstring("global_table_copy[" .. loop .. "]" ..  variab .. " =  random:uniform(" .. val.initial.min .. "," .. val.initial.max .. ")")()
			end
	    end
	end
end

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
		--print( tostring(type(orig)) .. " " .. tostring(string) .. " " .. tostring(orig) .. "\n")
			file:write( tostring(type(orig)) .. " " .. tostring(string) .. " " .. tostring(orig) .. "\n")
		end
    end
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
function evolutionToFile(experiment, generation, file)
    -- get all pred param.
	if generation == 0 then
		for num,table in pairs(experiment.Param.evolution.evolving_parameters) do
		   file:write(table.name .. " ")
		end
		for num,table in pairs(experiment.Param.evolution.to_be_saved) do
		   if table['template'] == nil then
		         file:write(table[1] .. " ")
		   else
		         loadstring(" global_ts = {}")()
			     loadstring(" TEMPLATE = " .. table['template'] )()
				 loadstring(" trajectoryTableOutput = " .. table['template'] )()
				 row_table_to_file(trajectoryTableOutput, generation, "",file)
				 print(" boeren")
           end
		end
		for num,table in pairs(experiment.Param.evolution.random_variables) do
		   file:write(table.name .. " ")
		end
		file:write("\n")
		return
	end
	for p in Simulation.Predators() do
	       pred_stat = p:GetHuntStat()
		   predator = p
	       pred_params = p.PredParams
		   predBird_params = p.BirdParams
		   theprey = p:GetTargetPrey()
	       prey_params = theprey.PreyParams
		   preyBird_params = theprey.BirdParams
	       for num,table in pairs(experiment.Param.evolution.evolving_parameters) do
		       var_name = table.name
			   loadstring("val_var = " .. var_name )()
			   file:write(tostring(val_var) .. " ")
		   end
		   for num,table in pairs(experiment.Param.evolution.to_be_saved) do
		       
		       var_name = table[1]
			   if table['template'] == nil then
					loadstring("val_var = " .. var_name )()
					file:write(tostring(val_var) .. " ")
			   else
			     loadstring(" global_ts = "  .. table[1])()
			     loadstring(" TEMPLATE = " .. table['template'] )()
				 loadstring(" trajectoryTableOutput = " .. table['template'] )()
				 userDatatoTable(TEMPLATE, "global_ts", "trajectoryTableOutput")
				 row_table_to_file(trajectoryTableOutput, generation, "",file)
			  end
		   end
		   for num,table in pairs(experiment.Param.evolution.random_variables) do
		       var_name = table.name
			   loadstring("val_var = " .. var_name )()
			   file:write(tostring(val_var) .. " ")
		   end
		   file:write("\n")
	end
end
--________________________________________________________________________________________________________________________________________________

function TrajectoryToFile(experiment, generation, file)


  if (generation == 0) then
		        global_ts = {}
				trajectoryTableLayout = TrajectoryTable()
				trajectoryTableOutput = TrajectoryTable()
			    row_table_to_file(trajectoryTableOutput, generation, "",file)
			    file:write("\n")
				print("poep")
  else
	  for p in Simulation.Predators() do
			   for ts in Simulation.getTrajectory(p) do
					global_ts = ts
					trajectoryTableLayout = TrajectoryTable()
					trajectoryTableOutput = TrajectoryTable()
					userDatatoTable(trajectoryTableLayout, "global_ts", "trajectoryTableOutput")
					row_table_to_file(trajectoryTableOutput, generation, "",file)
					file:write("\n")
			   end
	   end
  end
end



function tableToGlm(glm, table)
   if table.z == nil then
      loadstring(  tostring(glm) .. " =  glm.vec2(" .. tostring(table.x) .. "," .. tostring(table.y) ..")" )()
   else
		loadstring(  tostring(glm) .. " =  glm.vec3(" .. tostring(table.x) .. "," .. tostring(table.y) .. "," .. tostring(table.z) ..")" )()
   end
end

function row_table_to_file(orig, generation, string, file)
    local orig_type = type(orig)
    if orig_type == 'table' then
        for orig_key, orig_value in next, orig, nil do
			newstring = string .. "." .. orig_key
            row_table_to_file(orig_value, generation, newstring, file )
        end
    else 
		if tostring(orig_type) ~= 'nil' then 
		    if (generation == 0) then file:write(tostring(string) .. " " ) end
			if (generation > 0) then file:write(tostring(orig) .. " " ) end
		end
    end

end

--________________________________________________________________________________________________________________________________________________
function tableToUserData(orig, userdata)
    local orig_type = type(orig)
    if orig_type == 'table' then
        for orig_key, orig_value in next, orig, nil do
				newstring = userdata .. "." .. orig_key
				tableToUserData(orig_value, newstring )
        end
    else -- number, string, boolean, etc 
		if tostring(orig_type) ~= 'nil' then 		    
			if tostring(orig_type) == 'string' then orig = "\"" .. orig .. "\"" else orig = tostring(orig) end
			--print(tostring(userdata) .. " =  " .. orig)
			loadstring(  tostring(userdata) .. " =  " .. orig)()
		end
    end
end
--________________________________________________________________________________________________________________________________________________


function tableToUserDataTEMP(orig, bird)
    local orig_type = type(orig)
    if orig_type == 'table' then
        for orig_key, orig_value in next, orig, nil do
		    if orig_key ~= "x" and orig_key ~= "y" and orig_key ~= "z" then  
				newstring = bird .. "." .. orig_key
				tableToUserData(orig_value, newstring )
			end
			if orig_key == "x" then
			   tableToGlm(bird, orig)
			end
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
function userDatatoTable(output_table, userdata, tableName)
    local orig_type = type(output_table)
    if orig_type == 'table' then
        for orig_key, orig_value in next, output_table, nil do
		    --print(orig_key)
			newuserdata = userdata .. "." .. orig_key
			newtable = tableName .. "." .. orig_key
            userDatatoTable(orig_value, newuserdata, newtable)
        end
    else -- number, string, boolean, etc 
		if tostring(orig_type) ~= 'nil' then 
		--print(  tostring(tableName) .. " =  " .. tostring(userdata))
			loadstring(  tostring(tableName) .. " =  " .. tostring(userdata))()
		end
    end
end
--________________________________________________________________________________________________________________________________________________




--________________________________________________________________________________________________________________________________________________
function experimentToTable(expNum)
    exp_table = {}
	for key, value in pairs(experiments[expNum]) do
	   if type(experiments[expNum][key]) == 'table' then exp_table[key] = deepcopy(experiments[expNum][key]) end
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




function directory_exists( sPath )
  if type( sPath ) ~= "string" then return false end

  local response = os.execute( "cd " .. sPath )
  if response == 0 then
    return true
  end
  return false
end




--________________________________________________________________________________________________________________________________________________
function script_path() 
    -- remember to strip off the starting @ 
    return debug.getinfo(2, "S").source:sub(2) 
end 
--________________________________________________________________________________________________________________________________________________