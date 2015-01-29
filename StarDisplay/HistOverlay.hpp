//! \file HistOverlay.hpp Onscreen diagram
//! \ingroup Visualization

#ifndef HISTOVERLAY_HPP_INCLUDED
#define HISTOVERLAY_HPP_INCLUDED

#include <string>
#include "glmfwd.hpp"
#include "val2TexCoord.hpp"


class HistOverlay
{
public:
  HistOverlay();
  void Resize();
  bool HitTestAlphaSlider(int wx, int wy) const;
  float MoveAlphaSlider(int dx, float center, float width);
  
  void Display(const char* title) const;
  void Display(const class histogram& hist, const val2TexCoord& v2tex, const char* title) const;

private:
  void DisplayHistogram(const class histogram& hist, const val2TexCoord& v2tex) const;
  void DisplayLabels(float minVal, float maxVal, const char* title) const;
  void DisplayAlphaSlider() const;

  glm::vec4 drawArea_;    // in screen coordinates
  glm::vec4 alphaArea_;   // in screen coordinates
};


#endif
