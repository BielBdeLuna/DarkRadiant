/*
Copyright (C) 1999-2006 Id Software, Inc. and contributors.
For a list of contributors, see the accompanying CONTRIBUTORS file.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_MAINFRAME_H)
#define INCLUDED_MAINFRAME_H

#include "gtkutil/window.h"
#include "gtkutil/idledraw.h"
#include "gtkutil/widget.h"
#include "gtkutil/PanedPosition.h"
#include "string/string.h"

#include "iradiant.h"

class IPlugin;
class IToolbarButton;

class XYWnd;
class CamWnd;
class ZWnd;

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkWindow GtkWindow;

const int c_command_status = 0;
const int c_position_status = 1;
const int c_brushcount_status = 2;
const int c_texture_status = 3;
const int c_grid_status = 4;
const int c_count_status = 5;

class MainFrame
{
public:
  enum EViewStyle
  {
    eRegular = 0,
    eFloating = 1,
    eSplit = 2,
    eRegularLeft = 3,
  };

  MainFrame();
  ~MainFrame();

  GtkWindow* m_window;

  CopiedString m_command_status;
  CopiedString m_position_status;
  CopiedString m_brushcount_status;
  CopiedString m_texture_status;
  CopiedString m_grid_status;
private:

  void Create();
  void SaveWindowInfo();
  void Shutdown();

  GtkWidget* m_vSplit;
  GtkWidget* m_hSplit;
  GtkWidget* m_vSplit2;

	struct SplitPaneView {
		GtkWidget* horizPane;
		GtkWidget* vertPane1;
		GtkWidget* vertPane2;
		
		gtkutil::PanedPosition posHPane;
		gtkutil::PanedPosition posVPane1;
		gtkutil::PanedPosition posVPane2;
	} _splitPane;

  CamWnd* m_pCamWnd;

  bool m_bSleeping;

  GtkWidget *m_pStatusLabel[c_count_status];


  EViewStyle m_nCurrentStyle;
  WindowPositionTracker m_position_tracker;

  IdleDraw m_idleRedrawStatusText;

public:

  bool IsSleeping()
  {
    return m_bSleeping;
  }
  void OnSleep();

  void SetStatusText(CopiedString& status_text, const char* pText);
  void UpdateStatusText();
  void RedrawStatusText();
  typedef MemberCaller<MainFrame, &MainFrame::RedrawStatusText> RedrawStatusTextCaller;

  void SetGridStatus();
  typedef MemberCaller<MainFrame, &MainFrame::SetGridStatus> SetGridStatusCaller;

  CamWnd* GetCamWnd()
  {
    return m_pCamWnd;
  }

  void ReleaseContexts();
  void CreateContexts();

  EViewStyle CurrentStyle()
  {
    return m_nCurrentStyle;
  };
  bool FloatingGroupDialog()
  {
    return CurrentStyle() == eFloating || CurrentStyle() == eSplit;
  };
};

extern MainFrame* g_pParentWnd;

GtkWindow* MainFrame_getWindow();

template<typename Value>
class LatchedValue;
typedef LatchedValue<bool> LatchedBool;

void deleteSelection();

// Set the text to be displayed in the status bar
void Sys_Status(const std::string& statusText);


void ScreenUpdates_Disable(const char* message, const char* title = "");
void ScreenUpdates_Enable();
bool ScreenUpdates_Enabled();
void ScreenUpdates_process();

class ScopeDisableScreenUpdates
{
public:
  ScopeDisableScreenUpdates(const char* message, const char* title = "")
  {
    ScreenUpdates_Disable(message, title);
  }
  ~ScopeDisableScreenUpdates()
  {
    ScreenUpdates_Enable();
  }
};


void EnginePath_Realise();
void EnginePath_Unrealise();

class ModuleObserver;

void Radiant_attachEnginePathObserver(ModuleObserver& observer);
void Radiant_detachEnginePathObserver(ModuleObserver& observer);

extern CopiedString g_strEnginePath;
void EnginePath_verify();
const char* EnginePath_get();
const char* QERApp_GetGamePath();

extern CopiedString g_strAppPath;
const char* AppPath_get();

extern CopiedString g_strSettingsPath;
const char* SettingsPath_get();

const char* const g_pluginsDir = "plugins/"; ///< name of plugins directory, always sub-directory of toolspath
const char* const g_modulesDir = "modules/"; ///< name of modules directory, always sub-directory of toolspath

void Radiant_Initialise();
void Radiant_Shutdown();

void SaveMapAs();

void GlobalCamera_UpdateWindow();
void XY_UpdateAllWindows();
void UpdateAllWindows();

void updateTextureBrowser();
void ClipperChangeNotify();

void DefaultMode();

const char* basegame_get();
const char* gamename_get();
void gamename_set(const char* gamename);
const char* gamemode_get();
void gamemode_set(const char* gamemode);

void VFS_Construct();
void VFS_Destroy();

void HomePaths_Construct();
void HomePaths_Destroy();
void Radiant_attachHomePathsObserver(ModuleObserver& observer);
void Radiant_detachHomePathsObserver(ModuleObserver& observer);


void MainFrame_Construct();
void MainFrame_Destroy();


extern float (*GridStatus_getGridSize)();
extern int (*GridStatus_getFarClipDistance)();

#endif
