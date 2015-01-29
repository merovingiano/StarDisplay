print("  sourcing triggerPanic.lua")


local VK = require "VK"


-- Trigger panic reaction of the focal prey
--
local triggerPanic = function ()
  local camera = Simulation.GetActiveCamera()
  local prey = camera.GetFocalPrey()
  if prey == nil then return end
  prey.panicOnset = Simulation.SimulationTime()
  prey.predatorReaction = bit.bor(PredationReaction.Panic, PredationReaction.Detectable)
end


-- Bind to some key
--Simulation.AddKeybordHook("T", false, false, false, triggerPanic)

