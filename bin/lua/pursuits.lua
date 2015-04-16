print("  sourcing pursuit.lua")


local pursuits = {}


-- don't forget to alter the PNSave guidance names!
pursuits.Custom = 0
pursuits._ProportionalNavigation = 1
pursuits._DirectPursuit = 2
pursuits._DirectPursuit2 = 3
pursuits._PNDP=4


function pursuits.DirectPursuit(Weight)
  local weight = Weight

  return function (pred, simTime, targetPos)
    pred.cohesion =  weight * glm.normalize(targetPos - pred.position)
  end

end


function pursuits.ProportionalNavigation(Weight)
  local weight = Weight
  local lastT = 0
  local lastForward = glm.vec3(1,0,0)
  local red = glm.vec3(1,0,0);
  local blue = glm.vec3(0,0,1);
  local line = Simulation.line
  local minDist = 1000
  return function (pred, simTime, targetPos, targetHeading, targetSpeed)
    local dt = simTime - lastT
    lastT = simTime
    local H = pred.H    
    local r = targetPos - pred.position
    local relV = pred.velocity - (targetHeading * targetSpeed)
	local wLOS = glm.cross(relV, r) / glm.dot(r,r);
   

	--Code Robin
	if predData ~= nil then
		local hunt = pred:GetHuntStat()
		local wF = math.acos(glm.dot(pred.velocity, lastForward)/(glm.length(pred.velocity) * glm.length(lastForward))) * (1/dt)
		lastForward = pred.velocity
		if wF < 100 then
			predData[pred.id]["MeasuredN"] = predData[pred.id]["MeasuredN"]  + wF/glm.length(wLOS) 
			predData[pred.id]["MeasuredNcount"] = predData[pred.id]["MeasuredNcount"] + 1
		end
		if hunt.minDist < minDist then
			minDist = hunt.minDist
			predData[pred.id]["minDist"] = minDist
			predData[pred.id]["catchVelocity"] = pred.velocity
		end
		
		pnew = glm.vec3(glm.dot(r,glm.column(H,0)),glm.dot(r,glm.column(H,1)),glm.dot(r,glm.column(H,2)))
		-- H IS COLUMNAR!!
		
		phi = math.atan(pnew.z,pnew.x)
		blind = (math.abs(phi) < 0.8 and pnew.x < 0)
		--print("catches " .. hunt.catches)
		if (blind and glm.dot(r,r)<10) or hunt.catches > 0 then
			print("catches " .. hunt.catches)
			RESET = true
			pred:EndHunt(false)
		end
		
	end
	--End code 
	
	
    local cohesion = weight * glm.cross(wLOS, pred.velocity)  --Robin Changed this from pred.forward to pred.velocity
    pred.cohesion = cohesion
	
  end
end


return pursuits

