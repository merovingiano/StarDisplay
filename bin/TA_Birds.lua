print("  sourcing TA_Birds.lua")
-- TA_Birds.lua
--
-- A collection of bird parameterizations

local random = require "Random"
local pursuits = require "TA"
local gpws = require "gpws"
require "wave"


local rho = 1.2               -- air density [kg/m^3]


-- CL after Van Dijk (1964)
--
local CL = function (bird, alpha)
  alpha = alpha or 1
  local AR = bird.wingAspectRatio
  local piAR = math.pi * AR
  local CL = 2*math.pi * alpha /
             (1 + 2/AR + 16*(math.log(piAR) - 9/8) / (piAR * piAR))
  return CL
end


-- Allometric scaling with wing load
-- Alerstam et al PLOS Biol 5, 2007
--
local CruiseSpeed = function (bird)
  local wingLoad = bird.bodyMass * 9.81 / bird.wingArea
  local U =  4.8 * math.pow(wingLoad, 0.28)
  return U
end


local Birds = {
}


local PreySpecies = "Starling" 


function Birds.Starling (p)
  local bird = Params.Bird()
  local rtm, rts = 0.1, 0.01  -- maen & standard dev. of reaction time
  bird.reactionTime = random:normal_min_max(rtm, rts, math.max(0, rtm-4*rts), rtm+4*rts) -- [s]

  bird.rho = rho
  bird.bodyMass = 0.08        -- [kg]
  bird.wingSpan = 0.4         -- [m]
  bird.wingAspectRatio = 3.3
  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2]
  bird.CL = CL(bird)

  bird.maxForce = 1           -- max steering force [N]
  bird.maxLift = 1            -- [N]
  bird.cruiseSpeed = CruiseSpeed(bird)      -- [m/s]
  bird.minSpeed = 0.01
  bird.maxSpeed = 40
  
  -----------------------------------------------------------------------------
  -- Steering
  -----------------------------------------------------------------------------

  bird.wBetaIn = glm.vec3( 2, 0, 0 )    -- roll, pitch, yaw
  bird.wBetaOut = glm.vec3( 1, 0, 0 )   -- roll, pitch, yaw

  bird.maxRadius = 50.0     -- [m] 
  bird.topologicalRange = 15 -- 6.5
  bird.circularityInc = 0
  bird.neighborLerp = 0.1

  bird.blindAngle = 40                           -- full blind angle [deg]
  local separationREdge = random:uniform(2.3, 2.7)
  bird.separationStep = glm.vec2(0.4, separationREdge)       -- smootherstep(x,y,distance)
  bird.separationWeight = glm.vec3(1, 0.75, 1 )   -- forward, up, side
  bird.alignmentWeight = glm.vec2( 0.5, 0 )     -- usual, banking
  bird.cohesionWeight = glm.vec3( 1, 1, 1 ) -- forward, up, side
  bird.randomWeight = 0
    
  bird.boundaryWeight = glm.vec4(0.01, 0.005, 0.01)  -- horizontal-x, vertical-y (indiv.), horizontal-z
  bird.boundaryReflectAngle = 90                            -- [deg]
  bird.innerBoundary = 0.25   -- inner radius = (1 - innerBoundary) * Roost.Radius
  bird.altitude = 100         -- preferred altitude [m]
  bird.GPWS = { type = gpws.Default, threshold = 10, tti = 1, lift = 0.2 }

  -------------------------------------------------------------------------------
  -- Prey specifics
  -------------------------------------------------------------------------------
  local prey = Params.Prey()
  prey.DetectCruising = false              -- if false detect attacking predators only
  prey.DetectionDistance = 20.0            -- [m]
  prey.DetectionSurfaceProb = 0.30 
  prey.DetectionHemisphereFOV = 270        -- [deg]

  --prey.EvasionStrategy = { type = EvasionStrategies.MaximizeDist, weight = 2.0, edges = glm.vec4(5, 5, 10, 20) }
  --prey.EvasionStrategy = { type = EvasionStrategies.Drop, weight = 5.0, edges = glm.vec4(0, 2, 2, 2) }
  --prey.EvasionStrategy = { type = EvasionStrategies.TurnInward, weight = 1.0, edges = glm.vec4(0, 0, 15, 20) }
  --prey.EvasionStrategy = { type = EvasionStrategies.LeftRight, weight = 3, edges = glm.vec4(5, 0.25, 0.55, 3.0) }
  --prey.EvasionStrategy = { type = EvasionStrategies.Custom, hook = Wave(ChangeBodyaxisEx(2,10)) }
  
  prey.IncurNeighborPanic = bird.topologicalRange      -- absorb panic reaction from nth nearest neighbor
    
  prey.AlertnessRelexation = glm.vec2(2,2) -- time span to stay alerted [s]
  prey.AlertedReactionTimeFactor = 0.75     -- Reduction in reaction time while alerted (factor).
  prey.AlertedWBetaIn = bird.wBetaIn
  prey.AlertedWBetaOut = bird.wBetaOut
  prey.AlertedAlignmentWeight = bird.alignmentWeight
  prey.AlertedTopo = bird.topologicalRange
  
  prey.Return2Flock = false
  prey.ReturnRelaxation = 60      -- [s]
  prey.ReturnWeight = glm.vec3(0.05, 0.05, 0.05)
  prey.ReturnThreshold = glm.vec2( 
    25000,    -- relax in flocks of this size or bigger (overrides ReturnRelaxation)
    25000     -- remain alerted in flocks of less individuals (overrides ReturnRelaxation)
  )
  
  if p ~= nil then
    p.BirdParams = bird
    p.PreyParams = prey
  end
  return bird, prey
end


function Birds.Falcon (p)
  local bird = Params.Bird()
  bird.reactionTime = 0.01    -- [s]
  
  bird.rho = rho
  bird.bodyMass = 0.9         -- [kg]
  bird.wingSpan = 1.0         -- [m]
  bird.wingAspectRatio = 3.5
  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2]
  bird.CL = CL(bird)
  bird.maxForce = 2           -- max steering force [N]
  bird.maxLift = 2            -- [N}
  bird.cruiseSpeed = 20       --CruiseSpeed(bird)    -- [m/s]
  bird.minSpeed = 5
  bird.maxSpeed = 40
  
  -----------------------------------------------------------------------------
  -- Steering
  -----------------------------------------------------------------------------

  bird.wBetaIn = glm.vec3( 4, 1, 0 )    -- roll, pitch, yaw
  bird.wBetaOut = glm.vec3( 1, 0, 0 )   -- roll, pitch, yaw

  bird.maxRadius = 1000.0     -- [m] 
  bird.topologicalRange = 6.5   
  bird.neighborLerp = 0.025

  bird.blindAngle = 0                            -- full blind angle [deg]
  bird.hardcoreRadius = 10.0                      -- na.
  bird.separationRadius = 2                       -- na.
  bird.separationWeight = glm.vec3(0)             -- na.
  bird.alignmentWeight = glm.vec2(0)              -- na.
  bird.cohesionWeight = glm.vec3(0.5, 0.5, 0.5)   -- forward, up, side
  bird.randomWeight = 0.01
    
  bird.boundaryWeight = glm.vec4(0.005, 0.005, 0.005)  -- horizontal-x, vertical-y (indiv.), horizontal-z
  bird.boundaryReflectAngle = 180                           -- [deg]
  bird.innerBoundary = 0.5    -- inner radius = (1 - innerBoundary) * Roost.Radius
  bird.altitude = 100         -- preferred altitude [m]
  bird.GPWS = { type = gpws.Default, threshold = 10, tti = 1, lift = 0.1 }

  ---------------------------------------------------------------------------
  -- Predator specifics
  ---------------------------------------------------------------------------
  local predator = Params.Predator()
  predator.AttackWBetaIn = bird.wBetaIn;
  predator.AttackWBetaOut = bird.wBetaOut;
  predator.CatchDistance = 0.2                 -- [m]
  predator.StartAttack = StartAttacks.Manual   -- Manual, Auto, Evolve
  predator.PreySelection = PreySelections.Auto -- Auto, Picked, PickedTopo
  
  --predator.PursuitStrategy = { type = pursuits.Custom, hook = pursuits.DirectPursuit(0.5) }
  predator.PursuitStrategy = { type = pursuits.Custom, hook = pursuits.ProportionalNavigation(2) }

  predator.AttackSpan = 600                   -- max. attack time span w/o lock [s]
  predator.Dogfight = 1.0                     -- max. attack time span after first lock [s]
  predator.HoldLock = true 
  predator.LockBlindAngle = 360-45            -- [deg] 
  predator.LockDistance = 10.0                -- [m]
  predator.maxLocks = 1                       -- max locks per attack
  predator.ExposureThreshold = glm.vec2(0.35, 0.20) -- (lock on, lose lock)
  predator.AttractMix = 0.1                    -- mix between distance (1) and exposure - factor (0)
  predator.HandleTime = 10

  if p ~= nil then
    p.BirdParams = bird
    p.PredParams = predator
  end
  return bird, predator
end


return Birds

