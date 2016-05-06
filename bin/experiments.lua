
local pursuits = require "pursuits"
local Birds = require "Birds"
local inspect = require "inspect2"
require "helper_functions"

cBird, cPrey, preyBird, prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
cBird, cPred, predBird, pred = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Peregrine falcon", 1)






-- 2 reaction times
-- 4 species
-- 3 maneuvers

experiments = {}



local newExp = function (a)
    local starling_bird,  starling_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
    local dove_bird,  dove_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Rock dove", 0)
	local tired_dove_bird,  tired_dove_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Rock dove", 0)
	local chaffinch_bird,  chaffinch_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common chaffinch", 0)
    local robin_bird,  robin_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"European robin", 0)
    local peregrine_bird,  peregrine_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Peregrine falcon", 0)
	tired_dove_bird.wingBeatFreq = tired_dove_bird.wingBeatFreq / 2
	tired_dove_bird.birdName = "Tired_rock_dove" 
    local defaultPrey_bird, defaultPrey_prey = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
    local defaultPred_bird, defaultPred_pred = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Peregrine falcon", 1)
    local RTCounter = repeater(a, 1,2)
	local PreyCounter = repeater(a, 2,3)
	local maneuverCounter = repeater(a,6,4)

	local PreyCounter = repeater(a, 1,4)
	local maneuverCounter = repeater(a,4,3)

	local experiment = 
	{
		Param = deepcopy(gParam),
		preyBird = defaultPrey_bird,
		predBird = defaultPred_bird,
		prey = defaultPrey_prey,
		pred = defaultPred_pred,
	}
	experiment.pred.PursuitStrategy = 1
	experiment.Param.evolution.fileName =  "hoi"  .. ".txt"
	experiment.Param.evolution.title =  ""
	experiment.Param.evolution.description = ""
	experiment.Param.evolution.terminationGeneration = 300
	experiment.Param.evolution.durationGeneration = 40
	experiment.Param.evolution.evolveDPAdjParam = false

	--experiment.preyBird = starling_bird
   --experiment.preyBird = peregrine_bird 
   if (PreyCounter == 1) then experiment.preyBird = dove_bird  end
	if (PreyCounter == 2) then experiment.preyBird = tired_dove_bird  end
	if (PreyCounter == 3) then experiment.preyBird = starling_bird end
	if (PreyCounter == 4) then experiment.preyBird = chaffinch_bird end
	experiment.preyBird.maneuver = maneuverCounter
	
	experiment.predBird.reactionTime = 0.05
	experiment.Param.evolution.fileName = "no_evol_landscape_" .. experiment.preyBird.birdName .. "_" .. experiment.preyBird.maneuver .. "_" .. round2(experiment.predBird.reactionTime * 1000,0)
	experiment.pred['VisualError'] = 0

	print("experiment: " .. a)
	print("maneuver: " .. maneuverCounter)
	print("prey: " .. experiment.preyBird.birdName)

	return experiment


end


RT = {0.001,0.05}

thecounter = 0
for n = 3,11,1 do
	print(n)
	thecounter = thecounter + 1
    experiments[thecounter] = newExp(n)
end


