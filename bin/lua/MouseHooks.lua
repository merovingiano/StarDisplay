print("  sourcing MouseHooks.lua")
--
-- Default mouse hooks
--

local VK = require "VK"


local __MouseHooks = {}


local function AddMouseHook (button, shift, alt, ctrl, dbl, fun)
  __MouseHooks[ VK.Encode(button, shift or false, alt or false, ctrl or false, dbl or false) ] = fun
end


function __ProcessMouseHooks (encoded, x, y)
  local h = __MouseHooks[encoded]
  if nil ~= h then h(x, y) return true end
  return false
end


local function MOUSE_RemoveTrails (x, y)
  for prey in Simulation.Prey() do
    prey:SetTrail(false)
  end
--  for pred in Simulation.Predators() do
--    pred:SetTrail(false)
--  end
end


local function MOUSE_AddTrails (x, y)
  for prey in Simulation.Prey() do
    prey:SetTrail(true)
  end
--  for pred in Simulation.Predators() do
--    pred:SetTrail(true)
--  end
end


local function MOUSE_SelectFocalBird (x, y)
  local bird = Simulation.GetBirdAtScreenPosition(x, y)
  if bird then
    Simulation.GetActiveCamera():SetFocalBird(bird)
  end
end


local function MOUSE_SelectAndMarkFocalBird (x, y)
  local bird = Simulation.GetBirdAtScreenPosition(x, y)
  if bird then
    Simulation.GetActiveCamera():SetFocalBird(bird)
  	bird:SetTrail(bird:HasTrail() and false or true)
  end
end


local function MOUSE_SelectPredatorTarget (x, y)
  local pred = Simulation.GetActiveCamera():GetFocalPredator()
  if pred then
	  local bird = Simulation.GetBirdAtScreenPosition(x, y)
	  if bird and bird:isPrey() then
	    pred:SetTargetPrey(bird)
	    Simulation.ShowAnnotation("New predator target selected", 2)
	  end
  end
end


local function MOUSE_SelectAndFocusPredatorTarget (x, y)
  local camera = Simulation.GetActiveCamera()
  local pred = camera:GetFocalPredator()
  if pred then
	  local bird = Simulation.GetBirdAtScreenPosition(x, y)
	  if bird and bird:isPrey() then
	    camera:SetFocalBird(bird)
	    pred:SetTargetPrey(bird)
	    Simulation.ShowAnnotation("New predator target selected", 2)
	  end
  end
end


AddMouseHook(0, false, false, false, false, MOUSE_SelectFocalBird)
AddMouseHook(0, true, false, false, false, MOUSE_SelectAndMarkFocalBird)
AddMouseHook(0, false, false, true, false, MOUSE_SelectPredatorTarget)
AddMouseHook(0, true, false, true, false, MOUSE_SelectAndFocusPredatorTarget)
AddMouseHook(0, false, false, false, true, MOUSE_RemoveTrails)
AddMouseHook(0, false, true, false, true, MOUSE_AddTrails)


Simulation.AddMouseHook = AddMouseHook




