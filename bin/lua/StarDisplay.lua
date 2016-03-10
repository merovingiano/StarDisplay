print("  sourcing StarDisplay.lua")

-- some limits
-- 
Simulation.maxPrey = 50000
Simulation.maxPredators = 10000
Simulation.maxTopologicalRange = 16     -- memory optimization, keep it small


-- Maximum number of threads.
Simulation.concurrency_num_threads = Simulation.hardware_concurrency


-- Set concurrency_dynamic to false if you (really) need repeatability.
Simulation.concurrency_dynamic = true       -- dynamic adjustment (see OpenMP definition)


Simulation.DebugLevel = 0
Simulation.DebugLogOnce = true
Simulation.DebugLogStackLevel = 0


Simulation.IntegrationTimeStep = 0.0001  -- [s]
Simulation.slowMotion = 10              -- slowdown factor
Simulation.pausedSleep = 0              -- sleep time if paused [microseconds]
Simulation.realTime = true
Simulation.maxSkippedFrames = 50


-- Full screen antialiasing modes
local FSAAModes = {
--Name = { color samples, coverage samples }  
  Off = { 0, 0 }, 
  MSAA4 = { 4, 0 },
  MSAA8 = { 8, 0 },
  CSAA8x = { 4, 8 },
  CSAA8xQ = { 8, 8 },
  CSAA16x = { 4, 16 },
  CSAA16xQ = { 8, 16 },
}


Simulation.FSAA = FSAAModes.CSAA8x
Simulation.swap_control = 0             -- see WGL_EXT_swap_control


Simulation.Fonts = {
  Faces = {
    smallface = "Verdana14.fnt",
    mediumface = "Verdana16.fnt",
    bigface = "Verdana18.fnt",
    monoface = "Lucida12.fnt",
  },
  TextColor = glm.vec3(0.18, 0.18, 0.18),
  TextColorAlt = glm.vec3(0.75, 0.75, 0.75),
}


Simulation.Skybox = {
  name = "BlueishSky",
  ColorCorr = glm.vec3(1.0, 1.0, 1.0),
  ColorCorrAlt = glm.vec3(0.1, 0.1, 0.1),
  fovy = 45.0
}


Simulation.ModelSet = {
  { name = "Delta", LOD = { {acFile = "delta.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.0 }, 
  --{ name = "Delta", LOD = { {acFile = "delta.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.0 }, 
  --{ name = "Delta", LOD = { {acFile = "delta.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.0 }, 
  --{ name = "Peregrine", LOD = { {acFile = "peregrine.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.0 }, 
  -- { name = "DSMALLFILLthreePeregrineUV", LOD = { {acFile = "DSMALLFILLthreePeregrineUV.ac", pxCoverage = 0} }, Scale = 1.2, texMix = 0.0 }, 
  --{ name = "eagle2", LOD = { {acFile = "eagle2.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.0 }, 
  { name = "eagle3", LOD = { {acFile = "eagle3.ac", pxCoverage = 0} }, Scale = 0.7, texMix = 0.0 }, 
  { name = "Dove", LOD = { {acFile = "dove.ac", pxCoverage = 0} }, Scale = 1.6, texMix = 0.9 }, 
  --{ name = "Eagle", LOD = { {acFile = "eagle.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.9 }, 
   { name = "Humbird", 
    LOD = { 
      {acFile = "humbirdLOD1.ac", pxCoverage = 100},
      {acFile = "humbirdLOD2.ac", pxCoverage = 50},
      {acFile = "humbirdLOD3.ac", pxCoverage = 0},
    },
    { name = "Blancer", LOD = { {acFile = "blancer.ac", pxCoverage = 0} }, Scale = 0.165, texMix = 0.9 }, 
  { name = "Rabbit", LOD = { {acFile = "rabbit.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.9 }, 

    Scale = 1.0, texMix = 0.9 
  },  
  { name = "Condor", LOD = { {acFile = "condor.ac", pxCoverage = 0} }, Scale = 1.5, texMix = 0.9 },
  --{ name = "Sphere", LOD = {}, Scale = 1.0, texMix = 1.0 },

}


-- Bootstraped from StarDisplay.exe
--
function Simulation.Initialize (ExePath, ConfigFile, exeFolder)
  local sim = Simulation
  sim.Version = "6.3.0.0"
  sim.WorkingPath = ExePath .. "\\..\\..\\"
  sim.exeFolder = exeFolder
  sim.LuaPath = sim.WorkingPath .. "lua\\"
  sim.DataPath = sim.WorkingPath .. "data\\"
  sim.MediaPath = sim.WorkingPath .. "media\\"
  sim.ShaderPath = sim.WorkingPath .. "shader\\"
  sim.ConfigFile = sim.WorkingPath .. (ConfigFile or "config.lua")
  package.path = package.path .. ";" .. sim.LuaPath .. "?.lua"
  package.path = package.path .. ";" .. sim.WorkingPath .. "?.lua"
  dofile(sim.LuaPath .. "Help.lua")
  dofile(sim.LuaPath .. "Enums.lua")
  dofile(sim.LuaPath .. "KeyHooks.lua")
  dofile(sim.LuaPath .. "MouseHooks.lua")
  random:seed(os.time())
end


function InitHook ()
  print("\nFunction InitHook not defined.")
  return true
end


function Simulation.CheckVersion (Version)
  local sim = Simulation;
  if sim.Version ~= Version then
    error("Version mismatch: expecting " .. sim.Version .. " got " .. Version, 2)
  end
end


function Simulation.OutputFileName (mapping)
  local sim = Simulation
  local t = os.date("_%d-%m-%y_%H-%M-%S")
  if type(mapping) == "number" then
    mapping = ReverseFMappings[mapping]
  end
  local file = sim.DataPath .. mapping .. tostring(t) .. ".dat"
  return { name = file, append = false }
end


