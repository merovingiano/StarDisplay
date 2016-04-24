print("  sourcing Defaults.lua")


local Default = {

  Roost = {
    numPrey = 200,
    numPredators = 0,
    Radius = 100.0,
    minRadius = 50.0,
    maxRadius = 500.0,
  },


  Clustering = {
    DetectionTime = 0.1,  -- mandatory clustering [s];
    Distance1D = 3,       -- distance along hilbert curve (preclustering) [m]
    Distance3D = 2        -- distance between pre-clusters [m]
  },


  Ruler = {
    Tick = 1,        -- [m]
    MinFlockSize = 1
  },

    evolution = {
		type ="PN",
		pred_fitness_criterion = "pred_stat.minDist",
		prey_fitness_criterion = nil,
		fileName = "default.txt",
		durationGeneration = 25.0,   -- length in seconds in which one duration hunts
		startGen = 0,				-- When no randomization at start is desired, set it to >0 (such as when loading an old one)
		load = false,
		loadFolder = "",
		TrajectoryBestPredator = false,
		TrajectoryPrey = false,
		externalPrey = false,
		externalPreyFile = "pos_lure_flight1.txt",
		title = "Default Title",
		description = "default description",
		terminationGeneration = 100000000,

		evolving_parameters = {
			{name = "predBird_params.InitialPosition.y", type = "normal", a = 0, b = 0.1, scale = true, initial = {min = 0, max = 600}, },
		    {name = "predBird_params.InitialPosition.x", type = "normal", a = 0, b = 0.1, scale = true, initial = {min = 0, max = 600}, },
			{name = "pred_params.N", type = "normal", a = 0, b = 0.1, scale = true, initial = {min = 0, max = 100}, },
		
		},

		to_be_saved = {
		   {"pred_stat.minDist"},
		   {"pred_stat.velocityMinDist"},
		   {"pred_stat.seqTime"},
		   {"predBird_params.generation"},
		},

		random_variables = {
		  {name = "preyBird_params.reactionTime",  type = "normal", a = 0.07, b = 0.001},
		  {name = "preyBird_params.InitialHeading",  type = "vec_in_sphere",},
		},
  },

  Birds = {
     csv_convertor = "..\\..\\lua\\XlsToCsv.vbs",
     csv_file_species_xlsm = "lua\\bird_properties.xlsm",
     csv_file_species = "../../lua/bird_properties.csv",
	 csv_file_prey_predator_settings = "../../lua/prey_predator_settings.csv",
  },

  DataStorage = {
     folder = "D:\\ownCloud\\2013-2014\\phd hunting\\dataStarDisplay\\",
  
  },

  Trail = {
    Length = 5,           -- length in seconds (read-once)
    Width = 0.25,          -- [m]
    TickInterval = 0.5,   -- tick each trailTickInterval (s)
    TickWidth = 0.05,     -- fraction of trailTickInterval
    Skip = 2              -- skiped integration steps
  },


  RenderFlags = {
    show_local = false,
    show_head = false,
    show_search = false,
    show_forces = false,
    show_neighbors = false,
    show_pred = false,
    show_circ = false,
    show_trails = false,
    show_world = false,
    show_rulers = false,
    show_annotation = true,
    show_hist = true,
    show_numbers = false,
    show_header = true,
    show_fps = true,
    wireFrame = false,
    altBackground = false,
    alphaMasking = false,
    slowMotion = false,
    rtreeLevel = 0,
    helpMsg = "",
	turnOffGraphics = false,
  },

}



-----------------------------------------------------------------------------
-- Analysis parameterisation & additional visualisation
-----------------------------------------------------------------------------

Default.FeatureMap = {
  histKeepPercent = 99,    -- per statistic dt
  current = FMappings.Default, 
  Entries = { 
  --
  --  hist = { min, max, bins }
  --
    { title = "Correlations of fluctuations", enable = true,  dt = 0.1, hist = { 0, 0, 200 }, p = { 70, 2.1 }, colored = true },
    { title = "subflock PCA",                 enable = true,  dt = 0.1, hist = { 0, 90, 90 }, p = { 4, 1 }, colored = false },
    { title = "subflock detection",           enable = true,  dt = 0.1, hist = { 0, 1, 200 }, p = { 4 }, colored = false },

    { title = "Voxel Volume [m^3]",           enable = true,  dt = 0.05, hist = { 0, 30000, 1000 }, p = { 2.1 }, colored = false },
    { title = "TimeSeries",                   enable = false, dt = 1.0, hist = { 0, 0, 100 },  p = { 1 }, colored = false },
    { title = "NN",                           enable = true,  dt = 0.1, hist = { 0, 5.0, 100 }, p = { 20, 20 }, colored = false },
    { title = "Qm",                           enable = true,  dt = 0.1, hist = { 0, 5.0, 100 }, p = {}, colored = false },
    { title = "Predator Video analysis",      enable = false, dt = 0.02, hist = { 0, 0, 100 }, p = { 0 }, colored = false },
    { title = "Evolve deflection",            enable = false, dt = 0.1, hist = { 0, 0, 100 }, p = { 30, 0.1, 0.1, 0.0 }, colored = false },
    { title = "Custom",                       enable = true,  dt = 0.01, hist = { 0, 0, 0 }, p = { }, colored = false },
    { title = "Default",                      enable = true,  dt = 0, hist = { 0, 1, 100 }, p = {}, colored = false },
  
    { title = "global velocity deviations",   enable = true,  dt = 0.1, hist = { 0, 1.5, 100 }, p = { 500, 1000 }, colored = true },
    { title = "global speed deviations [m/s]",enable = true,  dt = 0.1, hist = { -1, 1, 100 }, p = { 500 }, colored = true },
    { title = "global polarization",          enable = true,  dt = 0.1, hist = { 0.9, 1.0, 100 }, p = { 500 }, colored = true },
    { title = "global frobenius distance",    enable = true,  dt = 0.1, hist = { 0, 0.5, 100 }, p = { 500 }, colored = true },
    
    { title = "speed [m/s]",                  enable = true,  dt = 0, hist = { 0, 20, 100 }, p = {}, colored = true },
    { title = "local density [1/m^3]",        enable = true,  dt = 0, hist = { 0, 1.0, 100 }, p = {}, colored = true },

    { title = "NND [m]",                      enable = true,  dt = 0, hist = { 0, 4, 200 }, p = {}, colored = true },
    { title = "NND interior [m]",             enable = true,  dt = 0, hist = { 0, 4, 200 }, p = { 0.35 }, colored = true },
    { title = "NND border [m]",               enable = true,  dt = 0, hist = { 0, 4, 200 }, p = { 0.35 }, colored = true },
    
    { title = "lateral g-force [g]",          enable = true,  dt = 0, hist = { 0, 2, 100 }, p ={}, colored = true },
    { title = "separation force [N]",         enable = true,  dt = 0, hist = { 0, 0.5, 100 }, p ={}, colored = true },
    { title = "cohesion force [N]",           enable = true,  dt = 0, hist = { 0, 0.1, 100 }, p ={}, colored = true },
    { title = "spacing force [N]",            enable = true,  dt = 0, hist = { 0, 0.2, 100 }, p ={}, colored = true },
    { title = "steering force [N]",           enable = true,  dt = 0, hist = { 0, 1, 100 }, p ={}, colored = true },
    { title = "tot. force [N]",               enable = true,  dt = 0, hist = { 0, 1, 100 }, p ={}, colored = true },
    { title = "g-force [g]",                  enable = true,  dt = 0, hist = { 0, 10, 100 }, p ={}, colored = true },
    { title = "polarisation [cos(a)]",        enable = true,  dt = 0, hist = { 0.95, 1, 500 }, p ={}, colored = true },
    
    { title = "local speed deviation (abs.) [m/s]", enable = false, dt = 0, hist = { 0, 5, 100 }, p ={}, colored = true },
    { title = "local speed deviation (sign.) [m/s]",enable = false, dt = 0, hist = { -2, 2, 100 }, p ={}, colored = true },
    
    { title = "bearing [deg]",                enable = true, dt = 0, hist = { -180, 180, 100 }, p = { 0.35, 0, 0, 0, 0}, colored = true },
    { title = "elevation [deg]",              enable = false, dt = 0, hist = { -90, 90, 100 }, p = { 1.0, 0, 0, 0, 0}, colored = true },
    { title = "border detection [yes/no]",    enable = true,  dt = 0, hist = { -1, 1, 100 }, p = { 0.35, 0, 0, 0, 0}, colored = true },
    { title = "3D bearing [cos(a)]",          enable = true,  dt = 0, hist = { -1, 1, 100 }, p = { 0.25, 0, 0, 0, 0}, colored = true },
    { title = "drag [N]",                     enable = false, dt = 0, hist = { -1, 1, 100 }, p ={}, colored = true },
    { title = "effective lift [N]",           enable = true,  dt = 0, hist = { -1, 1, 100 }, p ={}, colored = true },
    { title = "variometer [m/s]",             enable = true,  dt = 0, hist = { -10, 10, 100 }, p ={}, colored = true },
    { title = "altitude [m]",                 enable = true,  dt = 0, hist = {  0, 250, 250 }, p ={}, colored = true },
    { title = "banking [deg]",                enable = true,  dt = 0, hist = { -45, 45, 100 }, p ={}, colored = true },
    { title = "Topological range",            enable = true,  dt = 0, hist = { 0, Simulation.maxTopologicalRange, Simulation.maxTopologicalRange + 1}, p ={}, colored = true },
    { title = "perception radius",            enable = true,  dt = 0, hist = { 0, 5, 100 }, p ={}, colored = true },
    { title = "horiz. distance to center [%]",enable = false, dt = 0, hist = { 0, 200, 100 }, p ={}, colored = true },
    { title = "abs. banking [deg]",           enable = false, dt = 0, hist = { 0, 90, 100 }, p ={}, colored = true },
    { title = "roll rate [deg/s]",            enable = true,  dt = 0, hist = { 0, 500, 200 }, p ={}, colored = true },
    { title = "CS ratio",                     enable = false, dt = 0, hist = { 0.0, 1.5, 100 }, p ={}, colored = true },
    { title = "circularity",                  enable = true,  dt = 0, hist = { 0, 1, 100 }, p ={}, colored = true },
    { title = "Ave. NND",                     enable = false, dt = 0, hist = { 0, 5, 100 }, p ={}, colored = true },
    
    { title = "Predator detection",           enable = true,  dt = 0, hist = { 0, 1.2, 10 }, p ={}, colored = true },
    { title = "Reaction to predator",         enable = true,  dt = 0, hist = { 0, 20, 21 }, p ={}, colored = true },
    
    { title = "Alertness relaxation time [s]",       enable = true,  dt = 0, hist = { 0, 5, 100 }, p ={}, colored = true },
    { title = "Return to flock relaxation time [s]", enable = true,  dt = 0, hist = { 0, 60, 100 }, p ={}, colored = true },

    { title = "nearest predator distance [m]",enable = true,  dt = 0, hist = { 0, 50.0, 100 }, p ={}, colored = true },
    { title = "nearest prey distance [m]",    enable = true,  dt = 0, hist = { 0, 20.0, 100 }, p ={}, colored = true },
    
    { title = "Body mass [kg]",               enable = true,  dt = 0, hist = { 0, 1, 100 }, p ={}, colored = true },

    { title = "Separation Radius [m]",        enable = true,  dt = 0, hist = { 0, 5, 100 }, p ={}, colored = true },
    { title = "Flock id",                     enable = true,  dt = 0, hist = { 0, 20, 20 }, p ={}, colored = true },
    { title = "Predator stats.",              enable = true,  dt = 0, hist = { 0, 1, 100 }, p ={}, colored = false },
    { title = "local d(Dist)/dt [m*m/s]",     enable = true,  dt = 0, hist = { -2, 2, 100 }, p ={}, colored = true },

    { title = "Reaction time [s]",            enable = true,  dt = 0, hist = { 0, 0.2, 100 }, p ={}, colored = true },

    { title = "NN frobenius distance",        enable = true,  dt = 0, hist = { 0, math.sqrt(8), 100 }, p ={}, colored = true },
    { title = "Id",                           enable = true,  dt = 0, hist = { 0, Default.Roost.numPrey, Default.Roost.numPrey }, p ={}, colored = true },
    { title = "dummy mapping",                enable = false, dt = 0, hist = { -2, 2, 100 }, p ={}, colored = true }
  }
}



return Default
