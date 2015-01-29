print("  sourcing turn_statistic.lua")
-- turn_statistic.lua
--
-- Turning custom statistic

local va = require("valarray")


local delim = " "             -- use "," for csv output
local quote = '"'
local NaN = "-1"              -- replacement for 1.#QNAN


local camera = nil;


local turnStatistic = {}

local T0 = 0
local T = {}
local banking = {}
local accel = {}


function turnStatistic:finalize ()
  print("turnStatistic:finalize")
end


function turnStatistic:apply (stat_dt)
  local n = #T + 1
  T0 = T0 + stat_dt
  if T0 == 0 then return end
  T[n] = T0
  for p in Simulation.Prey() do
    local id = p.id;
    banking[id][n] = p:visit(FMappings.Banking)
    accel[id][n] = p:visit(FMappings.LateralGForce) * 9.81 
  end
end


function turnStatistic:display ()
--[[  
  if sequence > 0 then
    local hunt = hunts[sequence]
    tb:SetText(string.format(PredStatFormat, sequence, hunt.seqTime, hunt.locks, hunt.lockTime, hunt.catches, hunt.minDist, traveledDistance[sequence], climbedDistance[sequence] or 0, descendDistance[sequence] or 0))
  else
    tb:SetText(string.format(PredStatFormat, 0, 0, 0, 0, 0, 999.99, 0, 0, 0))
  end
  --]]
end


function turnStatistic:reset ()
  print("turnStatistic:reset")
  T0 = 0
  T = {}
  banking = {}
  accel = {}
  for p in Simulation.Prey() do
    local id = p.id;
    banking[id] = {}
    accel[id] = {}
  end
--[[
  camera = Simulation.GetActiveCamera()
  ext = Simulation.Text():extent(PredStatFormat)
  tb = Simulation.Text():CreateTextBox("PredStat", glm.vec4(-ext.x, 0, ext.x, ext.y), camera)
--]]
end


function turnStatistic:save (fname, append, run, repetition)
  print("turnStatistic:save", fname)
  local out = assert(io.open(fname, append and "a" or "w"))

  local MA = {}
  for id,x in pairs(banking) do
    local maxi = 0
    local max = -1000000
    for i, b in pairs(x) do
      if b > max then
        maxi = i
        max = b
      end
    end
    MA[id] = T[maxi]
  end
  table.sort(MA)
  for i,x in pairs(MA) do
    out:write(x - MA[1], "\n")
  end

--[[
  -- Header
  out:write(quote, "T", quote, delim)
  for id in pairs(banking) do
    out:write(quote, "b", id, quote, delim)
  end
  for id in pairs(banking) do
    out:write(quote, "a", id, quote, delim)
  end
  out:write("\n")
  -- Data
  for i = 1,#T do
    out:write(T[i], delim)
    for _,x in pairs(banking) do
      out:write(x[i], delim)
    end
    for _,x in pairs(accel) do
      out:write(x[i], delim)
    end
    out:write("\n")
  end
--]]
  out:close()
end


return turnStatistic    -- Return your custom statistic here



