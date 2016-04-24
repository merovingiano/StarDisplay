
local evolution = {}



function evolution.evolve_next()

    return function(expNum)

	    begin = os.clock()
	    local Pred_params = {}
	    local Bird_params = {}
	    fitness = {}
	    counter = 0
	    local Generation = Simulation.Generation()
	    print("Generation = " .. Generation)

	    crit = experiments[expNum].Param.evolution.pred_fitness_criterion
	   
		for p in Simulation.Predators() do
			counter = counter + 1
			Pred_params[counter] = p.PredParams
			Bird_params[counter] = p.BirdParams
			Bird_params[counter].generation = Generation
			pred_stat = p:GetHuntStat()
			if crit ~= nil then
				loadstring("fitness[counter] = {" .. crit .. " , counter}")()
				if (string.find(tostring(fitness[counter][1]),"#IND")) then fitness[counter][1] = 999 end
			end
		end

		if (Generation == 1) then 
			initialize_evolving_parameters(Pred_params, "pred_params",expNum) 
			initialize_evolving_parameters(Bird_params, "predBird_params",expNum) 
		end
		if crit ~= nil then
			Pred_params, fitness = sort_by(Pred_params, fitness)
			Bird_params, fitness = sort_by(Bird_params, fitness)
		end
		copy_half_and_mutate(Pred_params, "pred_params", expNum)
		copy_half_and_mutate(Bird_params, "predBird_params", expNum)
		randomize_random_variables(Pred_params, "pred_params", expNum)
		randomize_random_variables(Bird_params, "predBird_params", expNum)
		print("RT pred= " .. Bird_params[1].reactionTime)


	    Prey_params = {}
	    Bird_params = {}
	    fitness = {}
        counter = 0
	    crit = experiments[expNum].Param.evolution.prey_fitness_criterion
	   
		for p in Simulation.Prey() do
			counter = counter + 1
			Prey_params[counter] = p.PreyParams
			Bird_params[counter] = p.BirdParams
				print("RT prey = " .. Bird_params[1].reactionTime)
			Bird_params[counter].generation = Generation
			prey_stat = p.BirdParams
			if crit ~= nil then
				loadstring("fitness[counter] = {" .. crit .. " , counter}")()
				if (string.find(tostring(fitness[counter][1]),"#IND")) then fitness[counter][1] = 999 end
			end
		end

		if (Generation == 1) then 
			initialize_evolving_parameters(Prey_params, "prey_params",expNum) 
			initialize_evolving_parameters(Bird_params, "preyBird_params",expNum) 
		end

		if crit ~= nil then
			Prey_params, fitness = sort_by(Prey_params, fitness)
			Bird_params, fitness = sort_by(Bird_params, fitness)
		end
		copy_half_and_mutate(Prey_params, "prey_params", expNum)
		copy_half_and_mutate(Bird_params, "preyBird_params", expNum)
		randomize_random_variables(Prey_params, "prey_params", expNum)
		randomize_random_variables(Bird_params, "preyBird_params", expNum)
		print("RT prey = " .. Bird_params[1].reactionTime)

	   
	    print("seconds: " .. os.clock() - begin)
	end

	--next generation
	--sort on fitness criterium
	-- duplicate first half to second half
	-- random mutation
	-- randomize random variables


end




return evolution