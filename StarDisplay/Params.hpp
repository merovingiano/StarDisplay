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

  struct NepGLM
  {
	  float x;
	  float y;
	  float z;

  };

  struct Trajectory
  {
	  glm::vec3 Pred_position;
	  glm::vec3 Prey_position;
	  glm::vec3 Pred_up;
	  glm::vec3 Prey_up;
	  glm::vec3 Pred_forward;
	  glm::vec3 Prey_forward;
	  glm::vec3 Pred_acc;
	  glm::vec3 Prey_acc;
	  glm::vec3 pred_velocity;
	  glm::vec3 prey_velocity;
	  float Pred_id;
	  float Pred_gen;
	  float Prey_id;
	  double time;
  };

  struct DataStorage
  {
	  std::string folder;
  };
  class Vec3 : public glm::vec3
  {
	  float q;

  public:
	  float basis_;
  };




  struct poezie
  {
	  NepGLM jezus;
  };
	  
  struct poep
  {
	  float vlees;
	  float canaries;
	  NepGLM wonderDokter;
	  Vec3 trein;
	  glm::vec3 trein2;
	  poezie waarom;
	  glm::vec3 trein3;
  };


  struct Birds
  {
	  std::string csv_file_species;
	  std::string csv_file_prey_predator_settings;
  };

  struct Trajectories
  {
	  int amount;
	  float dt;
  };

  struct Evolution
  {
	  std::string type;
	  std::string fileName;
	  float durationGeneration;
	  int startGen;
	  bool load;
	  std::string loadFolder;
	  Trajectories Trajectories;
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
	int generation;
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
	glm::vec3 InitialPosition;
	glm::vec3 InitialHeading;
	float InitialSpeed;
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
	bool StoreTrajectory;
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
	
	float DPAdjParam;
	float N;
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
	bool turnOffGraphics;
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
	NepGLM draagkracht;
    double ClusterDetectionTime;
    float ClusterDistance1D;
	DataStorage DataStorage;
    float ClusterDistance3D;
    std::vector<std::pair<std::string, std::string> > Fonts;  // name filename pair
    glm::vec3 TextColor;
    glm::vec3 TextColorAlt;
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
