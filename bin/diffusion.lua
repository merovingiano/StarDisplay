print("  sourcing diffusion.lua")


local Statistic = {}
local va = require "valarray"

local T           -- time 
local Cm          -- center over time
local UV          -- flock forward direction over time 
local Ri          -- trajectories over time
local stoped = true


-- Return center of mass of the flock, an
-- vector of the individual positions and
-- the current forward direction of the flock
local function cm_pos_uv ()
  local ri = {}
  local pa = Analysis.average_V()
  local uv = glm.vec3(0)
  local FF = {}
  for p in Simulation.Prey() do
    local pos = p.position + 0.01 * random:vec_in_sphere()
    ri[p.id] = pos
    pa(pos)
    uv = uv + p.forward
  end
  return pa:mean(), ri, glm.normalize(glm.vec3(uv.x, 0, uv.z))
end


-- Returns the covariance matrix at time t0 + tau
local function cov (t0, tau)
  local M = glm.mat3(0)
  local cmt0 = Cm[t0]
  local cmt1 = Cm[t0+tau]
  local rit0 = Ri[t0]
  local rit1 = Ri[t0+tau]
  local n = 0
  for i,_ in pairs(rit0) do
    local term = (rit0[i] - cmt0) - (rit1[i] - cmt1)
    M = M + glm.outerProduct(term, term)
    n = n + 1
  end
  return M / n
end


function Statistic:finalize ()
end


function Statistic:apply (stat_dt)
  if stat_dt == 0 or stoped == true then return end
  T[#T + 1] = T[#T] + stat_dt
  Cm[#Cm + 1], Ri[#Ri + 1], UV[#UV+1] = cm_pos_uv()
  if T[#T] > 2 then
    print("Diffusion statistic done")
    stoped = true
  end
end


function Statistic:display ()
end


function Statistic:reset ()
  T, Cm, Ri, UV = {}, {}, {}, {}
  T[0] = 0
  Cm[0], Ri[0], UV[0] = cm_pos_uv()
  stoped = false
end


function Statistic:save ()
  local timeStamp = '' --os.date("_%d-%m-%y_%H-%M-%S")
  local DataFilePostfix = "Diffusion" .. timeStamp .. ".dat"
  local out = assert(io.open(Simulation.DataPath .. DataFilePostfix, "w"))

  local N = #Ri[1]
  local TT = #T
  local t = {}
  local R2 = {}
  local E = {}
  local g = glm.vec3(0,1,0)
  local u1uv,u2uv,u3uv, u1ug,u2ug,u3ug, u1uw, u2uw,u3uw = 
    Analysis.average_N(),Analysis.average_N(),Analysis.average_N(),
    Analysis.average_N(),Analysis.average_N(),Analysis.average_N(),
    Analysis.average_N(),Analysis.average_N(),Analysis.average_N()
  local t0 = 0
  for tau = t0+1, TT-t0 do
    local M = cov(t0,tau)
    t[#t+1] = T[t0+tau]
    R2[#R2+1] = glm.trace(M)
    E[#E+1] = glm.eigen_values(M)  -- increasing order (z is biggest)
    local EV = glm.eigen_vectors(M)
    local uv = UV[t0+tau]
    local uw = glm.cross(uv, g)    
    u1uv( math.abs(glm.dot(uv, glm.column(EV, 2))) )
    u2uv( math.abs(glm.dot(uv, glm.column(EV, 1))) )
    u3uv( math.abs(glm.dot(uv, glm.column(EV, 0))) )
    u1ug( math.abs(glm.dot(g, glm.column(EV, 2))) )
    u2ug( math.abs(glm.dot(g, glm.column(EV, 1))) )
    u3ug( math.abs(glm.dot(g, glm.column(EV, 0))) )
    u1uw( math.abs(glm.dot(uw, glm.column(EV, 2))) )
    u2uw( math.abs(glm.dot(uw, glm.column(EV, 1))) )
    u3uw( math.abs(glm.dot(uw, glm.column(EV, 0))) )

    out:write(t[#t], ' ', R2[#R2], ' ', E[#E].x, ' ', E[#E].y, ' ', E[#E].z, ' ', u1uv:mean(), ' ', u2uv:mean(), ' ', u3uv:mean(), ' ', u1ug:mean(), ' ', u2ug:mean(), ' ', u3ug:mean(), '\n')
  end
  out:close()
  DataFilePostfix = "Diffusion_summary_" .. timeStamp .. ".dat"
  out = assert(io.open(Simulation.DataPath .. DataFilePostfix, "w"))

  local a,b = va.linear_regression(va.log(t), va.log(R2))
  io.write("a = ", b, "\nD = ", math.exp(a), '\nDf = ', math.exp(a) * math.pow(0.1, b), '\n\n')
  out:write("a = ", b, "\nD = ", math.exp(a), '\nDf = ', math.exp(a) * math.pow(0.1, b), '\n\n')

  local R = {}
  for _,x in pairs(E) do R[#R+1] = x.z end
  a,b = va.linear_regression(va.log(t), va.log(R))
  io.write("a1 = ", b, "\nD1 = ", math.exp(a), '\nD1f = ', math.exp(a) * math.pow(0.1, b), '\n\n')
  out:write("a1 = ", b, "\nD1 = ", math.exp(a), '\nD1f = ', math.exp(a) * math.pow(0.1, b), '\n\n')

  R = {}
  for _,x in pairs(E) do R[#R+1] = x.y end
  a,b = va.linear_regression(va.log(t), va.log(R))
  io.write("a2 = ", b, "\nD2 = ", math.exp(a), '\nD2f = ', math.exp(a) * math.pow(0.1, b), '\n\n')
  out:write("a2 = ", b, "\nD2 = ", math.exp(a), '\nD2f = ', math.exp(a) * math.pow(0.1, b), '\n\n')

  R = {}
  for _,x in pairs(E) do R[#R+1] = x.x end
  a,b = va.linear_regression(va.log(t), va.log(R))
  io.write("a3 = ", b, "\nD3 = ", math.exp(a), '\nD3f = ', math.exp(a) * math.pow(0.1, b), '\n\n')
  out:write("a3 = ", b, "\nD3 = ", math.exp(a), '\nD3f = ', math.exp(a) * math.pow(0.1, b), '\n\n')

  io.write('u1uv = ', u1uv:mean(), '\nu2uv = ', u2uv:mean(), '\nu3uv = ', u3uv:mean(), '\nu1ug = ', u1ug:mean(), '\nu2ug = ', u2ug:mean(), '\nu3ug = ', u3ug:mean(), '\nu1uw = ', u1uw:mean(), '\nu2uw = ', u2uw:mean(), '\nu3uw = ', u3uw:mean(), '\n')
  out:write('u1uv = ', u1uv:mean(), '\nu2uv = ', u2uv:mean(), '\nu3uv = ', u3uv:mean(), '\nu1ug = ', u1ug:mean(), '\nu2ug = ', u2ug:mean(), '\nu3ug = ', u3ug:mean(), '\nu1uw = ', u1uw:mean(), '\nu2uw = ', u2uw:mean(), '\nu3uw = ', u3uw:mean(), '\n')
  out:close()
end


return Statistic    -- Return your custom statistic here



