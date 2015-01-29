#include <iostream>
#include <atlbase.h>
#include <atlconv.h>
#include <memory.h>
#include "Globals.hpp"
#include "menu.hpp"
#include "Params.hpp"


CMenu::CMenu(): in_menu_(false), hmenu_(NULL)
{
}


CMenu::~CMenu(void)
{
  DestroyMenu(hmenu_);
}


void CMenu::Init()
{
  if (hmenu_) { DestroyMenu(hmenu_); hmenu_ = NULL; }
  hmenu_ = CreatePopupMenu();
  MENUITEMINFO mii;
  memset((void*)&mii, 0, sizeof(MENUITEMINFO));
  mii.cbSize = sizeof(MENUITEMINFO);
  mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
  mii.fType = MFT_STRING;
  mii.fState = MFS_ENABLED;
  for (int i = 0; i < Param::FeatureMap::MaxFeatureMapping__; ++i)
  {
    if (PFM.Entries[i].enable)
    {
      mii.wID = i+1;
      CA2T pszt(PFM.Entries[i].title.c_str());
      mii.dwTypeData = pszt;
      BOOL ok = InsertMenuItem(hmenu_, i+1, FALSE, &mii);
      if (!ok) {
        throw std::exception("CMenu::Init failed");
      }
    }
  }
}


int CMenu::track(HWND hwnd, int winx, int winy)
{
  for (int i = 0; i < Param::FeatureMap::MaxFeatureMapping__; ++i)
  {
    CheckMenuItem(hmenu_, i+1, MF_BYCOMMAND | MF_UNCHECKED);
  }
  in_menu_ = true;
  CheckMenuItem(hmenu_, PFM.current + 1, MF_BYCOMMAND | MF_CHECKED);
  BOOL ret = TrackPopupMenuEx(hmenu_, TPM_LEFTALIGN | TPM_RETURNCMD, winx, winy, hwnd, NULL);
  return static_cast<int>(ret)-1;
}
