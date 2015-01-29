print("  sourcing Camera.lua")
-- camera.lua
--


local VK = require "VK"


local Camera = {}; Camera.__index = Camera


function Camera:new ()
  local obj = {}
  obj.cc = Simulation.CreateCamera()
  obj.fixedCenter = glm.vec3(15, 1.74, 0)
  obj.fixedEye = glm.vec3(0.001, 1.74, 0)
  obj.televisionDescent = 1.74
  obj.observerDistance = 10
  obj.localDistance = 5
  obj.firstOffset = 0.05
  obj.firstOffsetHead = 0.02

  obj.fixedFovy = 45.0
  obj.firstFovy = 60.0
  obj.firstOffsetFovy = 2 * math.deg( math.atan(720 / (2 * 1490)) )
  obj.observerFovy = 42.0
  obj.localFovy = 45.0

  obj.fixedLerp = glm.vec4(10, 10, 10, 1)      -- eye, center, up, FOV
  obj.firstLerp = glm.vec4(100, 100, 100, 1)
  obj.firstOffsetLerp = glm.vec4(10000, 10000, 10000, 1)
  obj.observerLerp = glm.vec4(1, 1, 1, 1)
  obj.localLerp = glm.vec4(10, 100, 1, 1)

  obj.radPerUI = 0.01 * 6.28       -- [rad]
  obj.zoomPerUI = 0.01 * 100.0     -- fovy [deg]
  obj.movePerUI = 1.0              -- [m]
  
  obj.RotationMode = {
    Rotate = 0,
    Orbit = 1,
    Ignore = 2
  }
  obj.mode = "Fixed"
  
  -- forward to cc
  function obj:GetViewport () return self.cc:GetViewport() end
  function obj:WindowViewport () return self.cc:WindowViewport() end
  function obj:SelectFocalBird (predator) self.cc:SelectFocalBird(predator) end
  function obj:SetFocalBird (bird) self.cc:SetFocalBird(bird) end
  function obj:SetFocalPrey (prey) self.cc:SetFocalPrey(prey) end
  function obj:SetFocalPredator (pred) self.cc:SetFocalPredator(pred) end
  function obj:GetFocalBird () return self.cc:GetFocalBird() end
  function obj:GetFocalPrey () return self.cc:GetFocalPrey() end
  function obj:GetFocalPredator () return self.cc:GetFocalPredator() end
  function obj:GetTargetInfo () return self.cc:GetTargetInfo() end
  function obj:HideFocal () self.cc:SetHideFocal() end
  function obj:flushLerp() self.cc:flushLerp() end 
  
  return setmetatable(obj, self)
end


function Camera:SetMode (mode, pred)
  pred = pred or false
  local cc = self.cc
  if cc:GetFocalBird() == nil then
    mode = "Fixed"
  end
  cc:SelectFocalBird(pred)
  local ti = cc:GetTargetInfo()
  cc.lerp = self.localLerp
  cc.fovy = self.localFovy
  cc:HideFocal(false)
  cc.up = glm.vec3(0,1,0)   -- 'untilt'
  if mode == "Television" then
    cc.center = ti.pos
    local eye = cc.eye
    eye.y = self.televisionDescent
    cc.eye = eye
    cc.lerp = self.fixedLerp
    cc.fovy = self.fixedFovy
    cc.up = glm.vec3(0,1,0)
    cc:SetRotationMode(self.RotationMode.Ignore)
  elseif mode == "First person" then
    cc.lerp = self.firstLerp
    cc.fovy = self.firstFovy
    cc:HideFocal(true)
    cc:SetRotationMode(self.RotationMode.Ignore)
  elseif mode == "First person offset" then
    cc.lerp = self.firstOffsetLerp
    cc.fovy = self.firstOffsetFovy
    cc:HideFocal(true)
    cc:SetRotationMode(self.RotationMode.Ignore)
  elseif mode == "First person head" then
    cc.lerp = self.firstLerp
    cc.fovy = self.firstFovy
    cc:HideFocal(true)
    cc:SetRotationMode(self.RotationMode.Ignore)
  elseif mode == "First person head offset" then
    cc.lerp = self.firstOffsetLerp
    cc.fovy = self.firstOffsetFovy
    cc:HideFocal(true)
    cc:SetRotationMode(self.RotationMode.Ignore)
  elseif mode == "Local 1" then
    cc.center = ti.pos
    cc.eye = ti.pos - self.localDistance * ti.forward
    cc.up = glm.vec3(0,1,0)
    cc:SetRotationMode(self.RotationMode.Orbit)
  elseif mode == "Local 2" then
    cc.center = ti.pos
    cc.eye = ti.pos + glm.vec3(0, self.localDistance, 0)
    cc.up = ti.forward
    cc:SetRotationMode(self.RotationMode.Orbit)
  elseif mode == "Local 3" then
    cc.center = ti.pos
    cc.eye = ti.pos + self.localDistance * ti.forward
    cc.up = glm.vec3(0,1,0)
    cc:SetRotationMode(self.RotationMode.Orbit)
  elseif mode == "Local 4" then
    cc.center = ti.pos
    cc.eye = ti.pos - glm.vec3(0, self.localDistance, 0)
    cc.up = ti.forward
    cc:SetRotationMode(self.RotationMode.Orbit)
  elseif mode == "Observer" then
    cc.center = ti.pos
    cc.eye = ti.pos - self.observerDistance * ti.forward
    cc.lerp = self.observerLerp
    cc.fovy = self.observerFovy
    cc.up = glm.vec3(0,1,0)
    cc:SetRotationMode(self.RotationMode.Ignore)
  else
    mode = "Fixed"
    cc.center = self.fixedCenter
    cc.eye = self.fixedEye
    cc.lerp = self.fixedLerp
    cc.fovy = self.fixedFovy
    cc.up = glm.vec3(0,1,0)
    cc:SetRotationMode(self.RotationMode.Rotate)
  end
  self.mode = mode
  Simulation.ShowAnnotation("Camera mode '" .. mode .. (pred and "' (Predator)" or "'"), 2)
end


function Camera:UpdateHook (sim_dt)
  local cc = self.cc
  local mode = self.mode
  if mode == "Fixed" or cc:GetFocalBird() == nil then
    return
  end
  local ti = cc:GetTargetInfo()
  if mode == "Television" then
    cc.center = ti.pos
  elseif mode == "Observer" then
    local dist = glm.length(cc.eye - cc.center)
    cc.center = ti.pos
    cc.eye = ti.pos - dist * ti.forward
    cc.up = ti.up
  elseif mode == "First person" then
    cc.center = ti.pos + ti.forward
    cc.eye = ti.pos
    cc.up = ti.up
  elseif mode == "First person offset" then
    cc.center = ti.pos + ti.forward + self.firstOffset * ti.up 
    cc.eye = ti.pos + self.firstOffset * ti.up
    cc.up = ti.up
  elseif mode == "First person head" then
    cc.center = ti.pos + ti.forwardH 
    cc.eye = ti.pos
    cc.up = ti.upH
  elseif mode == "First person head offset" then
    cc.center = ti.pos + ti.forwardH + self.firstOffsetHead * ti.upH 
    cc.eye = ti.pos + self.firstOffsetHead * ti.upH
    cc.up = ti.upH
  else -- local
    cc:shift(ti.pos)
  end
end


function Camera:KBH_Mode (mode, pred) 
  return function ()
    self:SetMode(mode, pred)
  end
end
    

function Camera:KBH_Move (motor, dir)
  motor = self.cc[motor]
  return function ()
    motor( self.cc, dir * self.movePerUI )
  end
end


function Camera:KBH_Rotate (motor, dir)
  motor = self.cc[motor]
  return function ()
    motor( self.cc, dir * self.radPerUI )
  end
end


function Camera:KBH_Zoom (dir)
  return function ()
    self.cc:zoom( dir * self.zoomPerUI )
    Simulation.ShowAnnotation("Camera FOVY: " .. tostring(self.cc.fovy) .. " deg", 2)
  end
end


function Camera:Use ()
  Simulation.AddKeyboardHook {
    { VK["1"], false, false, false, self:KBH_Mode("Fixed", false) },
    { VK["2"], false, false, false, self:KBH_Mode("Television", false) },
    { VK["2"], true, false, false, self:KBH_Mode("Television", true) },
    { VK["3"], false, false, false, self:KBH_Mode("Observer", false) },
    { VK["3"], true, false, false, self:KBH_Mode("Observer", true) },
    { VK["4"], false, false, false, self:KBH_Mode("First person", false) },
    { VK["4"], true, false, false, self:KBH_Mode("First person", true) },
    { VK["5"], false, false, false, self:KBH_Mode("Local 1", false) },
    { VK["5"], true, false, false, self:KBH_Mode("Local 1", true) },
    { VK["6"], false, false, false, self:KBH_Mode("Local 2", false) },
    { VK["6"], true, false, false, self:KBH_Mode("Local 2", true) },
    { VK["5"], false, true, false, self:KBH_Mode("Local 3", false) },
    { VK["5"], true, true, false, self:KBH_Mode("Local 3", true) },
    { VK["6"], false, true, false, self:KBH_Mode("Local 4", false) },
    { VK["6"], true, true, false, self:KBH_Mode("Local 4", true) },
    { VK["7"], false, false, false, self:KBH_Mode("First person offset", false) },
    { VK["7"], true, false, false, self:KBH_Mode("First person offset", true) },
    { VK["8"], false, false, false, self:KBH_Mode("First person head", false) },
    { VK["8"], true, false, false, self:KBH_Mode("First person head", true) },
    { VK["9"], false, false, false, self:KBH_Mode("First person head offset", false) },
    { VK["9"], true, false, false, self:KBH_Mode("First person head offset", true) },

    { VK.PRIOR, false, false, false, self:KBH_Move("moveForward", 1) },
    { VK.NEXT, false, false, false, self:KBH_Move("moveForward", -1) },
    { VK.PRIOR, false, true, false, self:KBH_Zoom(-1) },
    { VK.NEXT, false, true, false, self:KBH_Zoom(1) },
    { VK.LEFT, false, false, false, self:KBH_Rotate("rotateRightLeft", -1) },
    { VK.RIGHT, false, false, false, self:KBH_Rotate("rotateRightLeft", 1) },
    { VK.LEFT, true, false, false, self:KBH_Rotate("tilt", 1) },
    { VK.RIGHT, true, false, false, self:KBH_Rotate("tilt", -1) },
    { VK.UP, false, false, false, self:KBH_Rotate("rotateUpDown", -1) },
    { VK.DOWN, false, false, false, self:KBH_Rotate("rotateUpDown", 1) },
    { VK.NUMPAD4, false, false, false, self:KBH_Move("moveSidewardXZ", -1) },
    { VK.NUMPAD6, false, false, false, self:KBH_Move("moveSidewardXZ", 1) },
    { VK.NUMPAD2, false, false, false, self:KBH_Move("moveForwardXZ", -1) },
    { VK.NUMPAD8, false, false, false, self:KBH_Move("moveForwardXZ", 1) },
    { VK.NUMPAD3, false, false, false, self:KBH_Move("moveUpDown", -1) },
    { VK.NUMPAD9, false, false, false, self:KBH_Move("moveUpDown", 1) },
  }
  Simulation.SetActiveCamera(self)
  CameraUpdateHook = Camera.UpdateHook
end


return Camera




