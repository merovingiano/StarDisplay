print("  sourcing wave.lua")
 

-- circThreshold main purpose is to reduce the amount of data colected.
-- Set circThreshold = 0 to collect data for all birds involved in the wave
local WaveCircThreshold = 0.0
local WaveSummaryData = {}
local WaveData = {}
local WaveOut;
local WaveSummaryOut;
local Summary = {}
local SummaryFileName;


function KBH_WaveTrigger ()
  print("Wave")
  -- Select dorsal prey
	local Flock = Simulation.GetFlock()
	local prey = assert(Simulation.GetActiveCamera():GetFocalPrey())
	local cluster = prey:GetFlockCluster()
  local dir = glm.normalize(cluster.velocity)
	local center = glm.center(cluster.bbox)
	local mini = 1000000
  for fm in Simulation.Flockmates(prey) do
		local current = glm.dot(fm.position - center, dir)
		if current < mini then
			mini = current
			prey = fm
		end
	end

  -- Trigger evasion
  prey.panicOnset = Simulation.SimulationTime()
  prey.predatorReaction = bit.bor(PredationReaction.Panic, PredationReaction.Detectable)
  return prey
end



-- Trigger panic reaction of the focal prey
--
function WaveTrigger (Run, Repetition, ParameterSets)
  Run = Run or 0
  Repetition = Repetition or 1 
  WaveData = {}
  local NP, INP, IL, Rsep = 0, 0, 0, 0
  NP = Simulation.GetRoost().numPrey
  for prey in Simulation.Prey() do
    local P = prey.BirdParams
	  Rsep = P.separationStep.y
	  P = prey.PreyParams
	  INP = P.IncurNeighborPanic
	  IL = P.IncurLatency
    break
  end
  if ParameterSets then 
    NP = ParameterSets.numPrey[Run]
    INP = ParameterSets.IncurNeighborPanic[Run]
    IL = ParameterSets.IncurLatency[Run]
    Rsep = math.floor(100 * ParameterSets.separationRadius[Run])
    print(Rsep)
  end
  local FileName = Simulation.DataPath .. "wave_" .. Run .. "_" .. Repetition .. "_" .. NP .. "_" .. INP .. "_" .. IL .. "_" ..Rsep .. "_" ..os.date("_%d-%m-%y") .. ".dat"
  WaveOut = assert(io.open(FileName, "w"))
  FileName = Simulation.DataPath .. "summary_wave_" .. NP .. "_" .. INP .. "_" .. IL .. "_" .. Rsep .. "_" ..os.date("_%d-%m-%y") .. ".dat"
  SummaryFileName = Simulation.DataPath .. "summary_" .. NP .. "_" .. INP .. "_" .. IL .. "_" .. Rsep .. "_" ..os.date("_%d-%m-%y") .. ".dat"
  local wa = "a"
  if Repetition == 1 then
    wa = "w"
  end
  WaveSummaryOut = assert(io.open(FileName, wa))
  if wa == "w" then
    WaveSummaryData = {
      duration = Analysis.min_max_mean_N(),
      wv = Analysis.accumulator_N(),
      pc = Analysis.min_max_mean_N(),
    }
    Summary = {
      duration = Analysis.min_max_mean_N(),
      wv = Analysis.accumulator_N(),
      pc = Analysis.min_max_mean_N(),
    }
    WaveSummaryOut:write("nnd nndVar duration participance wv wvVar PcMax\n")
    Summary.N = Analysis.accumulator_N()
  end
  WaveSummaryData.N = 0

  -- Do the actual trigger
  local prey = KBH_WaveTrigger()

  -- Collect data at T0
  WaveData.T0 = Simulation.SimulationTime()
  WaveData.posT0 = prey.position
  WaveData.id = prey.id

  local nndi = Analysis.accumulator_N()

  for p in Simulation.Prey() do
    nndi(p:visit(FMappings.NNDInterior));
    if p.circularity >= WaveCircThreshold then
      local nni = p:NNInfo()
      WaveData[p.id] = { posT0 = p.position, nndT0 = nni and nni.distance or 100}
    end
  end
  WaveSummaryOut:write(nndi:mean(), ' ', nndi:variance(), ' ')
end



function WaveRepetitionSummary ()
  WaveSummaryOut:write(
    WaveSummaryData.N, ' ', 
    WaveSummaryData.wv:mean(), ' ', WaveSummaryData.wv:variance(), ' ',
    WaveSummaryData.pc:max(), '\n'
  )
  Summary.N( WaveSummaryData.N )
  Summary.wv( WaveSummaryData.wv:mean() )
  Summary.pc( WaveSummaryData.pc:max() )
end


function WaveRunSummary ()
  local SummaryOut = assert(io.open(SummaryFileName, "w"))
  SummaryOut:write(
    Summary.N:mean(), ' ',
    Summary.wv:mean(), ' ', Summary.wv:variance(), ' ',
    Summary.pc:max(), '\n'
  )  
end


function Wave (Evasion)
  local evasion = Evasion

  return function (prey, pred)
    local d = WaveData[prey.id]
    if d ~= nil then
      if d.dt == nil then
        d.dt = Simulation.SimulationTime() - WaveData.T0
        local posT1 = prey.position
        local pc = prey.panicCopy
        local D0 = glm.distance(WaveData.posT0, d.posT0)
        local flock = Simulation.GetFlock()
        local p = flock:FindPrey(WaveData.id)
        local D1 = 0
        if p ~= nil then
          D1 = glm.distance(p.position, posT1)
        end
        local nni = prey:NNInfo()
        nndT1 = nni and nni.distance or 100
        WaveOut:write(d.dt, ' ' , d.nndT0, ' ', nndT1, ' ', D0, ' ', D1, ' ', pc, ' ', D0 / d.dt, ' ', D1 / d.dt, '\n')

        local dt = math.max(d.dt, 0.00000001)
        WaveSummaryData.N = WaveSummaryData.N + 1
        WaveSummaryData.duration(dt)
        WaveSummaryData.wv( 0.5 * ((D0 / dt) + (D1 / dt)) )
        WaveSummaryData.pc(pc)
      end
    end    
    return evasion(prey, pred)
  end
end


