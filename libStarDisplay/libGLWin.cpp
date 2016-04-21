#include <luabind/luabind.hpp>
#include "libGlm.hpp"
#include "GLWin.hpp"


namespace libWin {

  glm::dvec4 GetDesktopRect(GLWin* obj)
  {
    HWND desktop = ::GetDesktopWindow();
    CRect r; GetClientRect(desktop, &r);
    return glm::dvec4(r.left, r.top, r.Width(), r.Height());
  }


  glm::dvec4 GetClientRect(GLWin* obj)
  {
    CRect r; obj->GetClientRect(&r);
    return glm::dvec4(r.left, r.top, r.Width(), r.Height());
  }


  void SetClientRect(GLWin* obj, const glm::dvec4& r)
  {
    CRect rw; obj->GetWindowRect(&rw);
    CRect rc; obj->GetClientRect(&rc);
    int dx = rw.Width() - rc.Width();
    int dy = rw.Height() - rc.Height();
    obj->SetWindowPos(0, (int)r.x, (int)r.y, dx + (int)r.z, dy + (int)r.w, SWP_NOZORDER);
  }


  bool IsFullscreen(GLWin* obj)
  {
    return ((obj->GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_WINDOWEDGE) == 0);
  }


  void ShowWindow(GLWin* obj, const std::string& mode)
  {
    if (IsFullscreen(obj))
    {
      WINDOWPLACEMENT wp; wp.length = sizeof(WINDOWPLACEMENT);
      obj->GetWindowPlacement(&wp);
      obj->SetWindowLongPtr(GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
      obj->SetWindowLongPtr(GWL_EXSTYLE, WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_WINDOWEDGE | WS_EX_APPWINDOW);
      CRect rc = wp.rcNormalPosition;
      obj->SetWindowPos(0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_FRAMECHANGED);
      obj->ShowWindow(SW_SHOWNORMAL);
    }
    if (mode == "centered") { obj->ShowWindow(SW_SHOWNORMAL); obj->CenterWindow(); }
    else if (mode == "hidden") obj->ShowWindow(SW_HIDE); 
    else if (mode == "maximize") obj->ShowWindow(SW_SHOWMAXIMIZED); 
    else if (mode == "minimize") obj->ShowWindow(SW_SHOWMINIMIZED); 
    else if (mode == "fullscreen") 
    {
      obj->SetWindowLongPtr(GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
      obj->SetWindowLongPtr(GWL_EXSTYLE, WS_EX_LEFT | WS_EX_LTRREADING);
      obj->SetWindowPos(0, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
      obj->ShowWindow(SW_SHOWMAXIMIZED);
    }
  }


  void SetTitle(GLWin* obj, const char* title)
  {
    obj->SetWindowText(title);
  }

}


void luaopen_libWin(lua_State* L)
{
  using namespace luabind;

  module(L)[
    class_<GLWin>("__glwin")
      .def("GetDesktopRect", &libWin::GetDesktopRect)
      .def("GetClientRect", &libWin::GetClientRect)
      .def("SetClientRect", &libWin::SetClientRect)
      .def("IsFullscreen", &libWin::IsFullscreen)
      .def("ShowWindow", &libWin::ShowWindow)
      .def("SetTitle", &libWin::SetTitle)
  ];
}

