local inspect = require "inspect2"
local DataStorage = {}

defaultPrey_bird_userdata, defaultPrey_prey_userdata, defaultPrey_bird_table, defaultPrey_prey_table = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 0)
defaultPred_bird_userdata, defaultPred_pred_userdata, defaultPred_bird_table, defaultPred_pred_table = Birds.newBird(nil, gParam.Birds.csv_file_species , gParam.Birds.csv_file_prey_predator_settings,"Common starling", 1)





function DataStorage.DataStorage()
--store data of experiment parameters, assuming all prey have the same parameters and all predators have too.


   return function(expNum)
	   folder = experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .. "\\" 
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\"" )
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .."\"" )
	   userDataPredator = experiments[expNum]['predBird']
	   predBird = deepcopy(defaultPred_bird_table)
	   print(predBird.reactionTime)
	   BirdtoTable2(predBird,"userDataPredator", "predBird")
	   print(predBird.reactionTime)
	   --predBird = birdToTable2(experiments[expNum]['predBird'])
	   predBird.reactionTime = 600
	   new = Params.Bird()
	   --print(new.reactionTime)
	   --print("type " .. tostring(experiments[expNum]['predBird']['PursuitStrategy']))
	   --print("type " .. tostring(predBird.PursuitStrategy))
	   --print("type " .. tostring(new.PursuitStrategy.type))
	   tableToBird(predBird,"new")
	   yo = new.PursuitStrategy
	  -- print("type " .. tostring(yo))
	   new.reactionTime = 7
	  -- print(new.reactionTime)
	   BirdtoTable2(predBird,"new", "predBird")
	   --print("type " .. tostring(predBird.PursuitStrategy))
	   --print(new.reactionTime)
	   --print("predBird "  .. predBird.reactionTime)

	   new.reactionTime = 600
	   --print(new.reactionTime)
	   Param = experiments[expNum]['Param']
		file = io.open (folder .. "test.txt", "w")
		tableToFile(Param,  "Clustering Ruler Trail RenderFlags FeatureMap", "Param", file)
		tableToFile(predBird, "", "predBird", file)
		file:close()
		--print(predBird.birdName)
   end
end

return DataStorage