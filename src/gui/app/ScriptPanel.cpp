//$Id: ScriptPanel.cpp 9585 2011-06-10 19:54:28Z lindajun $
//------------------------------------------------------------------------------
//                              ScriptPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Allison Greene
// Created: 2005/03/25
//
/**
 * Implements ScriptPanel class.
 */
//------------------------------------------------------------------------------

#include "ScriptPanel.hpp"
#include "GmatAppData.hpp"
#include "GmatMainFrame.hpp"
#include "ResourceTree.hpp"
#include "MessageInterface.hpp"
#include "GmatStaticBoxSizer.hpp"
#include <wx/file.h>              // for wxFile
#include <wx/gdicmn.h>            // for wxColourDatabase

// To add build and build&run at top of the panel
//#define __ADD_BUILD_AT_TOP__

//#define DEBUG_SCRIPTPANEL_LOAD
//#define DEBUG_SCRIPTPANEL_TEXT


//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ScriptPanel, GmatSavePanel)
   EVT_BUTTON(ID_BUTTON_SAVE, GmatSavePanel::OnSave)
   EVT_BUTTON(ID_BUTTON_SAVE_AS, GmatSavePanel::OnSaveAs)
   EVT_BUTTON(ID_BUTTON_CLOSE, GmatSavePanel::OnClosePanel)
   EVT_TEXT(ID_TEXTCTRL, ScriptPanel::OnTextUpdate)
   EVT_TEXT_ENTER(ID_TEXTCTRL, ScriptPanel::OnTextEnterPressed)
   EVT_TEXT_MAXLEN(ID_TEXTCTRL, ScriptPanel::OnTextOverMaxLen)
   EVT_BUTTON(ID_BUTTON, ScriptPanel::OnButton)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// ScriptPanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
ScriptPanel::ScriptPanel(wxWindow *parent, const wxString &name, bool isActive)
   : GmatSavePanel(parent, false, name, true, isActive)
{
   mScriptFilename = name;
   mOldLineNumber = 0;
   mUserModified = false;
   
   wxColourDatabase db;
   mBgColor = db.Find(wxT("CYAN"));
   
   Create();
   Show();
}


//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void ScriptPanel::Create()
{
   int bsize = 2; // border size
   
   // create sizers
   GmatStaticBoxSizer *topSizer = new GmatStaticBoxSizer( wxHORIZONTAL, this, wxT(""));
   wxGridSizer *middleSizer = new wxGridSizer( 1, 0, 0 );
   wxBoxSizer *pageSizer = new wxBoxSizer(wxVERTICAL);
   
   //------------------------------------------------------
   // for line number
   //------------------------------------------------------
   wxStaticText *lineNumberText =
      new wxStaticText(this, ID_TEXT, wxT("Go To Line Number:"),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   mLineNumberTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), wxDefaultPosition, wxSize(50, -1),
                     wxTE_PROCESS_ENTER);
      
#ifndef __WXMSW__
   mFileContentsTextCtrl = new
      wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                 wxDefaultPosition, wxDefaultSize,
                 wxTE_MULTILINE);
#else
   mFileContentsTextCtrl = new
      wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                 wxDefaultPosition, wxDefaultSize,
                 wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxTE_RICH2 | wxGROW | wxHSCROLL);
#endif
   
   mFileContentsTextCtrl->SetFont( GmatAppData::Instance()->GetFont() );
   
   //------------------------------------------------------
   // for build and build & run
   //------------------------------------------------------
   mBuildButton =
      new wxButton(this, ID_BUTTON, wxT("Save,Sync"), wxDefaultPosition, wxDefaultSize, 0);
   mBuildRunButton =
      new wxButton(this, ID_BUTTON, wxT("Save,Sync,Run"), wxDefaultPosition, wxDefaultSize, 0);   
   
   //------------------------------------------------------
   // add to sizer
   //------------------------------------------------------
   topSizer->Add(lineNumberText, 0, wxALIGN_LEFT | wxALL, bsize);
   topSizer->Add(mLineNumberTextCtrl, 0, wxALIGN_LEFT | wxALL, bsize);
   middleSizer->Add(mFileContentsTextCtrl, 0, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   
   #ifdef __ADD_BUILD_AT_TOP__
   topSizer->Add(80, 20, 0, wxALIGN_LEFT | wxALL, bsize);
   topSizer->Add(mBuildButton, 0, wxALIGN_CENTER | wxALL, bsize);
   topSizer->Add(mBuildRunButton, 0, wxALIGN_CENTER | wxALL, bsize);
   #else
   theButtonSizer->Insert(0, mBuildButton, 0, wxALIGN_LEFT | wxALL, bsize);
   theButtonSizer->Insert(1, mBuildRunButton, 0, wxALIGN_LEFT | wxALL, bsize);
   theButtonSizer->Insert(2, 50, 20);
   #endif
   
   //------------------------------------------------------
   // add to parent sizer
   //------------------------------------------------------
   pageSizer->Add(topSizer, 0, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   pageSizer->Add(middleSizer, 1, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   theMiddleSizer->Add(pageSizer, 1, wxGROW | wxALIGN_CENTER | wxALL, bsize);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void ScriptPanel::LoadData()
{
   #ifdef DEBUG_SCRIPTPANEL_LOAD
   MessageInterface::ShowMessage(wxT("ScriptPanel::LoadData() entered\n"));
   #endif
   
   wxFile *file = new wxFile();
   bool mFileExists = file->Exists(mScriptFilename);
   
   if (mFileExists)
      mFileContentsTextCtrl->LoadFile(mScriptFilename);
   else
      mFileContentsTextCtrl->SetValue(wxT(""));
   
   theSaveAsButton->Enable(true);
   theSaveButton->Enable(true);
   GmatAppData::Instance()->GetMainFrame()->SetActiveChildDirty(false);
   mEditorModified = false;
   hasFileLoaded = true;
   
   mFileContentsTextCtrl->SetDefaultStyle(wxTextAttr(wxNullColour, *wxWHITE));
   wxTextAttr defStyle = mFileContentsTextCtrl->GetDefaultStyle();
   mDefBgColor = defStyle.GetBackgroundColour();
   
   mOldLastPos = mFileContentsTextCtrl->GetLastPosition();
   
   #ifdef DEBUG_SCRIPTPANEL_LOAD
   MessageInterface::ShowMessage(wxT("ScriptPanel::LoadData() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void ScriptPanel::SaveData()
{
   GmatAppData *gmatAppData = GmatAppData::Instance();
   
   if (mScriptFilename != mFilename)
   {
      // add new script to tree
      gmatAppData->GetResourceTree()->AddScriptItem(mFilename);
      
      // rename this child window
      gmatAppData->GetMainFrame()->RenameActiveChild(mFilename);
      mScriptFilename = mFilename;
   }
   
   mFileContentsTextCtrl->SaveFile(mScriptFilename);
   gmatAppData->GetMainFrame()->SetActiveChildDirty(false);
   mUserModified = false;
}


//------------------------------------------------------------------------------
// void OnTextEnterPressed(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptPanel::OnTextEnterPressed(wxCommandEvent& event)
{
   if (event.GetEventObject() == mFileContentsTextCtrl)
   {
      mUserModified = true;
   }
   else if (event.GetEventObject() == mLineNumberTextCtrl)
   {
      #ifdef DEBUG_SCRIPTPANEL_TEXT
      MessageInterface::ShowMessage
         (wxT("ScriptPanel::OnTextEnterPressed() mDefBgColor=%ld, mBgColor=%ld\n"),
          mDefBgColor.GetPixel(), mBgColor.GetPixel());
      #endif
      
      int lineLength = 0;
      long pos = 0;
      
      // unshow old line number
      if (mOldLineNumber > 0)
      {
         lineLength = mFileContentsTextCtrl->GetLineLength(mOldLineNumber-1);
         pos = mFileContentsTextCtrl->XYToPosition(0, mOldLineNumber-1);
         
         #ifdef DEBUG_SCRIPTPANEL_TEXT
         MessageInterface::ShowMessage
            (wxT("===> mOldLineNumber=%d, pos=%d\n"), mOldLineNumber, pos);
         #endif
         
         mFileContentsTextCtrl->
            SetStyle(pos, pos+lineLength, wxTextAttr(wxNullColour, mDefBgColor));
      }
      
      // show new line number
      long lineNumber;
      wxString str = mLineNumberTextCtrl->GetValue();
      str.ToLong(&lineNumber);
      pos = mFileContentsTextCtrl->XYToPosition(0, lineNumber-1);
      lineLength = mFileContentsTextCtrl->GetLineLength(lineNumber-1);
      
      #ifdef DEBUG_SCRIPTPANEL_TEXT
      MessageInterface::ShowMessage(wxT("===> lineNumber=%d, pos=%d\n"), lineNumber, pos);
      #endif
      
      // This causes wxEVT_COMMAND_TEXT_UPDATED event generated.
      // Notice that this event will always be sent when the text controls
      // contents changes - whether this is due to user input or comes from the
      // program itself (for example, if SetValue() is called)
      mFileContentsTextCtrl->
         SetStyle(pos, pos+lineLength, wxTextAttr(wxNullColour, mBgColor));
      
      mFileContentsTextCtrl->ShowPosition(pos);
      
      mOldLineNumber = lineNumber;
   }
}


//------------------------------------------------------------------------------
// void OnTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptPanel::OnTextUpdate(wxCommandEvent& event)
{
   if (event.GetEventObject() == mFileContentsTextCtrl &&
       mFileContentsTextCtrl->IsModified())
   {
      int lastPos = mFileContentsTextCtrl->GetLastPosition();
      
      #ifdef DEBUG_SCRIPTPANEL_TEXT
      MessageInterface::ShowMessage
         (wxT("ScriptPanel::OnTextUpdate() mOldLastPos=%d, lastPos=%d\n"),
          mOldLastPos, lastPos);
      #endif
      
      // check contents size to determine if text is modified,
      // since SetStyle() for go to line number generates EVT_TEXT
      if (mUserModified || lastPos != mOldLastPos)
      {
         #ifdef DEBUG_SCRIPTPANEL_TEXT
         MessageInterface::ShowMessage(wxT("===> data modified\n"));
         #endif
         
         theSaveButton->Enable(true);
         GmatSavePanel::SetEditorModified(true);
         GmatAppData::Instance()->GetMainFrame()->SetActiveChildDirty(true);
         mOldLastPos = lastPos;
      }
   }
}


//------------------------------------------------------------------------------
// void OnTextOverMaxLen(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptPanel::OnTextOverMaxLen(wxCommandEvent& event)
{
   wxMessageBox(wxT("Text control is already filled up to the maximum length.\n")
                    wxT("The extra input will be discarded."),
                wxT("GMAT Warning"));
}


//------------------------------------------------------------------------------
// void OnButton(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptPanel::OnButton(wxCommandEvent& event)
{
   if (mFileContentsTextCtrl->GetValue() == wxT(""))
   {
      wxMessageDialog *msgDlg = new wxMessageDialog
         (this, wxT("Can not build an empty file "), wxT("Can not build..."),
          wxOK | wxICON_INFORMATION, wxDefaultPosition);
      msgDlg->ShowModal();
      return;
   }
   
   GmatAppData *gmatAppData = GmatAppData::Instance();
   bool continueBuild = true;
   
   if (event.GetEventObject() == mBuildButton ||
       event.GetEventObject() == mBuildRunButton)
   {
      // If this is not an active script, prompt the user for setting active
      if (!mIsScriptActive)
      {
         wxMessageDialog *msgDlg = new wxMessageDialog
            (this,wxT("Are you sure you want to make this script active?"), wxT("Save active..."),
             wxYES_NO | wxICON_QUESTION, wxDefaultPosition);
         int result = msgDlg->ShowModal();
         
         if (result == wxID_YES)
            continueBuild = true;
         else
            continueBuild = false;
      }
      
      
      if (mFileContentsTextCtrl->IsModified())
      {
         //=======================================
         #ifdef __PROMPT_USER_ON_MODIFIED__
         //=======================================
         
         // prompt user to save
         wxMessageDialog *msgDlg = new wxMessageDialog(this,
            wxT("Would you like to save changes?"), wxT("Save..."), wxYES_NO | wxICON_QUESTION ,
            wxDefaultPosition);
         int result = msgDlg->ShowModal();
         
         if (result == wxID_YES)
            OnSave(event);
         
         //=======================================
         #else
         //=======================================
         
         OnSave(event);
         
         //=======================================
         #endif
         //=======================================
      }
   }
   
   // If continue building, set script file name and build script
   if (continueBuild)
   {
      if (event.GetEventObject() == mBuildButton)
      {
         if (gmatAppData->GetMainFrame()->SetScriptFileName(mScriptFilename.c_str()))
            gmatAppData->GetMainFrame()->OnScriptBuildObject(event);
      }
      else if (event.GetEventObject() == mBuildRunButton)
      {
         if (gmatAppData->GetMainFrame()->SetScriptFileName(mScriptFilename.c_str()))
            gmatAppData->GetMainFrame()->OnScriptBuildAndRun(event);
      }
   }
}
