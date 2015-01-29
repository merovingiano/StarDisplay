print("  sourcing travelDist.lua")
-- travelDist.lua
-- Tracking traveld distances under attack.
-- To make it a bit more interesting, g-force and a TextBox is added.


local TravelDistance = {}


TravelDistance.tabFormat = [[
              samples   time      dist      speed     g-force 
Control:      %-9d %-9.2f %-9.2f %-9.2f %-9.2f
Under attack: %-9d %-9.2f %-9.2f %-9.2f %-9.2f
Panicking:    %-9d %-9.2f %-9.2f %-9.2f %-9.2f
]]

--
-- Helper functions
--

-- pre prey logging
local function Log (prey, dt, log)
  local id = prey.id
  local L = log[id]
  if nil == L then
    log[id] = { S = 0, T = 0, D = 0, A = 0 }
    L = log[id]
  end
  L.S = L.S + 1                                    -- accumulate samples
  L.T = L.T + dt                                   -- accumulate time
  L.D = L.D + prey.speed * dt                      -- accumulate linear distance
  L.A = L.A + glm.length(prey.acceleration) * dt   -- accumulate acceleration
end


local function calculateAve (log)
  local N, S, T, D, A = 0, 0, 0, 0, 0, 0
  for _, e in pairs(log) do
    N = N + 1
    S = S + e.S
    T = T + e.T
    D = D + e.D
    A = A + e.A
  end
  return S, T/N, D/N, A/N
end


--
-- Implementation of the Statistic interface
--

function TravelDistance:finalize () 
  if self.textBox then
    self.textBox = Simulation.Text():RemoveTextBox(self.textBox)
  end
end


function TravelDistance:reset ()
  local sim = Simulation
  self.fileName = sim.OutputFileName("TravelDistance").name
  local ext = sim.Text():extent("\\monoface{}" .. self.tabFormat)
  ext = glm.vec4(-ext.x, -ext.y, ext.x, ext.y)
  self.textBox = sim.Text():CreateTextBox("TravelDistance", ext, sim.GetActiveCamera())
  self.TDP = { }  -- holds timespan, distance traveled and acceleration per panicking prey
  self.TDA = { }  -- holds timespan, distance traveled and acceleration per prey under attack
  self.TDC = { }  -- holds timespan, distance traveled and acceleration per prey control group
end


function TravelDistance:apply (stat_dt)
  for p in Simulation.Prey() do
    local reaction = p.predatorReaction
    if reaction >= PredationReaction.Panic then
      Log(p, stat_dt, self.TDP)
    elseif (reaction > PredationReaction.Return) or (p.panicRelaxation > 0) then
      Log(p, stat_dt, self.TDA)
    elseif reaction == PredationReaction.None then
      Log(p, stat_dt, self.TDC)
    end
  end
end


function TravelDistance:save ()
  local SC,TC,DC,AC = calculateAve(self.TDC)
  local SA,TA,DA,AA = calculateAve(self.TDA)
  local SP,TP,DP,AP = calculateAve(self.TDP)
  local str = string.format(self.tabFormat, 
                            SC, TC, DC, DC / TC, AC /(TC * 9.81), 
                            SA, TA, DA, DA / TA, AA /(TA * 9.81), 
                            SP, TP, DP, DP / TP, AP /(TP * 9.81))
  self.textBox:SetText("\\monoface{}" .. str)
  -- save latest result to file
  local ofile = assert(io.open(self.fileName, "w"))
  ofile:write(str, "\n")
  ofile:close()
end


return TravelDistance;

