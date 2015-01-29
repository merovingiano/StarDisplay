print("  sourcing TA.lua")


local predData = {}
local preyData = {}


local TA = {}

TA.Deflection = 0
TA.Custom = 1


function TA.DirectPursuit(Weight)
  local weight = Weight

  return function (pred, simTime, targetPos)
    pred.cohesion =  weight * glm.normalize(targetPos - pred.position)
  end
end


function TA.ProportionalNavigation(Weight)
  local weight = Weight
  local lastT = 0
  local lastForward = glm.vec3(1,0,0)
  local red = glm.vec3(1,0,0);
  local blue = glm.vec3(0,0,1);
  local line = Simulation.line
      
  return function (pred, simTime, targetPos, targetHeading, targetSpeed)
    local dt = simTime - lastT
    lastT = simTime
        
    local r = targetPos - pred.position
    local relV = pred.velocity - (targetHeading * targetSpeed)

    local wLOS = glm.cross(relV, r) / glm.dot(r,r);
    local wF = (1/dt) * glm.cross(pred.forward, pred.forward - lastForward)
    lastForward = pred.forward
  
    local data = {}
    data.Id = pred.id
    data.T = simTime
    data.N = math.abs(glm.length(wF) / (glm.length(wLOS) + 0.00000001))
    data.Pos = pred.position
    data.RollRate = pred:visit(FMappings.RollRate)
    data.Accel = glm.length(pred:visit(FMappings.Accel))
    data.Distance = glm.length(r)
    data.RelVel = relV
    data.PreyPos = targetPos
    predData[#predData + 1] = data

    local cohesion = weight * glm.cross(wLOS, pred.forward)
    pred.cohesion = cohesion
  end
end


TA.Statistic = {}


function TA.Statistic:finalize()
  print("TA.Statistic finalize")
end


function TA.Statistic:apply (stat_dt)
end


function TA.Statistic:display ()
end


function TA.Statistic:reset ()
end


function TA.Statistic:save ()
  local timeStamp = os.date("_%d-%m-%y_%H-%M-%S")
  local DataFilePostfix = "TA" .. timeStamp .. ".dat"
  local out = assert(io.open(Simulation.DataPath .. "Pred_"..DataFilePostfix, "w"))
  out:write("Id T N PosX PosY PosZ RollRate Accel Distance RelVelX  RelVelY RelVelZ PreyPos.x PreyPos.y PreyPos.z \n")
  for _,x in ipairs(predData) do
    if x.T > 0.1 then
      out:write(x.Id, ' ', x.T, ' ', x.N, ' ', x.Pos.x, ' ', x.Pos.y, ' ', x.Pos.z, ' ', x.RollRate, ' ', x.Accel, ' ', x.Distance, ' ', x.RelVel.x, ' ', x.RelVel.y, ' ', x.RelVel.z, ' ', x.PreyPos.x, ' ', x.PreyPos.y, ' ', x.PreyPos.z, " \n")
    end
  end
  out:close()    
end


return TA





