#include <windows.h>
#include "KeyState.hpp"


// Transforms keyboard state (shift, alt, ctrl) into bitset
unsigned KeyState()
{
  unsigned shift = (GetAsyncKeyState(VK_SHIFT) < 0) ? 1 : 0;
  unsigned alt   = (GetAsyncKeyState(VK_MENU) < 0) ? 2 : 0;
  unsigned ctrl  = (GetAsyncKeyState(VK_CONTROL) < 0) ? 4 : 0;
  return shift | alt | ctrl;
}


