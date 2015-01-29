#include "resource.h"    // IDI_ICON
#include "GLWin.hpp"
#include "GLSLState.hpp"
#include "Globals.hpp"
#include "Simulation.hpp"
#include "KeyState.hpp"


GLWin::GLWin() : contextMenu()
{
}


GLWin::~GLWin()
{
}


bool GLWin::Create()
{
  CRect WindowExt;
  WindowExt.SetRect(0, 0, 20, 20);
  HWND hWnd = static_cast<CWindowImpl<GLWin>*>(this)->Create(
    NULL,
    WindowExt,
    "StarDisplay",
    WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_OVERLAPPEDWINDOW,
    WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
  return (0 != hWnd);
}



void GLWin::InitContextMenu()
{
  contextMenu.Init();
}


bool GLWin::in_menu() const 
{ 
  return contextMenu.in_menu(); 
}


LRESULT GLWin::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
  const HINSTANCE hInstance = _AtlBaseModule.GetResourceInstance();
  const HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
  SetIcon(hIcon);
  SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);  // Tweak Windows idle & power management
  return 0;
}


LRESULT GLWin::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
  SetThreadExecutionState(ES_CONTINUOUS);
  PostQuitMessage(0);
  return 0;
}


LRESULT GLWin::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  const int height = (HIWORD(lParam)) ? HIWORD(lParam) : 1;
  const int width = LOWORD(lParam);
  Sim.OnSize(height, width);
  return 0;
}


// Bypass WM_ERASEBKGND
LRESULT GLWin::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)
{
  return 1;
}


LRESULT GLWin::OnKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bool handled = true;
  switch (wParam)
  {
  case VK_ESCAPE: 
    PostMessage(WM_CLOSE); 
    break;
  default:
    handled = Sim.HandleKey(LOWORD(wParam), KeyState());
    break;
  }
  bHandled = handled ? TRUE : FALSE;
  return 0;
}


LRESULT GLWin::OnSysKeyDown(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = Sim.HandleKey(LOWORD(wParam), KeyState()) ? TRUE : FALSE;
  return 0;
}


LRESULT GLWin::OnSysKeyUp(UINT, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  bHandled = true;
  return 0;
}


// Map wheelmouse to cursor keys
LRESULT GLWin::OnMouseWheel(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
	OnKeyDown(WM_CHAR, (GET_WHEEL_DELTA_WPARAM(wParam) > 0) ? VK_NEXT : VK_PRIOR, 0, bHandled);
  return 0;
}


LRESULT GLWin::OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
  const int dx = mouse_x - static_cast<int>(LOWORD(lParam));
  const int dy = mouse_y - static_cast<int>(HIWORD(lParam));
  Sim.OnMouseMove(dx, dy, wParam & MK_LBUTTON);
  mouse_x = static_cast<int>(LOWORD(lParam));
  mouse_y = static_cast<int>(HIWORD(lParam));
  return 0;
}


LRESULT GLWin::OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  mouse_x = static_cast<int>(LOWORD(lParam));
  mouse_y = static_cast<int>(HIWORD(lParam));
  Sim.OnLButtonDown(mouse_x, mouse_y, KeyState());
  SetCapture();
  return 0;
}


LRESULT GLWin::OnLButtonUp(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  mouse_x = static_cast<int>(LOWORD(lParam));
  mouse_y = static_cast<int>(HIWORD(lParam));
  Sim.OnLButtonUp(mouse_x, mouse_x);
  ReleaseCapture();
  return 0;
}


LRESULT GLWin::OnLButtonDblClk(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  mouse_x = static_cast<int>(LOWORD(lParam));
  mouse_y = static_cast<int>(HIWORD(lParam));
  Sim.OnLButtonDblClk(mouse_x, mouse_y);
  return 0;
}


LRESULT GLWin::OnContextMenu(UINT, WPARAM, LPARAM lParam, BOOL&)
{
  int select = contextMenu.track(*this, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
  if (-1 != select) {
    Sim.OnContextMenu(select);
  }
  return 0;
}


LRESULT GLWin::OnEnterMenuLoop(UINT, WPARAM wParam, LPARAM, BOOL&)
{
  return 0;
}


LRESULT GLWin::OnExitMenuLoop(UINT, WPARAM wParam, LPARAM, BOOL&)
{
  return 0;
}


LRESULT GLWin::OnSysCommand(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
  switch (wParam) 
  {
    bHandled = true;
    case SC_SCREENSAVE: return 0;
  }
  bHandled = false;
  return 1;
}


LRESULT GLWin::OnEmulateKeyDown(UINT, WPARAM wParam, LPARAM, BOOL& bHandled)
{
  unsigned key = LOWORD(wParam) >> 8;
  unsigned state = LOWORD(wParam) & 0x00ff;
  bool handled = Sim.HandleKey(key, state);
  bHandled = handled ? TRUE : FALSE;
  return 0;
}
