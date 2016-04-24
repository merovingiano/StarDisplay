
local evolution = {}



function evolution.evolve_next()

    return function(expNum)

	   begin = os.clock()
	   local Pred_params = {}
	   local Bird_params = {}
	   fitness = {}
	   local counter = 0
	   local index = {}
	   local Generation = Simulation.Generation()
	   print("Generation = " .. Generation)

	   crit = experiments[expNum].Param.evolution.pred_fitness_criterion
	   if crit ~= nil then
		   for p in Simulation.Predators() do
				counter = counter + 1
				index = counter
				Pred_params[counter] = p.PredParams
				Bird_params[counter] = p.BirdParams
				Bird_params[counter].generation = Generation
				pred_stat = p:GetHuntStat()
				fitness[counter] = {pred_stat.minDist , counter}
				loadstring("fitness[counter] = {" .. crit .. " , counter}")()
				print(fitness[counter][1] )
				print(Pred_params[counter].N )
				print("AR " .. Bird_params[counter].wingAspectRatio)
				if (string.find(tostring(fitness[counter][1]),"#IND")) then fitness[counter][1] = 999 end
		   end

		   if (Generation == 1) then initialize_evolving_parameters(Pred_params,Bird_params,expNum) end
		   print(Bird_params[1].reactionTime)
		   Pred_params, Bird_params, fitness = sort_by(Pred_params, Bird_params, fitness)
		   print(Bird_params[1].reactionTime)
		   copy_half_and_mutate(Pred_params, Bird_params, expNum)
		   --randomize_random_variables(Pred_params, expNum)
	   end

	   print("seconds: " .. os.clock() - begin)
	end

	--next generation
	--sort on fitness criterium
	-- duplicate first half to second half
	-- random mutation
	-- randomize random variables


end




return evolution