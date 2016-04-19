local inspect = require "inspect2"
local DataStorage = {}

defaultPrey_bird_userdata, defaultPrey_prey_userdata, defaultPrey_bird_table, defaultPrey_prey_table = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
defaultPred_bird_userdata, defaultPred_pred_userdata, defaultPred_bird_table, defaultPred_pred_table = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 1)





function DataStorage.DataStorage()
--store data of experiment parameters, assuming all prey have the same parameters and all predators have too.
   return function(expNum)
   print("\n expNUM " .. expNum)
	   folder = experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .. "\\" 
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\"" )
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .."\"" )
	   exp = experimentToTable(expNum)
	   file = io.open (folder .. "settings.txt", "w")
	   tableToFile(exp,  "Clustering Ruler Trail RenderFlags FeatureMap", "", file)
	   file:close()
	   file = io.open (folder .. "evolutionn.txt", "a")
	  -- evolutionToFile(exp,  "Clustering Ruler Trail RenderFlags FeatureMap", "", file)
	   file:close()
   end
end

return DataStorage