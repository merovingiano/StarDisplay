-- GPWS Ground Proximity Warning System
--

print("  sourcing gpws.lua")


local gpws = {}


gpws.Default = 0
gpws.Custom = 1


function gpws.Landing (params)
  local default = gpws.DefaultPort(params)
  local arrival = {} -- keeps track of landing times
  local departure = {} -- keeps track of departure times
  local zero = glm.vec3(0,0,0)
 
  return function (bird)

    local boundary = bird.boundary
    local pos = bird.position
    local vel = bird.velocity
    local id = bird.id
    local T = Simulation.SimulationTime()

    -- Landed?

    local Ta = arrival[id] -- time of landing or nil
    if Ta ~= nil then
      local da = T - Ta
      if da > 1.0 then
        -- start
        arrival[id] = nil
        departure[id] = T
      else
        -- stay put
        bird.boundary = zero
        bird.steering = zero
        bird.speed = 0
        vel.y = 0
        bird.position = pos
      end
      return -- done
    end
 
    local Td = departure[id] -- time of departure

    local dd = T - (Td or 0)
    if Td ~= nil then 
      if dd > 10 then
        departure[id] = nil
      end
    elseif pos.y <= 0 then     -- landing
      arrival[id] = T
      departure[id] = nil
      return -- done
    end
    if dd < 1 then   -- start procedure
      vel.y = 1
      bird.forward = glm.normalize(vel)
      boundary = 1 * bird.forward
      boundary.y = 1
      bird.boundary = boundary
      bird.steering = zero
      return
    end

    default(bird)

  end
end

function gpws.skid (bird, pos, vel)
  pos.y = 0
  bird.position = pos
  vel.y = 0
  bird.forward = glm.normalize(vel)
  bird.speed = 0
   
end
  

function gpws.DefaultPort (params)
  local tti = params.tti            -- time to impact
  local lift = params.lift          -- mandatory lift (will be clamped to bird.maxLift)
  local maxLift = params.maxLift
  local plane = glm.vec4(0,-1,0,0)  -- ground plane (in constant-normal form)
  local default = gpws.DefaultPort(params)
  
  return function (bird)
    local boundary = bird.boundary
    local pos = bird.position
    local vel = bird.velocity
    boundary.y = boundary.y + lift   -- apply mandatory lift
    local t = glm.intersectRayPlane(pos, vel, plane)
    if t >= 0  and t <= tti then
      boundary.y = boundary.y + ((tti - t) / tti) * maxLift
    end
    if pos.y <= 0 then
      gpws.skid(bird, pos, vel, default)
    end 
    bird.boundary = boundary
  end
end

--[[
function gpws.SebasLinear (params)
  local land = {} 
  local start = {}
  local upper_border = params.upper_border  -- [s]
  local lower_border = params.lower_border  -- [s]
  local lift = params.lift          -- mandatory lift (will be clamped to bird.maxLift)
  local maxLift = params.maxLift
  local plane = glm.vec4(0,-1,0,0)  -- ground plane (in constant-normal form)

  return function (bird)
    local boundary = bird.boundary
    local steering = bird.steering
    local id = bird.id
    local pos = bird.position
    local vel = bird.velocity
  
    if start[id] == true then
      if pos.y > 15 then
        start[id] = nil
      else
        boundary.y = boundary.y + 1
        bird.boundary = boundary
      end
      return
    end

    if pos.y > 5 then start[id] = nil end
    if random:uniform01() <= 0.05 then
      land[id] = true
    end
    
    if land[id] == true then
      boundary.y =  boundary.y - maxLift      
    end
    
    boundary.y = boundary.y + lift   -- apply mandatory lift
    local time_to_impact = glm.intersectRayPlane(pos, vel, plane)
    if time_to_impact >= 0  and time_to_impact <= upper_border and land[id] == nil then
      local hit_time = math.max(0, (time_to_impact - lower_border) / (upper_border - lower_border)) 
      boundary.y = boundary.y + (1 - hit_time) * 0.5 * maxLift
      steering = hit_time * steering
    end
       
    if pos.y <= 0.05 then
      land[id] = nil
      start[id] = true
      boundary.y = boundary.y + maxLift
    end 
    
    bird.boundary = boundary
    bird.steering = steering
  end
end
--]]


function gpws.SimpleLanding ()
  local drop = {} 
  local climb = {}
  local sposy = {}

  return function (bird)
    local id = bird.id
    local pos = bird.position
    local gyro = glm.vec3(0)

    bird.speed = 15
--[[
    if drop[id] == true then
      gyro.z = -1
    elseif random:uniform01() <= 0.005 then
      drop[id] = true
      sposy[id] = pos.y
    end

    if pos.y <= 1 then
      drop[id] = nil
      climb[id] = true
      gyro.z = 1
    end 

    if climb[id] then
      if pos.y < sposy[id] then
        if bird.up.y < 0.707 then
          gyro.z = -1
        end
        gyro.x = 100
      else
        climb[id] = nil
      end
    end   
    ==]]
    print(drop[id], climb[id], gyro)
    bird.gyro = gyro
  end
end


function gpws.SebasLinear (params)
  local land = {} 
  local upper_border = params.upper_border  -- [s]
  local lower_border = params.lower_border  -- [s]
  local lift = params.lift          -- mandatory lift (will be clamped to bird.maxLift)
  local maxLift = params.maxLift
  local plane = glm.vec4(0,-1,0,0)  -- ground plane (in constant-normal form)

  return function (bird)
    local boundary = bird.boundary
    local steering = bird.steering
    local id = bird.id
    local pos = bird.position
    local vel = bird.velocity
  
    if random:uniform01() <= 0.005 then
      land[id] = true
    end
    
    if land[id] == true then
      boundary.y =  boundary.y - maxLift      
    end
    
    boundary.y = boundary.y + lift   -- apply mandatory lift
    local time_to_impact = glm.intersectRayPlane(pos, vel, plane)
    if time_to_impact >= 0  and time_to_impact <= upper_border and land[id] == nil then
      local hit_time = math.max(0, (time_to_impact - lower_border) / (upper_border - lower_border)) 
      boundary.y = boundary.y + (1 - hit_time) * maxLift
      steering = hit_time * steering
    end
       
    if pos.y <= 0.1 then
     land[id] = nil
     boundary.y =  boundary.y + maxLift      
    end 
    
    bird.boundary = boundary
    bird.steering = steering
  end
end


function gpws.SebasFixed (params)
  local upper_border = params.upper_border
  local lower_border = params.lower_border
  local middle_border = 0.5 * (upper_border + lower_border ) -- [s]
  local lift = params.lift          -- mandatory lift (will be clamped to bird.maxLift)
  local maxLift = params.maxLift
  local plane = glm.vec4(0,-1,0,0)  -- ground plane (in constant-normal form)

  return function (bird)
    local boundary = bird.boundary
    local steering = bird.steering
    local pos = bird.position
    local vel = bird.velocity
    boundary.y = boundary.y + lift   -- apply mandatory lift
    local time_to_impact = glm.intersectRayPlane(pos, vel, plane)
    if time_to_impact >= 0  and time_to_impact <= middle_border then
      boundary.y =  boundary.y + 0.5 * maxLift
      steering = 0.5 * steering -- Keeping steering at normal and changing boundary creates the wave patterns in smaller flocks.
    end
    
    local rand = random:uniform01()
    if rand <= 0.1 and pos.y <= 10 then
      boundary.y =  boundary.y - maxLift
    end
    
 
    bird.boundary = boundary
    bird.steering = steering
  end
end

function gpws.SebasNoInter ()

  return function (bird)
    local pos = bird.position
    local vel = bird.velocity
    
    if pos.y <= 0 then
      gpws.skid(bird, pos, vel)
    end 
  end 
end


return gpws
