-- egbert_statistic.lua
--
-- Predator custom statistic

print("egbert_statistic.lua")

local delim = " "             -- use "," for csv output

local camera = nil
local ext = nil
local tb = nil

local T0 = nil  --local T0 = nil           -- start time, T0 == nil after attack + T1 seconds
local T1 = 0
local traveledDistance = {}
local climbedDistance = {}
local decendDistance = {}
local flocksT0 = {}           -- number of flocks when attack started
local flocksT1 = {}           -- number of flocks after T1 seconds after the attack
local hunts = {}
local sequence = 0
local done = false


local PredStatFormat = [[
\monoface{}
Sequences: %d  
-------------------------
Seq. time:        %.2f  
Locks:            %d  
Lock time:        %.2f  
Catches:          %d   
min. dist:        %.2f   
min. dist locked: %.2f
traveled:         %.2f    
climb:            %.2f    
descend:          %.2f   ]]


local EgbertStatistic = {}


function EgbertStatistic:setT1 (newT1)
  T1 = newT1
  done = false
end


function EgbertStatistic:Done()
  return (T0 == nil) and done 
end


function EgbertStatistic:finalize()
end


local function copyHunt (hunt)
  local tab = {}
  tab.seqTime = hunt.seqTime
  tab.locks = hunt.locks
  tab.lockTime = hunt.lockTime
  tab.catches = hunt.catches
  tab.minDist = hunt.minDist
  tab.minDistLockedOn = hunt.minDistLockedOn
  return tab
end


function EgbertStatistic:apply (stat_dt)
  local pred = camera:GetFocalPredator()
  if nil == pred then return end
  local now = Simulation.SimulationTime()
  if pred:is_attacking() and T0 == nil then
    T0 = now
    sequence = sequence + 1
    flocksT0[sequence] = Simulation.GetFlock():num_clusters()
  end
  local hunt = pred:GetHuntStat()
  if pred:is_attacking() then
    hunts[sequence] = copyHunt(hunt)
    traveledDistance[sequence] = (traveledDistance[sequence] or 0) + glm.length(pred.velocity * stat_dt)
    if pred.velocity.y > 0 then
      climbedDistance[sequence] = (climbedDistance[sequence] or 0) + pred.velocity.y * stat_dt
    else
      descendDistance[sequence] = (descendDistance[sequence] or 0) - pred.velocity.y * stat_dt
    end
  elseif T0 then
    if (now - (T0 + hunts[sequence].seqTime or 0)) > T1 then
      T0 = nil
      done = true
      pred:ResetHunt()
    end
    flocksT1[sequence] = Simulation.GetFlock():num_clusters()
  end
end


function EgbertStatistic:display ()
  if sequence > 0 then
    local hunt = hunts[sequence]
    tb:SetText(string.format(PredStatFormat, sequence, hunt.seqTime, hunt.locks, hunt.lockTime, hunt.catches, hunt.minDist, hunt.minDistLockedOn, traveledDistance[sequence], climbedDistance[sequence] or 0, descendDistance[sequence] or 0))
  else
    tb:SetText(string.format(PredStatFormat, 0, 0, 0, 0, 0, 999.99, 999.99, 0, 0, 0))
  end
end


function EgbertStatistic:reset ()
  camera = Simulation.GetActiveCamera()
  ext = Simulation.Text():extent(PredStatFormat)
  tb = Simulation.Text():CreateTextBox("PredStat", glm.vec4(-ext.x, 0, ext.x, ext.y), camera)
  lastTime = nil
  flocksT0 = {}
  flocksT1 = {}
  traveledDistance = {}
  climbedDistance = {}
  descendDistance = {}
  hunts = {}
  T0 = nil
  sequence = 0
  done = false
end


local function file_exists (fname)
   local f=io.open(fname, "r")
   if f~=nil then f:close() return true else return false end
end


function EgbertStatistic:save (fname, append, run, repetition)
  if #hunts == 0 or run == nil then return end
  local headerWritten = file_exists(fname)  
  local out = assert(io.open(fname, append and "a" or "w"))
  -- write header if neccessary
  if not headerWritten then
    out:write("Run", delim, "Repetition", delim)
    for k in pairs(hunts[1]) do
      out:write(k, delim)
    end
    out:write("traveledDistance", delim, "climbedDistance", delim, "descendDistance", delim, "flocksT0", delim, "flocksT1\n")
  end
  -- write data
  for i, h in ipairs(hunts) do
    out:write(run, delim, repetition, delim)
    for _, v in pairs(h) do
      out:write(v, delim)          
    end
    out:write(traveledDistance[i] or 0, delim, climbedDistance[i] or 0, delim, descendDistance[i] or 0, delim, flocksT0[i] or 0, delim, flocksT1[i] or 0, "\n")
  end
  out:close()
end


return EgbertStatistic    -- Return your custom statistic here

