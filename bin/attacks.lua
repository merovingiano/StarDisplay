print("  sourcing attacks.lua")

-- Attack the highest prey in the flock
--
function AttackHighest ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
	for fm in Simulation.Flockmates(prey) do
	  if fm.position.y > prey.position.y then
		prey = fm
	  end
	end
	local trail = pred:HasTrail()
  pred:SetTrail(false)
	pred:SetTargetPrey(prey)
	local pos = prey.position - 1 * prey.forward + glm.vec3(0,50,0)
	pred.position = pos
	pred:SetTrail(trail)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
	pred:StartAttack()
	sim.ShowAnnotation("Attack highest started", 2)
end



-- Attack the prey most in the front of the flock
--
function AttackFrontal ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local Flock = Simulation.GetFlock()
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
	
	local id = prey:GetFlockId()
	local cluster = Flock:cluster(id)
	if cluster == nil then return false end
	local maxi = -10000
	for fm in Simulation.Flockmates(prey) do
		local current = glm.dot(fm.position - glm.center(cluster.bbox), glm.normalize(cluster.velocity))
		if current > maxi then
			maxi = current
			prey = fm
		end
	end
	
	local trail = pred:HasTrail()
    pred:SetTrail(false)
	pred:SetTargetPrey(prey)
	local pos = prey.position - 150 * prey.forward
	pred.position = pos
	pred:SetTrail(trail)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(0) }
	pred:StartAttack()
	sim.ShowAnnotation("Attack frontal started", 2)
end



-- Attack the prey most behind in the flock
--
function AttackDorsal ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local Flock = Simulation.GetFlock()
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
	
	local id = prey:GetFlockId()
	local cluster = Flock:cluster(id)
	if cluster == nil then return false end
	local mini = 1000000
	for fm in Simulation.Flockmates(prey) do
		local current = glm.dot(fm.position - glm.center(cluster.bbox), glm.normalize(cluster.velocity))
		if current < mini then
			mini = current
			prey = fm
		end
	end
	
	local trail = pred:HasTrail()
    pred:SetTrail(false)
	pred:SetTargetPrey(prey)
	local pos = prey.position - 50 * prey.forward
	pred.position = pos
  pred.forward = cluster.velocity
  pred:SetTrail(trail)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
	pred:StartAttack()
	sim.ShowAnnotation("Attack dorsal started", 2)
end

--Attack the prey most on the side of the flock
--
function AttackLateral ()
  local sim = Simulation
  local Camera = sim.GetActiveCamera()
  local pred = Camera:GetFocalPredator()
  local prey = Camera:GetFocalPrey()
  if pred and prey then
    if prey == nil then return end
    local side = glm.vec3(0)
    local center = glm.vec3(0)
    local n = 0
    for fm in Simulation.Flockmates(prey) do
      side = side + fm.side
      center = center + fm.position
      n = n + 1
    end
    side = glm.normalize(side)
    center = center / n
    local max = 0
    for fm in Simulation.Flockmates(prey) do
      local curr = glm.dot(fm.position - center, side)
      if math.abs(curr) > math.abs(max) then
        max = curr
        prey = fm
      end
    end
    local trail = pred:HasTrail()
    pred:SetTrail(false)
	  local pos = prey.position + 50 * (max > 0 and 1 or -1) * side
    pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
    pred.position = pos
    pred.forward = center - pos
    pred:SetTargetPrey(prey)
    pred:SetTrail(trail)
    pred:StartAttack()
    sim.ShowAnnotation("Attack lateral started", 2)
  end
end


--Attack the prey most in the center of the flock
--
function AttackCenter ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local Flock = Simulation.GetFlock()
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
	
	local id = prey:GetFlockId()
	local cluster = Flock:cluster(id)
	if cluster == nil then return false end
	local mini = 1000000
	for fm in Simulation.Flockmates(prey) do
		local current = glm.length(fm.position - glm.center(cluster.bbox))
		if current < mini then
			mini = current
			prey = fm
		end
	end
	
	local trail = pred:HasTrail()
  pred:SetTrail(false)
	pred:SetTargetPrey(prey)
	local pos = prey.position - 50 * prey.forward
	pred.position = pos
	pred:SetTrail(trail)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
	pred:StartAttack()
	sim.ShowAnnotation("Attack center started", 2)
end


-- Start attack for 'VideoAnalysis' 
--
function StartVideoAttack ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local trail = pred:HasTrail()
  pred:SetTrail(false)
	local pos = pred.position
  pos.y = pred.BirdParams.altitude
  pred.position = pos
	pred:SetTrail(trail)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
  Camera:SetMode("First person offset", true)
  Camera:flushLerp()
  local fm = sim.GetFeatureMap()
  fm.current = FMappings.PredatorVid
  sim.SetFeatureMap(fm)
	sim.ResetCurrentStatistic()
  KBH_ResetHuntStat()
  pred:StartAttack()
	sim.ShowAnnotation("Attack video started", 2)
end


-- Start attack from prefered altitude
function StartDefaultAttack ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
  local trail = pred:HasTrail()
  pred:SetTrail(false)
	local pos = pred.position;
  pos.y = pred.BirdParams.altitude;
  pred.position = pos
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
  pred:SetTrail(trail)
  pred:StartAttack()
  sim.ShowAnnotation("Default attack started", 2)
end


-- Attack the prey most behind in the flock
-- Egberts Version for repeated attack
-- Did not set predators position
--
function AttackDorsalRepeated ()
	local sim = Simulation
  local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	if pred == nil then return end
	local Flock = Simulation.GetFlock()
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
	
	local id = prey:GetFlockId()
	local cluster = Flock:cluster(id)
	if cluster == nil then return false end
	local mini = 1000000
	for fm in Simulation.Flockmates(prey) do
		local current = glm.dot(fm.position - glm.center(cluster.bbox), glm.normalize(cluster.velocity))
		if current < mini then
			mini = current
			prey = fm
		end
	end
  pred:SetTargetPrey(prey)
  pred.PredParams.PursuitStrategy = { type = PursuitStrategies.Deflection, deflection = glm.vec3(-0.25, 0.0, 0.0) }
	pred:StartAttack()
	sim.ShowAnnotation("Attack dorsal repeated started", 2)
end


-- Start attack from prefered altitude
function AttackHorizontally ()
	local sim = Simulation
	local Camera = sim.GetActiveCamera()
	local pred = Camera:GetFocalPredator()
	local prey = Camera:GetFocalPrey()
	if prey == nil then return end
  local posY = prey.position.y
	local trail = pred:HasTrail()
  pred:SetTrail(false)
	local pos = pred.position
  pos.y = posY
  pred.position = pos
	pred:SetTrail(trail)
	pred:StartAttack()
	sim.ShowAnnotation("Horizontal attack started", 2)
end



