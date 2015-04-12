print("  sourcing Enums.lua")


StartAttacks =
{
  Manual = 0,
  Auto = 1,
  Evolve = 2,
}


PreySelections = 
{
  Auto = 0,			  -- Topo in C++
  Picked = 1,
  PickedTopo = 2,
}

EvasionStrategies =
{
  MaximizeDist = 0,    -- maximize distance of closest approach
  TurnInward = 1,      -- turn along circularity vector
  TurnAway = 2,        -- Turn in opposite direction (Chris)
  Drop = 3,
  MoveCentered = 4,
  Zig = 5,
  Custom = 6,
  None = 7,
}

-- Used as bitset
PredationReaction =
{
  None = 0,
  Return = 1,
  Alerted = 2,
  Panic = 4,
  Detectable = 8,
}


-- GPWP
GPWS = {
  Default = 0,      -- build in
  Custom = 1,
}


FMappings =
{
  Correlation = 0,
  SubflockPCA = 1,
  Subflock = 2,
  VoxelVolume = 3,
  TimeSeries = 4,
  NN = 5,
  Qm = 6,
  PredatorVid = 7,
  EvolveDeflection = 8,
  Custom = 9,
  Default = 10,
  GlobalVelDev = 11,
  GlobalSpeedDev = 12,
  GlobalPolDev = 13,
  GlobalFrobenius = 14,
  Speed = 15,
  LocalDensity = 16,
  NND = 17,
  NNDInterior = 18,
  NNDBorder = 19,
  LateralGForce = 20,
  SeparationForce = 21,
  CohesionForce = 22,
  SpacingForce = 23,
  SteeringForce = 24,
  TotalForce = 25,
  Accel = 26,
  Polarization = 27,
  SpeedDev = 28,
  SpeedDevSign = 29,
  Bearing = 30,
  Elevation = 31,
  Border = 32,
  Bearing3D = 33,
  Drag = 34,
  EffectiveLift = 35,
  Vario = 36,
  Altitude = 37,
  Banking = 38,
  Topo = 39,
  Perception = 40,
  HorizDist = 41,
  AbsBanking = 42,
  RollRate = 43,
  CSRatio = 44,
  Circularity = 45,
  AveNND = 46,
  PredatorDetection = 47,
  PredatorReaction = 48,
  AlertnessRelexation = 49,
  ReturnReaxation = 50,
  NPredD = 51,
  NPreyD = 52,
  BodyMass = 53,
  SeparationRadius = 54,
  FlockId = 55,
  PredatorStats = 56,
  DDistanceDt = 57,
  ReactionTime = 58,
  Frobenius = 59,
  Id = 60,
  Debug = 61
}


ReverseFMappings = {}

for k, v in pairs(FMappings) do
  ReverseFMappings[v] = k
end




