#ifndef GLWIN_INCLUDED
#define GLWIN_INCLUDED

#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <glm/glm.hpp>
#include "menu.hpp"


class GLWin : public CWindowImpl<GLWin>
{
public:
  GLWin();
  virtual ~GLWin();
  bool Create();
  void InitContextMenu();
  bool in_menu() const;

  DECLARE_WND_CLASS_EX(0, CS_OWNDC|CS_DBLCLKS, NULL);

  BEGIN_MSG_MAP(GLWin)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
    MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown);
    MESSAGE_HANDLER(WM_SYSKEYDOWN, OnSysKeyDown);
    MESSAGE_HANDLER(WM_SYSKEYUP, OnSysKeyUp);
    MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel);
    MESSAGE_HANDLER(WM_SIZE, OnSize);
    MESSAGE_HANDLER(WM_CREATE, OnCreate);
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
    MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
    MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown);
    MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp);
    MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDblClk);
    MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand);
    MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
    MESSAGE_HANDLER(WM_ENTERMENULOOP, OnEnterMenuLoop);
    MESSAGE_HANDLER(WM_EXITMENULOOP, OnExitMenuLoop);
    MESSAGE_HANDLER(WM_USER+0, OnEmulateKeyDown);
  END_MSG_MAP()

  LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnKeyDown(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnSysKeyDown(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnSysKeyUp(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnMouseWheel(UINT, WPARAM wParam, LPARAM, BOOL& bHandled);
  LRESULT OnSize(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
  LRESULT OnMouseMove(UINT, WPARAM wParam, LPARAM lParam, BOOL&);
  LRESULT OnLButtonDown(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnLButtonUp(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnLButtonDblClk(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnSysCommand(UINT, WPARAM wParam, LPARAM, BOOL&);
  LRESULT OnContextMenu(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnEnterMenuLoop(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnExitMenuLoop(UINT, WPARAM, LPARAM lParam, BOOL&);
  LRESULT OnEmulateKeyDown(UINT, WPARAM, LPARAM lParam, BOOL&);

private:
  int               mouse_x;
  int               mouse_y;
  CMenu             contextMenu;
};

#endif
