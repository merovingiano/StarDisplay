
local DataStorage = {}

function DataStorage.DataStorage()
--store data of experiment parameters, assuming all prey have the same parameters and all predators have too.

local sim = Simulation
  for p in sim.Prey() do
    Birds.Starling(p)
  end

   return function()
       print("test")
	   
   end
end

return DataStorage