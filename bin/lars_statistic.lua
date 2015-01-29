print("  sourcing lars_statistic.lua")
-- Lars_statistic.lua
--
-- Predator custom statistic


local delim = " "             -- use "," for csv output
local NaN = "-1"              -- replacement for 1.#QNAN


local camera = nil;


local PredStatFormat = [[
\monoface{}
Sequences: %d  
-----------------------
Seq. time:      %.2f  
Locks:          %d  
Lock time:      %.2f  
Catches:        %d   
min. dist:      %.2f   
traveled:       %.2f    
climb:          %.2f    
descend:        %.2f   ]]


local LarsStatistic = {}


function LarsStatistic:Done()
end


function LarsStatistic:finalize()
end


function LarsStatistic:apply (stat_dt)
end


function LarsStatistic:display ()
--[[  
  if sequence > 0 then
    local hunt = hunts[sequence]
    tb:SetText(string.format(PredStatFormat, sequence, hunt.seqTime, hunt.locks, hunt.lockTime, hunt.catches, hunt.minDist, traveledDistance[sequence], climbedDistance[sequence] or 0, descendDistance[sequence] or 0))
  else
    tb:SetText(string.format(PredStatFormat, 0, 0, 0, 0, 0, 999.99, 0, 0, 0))
  end
  --]]
end


function LarsStatistic:reset ()
--[[
  camera = Simulation.GetActiveCamera()
  ext = Simulation.Text():extent(PredStatFormat)
  tb = Simulation.Text():CreateTextBox("PredStat", glm.vec4(-ext.x, 0, ext.x, ext.y), camera)
--]]
end


function LarsStatistic:save (fname, append, run, repetition)
end


return LarsStatistic    -- Return your custom statistic here



