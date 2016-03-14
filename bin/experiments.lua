
local pursuits = require "pursuits"
local Birds = require "Birds"
local inspect = require "inspect2"
local gParam = require "Defaults"
--You only need to change those variables that deviate from the first experiment

starling_bird, starling_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
dove_bird, dove_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Rock dove", 0)
robin_bird, robin_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"European robin", 0)
peregrine_bird, peregrine_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Peregrine falcon", 0)

print(inspect(getmetatable(starling_bird)))
print(inspect(starling_bird))
print(type(starling_bird.bodyMass))

for k,v in pairs(starling_bird) do
  print(k)
end
while 1 ==1 do 

end

-- 2 reaction times
-- 4 species
-- 3 maneuvers

experiments = {}



male =
{


}


female =
{


}

local getBirdData = function (birdy)
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

function deepcopy(orig)
    local orig_type = type(orig)
	print(orig_type)
    local copy
    if orig_type == 'table' or orig_type == 'userdata' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[deepcopy(orig_key)] = deepcopy(orig_value)
			print( orig_key .. " = " .. orig_value)
        end
        setmetatable(copy, deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

repeater = function(n, reps, counts)
  return ((n - (n % reps)) / reps ) % counts + 1
end



local newExp = function (a)
			experiment = 
	{
		Param = 
		{
			evolution = 
			{
						fileName =  "hoi"  .. ".txt",
						TrajectoryBestPredator = false,
						title =  "",
						description = "",
						terminationGeneration = 50,
						durationGeneration = 25,
						evolveDPAdjParam = false,
			},
			roost = 
			{
			 numPrey = 1,
			 numPredators =2000,
			},
		},
		preyBird =
		{

		},
		predBird = 
		{
		},
		prey = 
		{
		},
		pred = 
		{
		   VisualError = 0,
		   VisualBias = glm.vec2(0,0)
		},
	}
	experiment.pred.PursuitStrategy = { type = pursuits._ProportionalNavigation, hook = pursuits.ProportionalNavigation(5) }

	experiment.predBird = shallowcopy(male) 
	

	RTCounter = repeater(a, 1,2)
	PreyCounter = repeater(a, 2,4)
	maneuverCounter = repeater(a,8,3)
    if (PreyCounter == 1) then experiment.preyBird = getBirdData(robin_bird) end
	if (PreyCounter == 2) then experiment.preyBird = getBirdData(dove_bird)  end
	if (PreyCounter == 3) then experiment.preyBird = getBirdData(starling_bird) end
	if (PreyCounter == 4) then experiment.preyBird = getBirdData(peregrine_bird) end
	experiment.preyBird.maneuver = maneuverCounter
	experiment.predBird.reactionTime = RT[RTCounter]
	experiment.Param.evolution.fileName = "n" .. (a + 1) .. "_" .. experiment.preyBird.birdName .. "_RT_" ..   RT[RTCounter] .. "_man_" ..   maneuverCounter .. ".txt"
	experiment.pred['VisualError'] = 0

	return experiment
end


RT = {0.001,0.05}

counter = 0
for n = 20,(30),1 do
print(n)
counter = counter + 1
    experiments[counter] = newExp(n)


end


--test
--tralala