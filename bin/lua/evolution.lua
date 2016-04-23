
local evolution = {}

function evolution.evolve_next()

    return function()
	   print("test yo")
	   --while 1==1 do
	   --end
	   local Alleles = {}
	   local fitness = {}
	   local counter = 0
	   local index = {}
	   for p in Simulation.Predators() do
	        counter = counter + 1
			index = counter
			Alleles[counter] = p.PredParams
			pred_stat = p:GetHuntStat()
			fitness[counter] = {pred_stat.minDist , counter}
	
	   end

	   table.sort(fitness,function (k1, k2) return k1[1] < k2[1] end )
	   
	   local sorted_Alleles = {}
       for i,n in ipairs(fitness) do
	    sorted_Alleles[i] = Alleles[n[2]]
		print(sorted_Alleles[i].InitialPosition.x)
		end

	   
	end

	--next generation
	--sort on fitness criterium
	-- duplicate first half to second half
	-- random mutation
	-- randomize random variables


end




return evolution