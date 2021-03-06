//$Id: GmatMdiChildFrame.cpp 10027 2011-11-30 23:26:45Z djcinsb $
//------------------------------------------------------------------------------
//                             GmatMdiChildFrame
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Allison Greene
// Created: 2003/08/28
/**
 * This class provides the layout of a mdi child frame
 */
//------------------------------------------------------------------------------

#include "GmatMdiChildFrame.hpp"
#include "GmatAppData.hpp"
#include "GmatMainFrame.hpp"
#include "SubscriberException.hpp"
#include "GuiInterpreter.hpp"
#include "FileManager.hpp"
#include "MessageInterface.hpp"
#include "GuiItemManager.hpp"
#include "GmatTreeItemData.hpp"
#include "GmatMenuBar.hpp"       // for namespace GmatMenu

// @todo We cannot create own MenuBar yet.
// Double Window menu appears when more than one child is open and cannot
// delete theMenuBar in the destructor.
//#define __CREATE_CHILD_MENU_BAR__

//#define DEBUG_MDI_CHILD_FRAME
//#define DEBUG_UPDATE_GUI_ITEM
//#define DEBUG_CLOSE
//#define DEBUG_PERSISTENCE
//#define DEBUG_ACTIVATE


Integer GmatMdiChildFrame::maxZOrder = 0;

//------------------------------
// event tables for wxWindows
//------------------------------

//------------------------------------------------------------------------------
// EVENT_TABLE(GmatMdiChildFrame, wxMDIChildFrame)
//------------------------------------------------------------------------------
/**
 * Events Table for the menu and tool bar
 *
 * @note Indexes event handler functions.
 */
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(GmatMdiChildFrame, wxMDIChildFrame)
   EVT_CLOSE(GmatMdiChildFrame::OnClose) 
   EVT_ACTIVATE(GmatMdiChildFrame::OnActivate)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// GmatMdiChildFrame::GmatMdiChildFrame(...)
//------------------------------------------------------------------------------
GmatMdiChildFrame::GmatMdiChildFrame(wxMDIParentFrame *parent, 
                                     const wxString &name,
                                     const wxString &title, 
                                     const GmatTree::ItemType type,
                                     wxWindowID id, 
                                     const wxPoint &pos, 
                                     const wxSize &size, 
                                     long style)
   : wxMDIChildFrame(parent, id, title, pos, size, style, name)
{
   #ifdef DEBUG_MDI_CHILD_FRAME
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::GmatMdiChildFrame() entered, type=%d\n    ")
       wxT("name='%s'\n   title='%s'"), type, name.c_str(), title.c_str());
   #endif
   
   relativeZOrder          = maxZOrder++;
   usingSavedConfiguration = false;
   mPlotName = name;
   theParent = parent;
   mDirty = false;
   mOverrideDirty = false;
   mCanClose = true;
   mCanSaveLocation = true;
   mItemType = type;
   theScriptTextCtrl = NULL;
   theMenuBar = NULL;
   theAssociatedWin = NULL;
   
   GmatAppData *gmatAppData = GmatAppData::Instance();
   theGuiInterpreter = gmatAppData->GetGuiInterpreter();

   #ifdef __USE_STC_EDITOR__
   theEditor = NULL;
   #endif
   
   #ifdef __WXMAC__
   childTitle = title;
   #endif
   
   #ifdef __CREATE_CHILD_MENU_BAR__
      // create a menu bar, pass Window menu if Windows
      #ifdef __WXMSW__
         theMenuBar = new GmatMenuBar(mItemType, parent->GetWindowMenu());
      #else
         theMenuBar = new GmatMenuBar(mItemType, NULL);
      #endif
   
      #ifdef DEBUG_MENUBAR
      MessageInterface::ShowMessage
         (wxT("GmatMdiChildFrame::GmatMdiChildFrame() theMenuBarCreated %p\n"), theMenuBar);
      #endif
      
      // Commented out so that Window menu works for MdiChildFrame (loj: 2008.02.08)
      // Double Window menu appears when more than one child is open and cannot
      // delete theMenuBar in the destructor.
      //SetMenuBar(theMenuBar);
   #else
      theMenuBar = (GmatMenuBar*)(parent->GetMenuBar());      
   #endif
   
   // Enable Edit menu and tools if ScriptFile
   #ifdef DEBUG_UPDATE_GUI_ITEM
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame() constructor calling UpdateGuiItem()\n"));
   #endif
   UpdateGuiItem(1, 0);
   
   // Set icon if icon file is in the start up file
   FileManager *fm = FileManager::Instance();
   try
   {
      wxString iconfile = fm->GetFullPathname(wxT("MAIN_ICON_FILE")).c_str();
      #if defined __WXMSW__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_ICO));
      #elif defined __WXGTK__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_XPM));
      #elif defined __WXMAC__
         SetIcon(wxIcon(iconfile, wxBITMAP_TYPE_PICT_RESOURCE));
      #endif
   }
   catch (GmatBaseException &e)
   {
      MessageInterface::ShowMessage(e.GetFullMessage());
   }
}


//------------------------------------------------------------------------------
// ~GmatMdiChildFrame()
//------------------------------------------------------------------------------
GmatMdiChildFrame::~GmatMdiChildFrame()
{
   #ifdef DEBUG_MDI_CHILD_FRAME
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::~GmatMdiChildFrame() name='%s', title='%s' entered\n"),
       GetName().c_str(), GetTitle().c_str());
   #endif
   
   #ifdef __CREATE_CHILD_MENU_BAR__
      delete theMenuBar;
   #else
      // There should be only one MenuBar associated with GmatMainFrame,
      // so we cannot delete it here.
      // Disable Edit menu and tools
      #ifdef DEBUG_UPDATE_GUI_ITEM
      MessageInterface::ShowMessage
         (wxT("GmatMdiChildFrame() destructor calling UpdateGuiItem()\n"));
      #endif
      UpdateGuiItem(2, 0);
   #endif
      
   #ifdef DEBUG_MDI_CHILD_FRAME
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::~GmatMdiChildFrame() name='%s', title='%s' exiting\n"),
       GetName().c_str(), GetTitle().c_str());
   #endif
}

//------------------------------------------------------------------------------
// wxString GetPlotName()
//------------------------------------------------------------------------------
wxString GmatMdiChildFrame::GetPlotName()
{
   return mPlotName;
}

//------------------------------------------------------------------------------
// void SetPlotName(const wxString &name)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetPlotName(const wxString &name)
{
   mPlotName = name;
}

//------------------------------------------------------------------------------
// void SetSavedConfigFlag(bool isPreset)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetSavedConfigFlag(bool isPreset)
{
   usingSavedConfiguration = isPreset;
}

//------------------------------------------------------------------------------
// bool GetSavedConfigFlag()
//------------------------------------------------------------------------------
bool GmatMdiChildFrame::GetSavedConfigFlag()
{
   return usingSavedConfiguration;
}

//------------------------------------------------------------------------------
// wxMenuBar* GetMenuBar()
//------------------------------------------------------------------------------
wxMenuBar* GmatMdiChildFrame::GetMenuBar()
{
   return theMenuBar;
}


//------------------------------------------------------------------------------
// GmatTree::ItemType GetItemType()
//------------------------------------------------------------------------------
GmatTree::ItemType GmatMdiChildFrame::GetItemType()
{
   return mItemType;
}


//------------------------------------------------------------------------------
// void SetDataType(GmatTree::ItemType type)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetDataType(GmatTree::ItemType type)
{
   mItemType = type;
}


//------------------------------------------------------------------------------
// wxTextCtrl *GetScriptTextCtrl()
//------------------------------------------------------------------------------
wxTextCtrl* GmatMdiChildFrame::GetScriptTextCtrl()
{
   return theScriptTextCtrl;
}


//------------------------------------------------------------------------------
// void SetScriptTextCtrl(wxTextCtrl *textCtrl)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetScriptTextCtrl(wxTextCtrl *textCtrl)
{
   theScriptTextCtrl = textCtrl;
}


#ifdef __USE_STC_EDITOR__
//------------------------------------------------------------------------------
// Editor* GetEditor()
//------------------------------------------------------------------------------
Editor* GmatMdiChildFrame::GetEditor()
{
   return theEditor;
}


//------------------------------------------------------------------------------
// void SetEditor(Editor *editor)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetEditor(Editor *editor)
{
   theEditor = editor;
}
#endif


//------------------------------------------------------------------------------
// void SetDirty(bool dirty)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetDirty(bool dirty)
{
   mDirty = dirty;
}


//------------------------------------------------------------------------------
// void OverrideDirty(bool flag)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::OverrideDirty(bool flag)
{
   mOverrideDirty = flag;
}


//------------------------------------------------------------------------------
// bool IsDirty()
//------------------------------------------------------------------------------
bool GmatMdiChildFrame::IsDirty()
{
   return mDirty;
}


//------------------------------------------------------------------------------
// bool CanClose()
//------------------------------------------------------------------------------
bool GmatMdiChildFrame::CanClose()
{
   return mCanClose;
}


//------------------------------------------------------------------------------
// wxWindow* GetAssociatedWindow()
//------------------------------------------------------------------------------
wxWindow* GmatMdiChildFrame::GetAssociatedWindow()
{
   return theAssociatedWin;
}


//------------------------------------------------------------------------------
// void SetAssociatedWindow(wxWindow *win)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetAssociatedWindow(wxWindow *win)
{
   theAssociatedWin = win;
}


//------------------------------------------------------------------------------
//void OnActivate(wxActivateEvent &event)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::OnActivate(wxActivateEvent &event)
{
   #ifdef DEBUG_ACTIVATE
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::OnActivate() entered, title='%s', mItemType=%d\n"),
       GetTitle().c_str(), mItemType);
   #endif
   
   // update both edit and animation tools
   #ifdef DEBUG_UPDATE_GUI_ITEM
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame()::OnActivate calling UpdateGuiItem()\n"));
   #endif
   UpdateGuiItem(1, 1);
   
   relativeZOrder = maxZOrder++;
   #ifdef DEBUG_ACTIVATE
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::OnActivate() setting zOrder for %s to %d, and maxZOrder set to %d\n"),
       GetTitle().c_str(), relativeZOrder, maxZOrder);
   #endif
   event.Skip();
}


//------------------------------------------------------------------------------
// void OnClose(wxCloseEvent &event)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::OnClose(wxCloseEvent &event)
{
   #ifdef DEBUG_CLOSE
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::OnClose() entered, mDirty=%d, mOverrideDirty=%d\n"),
       mDirty, mOverrideDirty);
   #endif
   
   mCanClose = true;
   
   // We don't want to show duplicate save message, so check for override dirty flag
   if (mOverrideDirty)
   {
      #ifdef __USE_STC_EDITOR__
      if (theEditor)
      {
         if (theEditor->IsModified())
            mDirty = true;
         else
            mDirty = false;
      }
      #else
      if (theScriptTextCtrl)
      {
         if (theScriptTextCtrl->IsModified())
            mDirty = true;
         else
            mDirty = false;
      }
      #endif
   }
   
   // check if window is dirty?
   if (mDirty)
   {
      #ifdef DEBUG_CLOSE
      MessageInterface::ShowMessage
         (wxT("   Panel has been changed, show exit confirm message\n"));
      #endif
      
      if ( wxMessageBox(wxT("There were changes made to \"" + GetTitle() + "\" panel")
                       wxT(" which will be lost on Close. \nDo you want to close anyway?"),
                        wxT("Please Confirm Close"),
                        wxICON_QUESTION | wxYES_NO) != wxYES )
      {
         event.Veto();
         mCanClose = false;
         
         #ifdef DEBUG_CLOSE
         MessageInterface::ShowMessage
            (wxT("GmatMdiChildFrame::OnClose() exiting, close event was vetoed\n"));
         #endif
         
         return;
      }
   }
   else
   {
      #ifdef DEBUG_CLOSE
      MessageInterface::ShowMessage(wxT("   No changes made to panel\n"));
      #endif
   }
   
   #ifdef DEBUG_CLOSE
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::OnClose() will call GmatMainFrame to remove child and exit\n"));
   #endif
   SaveChildPositionAndSize();
   // remove from list of frames
   GmatAppData::Instance()->GetMainFrame()->RemoveChild(GetName(), mItemType);   
   wxSafeYield();
   
}


//------------------------------------------------------------------------------
// void UpdateScriptActiveStatus(bool isActive)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::UpdateScriptActiveStatus(bool isActive)
{
   // Do nothing here
}

//------------------------------------------------------------------------------
// void SetSaveLocationFlag(bool tf)
//------------------------------------------------------------------------------
/**
 * Allows the programmer to turn off location saving for a frame
 *
 * OwnedPlot objects use this method to turn off saving, which is currently
 * not working.
 *
 * @param tf The flag value; true to enable saving, false to disable it
 */
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetSaveLocationFlag(bool tf)
{
   mCanSaveLocation = tf;
}

//------------------------------------------------------------------------------
// void SaveChildPositionAndSize()
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SaveChildPositionAndSize()
{
   if (mCanSaveLocation == false)
      return;

   // Get the position and size of the window first
   #ifdef __WXMAC__
      Integer screenWidth  = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
      Integer screenHeight = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);
   #else
      Integer screenWidth;
      Integer screenHeight;
      //theParent->GetClientSize(&screenWidth, &screenHeight);
      GmatAppData::Instance()->GetMainFrame()->GetActualClientSize(&screenWidth, &screenHeight, true);
   #endif

//   #ifdef DEBUG_PERSISTENCE
//   wxRect      wxR         = GetScreenRect();
//   wxPoint     wxP         = wxR.GetPosition();
//   wxSize      wxS         = wxR.GetSize();
//   Integer     x           = (Integer) wxP.x;
//   Integer     y           = (Integer) wxP.y;
//   Integer     w           = (Integer) wxS.GetWidth();
//   Integer     h           = (Integer) wxS.GetHeight();
//   MessageInterface::ShowMessage
//      (wxT("wxP.x = %d, wxP.y = %d, wxS.w = %d, wxS.h = %d\n"), x, y, w, h);
//   #endif

   int tmpX = -1, tmpY = -1;
   int tmpW = -1, tmpH = -1;
   GetPosition(&tmpX, &tmpY);
   GetSize(&tmpW, &tmpH);
   Rvector upperLeft(2, ((Real) tmpX /(Real)  screenWidth), ((Real) tmpY /(Real)  screenHeight));
   Rvector childSize(2,  ((Real) tmpW /(Real)  screenWidth), ((Real) tmpH /(Real)  screenHeight));

   #ifdef DEBUG_PERSISTENCE
   // ======================= begin temporary ==============================
   MessageInterface::ShowMessage(wxT("*** Size of SCREEN %s is: width = %d, height = %d\n"), mPlotName.c_str(), screenWidth, screenHeight);
   MessageInterface::ShowMessage(wxT("Position of View plot %s is: x = %d, y = %d\n"), mPlotName.c_str(), tmpX, tmpY);
   MessageInterface::ShowMessage(wxT("Size of View plot %s is: width = %d, height = %d\n"), mPlotName.c_str(), tmpW, tmpH);
//   MessageInterface::ShowMessage(wxT("Position of View plot %s in pixels rel. to parent window is: x = %d, y = %d\n"),
//                                 mPlotName.c_str(), (Integer) tmpX, (Integer) tmpY);
//   MessageInterface::ShowMessage(wxT("Size of View plot %s in pixels rel. to parent window is: x = %d, y = %d\n"),
//                                 mPlotName.c_str(), (Integer) tmpW, (Integer) tmpH);
//   wxPoint tmpPt = ScreenToClient(wxP);
//   MessageInterface::ShowMessage(wxT("--- Position of View plot %s in client coords is: x = %d, y = %d\n"),
//                                 mPlotName.c_str(), (Integer) tmpPt.x, (Integer) tmpPt.y);
   // ======================= end temporary ==============================
   #endif

   if ((mItemType == GmatTree::OUTPUT_REPORT)  || (mItemType == GmatTree::OUTPUT_ORBIT_VIEW) ||
       (mItemType == GmatTree::OUTPUT_XY_PLOT) || (mItemType == GmatTree::OUTPUT_GROUND_TRACK_PLOT)
       // We'll want to add the event reports eventually, but they are not subscriber based
       //|| (mItemType == GmatTree::EVENT_REPORT)
       )
   {
      GmatBase *obj =
         (Subscriber*)theGuiInterpreter->GetConfiguredObject(mPlotName.c_str());
         
      if (!obj || !obj->IsOfType(wxT("Subscriber")))
      {
         wxString errmsg = wxT("Cannot find subscriber ");
         errmsg += mPlotName + wxT("\n");
         throw SubscriberException(errmsg);
      }
      Subscriber *sub = (Subscriber*) obj;

      #ifdef DEBUG_PERSISTENCE
         MessageInterface::ShowMessage("...... Now saving plot data to %s:\n", (sub->GetName()).c_str());
         MessageInterface::ShowMessage("       Upper left             = %12.10f   %12.10f\n", upperLeft[0], upperLeft[1]);
         MessageInterface::ShowMessage("       Size                   = %12.10f   %12.10f\n", childSize[0], childSize[1]);
         MessageInterface::ShowMessage("       RelativeZOrder         = %d\n", relativeZOrder);
      #endif
      sub->SetRvectorParameter(sub->GetParameterID(wxT("UpperLeft")), upperLeft);
      sub->SetRvectorParameter(sub->GetParameterID(wxT("Size")), childSize);
      sub->SetIntegerParameter(sub->GetParameterID(wxT("RelativeZOrder")), relativeZOrder);
   }
   else if (mItemType == GmatTree::MISSION_TREE_UNDOCKED)
   {
      // get the config object
      wxFileConfig *pConfig;
      pConfig = (wxFileConfig *) GmatAppData::Instance()->GetPersonalizationConfig();
      wxString location;
      location << upperLeft[0] << wxT(" ") << upperLeft[1];
      wxString size;
      size << childSize[0] << wxT(" ") << childSize[1];
      pConfig->Write(wxT("/MissionTree/UpperLeft"), location.c_str());
      pConfig->Write(wxT("/MissionTree/Size"), size.c_str());
   }
}

#ifdef __WXMAC__
//------------------------------------------------------------------------------
// void SetTitle(wxString newTitle)
//------------------------------------------------------------------------------
void GmatMdiChildFrame::SetTitle(wxString newTitle)
{
   childTitle = newTitle;
}


//------------------------------------------------------------------------------
// wxString GetTitle()
//------------------------------------------------------------------------------
wxString GmatMdiChildFrame::GetTitle()
{
   if (childTitle.IsNull())
      return wxT("");
   else
      return childTitle;
}
#endif


//------------------------------------------------------------------------------
// void UpdateEditGuiItem(int updateEdit, int updateAnimation)
//------------------------------------------------------------------------------
/**
 * Updates Edit menu items and tools.
 *
 * @param  updateEdit
 *            0, Ignore edit menu and tools
 *            1, Update edit menu and tools
 *            2, Disable edit menu and tools
 * @param  updateAnimation
 *            0, Ignore animation tools
 *            1, Update animation tools
 *            2, Disable animation tools
 */
//------------------------------------------------------------------------------
void GmatMdiChildFrame::UpdateGuiItem(int updateEdit, int updateAnimation)
{
   #ifdef DEBUG_UPDATE_GUI_ITEM
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::UpdateGuiItem() updateEdit=%d, updateAnimation=%d\n"),
       updateEdit, updateAnimation);
   #endif
   
   wxToolBar *toolBar = theParent->GetToolBar();
   if (toolBar == NULL)
      return;
   
   int editIndex = theMenuBar->FindMenu(wxT("Edit"));
   
   //------------------------------------------------------------
   // update edit from menubar and toolbar
   //------------------------------------------------------------
   // Update MenuBar for this child
   if (updateEdit == 1 && (mItemType == GmatTree::SCRIPT_FILE ||
                           mItemType == GmatTree::GMAT_FUNCTION ||
                           mItemType == GmatTree::SCRIPT_EVENT))
   {
      theMenuBar->EnableTop(editIndex, true);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_CUT, TRUE);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_COPY, TRUE);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_PASTE, TRUE);
   }
   else
   {
      theMenuBar->EnableTop(editIndex, false);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_CUT, FALSE);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_COPY, FALSE);
      toolBar->EnableTool(GmatMenu::MENU_EDIT_PASTE, FALSE);
   }
   
   //------------------------------------------------------------
   // update animation icons from toolbar
   //------------------------------------------------------------
   // If mission is running, ignore   
   if (updateAnimation == 1 &&
      (mItemType == GmatTree::OUTPUT_ORBIT_VIEW || mItemType == GmatTree::OUTPUT_GROUND_TRACK_PLOT))
   {
      // If Play button is enabled, mission is not running
      if (toolBar->GetToolEnabled(GmatMenu::TOOL_RUN))
      {
         toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_PLAY, true);
         toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_STOP, true);
         toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_FAST, true);
         toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_SLOW, true);
         #ifdef DEBUG_UPDATE_GUI_ITEM
         MessageInterface::ShowMessage(wxT("   Animation tools enabled\n"));
         #endif
      }
      else
      {
         #ifdef DEBUG_UPDATE_GUI_ITEM
         MessageInterface::ShowMessage
            (wxT("   Cannot enable animation tools, mission is still running\n"));
         #endif
      }
   }
   else
   {
      toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_PLAY, false);
      toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_STOP, false);
      toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_FAST, false);
      toolBar->EnableTool(GmatMenu::TOOL_ANIMATION_SLOW, false);
   }
   
   #ifdef DEBUG_UPDATE_GUI_ITEM
   MessageInterface::ShowMessage
      (wxT("GmatMdiChildFrame::UpdateGuiItem() exiting\n"));
   #endif
}
