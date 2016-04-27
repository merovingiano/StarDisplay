#include <exception>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "filesystem.hpp"
#include "GLWin.hpp"
#include "Bird.hpp"
#include "Predator.hpp"
#include "Flock.hpp"
#include "Clock.hpp"
#include "Simulation.hpp"
#include "Flock.hpp"
#include "Camera.hpp"
#include "GLSLState.hpp"
#include "HistOverlay.hpp"
#include "DefaultStatistic.hpp"
#include "HistogramStatistic.hpp"
#include "VoxelVolumeStatistic.hpp"
#include "TimeSeriesStatistic.hpp"
#include "ClusterStatistic.hpp"
#include "NNStatistic.hpp"
#include "QmStatistic.hpp"
#include "GlobalDevStatistic.hpp"
#include "FCorrelationStatistic.hpp"
#include "PredatorStatistic.hpp"
#include "EvolveDeflection.hpp"
#include "EvolvePN.hpp"
#include "KeyState.hpp"
#include "libLua.hpp"
#include "debug.hpp"
#include "random.hpp"
#include "Mmsystem.h"
#include "libParam.hpp"
#include <glmutils/random.hpp>


using namespace Param;


static const char header_fmt[] =
  "\n[F1] Help\n[F2] Prey: %d + %d\n[F3] Boundary radius: %d\n[F4] HRTree level: %d\nh.hildenbrandt@rug.nl\n";

static const char footer_fmt[] =
  "Sim. time: %02.0f:%02.0f:%02.0f\nupdate %.4f s\nfps: %d";


namespace {

  void printExceptionMsg(const char* prefix)
  {
    std::cerr << (prefix ? prefix : "Unknown error") << ": ";
    const char* luaErr = Lua.ErrMsg();
    std::cerr << (luaErr ? luaErr : "") << '\n';
    debug::StackDump(PARAMS.DebugLogStackLevel);
    std::cerr << "\nBailing out\n";
  }

}


Simulation::Simulation()
: SimulationTime_(0.0),
  UpdateTime_(0.0),
  FrameTime_(0.1),
  statisticsPaused_(false),
  paused_(false),
  track_alpha_(false),
  lastStatTime_(0.0),
  lastClusterTime_(0.0),
  camera_(0)
{
}


Simulation::~Simulation()
{
  if (CustomStatistic_) CustomStatistic_->finalize();
}



void Simulation::SetParams(const Param::Params& param)
{
	params_ = param;
}

void Simulation::SetInitialParameter(const Param::Params& param)
{
  params_ = param;
	gl_.reset( new GLSLState() );
	gl_->Init(AppWindow.GetDC());
  flock_.reset( new CFlock(params_.roost.numPrey) );
  trails_.reset( new trail_buffer_pool() );
  CustomStatistic_.reset( new DefaultStatistic() );
  SetPFeatureMap(param.featureMap);
}


void Simulation::GetExperimentSettings(const luabind::object& obj)
{

	luabind::object experiments = obj;

	for (luabind::iterator i(experiments), end; i != end; ++i)
	{
		CFlock::prey_iterator Prey = GFLOCKNC.prey_begin();
		CFlock::pred_iterator Pred = GFLOCKNC.predator_begin();
		Param::Experiment experiment;
		int intKey = luabind::object_cast<int>(i.key());
		experiment.preyBird = luabind::object_cast<Param::Bird>(obj[intKey]["preyBird"]);
		experiment.predBird = luabind::object_cast<Param::Bird>(obj[intKey]["predBird"]);
		experiment.pred = luabind::object_cast<Param::Predator>(obj[intKey]["pred"]);
		experiment.prey = luabind::object_cast<Param::Prey>(obj[intKey]["prey"]);
		experiment.param = libParam::FromLua<Param::Params>(obj[intKey]["Param"]);	
		Sim.experiments.push_back(experiment);
	}
}

void Simulation::SetPFeatureMap(const Param::FeatureMap& featureMap)
{
  params_.featureMap = featureMap;
  SelectStatistic(featureMap.current);
}


void Simulation::SetPRoost(const Param::Roost& roost)
{
  params_.roost.maxRadius = roost.maxRadius;
  params_.roost.minRadius = roost.minRadius;
  params_.roost.Radius = roost.Radius;
  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.RoostChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.RoostChanged(); });
}


void Simulation::SetPRenderFlags(const Param::RenderFlags& flags)
{
  params_.renderFlags = flags;
}


void Simulation::RegisterFactories(const luabind::object& PreyFactory, const luabind::object& PredatorFactory)
{
  PreyFactory_ = PreyFactory;
  PredatorFactory_ = PredatorFactory;
}

void Simulation::RegisterDataStorage(const luabind::object& dataStorage)
{
	StorageData_ = dataStorage;
}

void Simulation::RegisterEvolution(const luabind::object& evolution_next)
{
	evolution_next_ = evolution_next;
}

void Simulation::RegisterCustomStatistic(IStatistic* sb)
{
  CustomStatistic_.reset(sb);
  if (params_.featureMap.current == FeatureMap::Custom)
  {
    statistic_ = CustomStatistic_;
    statistic_->Reset();
  }
}


luabind::object& Simulation::GetActiveCamera()
{ 
  return luaCamera_; 
}


void Simulation::SetActiveCamera(const luabind::object& luaobj)
{ 
  luaCamera_ = luaobj;
  camera_ = luabind::object_cast<ICamera*>(luaCamera_["cc"]);
}


void Simulation::deleteInvisibles()
{
  const CPrey* focalPrey = camera_->GetFocalPrey();
  int focalId = (focalPrey) ? focalPrey->id() : -1;
  flock_->remove_invisibles(camera_->ModelViewMatrix(), camera_->ProjectionMatrix());
  if (0 == (focalPrey = flock_->FindPrey(focalId)))
  {
    // the target individual was deleted.
    focalPrey = flock_->num_prey() ? &*(flock_->prey_begin()) : 0;
  }
  camera_->SetFocalPrey(focalPrey);
  params_.roost.numPrey = flock_->num_prey();
}


void Simulation::SelectStatistic(int selectedId)
{
  if (statistic_ && params_.featureMap.current == (FeatureMap::FMappings)selectedId)
  {
    return;
  }
  params_.featureMap.current = (FeatureMap::FMappings)selectedId;
  CustomStatistic_->finalize();
  params_.renderFlags.alphaMasking = false;

  // Statistic
  switch (params_.featureMap.current)
  {
  case FeatureMap::Default:
    statistic_.reset( new DefaultStatistic() );
    break;
  case FeatureMap::Correlation:
    statistic_.reset( new FCorrelationStatistic() );
    break;
  case FeatureMap::TimeSeries:
    statistic_.reset( new TimeSeriesStatistic() );
    break;
  case FeatureMap::NN:
    statistic_.reset( new NNStatistic() );
    break;
  case FeatureMap::Qm:
    statistic_.reset( new QmStatistic() );
    break;
  case FeatureMap::PredatorStats:
    statistic_.reset( new PredatorStatistic() );
    break;
  case FeatureMap::PredatorVid:
    statistic_.reset( new PredatorVidStatistic() );
    break;
  case FeatureMap::EvolveDeflection:
    statistic_.reset( new EvolveDeflection() );
    break;
  case FeatureMap::Custom:
    std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.setDefaultColorTex(); });
    std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.setDefaultColorTex(); });
    statistic_ = CustomStatistic_;
    statistic_->Reset();
    break;
  default: 
    {
      float fSize = static_cast<float>(flock_->num_prey());
      params_.featureMap.Entries[FeatureMap::Subflock].hist.max = fSize;

      switch (params_.featureMap.current)
      {
      case FeatureMap::SubflockPCA:
        statistic_.reset( new SubflockPCAStatistic() );
        break;
      case FeatureMap::Subflock:
        statistic_.reset( new SubflockStatistic() );
        break;
      case FeatureMap::VoxelVolume:
        statistic_.reset( new VoxelVolumeStatistic() );
        break;
      case FeatureMap::GlobalVelDev:
      case FeatureMap::GlobalSpeedDev:
      case FeatureMap::GlobalPolDev:
      case FeatureMap::GlobalFrobenius:
        statistic_.reset( new GlobalDevStatistic(params_.featureMap.current) );
        break;
      default:
        statistic_.reset( new HistogramStatistic(params_.featureMap.current) );
        break;
      }
    }
  }
  lastStatTime_ = SimulationTime_;
}


void Simulation::DisplayStatistic() const
{
  statistic_->Display();
}


void Simulation::ResetCurrentStatistic()
{
  lastStatTime_ = SimulationTime_;
  statistic_->Reset();
}


void Simulation::ResumeCurrentStatistic()
{
  lastStatTime_ = SimulationTime_;
  statisticsPaused_ = false;
}


void Simulation::PauseCurrentStatistic()
{
  statisticsPaused_ = true;
}


void Simulation::PrintVector(glm::vec3 input, std::string text)
{

	std::cout << "\n" << text << " " << input.x << " " << input.y << " " << input.z;
}

void Simulation::PrintVec2(glm::vec2 input, std::string text)
{

	std::cout << "\n" << text << " " << input.x << " " << input.y << " ";
}

void Simulation::PrintFloat(float input, std::string text)
{

	std::cout << "\n" << text << " " << input;
}

void Simulation::PrintString(std::string text)
{

	std::cout << "\n" << text;
}

void Simulation::SaveCurrentStatistic(const char* fname, bool append)
{
  statistic_->save(fname, append);
  
}


bool Simulation::HandleKey(unsigned key, unsigned ks)
{
  if (Lua.ProcessKeyboardHooks(key, ks))
  {
    return true;
  }
  bool handled = false;
  switch (key) 
  {
  case 'Z':
    {
      params_.realTime = !params_.realTime;
    }
  case 'M':  
    if (KEYSTATE_IS_CTRL(ks) || KEYSTATE_IS_SHIFT_CTRL(ks)) 
    {
      handled = true;
      unsigned& current = KEYSTATE_IS_SHIFT_CTRL(ks) ? gl_->currentPredatorModel() : gl_->currentPreyModel();
      if (++current >= params_.ModelSet.size()) current = 0;
      if (KEYSTATE_IS_SHIFT_CTRL(ks)) gl_->currentPredatorModel() = current;
      else gl_->currentPreyModel() = current;
      gl_->setAnnotation("3D model changed");
      gl_->LoadModels();
    } 
    break;
  case VK_UP:
	  //std::cout <<  "\nUp\n";//key up
  case VK_DOWN:
	  //std::cout << "\nUp\n";//key up
  case VK_RIGHT:
	  //std::cout << "\nUp\n";//key up
  case VK_LEFT:
	  //std::cout << "\nUp\n";//key up
  case 'R':
	  //std::cout << "\nUp\n";//key up
    if (KEYSTATE_IS_ALT_CTRL(ks)) 
    {
      handled = true;
      SimulationTime_ = lastStatTime_ = lastClusterTime_ = 0.0;
    } 
    break;
  case VK_OEM_102:
    if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks))
    {
      handled = true;
      gl_->alphaMaskCenter() += (KEYSTATE_IS_SHIFT(ks)) ? 0.005f : -0.005f;
      gl_->alphaMaskCenter() = glm::clamp(gl_->alphaMaskCenter(), 0.0f, 1.0f);
    }
    break;
  case VK_F2:
    if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_SHIFT_CTRL(ks))
    {
      handled = true;
      bool shift = KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_SHIFT_CTRL(ks);
      bool ctrl = KEYSTATE_IS_CTRL(ks) || KEYSTATE_IS_SHIFT_CTRL(ks);
      double n = static_cast<double>(flock_->num_prey());
      n = n + (shift ? +1.0 : -1.0);
      int dn = (ctrl || (n <= 0.0)) ? 1 : static_cast<int>((::pow(10.0, ::floor(::log10(n)))));
      dn = glm::clamp(dn, 1, 1000);
      unsigned newNum = std::max(0, static_cast<int>(flock_->num_prey() + (shift ? +dn : -dn)));
      setNumPrey(newNum);
    }
    break;
  case VK_F11:
    if (KEYSTATE_IS_SHIFT(ks))
    {
      handled = true;
      if (statisticsPaused_) { ResumeCurrentStatistic(); gl_->setAnnotation("Statistics resumed"); }
      else { PauseCurrentStatistic(); gl_->setAnnotation("Statistics paused"); } 
    }
    else if (KEYSTATE_IS_CTRL(ks)) 
    {
      handled = true;
      ResetCurrentStatistic();
      gl_->setAnnotation("Statistic reseted");
    }
    else if (KEYSTATE_IS_BLANK(ks))
    {
      handled = true;
      luabind::object t = Lua("OutputFileName")(params_.featureMap.current);
      SaveCurrentStatistic( luabind::object_cast<const char*>(t["name"]), 
                            luabind::object_cast<bool>(t["append"]) );
      gl_->setAnnotation("Statistic saved");
    }
    break;
  case VK_PAUSE: 
    handled = true;
    paused_ = !paused_; 
    gl_->setAnnotation(paused_ ? "Simulation paused" : ""); 
    break; 
  case VK_DELETE:
    if (KEYSTATE_IS_CTRL(ks)) 
    {
      handled = true;
      deleteInvisibles();
      gl_->setAnnotation("Invisible birds deleted");
    }
    break;
  }
  return handled;
}


const CBird* Simulation::PickNearestBird2Ray(const glm::vec3& ray_position, const glm::vec3& ray_direction)
{
  return flock_->pickNearest2Ray(ray_position, ray_direction);
}


const CBird* Simulation::PickById(int id, bool showTrail)
{
  const CBird* bird = flock_->FindPredator(id);
  if (0 == bird) bird = flock_->FindPrey(id);
  if (bird) SetFocalBird(bird, showTrail);
  return bird;
}


void Simulation::SetPredatorTarget(const CBird* bird)
{
  CPredator* focal = const_cast<CPredator*>(camera_->GetFocalPredator());
  if (focal)
  {
    focal->SetTargetPrey(static_cast<const CPrey*>(bird));
  }
}


void Simulation::SetFocalBird(const CBird* bird, bool showTrail)
{
  if (0 == bird) return;
  camera_->SetFocalBird(bird);
  const_cast<CBird*>(bird)->setTrail(showTrail);
}


void Simulation::setNumPrey(unsigned newNum)
{
  newNum = std::min(params_.maxPrey, newNum);
  const CPrey* focalPrey = camera_->GetFocalPrey();
  int focalId = (focalPrey) ? focalPrey->id() : -1;
  if (newNum > flock_->num_prey())
  {
    while (flock_->num_prey() < newNum)
    {
      CPrey* prey = luabind::object_cast<CPrey*>(PreyFactory_(flock_->nextID()));
      flock_->insert_prey(prey);
    }
  } 
  else 
  {
    while (flock_->num_prey() > newNum)
    {
      flock_->erase_prey();
    }
  }
  params_.roost.numPrey = flock_->num_prey();
  flock_->refresh();
  if (0 == (focalPrey = flock_->FindPrey(focalId)))
  {
    focalPrey = flock_->num_prey() ? &*(flock_->prey_begin()) : 0;
  }
  camera_->SetFocalPrey(focalPrey);

  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.NumPreyChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.NumPreyChanged(); });
}


void Simulation::setNumPredators(unsigned newNum)
{
  newNum = std::min(params_.maxPredators, newNum);
  const CPredator* focalPredator = camera_->GetFocalPredator();
  int focalId = (focalPredator) ? focalPredator->id() : -1;
  if (newNum > flock_->num_pred())
  {
    while (flock_->num_pred() < newNum)
    {
      //CPredator* pred = luabind::object_cast<CPredator*>(PredatorFactory_(flock_->nextID()));
	  // Robin temp
	  CPredator* pred = luabind::object_cast<CPredator*>(PredatorFactory_(flock_->nextID()));
      flock_->insert_pred( pred );
    }
  } 
  else 
  {
    while (flock_->num_pred() > newNum)
    {
      flock_->erase_pred();
    }
  }
  params_.roost.numPredators = flock_->num_pred();
  //flock_->refresh();
  if (0 == (focalPredator = flock_->FindPredator(focalId))) 
  {
     focalPredator = flock_->num_pred() ? &*flock_->predator_begin() : 0;
  }
  camera_->SetFocalPredator(focalPredator);

  std::for_each(flock_->prey_begin(), flock_->prey_end(), [] (CPrey& prey) { prey.NumPredChanged(); });
  std::for_each(flock_->predator_begin(), flock_->predator_end(), [] (CPredator& pred) { pred.NumPredChanged(); });
}


unsigned Simulation::getNumPrey() const
{ 
  return flock_->num_prey(); 
}


unsigned Simulation::getNumPredators() const
{ 
  return flock_->num_pred(); 
}


void Simulation::UpdateBirds(const float sim_dt)
{
  std::exception_ptr eptr;
  const bool setDefaultColorTex = !params_.featureMap.gCFME().colored;
# pragma omp parallel firstprivate(sim_dt)
  {
    try
    {
      const int Nprey = static_cast<int>(flock_->num_prey());
      const CFlock::prey_iterator firstPrey(flock_->prey_begin());
#     pragma omp for
      for (int i = 0; i < Nprey; ++i) 
      {
       // (*(firstPrey+i)).updateNeighbors(sim_dt, *flock_);
       // if (setDefaultColorTex) (*(firstPrey+i)).setDefaultColorTex();
      }

#     pragma omp for
      for (int i = 0; i < Nprey; ++i) 
      { 
        (*(firstPrey+i)).update(sim_dt, *flock_);
      }

      const int Npred = static_cast<int>(flock_->num_pred());
      const CFlock::pred_iterator firstPred(flock_->predator_begin());
#     pragma omp for nowait
      for (int i = 0; i < Npred; ++i) 
      { 
        //(*(firstPred+i)).updateNeighbors(sim_dt, *flock_);
        (*(firstPred+i)).update(sim_dt, *flock_);
        if (setDefaultColorTex) (*(firstPred+i)).setDefaultColorTex();
      }
    }
    catch (...)
    {
      std::lock_guard<std::mutex> lock(omp_exception_mutex_);
      eptr = std::current_exception();
    }
  }
  if (eptr != nullptr) std::rethrow_exception(eptr);
}


void Simulation::UpdateCurrentStatistic(double stat_dt)
{
  statistic_->apply(stat_dt);
}


void Simulation::next_experiment()
{
	Sim.expNumb++;
	Generation_ = 0;
	if (Sim.expNumb > Sim.experiments.size()) AppWindow.PostMessage(WM_CLOSE);
	CFlock::pred_iterator firstPred(GFLOCKNC.predator_begin());
	CFlock::pred_iterator lastPred(GFLOCKNC.predator_end());
	CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());
	CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());

	Param::Params p = Sim.experiments[Sim.expNumb - 1].param;
	Sim.SetParams(p);
	for (; firstPred != lastPred; ++firstPred)
	{
		firstPred->SetPredParams(Sim.experiments[Sim.expNumb - 1].pred);
		firstPred->SetBirdParams(Sim.experiments[Sim.expNumb - 1].predBird);
	}
	for (; firstPrey != lastPrey; ++firstPrey)
	{
		firstPrey->SetPreyParams(Sim.experiments[Sim.expNumb - 1].prey);
		firstPrey->SetBirdParams(Sim.experiments[Sim.expNumb - 1].preyBird);
	}
	std::cout << "\n Experiment number: " << Sim.expNumb;

	//save files of experiment
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d-%m-%Y", &tstruct);


	std::string bufS(Sim.params_.DataStorage.folder);
	bufS.append(buf);
	bufS.append("/");
	bufS.append(Sim.params_.evolution.fileName);
	bufS.append("/");
	std::string luaName("experiment.lua");
	std::string fnameTrunc(std::string(params_.evolution.fileName.c_str()).substr(0, std::string(params_.evolution.fileName.c_str()).find(".txt")));


	CreateDirectory(bufS.c_str(), NULL);
	CopyFile("../../experiments.lua", (bufS + luaName).c_str(), TRUE);
	CopyFile((Sim.experiments[Sim.expNumb - 1].param.birds.csv_file_prey_predator_settings).c_str(), (bufS +  "pred_prey.csv").c_str(), TRUE);
	CopyFile((Sim.experiments[Sim.expNumb - 1].param.birds.csv_file_species).c_str(), (bufS + "species.csv").c_str(), TRUE);

}

void Simulation::Initialize_birds()
{
	CFlock::pred_iterator firstPred(GFLOCKNC.predator_begin());
	CFlock::pred_iterator lastPred(GFLOCKNC.predator_end());
	CFlock::prey_iterator firstPrey(GFLOCKNC.prey_begin());
	CFlock::prey_iterator lastPrey(GFLOCKNC.prey_end());

	float meanN = 0;
	float meanStartAltitude = 0;
	float meanXDist = 0;
	
	int N = static_cast<int>(GFLOCKNC.num_pred());
	int N_prey = static_cast<int>(GFLOCKNC.num_prey());
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, N_prey-1);
	for (int nn = 0; nn < Sim.Params().evolution.Trajectories.amount && nn < N; ++nn)
	{
		((firstPred + nn))->pPred_.StoreTrajectory = true;
	};
	for (; firstPred != lastPred; ++firstPred)
	{
		
		firstPred->ResetHunt();
		firstPred->setTrail(false);
		firstPred->position_ = firstPred->pBird_.InitialPosition;
		firstPred->B_[0] = glm::normalize(-firstPred->position_);
		firstPred->velocity_ = firstPred->pBird_.InitialSpeed * firstPred->B_[0];
		firstPred->SetSpeed(firstPred->pBird_.InitialSpeed);
		firstPred->setTrail(true);
		firstPred->BeginHunt();
		firstPred->SetTargetPrey(&(*(firstPrey + dis(gen))));
		meanN += firstPred->pPred_.N * 1.0f / float(N);
		meanStartAltitude += firstPred->pBird_.InitialPosition.y * 1.0f / float(N);
		meanXDist += firstPred->pBird_.InitialPosition.x * 1.0f / float(N);

	};

	GFLOCKNC.meanN = meanN;
	GFLOCKNC.meanStartAltitude = meanStartAltitude;
	GFLOCKNC.meanXDist = meanXDist;

	for (; firstPrey != lastPrey; ++firstPrey)
	{
		firstPrey->setTrail(false);
		firstPrey->position_ = firstPrey->pBird_.InitialPosition;
		firstPrey->B_[0] = firstPrey->pBird_.InitialHeading;
		// reset the couunter to compute the averages
		firstPrey->velocity_ = firstPrey->pBird_.InitialSpeed * firstPrey->B_[0];
		firstPrey->SetSpeed(firstPrey->pBird_.InitialSpeed);
		firstPrey->setTrail(true);
	};
}

void Simulation::UpdateSimulation(double sim_dt)
{
  const double stat_dt = SimulationTime_ - lastStatTime_;
  const double cluster_dt = SimulationTime_ - lastClusterTime_;
  const bool needStat = (!statisticsPaused_) && approx_ge(stat_dt, params_.featureMap.gCFME().dt);
  const bool recluster = approx_ge(cluster_dt, params_.ClusterDetectionTime);
  if (!paused_) 
  {
    UpdateBirds(static_cast<float>(sim_dt));
    flock_->update(static_cast<float>(sim_dt), recluster);
    if (recluster) lastClusterTime_ = SimulationTime_;
    if (needStat) 
    {
      UpdateCurrentStatistic(stat_dt);
      lastStatTime_ = SimulationTime_;
    }

	// specifically for evolution setting:
	if (params_.evolution.type == "PN")
	{
		if (timeSinceEvolution > params_.evolution.durationGeneration)
		{
            
			if (expNumb == 0) next_experiment();
			//evolution.apply();
			//evolution.save(params_.evolution.fileName.c_str(), 0); 
			Sim.StorageData_(expNumb);
			Generation_ += 1;
			Sim.evolution_next_(expNumb);
			Initialize_birds();
			timeSinceEvolution = 0.0f;
		}
	}
	// end evolution setting

  }
  luabind::globals(Lua)["CameraUpdateHook"](luaCamera_, sim_dt);
  camera_->Update(sim_dt);
}


void Simulation::OnSize(int height, int width)
{
  glm::ivec4 ClientRect(0, 0, width, height);
  glm::ivec4 Viewport(ClientRect);
  camera_->OnSize(Viewport, ClientRect);
  gl_->Resize();
}


void Simulation::OnLButtonDown(int x, int y, unsigned ks)
{
  if (KEYSTATE_IS_BLANK(ks) || KEYSTATE_IS_SHIFT(ks) || KEYSTATE_IS_CTRL(ks))
  {
    track_alpha_ = gl_->Overlay->HitTestAlphaSlider(x ,y);

    if (!track_alpha_)
    {
			Lua.ProcessMouseHooks(x, y, 0, false, ks);
    }
  }
}


void Simulation::OnLButtonUp(int, int)
{
  track_alpha_ = false;
}


void Simulation::OnLButtonDblClk(int x, int y)
{
	Lua.ProcessMouseHooks(x, y, 0, true, KeyState());
}


void Simulation::OnMouseMove(int dx, int dy, bool LButtonPressed)
{
  if (track_alpha_ && LButtonPressed) 
  {
    gl_->alphaMaskCenter() = gl_->Overlay->MoveAlphaSlider(dx, gl_->alphaMaskCenter(), gl_->alphaMaskWidth());
  }
}


void Simulation::OnContextMenu(int menuEntry)
{
  SelectStatistic(menuEntry);
}


// Main simulation loop
void Simulation::EnterGameLoop()
{
  GlobalTimerRestart();
  const double dt = params_.IntegrationTimeStep;
  SimulationTime_ = 0.0;
  double lastFrameDuration = 0.0;
  double lastRender = 0.0;
  double timeDrift = 0.0;
  bool done = 0;
  bool odd = true;
  int skippedFrame = 0;
  
  auto renderFun = [&] {
    if (odd) gl_->Flush(); else gl_->Render();
    odd = !odd;
    FrameTime_ = glm::mix(FrameTime_, GlobalTimerSinceReplace(lastRender), 0.005);
  };

  double frameBegin = GlobalTimerNow();
  luabind::object LuaTimeTickHook;
  for (;;)
  {
    // Serve message pump
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0) 
    {
      if (msg.message == WM_QUIT)
      {
        return;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      LuaTimeTickHook = luabind::globals(Lua)["TimeTickHook"];
    }

    // waste some time
    double udt = dt * (params_.renderFlags.slowMotion ? params_.slowMotion : 1);
    timeDrift = params_.realTime ? glm::mix(timeDrift, udt - lastFrameDuration, 0.1) : 0.0;
    double nowaitUpdate;
    while (timeDrift > GlobalTimerSinceCopy(frameBegin, nowaitUpdate)) 
    {
      if (params_.renderFlags.slowMotion) renderFun();  // super smooth slow motion
      std::this_thread::yield();                        // be nice
    }

    if (LuaTimeTickHook) LuaTimeTickHook(SimulationTime_, timeDrift);
    UpdateSimulation(dt);
    UpdateTime_ = glm::mix(UpdateTime_, GlobalTimerSince(nowaitUpdate), 0.01);

    if (!paused_)
    {
      SimulationTime_ += dt;
	  timeSinceEvolution += dt;
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::microseconds(params_.pausedSleep));
    }

    if ((timeDrift >= 0) || (skippedFrame == params_.maxSkippedFrames))
    {
		//for (int i = 0; i < 10; i++){
			renderFun();
		//}
      
	  //if (int(SimulationTime_) % 15 == 0) {
		//  PlaySound("C:/Users/Robin/Documents/Cpp/starlings/bin/media/wavs/birds003.wav", NULL, SND_FILENAME | SND_ASYNC);
	  //}
      skippedFrame = 0;
    }
    else
    {
      ++skippedFrame;
    }
    lastFrameDuration = GlobalTimerSinceCopy(nowaitUpdate, frameBegin);

	


	}
}


void Simulation::main(int argc, char** argv)
{
  liblua::Open((argc == 2) ? filesystem::path(argv[1]).string().c_str() : 0, "");

  std::string WinTitle("StarDisplay V");
  WinTitle += luabind::object_cast<const char*>(Lua("Version"));
  if (!AppWindow.Create())
  {
    throw std::exception("Creating main window failed");
  }
  filesystem::create_directory(filesystem::path(luabind::object_cast<const char*>(Lua("DataPath"))));

  std::cout << "Starting simulation.\n";
  Lua.DoFile(luabind::object_cast<const char*>(Lua("ConfigFile")));
  boost::optional<bool> quit = luabind::object_cast_nothrow<bool>(luabind::globals(Lua)["InitHook"]());
  if (quit) return;

  luabind::object rnd = luabind::globals(Lua)["random"];
  rnd_seed(luabind::object_cast<unsigned long>(rnd["getSeed"](rnd)));
  setNumPrey(params_.roost.numPrey);
  setNumPredators(params_.roost.numPredators);
  AppWindow.InitContextMenu();
   ResetCurrentStatistic();
   if (params_.renderFlags.turnOffGraphics) EnterGameLoopNoGraphicsNoLua(); else EnterGameLoop();
}


// Entry point
//
int main(int argc, char** argv)
{
  std::locale::global(std::locale("C"));
  std::cout << "StarDisplay\nInitialising simulation.\n";
  try 
  {
    Sim.main(argc, argv);
  }
  catch (const char* w)
  {
    printExceptionMsg(w);
  }
  catch (const luabind::error& e)
  {
    printExceptionMsg(lua_tostring(e.state(), -1));
  }
  catch (const std::exception& e)
  {
    printExceptionMsg(e.what());
  }
  catch (...)
  {
    printExceptionMsg(0);
  }
  if (AppWindow.IsWindow()) AppWindow.DestroyWindow();
  std::cout << "Regards\n";
  return 0;
}



void Simulation::EnterGameLoopNoGraphicsNoLua()
{
	GlobalTimerRestart();
	const double dt = params_.IntegrationTimeStep;
	SimulationTime_ = 0.0;

	double frameBegin = GlobalTimerNow();
	luabind::object LuaTimeTickHook;
	for (;;)
	{
		// Serve message pump
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0)
		{
			if (msg.message == WM_QUIT)
			{
				return;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
		}

		UpdateSimulationNoGraphicsNoFlock(dt);
		SimulationTime_ += dt;
		if (fmod(float(SimulationTime_), 1.0f) > 0 && fmod(float(SimulationTime_), 1.0f) <= dt) PrintFloat(float(SimulationTime_), "time = ");
		timeSinceEvolution += dt;
	}
}



void Simulation::UpdateSimulationNoGraphicsNoFlock(double sim_dt)
{
	
		UpdateBirds(static_cast<float>(sim_dt));
		flock_->update(static_cast<float>(sim_dt), 0);
		

		// specifically for evolution setting:
		if (params_.evolution.type == "PN")
		{
			if (timeSinceEvolution > params_.evolution.durationGeneration)
			{
				
				if (expNumb == 0) next_experiment();
				//evolution.apply();
				//evolution.save(params_.evolution.fileName.c_str(), 0); 
				Sim.StorageData_(expNumb);
				Generation_ += 1;
				Sim.evolution_next_(expNumb);
				Initialize_birds();
				timeSinceEvolution = 0.0f;
			}
		}

	
}