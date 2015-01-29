print("  sourcing evasion.lua")


local bor = bit.bor


function DropEx (upW, upTD)
	local weight = upW or 2
	local trigDist = upTD or 5

  -- Called with state combinations as follows:
  --   prey.panicOnset < 0 and pred ~= 0    : no panic reaction yet but predator detected
  --   prey.panicOnset >= 0 and pred ~= nil : ongoing panic reaction, predator detected
	--   prey.panicOnset >= 0 and pred == nil : ongoing panic reaction, no predator
	--
  return function (prey, pred)
    local now = Simulation.SimulationTime()
    local t0 = prey.panicOnset
    if t0 < 0 then
      if prey.predatorDist > trigDist then 
        return 
      end
      t0, prey.panicOnset = now, now    -- new individual panic reaction
    end

    local reaction = bor(prey.predatorReaction, PredationReaction.Alerted, PredationReaction.Panic)
    local dt = now - t0
		local evasion = glm.vec3(0, 0, 0)
		if dt < 0.5 then
			evasion = -prey.up
		elseif dt < 1 then
      prey.predatorReaction = reaction
      return
    else
      return  -- done, don't set prey.predatorReaction
  	end

    prey.predatorReaction = bor(reaction, PredationReaction.Detectable)
    prey.predatorForce = prey.predatorForce + (weight * evasion)
	end
end

--]]

function MoveCenteredEx (upW, upTD)
	local weight = upW or 1
	local trigDist = upTD or 10
	local Flock = Simulation.GetFlock()
	
  -- Called with state combinations as follows:
  --   prey.panicOnset < 0 and pred ~= 0    : no panic reaction yet but predator detected
  --   prey.panicOnset >= 0 and pred ~= nil : ongoing panic reaction, predator detected
	--   prey.panicOnset >= 0 and pred == nil : ongoing panic reaction, no predator
	--
	return function (prey, pred)
    local id = prey:GetFlockId()
		local cluster = Flock:cluster(id)
		if cluster == nil then return false end

    local now = Simulation.SimulationTime()
    local t0 = prey.panicOnset
    if t0 < 0 then
      if prey.predatorDist > trigDist then 
        return 
      end
      t0, prey.panicOnset = now, now    -- new individual panic reaction
    end

    local reaction = bor(prey.predatorReaction, PredationReaction.Alerted, PredationReaction.Panic)
    local dt = now - t0
    local evasion = glm.vec3(0,0,0)
		if dt < 0.1 then
		  local f = Flock:cluster(prey:GetFlockId())
      if f ~= nil then
        evasion = glm.normalize(glm.center(f.bbox) - prey.position)
      end
		elseif dt < 1 then
      prey.predatorReaction = reaction
      return
    else
      return  -- done, don't set prey.predatorReaction
  	end

    prey.predatorReaction = bor(reaction, PredationReaction.Detectable)
    prey.predatorForce = prey.predatorForce + (weight * evasion)
	end
end



function TurnInwardEx (upW, upTD)
	local weight = upW or 1
	local trigDist = upTD or 10
	
  -- Called with state combinations as follows:
  --   prey.panicOnset < 0 and pred ~= 0    : no panic reaction yet but predator detected
  --   prey.panicOnset >= 0 and pred ~= nil : ongoing panic reaction, predator detected
	--   prey.panicOnset >= 0 and pred == nil : ongoing panic reaction, no predator
	--
	return function (prey, pred)
    local now = Simulation.SimulationTime()
    local t0 = prey.panicOnset
    if t0 < 0 then
      if prey.predatorDist > trigDist then 
        return 
      end
      t0, prey.panicOnset = now, now    -- new individual panic reaction
    end

    local reaction = bor(prey.predatorReaction, PredationReaction.Alerted, PredationReaction.Panic)
    local dt = now - t0
    local evasion = glm.vec3(0,0,0)
		if dt < 0.1 then
      evasion = prey.circularityVec / prey.circularity
		elseif dt < 1 then
      prey.predatorReaction = reaction
      return
    else
      return  -- done, don't set prey.predatorReaction
  	end

    prey.predatorReaction = bor(reaction, PredationReaction.Detectable)
    prey.predatorForce = prey.predatorForce + (weight * evasion)
	end
end


-- Custom Evasion Strategy Closure
--
function Zig (upW, upTD)
	local weight = upW or 1
	local trigDist = upTD or 10

  -- Called with state combinations as follows:
  --   prey.panicOnset < 0 and pred ~= 0    : no panic reaction yet but predator detected
  --   prey.panicOnset >= 0 and pred ~= nil : ongoing panic reaction, predator detected
	--   prey.panicOnset >= 0 and pred == nil : ongoing panic reaction, no predator
	--
  return function (prey, pred)
    local now = Simulation.SimulationTime()
    local t0 = prey.panicOnset
    if t0 < 0 then
      if prey.predatorDist > trigDist then 
        return 
      end
      t0, prey.panicOnset = now, now    -- new individual panic reaction
    end

    local reaction = bor(prey.predatorReaction, PredationReaction.Alerted, PredationReaction.Panic)
    local dt = now - t0
		local evasion = 0
    if dt < 0.25 then
			evasion = -1
    elseif dt < 0.55 then
			evasion = 1
		elseif dt < 2 then
      prey.predatorReaction = reaction
      return
    else
      return  -- done, don't set prey.predatorReaction
  	end
    prey.predatorReaction = bor(reaction, PredationReaction.Detectable)
    prey.predatorForce = (weight * evasion) * prey.side
	end
end


function FeintEx (upW, upTD)
	local weight = upW or 1
	local trigDist = upTD or 5
	
  -- Called with state combinations as follows:
  --   prey.panicOnset < 0 and pred ~= 0    : no panic reaction yet but predator detected
  --   prey.panicOnset >= 0 and pred ~= nil : ongoing panic reaction, predator detected
	--   prey.panicOnset >= 0 and pred == nil : ongoing panic reaction, no predator
	--
	return function (prey, pred)
    local now = Simulation.SimulationTime()
    local t0 = prey.panicOnset
    if t0 < 0 then
      if prey.predatorDist > trigDist then 
        return 
      end
      t0, prey.panicOnset = now, now    -- new individual panic reaction
    end

    local reaction = bor(prey.predatorReaction, PredationReaction.Alerted, PredationReaction.Panic)
    local dt = now - t0
		local evasion = glm.vec3(0, 0, 0)
		if dt < 0.3 then
			evasion = prey.side * -2
		elseif dt < 0.6 then
			evasion = prey.up * -2
		elseif dt < 0.8 then
			evasion = prey.side * 1.2
		elseif dt < 1.0 then
			evasion = prey.up * 1.2
		elseif dt < 1.2 then
			evasion = prey.forward * 2
		elseif dt < 2 then
      prey.predatorReaction = reaction
      return
    else
      return  -- done, don't set prey.predatorReaction
  	end

    prey.predatorReaction = bor(reaction, PredationReaction.Detectable)
    prey.predatorForce = prey.predatorForce + evasion
  end
end

 