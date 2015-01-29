#ifndef MENU_HPP_INCLUDED
#define MENU_HPP_INCLUDED

#include <windows.h>


class CMenu
{
public:
  CMenu();
  ~CMenu(void);
  void Init();
  int track(HWND hwnd, int x, int y);
  bool in_menu() const 
  { 
    bool ret = in_menu_; 
    in_menu_ = false; 
    return ret; 
  }

private:
  mutable bool in_menu_;
  HMENU hmenu_; 
};


#endif
