#include <glsl/shader_pool.hpp>
#include "GLSLLocals.hpp"
#include "GLSLImm.hpp"
#include "GLSLState.hpp"
#include "Bird.hpp"
#include "Params.hpp"
#include "visitors.hpp"
#include "glmfwd.hpp"
#include "ICamera.hpp"
#include "KeyState.hpp"
#include "Globals.hpp"


namespace 
{
  const color32 separationColor    = color32( 1.0f, 1.0f, 0.0f, 1.0f );
  const color32 alignmentColor     = color32( 0.0f, 1.0f, 0.0f, 1.0f );
  const color32 cohesionColor      = color32( 0.8f, 0.8f, 1.0f, 1.0f );
  const color32 decelerationColor  = color32( 1.0f, 0.0f, 1.0f, 1.0f );
  const color32 velocityColor      = color32( 0.0f, 0.0f, 0.0f, 1.0f );
  const color32 predatorForceColor = color32( 1.0f, 1.0f, 1.0f, 1.0f );
  const color32 flightColor        = color32( 0.5f, 0.0f, 0.25f, 1.0f );
  const color32 liftColor          = color32( 0.0f, 1.0f, 0.35f, 1.0f );
  const color32 dragColor          = color32( 1.0f, 0.5f, 0.25f, 1.0f );
  const color32 steeringColor      = color32( 1.0f, 0.0f, 0.0f, 1.0f );
  const color32 lockedOnColor      = color32( 1.0f, 0.0f, 0.0f, 1.0f );
  const color32 connectColor       = color32( 1.0f, 1.0f, 1.0f, 0.75f );
  const color32 connectColor2      = color32( 0.0f, 0.0f, 1.0f, 0.75f );
  const color32 connectColor3      = color32( 1.0f, 0.0f, 0.0f, 0.75f );
  const color32 forwardColorB      = color32( 1.0f, 0.0f, 0.0f, 1.0f );
  const color32 sideColorB         = color32( 0.0f, 0.0f, 1.0f, 1.0f );
  const color32 upColorB           = color32( 0.0f, 1.0f, 0.0f, 1.0f );
  const color32 forwardColorH      = color32( 0.8f, 0.0f, 0.0f, 1.0f );
  const color32 sideColorH         = color32( 0.0f, 0.0f, 0.8f, 1.0f );
  const color32 upColorH           = color32( 0.0f, 0.8f, 0.0f, 1.0f );
  const color32 circColor          = color32( 0.0f, 1.0f, 1.0f );
  const color32 targetPointColor   = color32( 0.98f, 0.52f, 0.18f );


  void EmitBird(GLSLImm* imm, const Param::RenderFlags& f, const CBird& b)
  {
    const glm::vec3 position(b.position());
    if (f.show_local) 
    {
      imm->Begin(IMM_LINES);
      imm->Emit(position, forwardColorB);
      imm->Emit(position + b.forward(), forwardColorB);
      imm->Emit(position, sideColorB);
      imm->Emit(position + b.side(), sideColorB);
      imm->Emit(position, upColorB);
      imm->Emit(position + b.up(), upColorB);
      imm->End();
    }
    if (f.show_head) 
    {
      glm::mat3 const& H = b.H();
      imm->Begin(IMM_LINES);
      imm->Emit(position, forwardColorH);
      imm->Emit(position + H[0], forwardColorH);
      imm->Emit(position, sideColorH);
      imm->Emit(position + H[2], sideColorH);
      imm->Emit(position, upColorH);
      imm->Emit(position + H[1], upColorH);
      imm->End();
    }
    if (f.show_circ)
    {
      const float circ = b.isPrey() ? static_cast<const CPrey&>(b).circularity() : 0.0f;
      const glm::vec3 circVec = b.isPrey() ? static_cast<const CPrey&>(b).circularityVec() : glm::vec3(0);
      imm->Begin(IMM_LINES);
      imm->Emit(position, circ);
      imm->Emit(position + circVec, circ);
      imm->End();
    }
    if (b.hasTrail()) 
    {
      if (f.show_search) 
      {
        imm->Box(position + glm::vec3(-b.searchRadius()), position + glm::vec3(b.searchRadius()), color32(1.0f, 0.0f, 0.0f, 1.0f));
      }
      if (f.show_forces)
      {
        imm->Begin(IMM_LINES);
        imm->Emit(position, steeringColor);
        imm->Emit(position + b.steering(), steeringColor);
        imm->Emit(position, flightColor);
        imm->Emit(position + b.flightForce(), flightColor);
        imm->Emit(position, liftColor);
        imm->Emit(position + b.up() * glm::dot(b.flightForce(), b.up()), liftColor);
        imm->Emit(position, dragColor);
        imm->Emit(position + b.forward() * glm::dot(b.flightForce(), b.forward()), dragColor);
        if (b.isPrey())
        {
          imm->Emit(position, predatorForceColor);
          imm->Emit(position + static_cast<const CPrey&>(b).predatorForce(), predatorForceColor);
        }
        imm->Emit(position, cohesionColor);
        imm->Emit(position + b.cohesion(), cohesionColor);
        imm->End();
      }
      if (f.show_neighbors)
      {
        if (b.isPrey() || (0 == static_cast<const CPredator&>(b).GetLockedOn()))
        {
          const float elapsed = b.ReactionTime();
          imm->Begin(IMM_LINES);
          std::for_each(b.nbegin(), b.nend(), [position, elapsed, imm] (const neighborInfo& ni)
          {
            const color32& cc = (0 != (ni.predatorReaction & PredationReactions::Panic)) ?
              connectColor3 :
              (0 != (ni.predatorReaction & PredationReactions::Alerted)) ? connectColor2 : connectColor;
            imm->Emit(ni.position + (ni.forward * (ni.speed * elapsed)), cc);
            imm->Emit(position, connectColor);
          });
          imm->End();
        }
      }
    }
    if (f.show_pred)
    {
      if (b.isPredator())
      {
        const CPredator& p = static_cast<const CPredator&>(b);
        if (p.is_attacking())
        {
          const CPrey* prey = p.GetLockedOn();
          if (0 == prey) prey = p.GetTargetPrey();
          if (0 != prey)
          {
            imm->Box(glmutils::bbox3(prey->position(), p.GetPredParams().CatchDistance), lockedOnColor);
            imm->Begin(IMM_LINES);
            imm->Emit(prey->position(), lockedOnColor);
            imm->Emit(position, lockedOnColor);
            imm->End();
          }
          if (p.nsize())
          {
            imm->Box(glmutils::bbox3(p.TargetPoint(), p.GetPredParams().CatchDistance), targetPointColor);
            imm->Begin(IMM_LINES);
            imm->Emit(p.TargetPoint(), targetPointColor);
            imm->Emit(position, targetPointColor);
            imm->End();
          }
        }
      }
    }
  }

}


GLSLLocals::GLSLLocals(const char* Progname)
  : imm_(new GLSLImm(Progname))
{
}


void GLSLLocals::Emmit()
{
  const Param::RenderFlags& f = PRENDERFLAGS;
  GLSLImm* imm = imm_.get();
  if (f.show_local || f.show_head || f.show_search || f.show_forces || f.show_neighbors || f.show_pred || f.show_circ)
  {
    std::for_each(GFLOCK.prey_begin(), GFLOCK.prey_end(), [imm, &f](const CBird& bird) { EmitBird(imm, f, bird); });
    std::for_each(GFLOCK.predator_begin(), GFLOCK.predator_end(), [imm, &f](const CBird& bird) { EmitBird(imm, f, bird); });
  }
  if (f.rtreeLevel)
  {
    size_t level = std::min((unsigned)f.rtreeLevel - 1u, GFLOCK.height()-1u);
    std::for_each(GFLOCK.level_begin(level), GFLOCK.level_end(level), [imm] (const glmutils::bbox3& box)
    {
      imm->Box(box, color32(1.0f, 0.7f, 0.7f, 1.0f));
    });
  }
}


void GLSLLocals::Flush()
{
  imm_->Flush();
}


void GLSLLocals::Render()
{
  imm_->Render();
}

