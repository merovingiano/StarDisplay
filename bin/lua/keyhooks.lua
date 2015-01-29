print("  sourcing KeyHooks.lua")
--
--Default keyboard hooks
--
local VK = require "VK"
local Camera = require "Camera"


local __KeyboardHooks = {}
local __KeyboardMap = {}


local function AddKeyboardHook (key, shift, alt, ctrl, fun)
  if type(key) == "table" then
    for _, a in pairs(key) do
      __KeyboardHooks[ VK.Encode(a[1], a[2] or false, a[3] or false, a[4] or false) ] = a[5]
    end
  else
    __KeyboardHooks[ VK.Encode(key, shift or false, alt or false, ctrl or false) ] = fun
  end
end


local function RemapKey (src, dst)
  __KeyboardMap[ VK.Encode(src[1], src[2] or false, src[3] or false, src[4] or false) ] = 
    VK.Encode(dst[1], dst[2] or false, dst[3] or false, dst[4] or false)
end


function __ProcessKeyboardHooks (encoded)
  encoded = __KeyboardMap[encoded] or encoded
  local h = __KeyboardHooks[encoded]
  if nil ~= h then h() return true end
  return false
 end
 

Simulation.AddKeyboardHook = AddKeyboardHook
Simulation.RemapKey = RemapKey


function Simulation.EmulateKeyDown (key, shift, alt, ctrl)
  Simulation.DoEmulateKeyDown(VK.Encode(key, shift, alt, ctrl))
end


local function KBH_Fullscreen ()
  local win = Simulation.Window()
  local mode = win:IsFullscreen() and "normal" or "fullscreen"
  win:ShowWindow(mode)
end


local function KBH_DecrRoostRadius ()
  local roost = Simulation.GetRoost()
  local r = roost.Radius
  local dr = 1
  if r >= 100 then dr = 10 end
  if r >= 1000 then dr = 100 end

  roost.Radius = math.max(r - dr, roost.minRadius)
  Simulation.SetRoost(roost)
end


local function KBH_IncrRoostRadius ()
  local roost = Simulation.GetRoost()
  local r = roost.Radius
  local dr = 1
  if r >= 100 then dr = 10 end
  if r >= 1000 then dr = 100 end
  roost.Radius = math.min(r + dr, roost.maxRadius)
  Simulation.SetRoost(roost)
end


local function KBH_MaximizeRoostRadius ()
  local roost = Simulation.GetRoost()
  roost.Radius = roost.maxRadius
  Simulation.SetRoost(roost)
  Simulation.ShowAnnotation("Roost radius maximized", 2)
end


local function KBH_MinimizeRoostRadius ()
  local roost = Simulation.GetRoost()
  roost.Radius = roost.minRadius
  Simulation.SetRoost(roost)
  Simulation.ShowAnnotation("Roost radius minimized", 2)
end


function KBH_StartAttack ()
  local sim = Simulation
  local pred = sim.GetActiveCamera():GetFocalPredator()
  if pred ~= nil then
    if pred:is_attacking() then
      pred:EndHunt(false)
	end
    if (pred.PredParams.PreySelection == PreySelections["Auto"]) or (nil ~= pred:GetTargetPrey()) then
      pred:StartAttack()
      sim.ShowAnnotation("Attack started", 2)
    else
      sim.ShowAnnotation("No target selected!", 2)
    end
  end
end


function KBH_AbortAttack ()
  local sim = Simulation
  local pred = sim.GetActiveCamera():GetFocalPredator()
  if pred ~= nil then
    if pred:is_attacking() then 
      pred:EndHunt(false)
      sim.ShowAnnotation("Attack aborted", 2)
    end
  end
end


function KBH_ResetHuntStat ()
  local sim = Simulation
  local pred = sim.GetActiveCamera():GetFocalPredator()
  if nil ~= pred then 
    pred:ResetHunt() 
    sim.ShowAnnotation("Hunt statistics reseted", 2)
  end
end


function KBH_ToggleRenderFlag (flag, annOn, annOff)
  return function ()
    local sim = Simulation
    local flags = sim.GetRenderFlags()
    flags[flag] = not flags[flag]
    sim.SetRenderFlags(flags)
    sim.ShowAnnotation(flags[flag] and annOn or annOff, 2)
  end
end


function KBH_ToggleHelpMsg ()
  local sim = Simulation
  local flags = sim.GetRenderFlags()
  if flags.helpMsg == "" then
    flags.helpMsg = sim.HelpMessage
  else 
    flags.helpMsg = ""
  end
  sim.SetRenderFlags(flags)
end


function KBH_IncrRtreeLevel (inc)
  return function ()
    local sim = Simulation
    local flags = sim.GetRenderFlags()
    flags.rtreeLevel = flags.rtreeLevel + inc
    if flags.rtreeLevel < 0 then
      flags.rtreeLevel = 0
    end
    sim.SetRenderFlags(flags)
  end
end


AddKeyboardHook(VK.A, false, false, false, KBH_ToggleRenderFlag("alphaMasking", "Alpha masking on", "Alpha masking off"))
AddKeyboardHook(VK.B, false, false, false, KBH_ToggleRenderFlag("show_world", "Boundary shown", "Boundary hidden"))
AddKeyboardHook(VK.B, false, true, false, KBH_ToggleRenderFlag("altBackground", "", ""))
AddKeyboardHook(VK.C, false, false, false, KBH_ToggleRenderFlag("show_circ", "Circularity shown", ""))
AddKeyboardHook(VK.F, false, false, false, KBH_ToggleRenderFlag("show_forces", "Forces shown", ""))
AddKeyboardHook(VK.H, false, false, true, KBH_ToggleRenderFlag("show_hist", "", ""))
AddKeyboardHook(VK.L, false, false, false, KBH_ToggleRenderFlag("show_local", "Local body coordinate systems shown", ""))
AddKeyboardHook(VK.H, false, false, false, KBH_ToggleRenderFlag("show_head", "Local head coordinate systems shown", ""))
AddKeyboardHook(VK.N, false, false, false, KBH_ToggleRenderFlag("show_neighbors", "Links to neighbors shown", ""))
AddKeyboardHook(VK.P, false, false, false, KBH_ToggleRenderFlag("show_pred", "Predator targeting shown", ""))
AddKeyboardHook(VK.R, false, false, false, KBH_ToggleRenderFlag("show_search", "Search box shown", ""))
AddKeyboardHook(VK.S, false, false, false, KBH_ToggleRenderFlag("slowMotion", "Slow motion", ""))
AddKeyboardHook(VK.T, false, false, false, KBH_ToggleRenderFlag("show_trails", "Trails shown", "Trails hidden"))
AddKeyboardHook(VK.V, false, false, false, KBH_ToggleRenderFlag("show_numbers", "", ""))
AddKeyboardHook(VK.W, false, false, false, KBH_ToggleRenderFlag("wireFrame", "Polygon mode wire frame", "Polygon mode solid"))
AddKeyboardHook(VK.F1, false, false, false, KBH_ToggleHelpMsg)
AddKeyboardHook(VK.F4, false, false, false, KBH_IncrRtreeLevel(-1))
AddKeyboardHook(VK.F4, true, false, false, KBH_IncrRtreeLevel(1))
AddKeyboardHook(VK.F5, false, false, false, KBH_ToggleRenderFlag("show_rulers", "Ruler shown", ""))


AddKeyboardHook(VK.F8, false, false, false, KBH_Fullscreen)
AddKeyboardHook(VK.F3, false, false, false, KBH_DecrRoostRadius)
AddKeyboardHook(VK.F3, true, false, false, KBH_IncrRoostRadius)
AddKeyboardHook(VK.M, true, false, false, KBH_MaximizeRoostRadius)
AddKeyboardHook(VK.M, false, false, false, KBH_MinimizeRoostRadius)
AddKeyboardHook(VK.P, true, false, false, KBH_StartAttack)
AddKeyboardHook(VK.P, false, true, false, KBH_AbortAttack)
AddKeyboardHook(VK.P, false, false, true, KBH_ResetHuntStat)

-- Dell XPS hack
AddKeyboardHook(VK.SPACE, false, false, false, function () Simulation.DoEmulateKeyDown(VK.Encode(VK.PAUSE, false, false, false)) end)

