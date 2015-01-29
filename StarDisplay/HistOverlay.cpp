#include <stdio.h>
#include "HistOverlay.hpp"
#include "histogram.hpp"
#include "GLSLImm.hpp"
#include "IText.hpp"
#include "ICamera.hpp"
#include "GLSLState.hpp"
#include "Params.hpp"
#include "Globals.hpp"


HistOverlay::HistOverlay()
{
}


void HistOverlay::Resize()
{
  GTEXT->print("\\smallface{}");
  glm::ivec2 fext = GTEXT->extent("M");
  glm::vec4 viewPort(GCAMERA.GetViewport());
  drawArea_[0] = viewPort[2] * (1.2f/3.0f);
  drawArea_[1] = viewPort[2] - 3.0f * fext.x;
  drawArea_[2] = viewPort[3] - 2.75f * fext.y;
  drawArea_[3] = viewPort[3] - 1.75f * fext.y;
  MoveAlphaSlider(0, GGl.alphaMaskCenter(), GGl.alphaMaskWidth());    // init alphaArea_
}


bool HistOverlay::HitTestAlphaSlider(int wx, int wy) const
{
  return wx > alphaArea_[0] && wx < alphaArea_[1] && wy > alphaArea_[2] && wy < alphaArea_[3];
}


float HistOverlay::MoveAlphaSlider(int dx, float center, float width)
{
  center = glm::clamp(center - float(dx)/(drawArea_[1]-drawArea_[0]), 0.0f, 1.0f);
  const float x = drawArea_[0] + center*(drawArea_[1]-drawArea_[0]);
  const float aw = width*(drawArea_[1]-drawArea_[0]);
  alphaArea_[0] = std::max(x-aw, drawArea_[0]);
  alphaArea_[1] = std::min(x+aw, drawArea_[1]);
  alphaArea_[2] = drawArea_[2];
  alphaArea_[3] = drawArea_[3];
  return center;
}


void HistOverlay::Display(const char* title) const
{
  DisplayLabels(1, 0, title);
}


void HistOverlay::Display(const histogram& hist, const val2TexCoord& v2tex, const char* title) const
{
  DisplayHistogram(hist, v2tex);
  glm::dvec3 quart = hist.quartiles();
  if (PRENDERFLAGS.show_numbers) 
  {
    char buf[256];
    if (PRENDERFLAGS.alphaMasking)
    {
      _snprintf_s(buf, 255, "%s (alpha center:% #8.4g)", title, v2tex.inverse(GGl.alphaMaskCenter()));
    }
    else
    {
      _snprintf_s(buf, 255, "%s (q25:% #8.4g, q50:% #8.4g, q75:% #8.4g)", title, quart.x, quart.y, quart.z);
    }
    DisplayLabels(v2tex.minVal, v2tex.maxVal, buf);
  }
  else
  {
    DisplayLabels(v2tex.minVal, v2tex.maxVal, title);
  }
  // Triangle as quartile marker
  const color32 color(GGl.textColor());
  float x = drawArea_[0] + v2tex(static_cast<float>(quart.y)) * (drawArea_[1]-drawArea_[0]);
  GGl.imm2D->Begin(IMM_FILLED_TRIANGLES);    
    GGl.imm2D->Emit(x-5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x+5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x, drawArea_[3]-5.0f, color);
  GGl.imm2D->End();
  x = drawArea_[0] + v2tex(static_cast<float>(quart.x)) * (drawArea_[1]-drawArea_[0]);
  GGl.imm2D->Begin(IMM_TRIANGLES);    
    GGl.imm2D->Emit(x-5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x+5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x, drawArea_[3]-5.0f, color);
    x = drawArea_[0] + v2tex(static_cast<float>(quart.z)) * (drawArea_[1]-drawArea_[0]);
    GGl.imm2D->Emit(x-5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x+5.0f, drawArea_[3], color);
    GGl.imm2D->Emit(x, drawArea_[3]-5.0f, color);
  GGl.imm2D->End();
}


void HistOverlay::DisplayHistogram(const histogram& hist, const val2TexCoord& v2tex) const
{
  if (PRENDERFLAGS.show_hist)
  {
    const float dx = (drawArea_[1] - drawArea_[0]) / hist.num_bins();
    const float y0 = drawArea_[3];
    float x0 = drawArea_[0];
    GGl.imm2D->Begin(IMM_LINES);
      GGl.imm2D->Emit(x0, y0, -0.1f, 0.0f);
      GGl.imm2D->Emit(drawArea_[1], y0, -0.1f, 1.0f);
    GGl.imm2D->End();
    float maxN = static_cast<float>(hist.max_count());
    if (0.0f < maxN) 
    {
      const float sH = 60.0f / maxN;
      size_t bins = hist.num_bins();
      float scale = 1.0f / bins;
      x0 -= 0.5f*dx;    // shift bin center
      GGl.imm2D->Begin(IMM_LINE_STRIP);
      for (size_t i=0; i<bins; ++i, x0 += dx) 
      {
        const float x1 = x0 + dx;
        const float y1 = y0 - sH * static_cast<float>(hist[i].y);
        GGl.imm2D->Emit(x0, y0, -0.1f, float(i) * scale);
        GGl.imm2D->Emit(x0, y1, -0.1f, float(i) * scale);
        GGl.imm2D->Emit(x1, y1, -0.1f, float(i+1) * scale);
        GGl.imm2D->Emit(x1, y1, -0.1f, float(i+1) * scale);
      }
      GGl.imm2D->End();
    }
  }
  else
  {
    GGl.imm2D->Begin(IMM_FILLED_TRIANGLE_STRIP);
      GGl.imm2D->Emit(drawArea_[0], drawArea_[2], -0.1f, 0.0f);
      GGl.imm2D->Emit(drawArea_[0], drawArea_[3], -0.1f, 0.0f);
      GGl.imm2D->Emit(drawArea_[1], drawArea_[2], -0.1f, 1.0f);
      GGl.imm2D->Emit(drawArea_[1], drawArea_[3], -0.1f, 1.0f);
    GGl.imm2D->End();
  }
}


void HistOverlay::DisplayLabels(float minVal, float maxVal, const char* title) const
{
  glm::ivec4 vpc(GCAMERA.GetViewport());
  GTEXT->print("\\smallface{}");
  GTEXT->set_orig(glm::ivec2(vpc.x, vpc.w - 8));
  GTEXT->set_color(GGl.textColor());
  char buf[32];
  if (minVal < maxVal) 
  {
    _snprintf_s(buf, 31, "%g", minVal);
    glm::ivec2 ext = GTEXT->extent(buf);
    GTEXT->set_cursor(glm::ivec2(int(drawArea_[0] - 0.5f*ext.x), 0));
    GTEXT->print(buf);
    _snprintf_s(buf, 31, "%g", maxVal);
    ext = GTEXT->extent(buf);
    GTEXT->set_cursor(glm::ivec2(int(drawArea_[1] - 0.5f*ext.x), 0));
    GTEXT->print(buf);
    ext = GTEXT->extent(title);
    GTEXT->set_cursor(glm::ivec2(int(drawArea_[0] + 0.5f*((drawArea_[1] - drawArea_[0])-ext.x)), 0));
    GTEXT->print(title);
    DisplayAlphaSlider();
  }
  else
  {
    std::string tmp("Analysis: "); tmp += title;
    glm::ivec2 ext = GTEXT->extent(tmp.c_str());
    GTEXT->set_cursor(glm::ivec2(int(drawArea_[1]) - ext.x, 0));
    GTEXT->print(tmp.c_str());
  }
}


void HistOverlay::DisplayAlphaSlider() const
{
  if (PRENDERFLAGS.alphaMasking)
  {
    const color32 color(GGl.textColor());
    GGl.imm2D->Begin(IMM_LINE_STRIP);    
      GGl.imm2D->Emit(alphaArea_[0], alphaArea_[2], color);
      GGl.imm2D->Emit(alphaArea_[1], alphaArea_[2], color);
      GGl.imm2D->Emit(alphaArea_[1], alphaArea_[3], color);
      GGl.imm2D->Emit(alphaArea_[0], alphaArea_[3], color);
      GGl.imm2D->Emit(alphaArea_[0], alphaArea_[2], color);
    GGl.imm2D->End();
  }
}


