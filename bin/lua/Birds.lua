print("  sourcing Birds.lua")
-- Birds.lua
--
-- A collection of bird parameterizations

local random = require "Random"
local pursuits = require "pursuits"
local gpws = require "gpws"




local rho = 1.2               -- air density [kg/m^3]


-- CL after Van Dijk (1964)
--
local CL = function (bird, alpha)
  alpha = alpha or 0.28
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

--drag to lift ratio

local CDCL = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird))
	return cdcl
end

local CDCL2 = function (bird)
	local cdcl = 1.0 / ((math.pi * bird.wingAspectRatio) / CL(bird,1))
	return cdcl
end

local maxForce = function (bird)
	local maxForce = (bird.maxSpeed^2 / bird.cruiseSpeed^2) * bird.bodyMass * 9.81 * (CDCL(bird) + bird.bodyDrag/CL(bird)) -- removed:  - CDCL(bird)* bird.bodyMass * 9.81 
	return maxForce
end 


local skipHemisphere = function (P)
  if random:uniform01() < P then
    return 0, 1
  else
    return 1, 0
  end
end

local Birds = {
}


function Birds.Starling (p)
  local bird = Params.Bird()

  -- Pomeroy 1977 AnimBehav: 76.38 ms startle reaction time

  local rtm, rts = 76.38 / 1000, 0.01     -- mean & standard dev. of reaction time
  bird.reactionTime = random:normal_min_max(rtm, rts, math.max(0, rtm-4*rts), rtm+4*rts) -- [s]
  bird.reactionStochastic = 0.005 -- reactionTime <- (1 + uniform(-reactionStochastic, + reactionStocastic)) * reactionTime
  
  bird.skipLeftHemisphere, bird.skipRightHemisphere = 0,0 --skipHemisphere(0.5)

  bird.rho = rho
  bird.bodyMass = 0.08        -- [kg]
  bird.wingSpan = 0.38         -- [m]
  bird.wingAspectRatio = 8.333
  bird.wingBeatFreq = 9.5
  bird.theta = 0.5*3.14
  bird.wingLength = 0.15
  bird.bodyArea = 0.001246019
  bird.cBody = 0.41
  bird.cFriction = 0.014
  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2]
  bird.CL = CL(bird,1)
  bird.CDCL= CDCL2(bird)
  bird.controlCL = false
  bird.bodyDrag = 0
  bird.wingRetractionSpeed = 100
  bird.maxForce = 2  -- max steering force [N] 
  bird.maxLift = 3      -- [N]
  bird.cruiseSpeed = CruiseSpeed(bird)      -- [m/s]
  -- for experiment
  bird.cruiseSpeed = 20
  bird.speedControl = 2		-- one over tau 
  bird.minSpeed = bird.cruiseSpeed -5
  bird.maxSpeed = bird.cruiseSpeed + 5
  bird.houjebek = 5
  bird.rollRate= 600
  
  -----------------------------------------------------------------------------
  -- Steering
  -----------------------------------------------------------------------------

  bird.wBetaIn = glm.vec3( 4, 100, 0 )    -- roll, pitch, yaw
  bird.wBetaOut = glm.vec3( 0, 0, 0 )     -- roll, pitch, yaw 

  bird.maxRadius = 200.0     -- [m] 
  bird.topologicalRange = 6.5
  bird.circularityInc = 0
  bird.neighborLerp = 0.1
   
  bird.binocularOverlap = 0               -- full angle [deg]
  bird.blindAngle = 36                    -- full blind angle [deg]
  
  bird.maxSeparationTopo = 7
  local separationREdge = 2.0
  bird.separationStep = glm.vec2(0.4, separationREdge)       -- smootherstep(x,y,distance)
  bird.separationWeight = 1 * glm.vec3(1, 1, 1)   -- forward, up, side
  bird.alignmentWeight = 1 * glm.vec2( 1, 1 )     -- usual, banking
  bird.cohesionWeight = 1 * glm.vec3( 1, 1, 1 )   -- forward, up, side
  bird.randomWeight = 0                 
    
  bird.boundaryWeight = glm.vec3(0.01, 0.05 , 0.01)  -- horizontal-x, vertical-y (indiv.), horizontal-z
  bird.boundaryReflectAngle = 180                           -- [deg]
  bird.innerBoundary = 0   -- inner radius = (1 - innerBoundary) * Roost.Radius
  bird.altitude = 120         -- preferred altitude [m]  @Rolf only used at initialisation
  --bird.GPWS = { type = gpws.Custom, threshold = 0, hook = Rolf.HelloWorld(bird.altitude) }
  
  -------------------------------------------------------------------------------
  -- Prey specifics
  -------------------------------------------------------------------------------
  local prey = Params.Prey()
  prey.DetectCruising = false              -- if false detect attacking predators only
  prey.DetectionDistance = 20.0            -- [m]
  prey.DetectionSurfaceProb = 0.30 
  prey.DetectionHemisphereFOV = 270        -- [deg]

  --prey.EvasionStrategy = { type = EvasionStrategies.Custom, hook = Wave(DropEx(5,10)) }
 -- prey.EvasionStrategy = { type = EvasionStrategies.MaximizeDist, weight = 5.0, edges = glm.vec4(0, 2, 2, 2) }


  prey.IncurNeighborPanic = 2        -- absorb panic reaction from nth nearest neighbor
  prey.IncurLatency = 0.05           -- absortion is possible after IncurLatency seconds
    
  prey.AlertnessRelexation = glm.vec2(1,1) -- time span to stay alerted [s]
  prey.AlertedReactionTimeFactor = 0.5     -- Reduction in reaction time while alerted (factor).
  prey.AlertedWBetaIn = bird.wBetaIn
  prey.AlertedWBetaOut = bird.wBetaOut
  prey.AlertedAlignmentWeight = bird.alignmentWeight
  prey.AlertedTopo = bird.topologicalRange
  
  prey.Return2Flock = true
  prey.ReturnRelaxation = 60      -- [s]
  prey.ReturnWeight = 0.1 * glm.vec3(1, 1, 1)
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
  bird.reactionTime = 0.01          -- [s]
  bird.reactionStochastic = 0.000   -- reactionTime <- (1 + uniform(-reactionStochastic, + reactionStocastic)) * reactionTime
  bird.skipLeftHemisphere = 0       -- spacial hemisphere (not brain hemisphere)
  bird.skipRightHemisphere = 0      -- spacial hemisphere (not brain hemisphere)

  bird.rho = rho
  bird.bodyMass = 0.55         -- [kg]
  bird.wingSpan = 0.873         -- [m]
  bird.wingAspectRatio = 7.92
  bird.wingBeatFreq = 5.1
  bird.theta = 0.5*3.14
  bird.wingLength = 0.285
  bird.bodyArea = 0.0058
  bird.cBody = 0.14
  bird.cFriction = 0.014
  bird.wingArea = bird.wingSpan * (bird.wingSpan / bird.wingAspectRatio)   -- [m^2]
  bird.CL = CL(bird)
  bird.CDCL= CDCL(bird)
  bird.controlCL = false
  bird.bodyDrag = 0.017
  bird.wingRetractionSpeed = 59
  bird.maxLift = 40           -- [N}
  
  bird.cruiseSpeed = 20          -- CruiseSpeed(bird)    -- [m/s]
  --bird.speedControl = 1 / 1000   -- one over tau 
  bird.minSpeed = 5
  bird.maxSpeed = 40
  bird.rollRate=10
  bird.maxForce = maxForce(bird)          -- max steering force [N]


  -----------------------------------------------------------------------------
  -- Steering
  -----------------------------------------------------------------------------

  bird.wBetaIn = glm.vec3( 1, 1, 0 )    -- roll, pitch, yaw  
  bird.wBetaOut = glm.vec3( 0, 0, 0 )   -- roll, pitch, yaw

  bird.maxRadius = 1000.0     -- [m] 
  bird.topologicalRange = 6.5   
  bird.neighborLerp = 0.025

  bird.binocularOverlap = 0             -- full angle [deg]
  bird.blindAngle = 0                   -- full blind angle [deg]
  
  bird.hardcoreRadius = 10.0                      -- na.
  bird.separationRadius = 2                       -- na.
  bird.separationWeight = glm.vec3(0)             -- na.
  bird.alignmentWeight = glm.vec2(0)              -- na.
  bird.cohesionWeight = glm.vec3(1, 1, 1)   -- forward, up, side
  bird.randomWeight = 0
    
  bird.boundaryWeight = glm.vec3(0.005, 0.00001 , 0.005)  -- horizontal-x, vertical-y (indiv.), horizontal-z
  bird.boundaryReflectAngle = 180                           -- [deg]
  bird.innerBoundary = 0    -- inner radius = (1 - innerBoundary) * Roost.Radius
  bird.altitude = 100         -- preferred altitude [m]
  --bird.GPWS = { type = gpws.Default, threshold = 25, tti = 1, lift = 1 }

  ---------------------------------------------------------------------------
  -- Predator specifics
  ---------------------------------------------------------------------------
  local predator = Params.Predator()
  predator.AttackWBetaIn = bird.wBetaIn;
  predator.AttackWBetaOut = bird.wBetaOut;
  predator.CatchDistance = 0.2                -- [m]
  predator.StartAttack = StartAttacks.Manual   -- Manual, Auto, Evolve
  predator.PreySelection = PreySelections.Picked -- Auto, Picked, PickedTopo
  
  --predator.PursuitStrategy = { type = pursuits.Custom, hook = pursuits.DirectPursuit(10) }
  predator.PursuitStrategy = { type = pursuits._ProportionalNavigation, hook = pursuits.ProportionalNavigation(5) }

  predator.AttackSpan = 6000                   -- max. attack time span w/o lock [s]
  predator.Dogfight = 20000                  -- max. attack time span after first lock [s]
  predator.HoldLock = true 
  predator.LockBlindAngle = 360-45          -- [deg] 
  predator.LockDistance = 10.0                -- [m]
  predator.maxLocks = 10000                       -- max locks per attack
  predator.ExposureThreshold = glm.vec2(0.35, 0.20) -- (lock on, lose lock)
  predator.AttractMix = 0.1                   -- mix between distance (1) and exposure - factor (0)
  predator.HandleTime = 100

  if p ~= nil then
    p.BirdParams = bird
    p.PredParams = predator
  end
  return bird, predator
end




return Birds

