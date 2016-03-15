#ifndef STARDISPLAY_PARAMS_HPP_INCLUDED
#define STARDISPLAY_PARAMS_HPP_INCLUDED

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <luabind/luabind.hpp>


namespace Param {

  struct Roost
  {
    unsigned numPrey;
    unsigned numPredators;
    float Radius;
    float minRadius;
    float maxRadius;
  };

  struct Birds
  {
	  std::string csv_file_species;
	  std::string csv_file_prey_predator_settings;
  };


  struct Evolution
  {
	  std::string type;
	  std::string fileName;
	  float durationGeneration;
	  int startGen;
	  bool load;
	  std::string loadFolder;
	  bool evolvePN;
	  bool evolveDPAdjParam;
	  bool evolveAlt;
	  bool evolveX;
	  bool evolveZ;
	  bool evolveCL;
	  bool evolvewingAspectRatio;
	  bool evolvewingBeatFreq;
	  bool evolvetheta;
	  bool evolvewingLength;
	  bool evolvebodyArea;
	  bool evolvecBody;
	  bool evolvecFriction;
	  bool evolvemaxForce;
	  bool evolvewingSpan;
	  bool evolvebodyMass;
	  bool evolvewingMass;
	  bool evolveInertiaWing;
	  bool evolveInertiaBody;
	  bool evolveJ;
	  bool evolvecontrolCL;
	  bool evolvecruiseSpeed;
	  bool evolvemaxLift;
	  bool evolvemaxSpeed;
	  bool evolverollRate;
	  bool evolveminSpeed;
	  bool evolvereactionTime;
	  bool evolvealignmentWeight;
	  bool evolvecohesionWeight;
	  bool evolveHandleTime;
	  bool evolveLockDistance;
	  bool TrajectoryBestPredator;
	  bool TrajectoryPrey;
	  int terminationGeneration;
	  bool externalPrey;
	  std::string externalPreyFile;
	  std::string title;
	  std::string description;
  };





  struct vaderJacob
  {
	  float Eitje;
  };

  struct ModelLod {
    std::string acFile;
    float pxCoverage;
  };


  struct ModelDef { 
    std::string name;
    std::vector<ModelLod> LOD;
    float Scale;
    float texMix;
  };


  struct FeatureMap 
  {
    enum FMappings 
    {
      Correlation,
      SubflockPCA,
      Subflock,
      VoxelVolume,
      TimeSeries,
      NN,
      Qm,
      PredatorVid,
      EvolveDeflection,
      Custom,
      Default,
      GlobalVelDev,
      GlobalSpeedDev,
      GlobalPolDev,
      GlobalFrobenius,
      Speed,
      LocalDensity,
      NND,
      NNDInterior,
      NNDBorder,
      LateralGForce,
      SeparationForce,
      CohesionForce,
      SpacingForce,
      SteeringForce,
      TotalForce,
      Accel,
      Polarization,
      SpeedDev,
      SpeedDevSign,
      Bearing,
      Elevation,
      Border,
      Bearing3D,
      Drag,
      EffectiveLift,
      Vario,
      Altitude,
      Banking,
      Topo,
      Perception,
      HorizDist,
      AbsBanking,
      RollRate,
      CSRatio,
      Circularity,
      AveNND,
      PredatorDetection,
      PredatorReaction,
      AlertnessRelexation,
      ReturnReaxation,
      NPredD,
      NPreyD,
      BodyMass,
      SeparationRadius,
      FlockId,
      PredatorStats,
      DDistanceDt,
      ReactionTime,
      Frobenius,
      Id,
      Debug,
      MaxFeatureMapping__
    } current;


    struct hist {
      float min;
      float max;
      int bins;
    };


    struct Entry {
      bool enable;
      double dt;         
      hist hist;        
      std::string title;      
      float p[5];
      bool colored;
    };

    double histKeepPercent;
    typedef std::vector<Entry> Entries_type;
    Entries_type Entries;

    // aberration to something frequently used
    const Entry& gFME(FMappings m) const { return Entries[m]; }
    const Entry& gCFME() const { return Entries[current]; }
    Entry& gFME(FMappings m) { return Entries[m]; }
    Entry& gCFME() { return Entries[current]; }

    bool ColorMapping() const { return Entries[current].colored; }
  };


  struct Skybox {
    std::string name;
    glm::vec3 ColorCorr;
    glm::vec3 ColorCorrAlt;
    float fovy;
  };


  struct GPWS {
    GPWS() : type(0), threshold(0) {}
    int type;
    float threshold;
    float tti;
    float lift;
    luabind::object hook;
  };


  struct Bird {
    float reactionTime;
    float reactionStochastic;
    int skipLeftHemisphere;       
    int skipRightHemisphere;
    float rho;
    float bodyMass;
	float wingMass;
	float InertiaWing;
	float InertiaBody;
	float J;
    float bodyWeight;
    float wingSpan;
    float wingAspectRatio;
    float wingArea;
	float wingBeatFreq;
	std::string birdName; 
	float theta;
	float wingLength;
	float bodyArea;
	float cBody;
	float cFriction;
    float CL;
    float maxForce;
    float maxLift;
    float cruiseSpeed;
    float speedControl;
	float houjebek;
    float maxSpeed;
	float rollRate;
    float minSpeed;
    glm::vec3 wBetaOut;
    glm::vec3 wBetaIn;
    float maxRadius;
    float neighborLerp;
    float topologicalRange;
    float circularityInc;
    float binocularOverlap;
    float blindAngle;
    int maxSeparationTopo;
    glm::vec2 separationStep;
    glm::vec3 separationWeight;
    glm::vec2 alignmentWeight;
    glm::vec3 cohesionWeight;
    float randomWeight;
    glm::vec3 boundaryWeight;
    float boundaryReflectAngle;
    float outerBoundary;
    float innerBoundary;
    float altitude;
    GPWS gpws;
	int wingRetractionSpeed;
	int maneuver;
	bool controlCL;
	float CDCL;
  };


  struct Panic {
    Panic() : type(0), weight(0), edges(0) {}
    int type;
    float weight;
    glm::vec4 edges;
    luabind::object hook;
  };


  struct Prey {
    enum EvasionStrategies
    {
		//! important: when adding new evasion strategies, don't forget to change the names as well!
      MaximizeDist,     // maximize distance of closest approach
      TurnInward,       // turn along circularity vector
      TurnAway,         // Turn in opposite direction (Chris)
      Drop,             // Drop out of sky
			MoveCentered,			// Move towards center
      Zig,              // Left-Right evasion. Parameter edge is reinterpreted as (TirgDist, t_left, t_right, t_handle)
      Custom,	  			  // Lua
      MaxEvasionStrategy__
    };

    Panic EvasionStrategy[MaxEvasionStrategy__];
	int EvasionStrategyTEMP;
    bool DetectCruising;
    float DetectionDistance;
    float DetectionSurfaceProb;
    float DetectionHemisphereFOV;
    int IncurNeighborPanic;
    double IncurLatency;
    glm::vec2 AlertnessRelexation;
    float AlertedReactionTimeFactor;
    glm::vec3 AlertedWBetaIn;
    glm::vec3 AlertedWBetaOut;
    float AlertedTopo;
    glm::vec2 AlertedAlignmentWeight;
    bool Return2Flock;
    float ReturnRelaxation;
    glm::vec3 ReturnWeight;
    glm::vec2 ReturnThreshold;
  };


  struct Pursuit {
    Pursuit() : type(0), deflection(0) {}
    int type;
    glm::vec3 deflection;
    luabind::object hook;
  };


  struct Predator {
    enum StartAttacks
    {
      Manual = 0,
      Auto = 1,
      Evolve = 2,
      MaxAttackStrategy__
    } StartAttack;

    enum PreySelections
    {
      Topo = 0,
      Picked = 1,
      PickedTopo = 2,
      MaxPreySelection__
    } PreySelection;

    enum PursuitStrategies
    {
      CUSTOM = 0,
	  ProportionalNavigation = 1,
	  DirectPursuit = 2,
	  DirectPursuit2 = 3,
	  PNDP = 4,
      MaxPursuitStrategy__
    };

    Pursuit pursuit;
	int PursuitStrategy;
    glm::vec3 AttackWBetaIn;
    glm::vec3 AttackWBetaOut;
    float CatchDistance;
    float AttackSpan;
    float Dogfight;
    bool HoldLock;
    float LockBlindAngle;
    float LockDistance;
    int maxLocks;
    glm::vec2 ExposureThreshold;
    float AttractMix;
    float HandleTime;
	float VisualError;
	glm::vec2 VisualBias;
  };


	struct RenderFlags {
    bool show_local;
    bool show_head;
    bool show_search;
    bool show_forces;
    bool show_neighbors;
    bool show_pred;
    bool show_circ;
    bool show_trails;
    bool show_world;
    bool show_rulers;
    bool show_annotation;
    bool show_hist;
    bool show_numbers;
    bool show_header;
    bool show_fps;
    bool wireFrame;
    bool altBackground;
    bool alphaMasking;
    bool slowMotion;
    int rtreeLevel;
    std::string helpMsg;
  };


  struct Params {
    int DebugLevel;
    bool DebugLogOnce;
    int DebugLogStackLevel;
    int FSAA[2];
    int swap_control;
    double IntegrationTimeStep;
    int slowMotion;
    int pausedSleep;
    bool realTime;
    int maxSkippedFrames;
    unsigned maxPrey;
    unsigned maxPredators;
    unsigned maxTopologicalRange;
    Roost roost;
	vaderJacob vaderJacob;
    double ClusterDetectionTime;
    float ClusterDistance1D;
    float ClusterDistance3D;
    std::vector<std::pair<std::string, std::string> > Fonts;  // name filename pair
    glm::dvec3 TextColor;
    glm::dvec3 TextColorAlt;
    Skybox skybox;
    std::vector<ModelDef> ModelSet;
    float RulerTick;
    unsigned RulerMinFlockSize;
    double TrailLength;
    float TrailWidth;
    float TrailTickInterval;
    float TrailTickWidth;
    int   TrailSkip;
    FeatureMap featureMap;
    RenderFlags renderFlags;
	Evolution evolution;
	Birds     birds;
  };




  struct Experiment
  {
	  Params param;
	  Bird preyBird;
	  Bird predBird;
	  Predator pred;
	  Prey prey;
  };

}


#endif
