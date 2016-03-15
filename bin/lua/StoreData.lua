local inspect = require "inspect2"
local DataStorage = {}

function DataStorage.DataStorage()
--store data of experiment parameters, assuming all prey have the same parameters and all predators have too.



   return function(expNum)
	   folder = experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .. "\\" 
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\"" )
	   os.execute( "mkdir " ..  "\"" .. experiments[expNum]['Param']['DataStorage']['folder'] .. "test\\" .. tostring(expNum) .."\"" )
	   predBird = birdToTable(experiments[expNum]['predBird'])
	   predBird.reactionTime = 600
	   new = Params.Bird()
	   print(new.reactionTime)
	   tableToBird(predBird,"new")
	   new.reactionTime = 7
	   print(new.reactionTime)
	   BirdtoTable2(predBird,"new", "predBird")
	   print(new.reactionTime)
	   print("predBird "  .. predBird.reactionTime)

	   new.reactionTime = 600
	   print(new.reactionTime)
	   Param = experiments[expNum]['Param']
		file = io.open (folder .. "test.txt", "w")
		tableToFile(Param,  "Clustering Ruler Trail RenderFlags FeatureMap", "Param", file)
		tableToFile(predBird, "", "predBird", file)
		file:close()
		print(predBird.birdName)
   end
end

return DataStorage