//$Id: ScriptEventPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              ScriptEventPanel
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
// Created: 2005/1/12
//
/**
 * Implements ScriptEventPanel class.
 */
//------------------------------------------------------------------------------

#include "ScriptEventPanel.hpp"
#include "GmatAppData.hpp"
#include "MessageInterface.hpp"
#include "CommandUtil.hpp"       // for GetCommandSeqString()
#include "NoOp.hpp"
#include <algorithm>             // for sort(), set_difference()
#include <wx/sstream.h>               // for wxString

// This needs more working on size, so commented out
//#define __USE_SASH_WINDOW__

//#define DBGLVL_SEPANEL_CREATE 1
//#define DBGLVL_SEPANEL_LOAD 1
//#define DBGLVL_SEPANEL_SAVE 1
//#define DBGLVL_SEPANEL_REPLACE_CMDS 1

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(ScriptEventPanel, GmatPanel)
   EVT_TEXT(ID_COMMENT_CTRL, ScriptEventPanel::OnCommentChange)
   EVT_TEXT(ID_SCRIPT_CTRL, ScriptEventPanel::OnCommentChange)
   EVT_SASH_DRAGGED (ID_SASH_WINDOW, ScriptEventPanel::OnSashDrag)
   EVT_SIZE (ScriptEventPanel::OnSize)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// ScriptEventPanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
ScriptEventPanel::ScriptEventPanel(wxWindow *parent, MissionTreeItemData *item)
   : GmatPanel(parent)
{
   theItem = item;
   theCommand = item->GetCommand();
   
   #if DBGLVL_SEPANEL
   ShowCommand(wxT("ScriptEventPanel() theCommand="), theCommand);
   #endif
   
   mPrevCommand = NULL;
   mNextCommand = NULL;
   mNewCommand = NULL;
   
   #ifdef __USE_STC_EDITOR__
   mEditor = NULL;
   #endif
   
   if (theCommand != NULL)
   {
     Create();
     Show();
   }
}


//------------------------------------------------------------------------------
// ~ScriptEventPanel()
//------------------------------------------------------------------------------
/**
 * A destructor.
 *
 * If new command sequence was created, it replaces the old sequence of commands
 * with new sequence via command->ForceSetNext() and command->ForceSetPrevious().
 */
//------------------------------------------------------------------------------
ScriptEventPanel::~ScriptEventPanel()
{
   #ifdef __USE_STC_EDITOR__
   if (mEditor)
   {
      delete mEditor;
      mEditor = NULL;
   }
   #endif
}


//------------------------------------------------------------------------------
// void SetEditorModified(bool isModified)
//------------------------------------------------------------------------------
void ScriptEventPanel::SetEditorModified(bool isModified)
{
   EnableUpdate(isModified);
   mEditorModified = isModified;
}


//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void ScriptEventPanel::Create()
{
   #if DBGLVL_SEPANEL_CREATE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::Create() entered\n"));
   #endif
   
   int bsize = 3; // border size
   
   //=================================================================
   #ifdef __USE_SASH_WINDOW__
   //=================================================================
   
   int w, h;
   
   // A window w/sash for comments
   theCommentsWin =
      new wxSashLayoutWindow(this, ID_SASH_WINDOW,
                             wxDefaultPosition, wxSize(-1, -1), wxSW_3D);
   // A window w/sash for scripts
   theScriptsWin =
      new wxSashLayoutWindow(this, -1,
                             wxDefaultPosition, wxSize(-1, -1), wxSW_3D);
   
   theCommentsWin->SetDefaultSize(wxSize(20, 70));
   theCommentsWin->SetMinimumSizeY(50);
   theCommentsWin->SetMaximumSizeY(350);
   theCommentsWin->SetOrientation(wxLAYOUT_HORIZONTAL);
   theCommentsWin->SetAlignment(wxLAYOUT_TOP);
   theCommentsWin->SetSashVisible(wxSASH_BOTTOM, TRUE);
   
   theScriptsWin->SetMinimumSizeY(100);
   theScriptsWin->SetMaximumSizeY(500);
   theScriptsWin->SetOrientation(wxLAYOUT_HORIZONTAL);
   theScriptsWin->SetAlignment(wxLAYOUT_BOTTOM);
   //theScriptsWin->SetSashVisible(wxSASH_TOP, TRUE);
   
   
   // Comment label
   wxStaticText *commentText =
      new wxStaticText(this, ID_TEXT,
                       wxT("Comments"), wxDefaultPosition, wxDefaultSize, 0);
   // Comment text
   mCommentTextCtrl =
      new wxTextCtrl(theCommentsWin, ID_COMMENT_CTRL, wxT(""), wxDefaultPosition,
                     wxSize(-1, 70), wxTE_MULTILINE | wxTE_DONTWRAP);
   
   // BeginScript label
   wxStaticText *beginScriptText =
      new wxStaticText(this, ID_TEXT,
                       wxT("BeginScript;"), wxDefaultPosition, wxDefaultSize, 0);
   // EndScript label
   wxStaticText *endScriptText =
      new wxStaticText(this, ID_TEXT,
                       wxT("EndScript;"), wxDefaultPosition, wxDefaultSize, 0);
   // Change font color to blue
   beginScriptText->SetForegroundColour(*wxBLUE);
   endScriptText->SetForegroundColour(*wxBLUE);
   
#ifdef __USE_STC_EDITOR__
   #ifdef DEBUG_CREATE
   MessageInterface::ShowMessage
      (wxT("ScriptEventPanel::Create(), creating Editor from parent = <%p>\n"), this);
   #endif
   mEditor = new Editor(theScriptsWin, true, ID_STC);
#else
   // We don't want TextCtrl to wrap text, so add wxTE_DONTWRAP to style
   mFileContentsTextCtrl =
      new wxTextCtrl(theScriptsWin, ID_SCRIPT_CTRL, wxT(""), wxDefaultPosition,
                     wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP);
   
   mFileContentsTextCtrl->SetFont( GmatAppData::Instance()->GetFont() );
#endif
   
   
   //------------------------------------------------------
   // add to sizer
   //------------------------------------------------------
   mPageSizer = new wxBoxSizer(wxVERTICAL);
   mPageSizer->Add(commentText, 0);
   mPageSizer->Add(theCommentsWin, 0, wxGROW);
   mPageSizer->Add(beginScriptText, 0);
   mPageSizer->Add(theScriptsWin, 1, wxGROW);
   mPageSizer->Add(endScriptText, 0);
   theMiddleSizer->Add(mPageSizer, 1, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   
   // Not using sash window
   //=================================================================
   #else
   //=================================================================
   
   theCommentsWin = NULL;
   theScriptsWin = NULL;
   
   // Comment label
   wxStaticText *commentText =
      new wxStaticText(this, ID_TEXT,
                       wxT("Comments"), wxDefaultPosition, wxDefaultSize, 0);
   // Comment text
   mCommentTextCtrl =
      new wxTextCtrl(this, ID_COMMENT_CTRL, wxT(""), wxDefaultPosition,
                     wxSize(-1, 70), wxTE_MULTILINE | wxTE_DONTWRAP);
   
   // BeginScript label
   wxStaticText *beginScriptText =
      new wxStaticText(this, ID_TEXT,
                       wxT("BeginScript;"), wxDefaultPosition, wxDefaultSize, 0);
   // EndScript label
   wxStaticText *endScriptText =
      new wxStaticText(this, ID_TEXT,
                       wxT("EndScript;"), wxDefaultPosition, wxDefaultSize, 0);
   // Change font color to blue
   beginScriptText->SetForegroundColour(*wxBLUE);
   endScriptText->SetForegroundColour(*wxBLUE);
   
#ifdef __USE_STC_EDITOR__
   #ifdef DEBUG_CREATE
   MessageInterface::ShowMessage
      (wxT("ScriptEventPanel::Create(), creating Editor from parent = <%p>\n"), this);
   #endif
   mEditor = new Editor(this, true, ID_STC);
#else
   // We don't want TextCtrl to wrap text, so add wxTE_DONTWRAP to style
   mFileContentsTextCtrl =
      new wxTextCtrl(this, ID_SCRIPT_CTRL, wxT(""), wxDefaultPosition,
                     wxDefaultSize, wxTE_MULTILINE | wxTE_DONTWRAP);
   
   mFileContentsTextCtrl->SetFont( GmatAppData::Instance()->GetFont() );
#endif
   
   //------------------------------------------------------
   // add to sizer
   //------------------------------------------------------
   
   mBottomSizer = new wxGridSizer( 1, 0, 0 );
   
#ifdef __USE_STC_EDITOR__
   mBottomSizer->Add(mEditor, 0, wxGROW | wxALIGN_CENTER | wxALL, bsize);
#else
   mBottomSizer->Add(mFileContentsTextCtrl, 0, wxGROW | wxALIGN_CENTER | wxALL, 
                     bsize);
#endif
   
   //------------------------------------------------------
   // add to parent sizer
   //------------------------------------------------------
   
   mPageSizer = new wxBoxSizer(wxVERTICAL);
   mPageSizer->Add(commentText, 0, wxALIGN_LEFT | wxALL, bsize);
   mPageSizer->Add(mCommentTextCtrl, 0, wxALIGN_LEFT | wxGROW | wxALL, bsize);
   mPageSizer->Add(beginScriptText, 0, wxALIGN_LEFT | wxALL, bsize);
   mPageSizer->Add(mBottomSizer, 1, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   mPageSizer->Add(endScriptText, 0, wxALIGN_LEFT | wxALL, bsize);
   theMiddleSizer->Add(mPageSizer, 1, wxGROW | wxALIGN_CENTER | wxALL, bsize);
   
   //=================================================================
   #endif
   //=================================================================
   
   #if DBGLVL_SEPANEL_CREATE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::Create() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void ScriptEventPanel::LoadData()
{
   
   // Set the pointer for the "Show Script" button
   mObject = theCommand;
   
   #if DBGLVL_SEPANEL_LOAD
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::LoadData()\n"));
   MessageInterface::ShowMessage(wxT("     theCommand=%s\n"), theCommand->GetTypeName().c_str());
   wxString cmdString = GmatCommandUtil::GetCommandSeqString(theCommand);
   MessageInterface::ShowMessage(wxT("     theCommand seuqence=%s\n"), cmdString.c_str());
   
   GmatCommand *firstCmd = theGuiInterpreter->GetFirstCommand();
   cmdString = GmatCommandUtil::GetCommandSeqString(firstCmd);
   MessageInterface::ShowMessage(wxT("===> mission seuqence=%s\n"), cmdString.c_str());
   
   mPrevCommand = theCommand->GetPrevious();
   ShowCommand(wxT("     mPrevCommand = "), mPrevCommand);
   #endif
   
   wxString text;
   
   // We don't want to include Begin/EndScript,
   // so pass Gmat::GUI_EDITOR to GetGeneratingString()
   text << theCommand->GetGeneratingString(Gmat::GUI_EDITOR);
   mCommentTextCtrl->AppendText(theCommand->GetCommentLine().c_str());
   mCommentTextCtrl->SetModified(false);
   
   wxString scriptText = text.c_str();
   
   #ifdef __USE_STC_EDITOR__
      mEditor->AppendText(scriptText);
      mEditor->EmptyUndoBuffer();
      mEditor->SetSavePoint();
   #else
      mFileContentsTextCtrl->AppendText(scriptText);
      mFileContentsTextCtrl->SetModified(false);
   #endif
   
   EnableUpdate(false);
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
/*
 * In order for ScriptEvent to save new scripts, it creates new sequence of
 * commands and replace the old sequence of commands via command->ForceSetNext()
 * and command->ForceSetPrevious().
 */
//------------------------------------------------------------------------------
void ScriptEventPanel::SaveData()
{
   #if DBGLVL_SEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::SaveData() Entered\n"));
   #endif
   
   canClose = false;
   wxString scriptText1;
   
   // If only comment changed, just set comment and return
   #ifdef __USE_STC_EDITOR__
   if (!mEditor->IsModified() && mCommentTextCtrl->IsModified())
   #else
   if (!mFileContentsTextCtrl->IsModified() && mCommentTextCtrl->IsModified())
   #endif
   {
      theCommand->SetCommentLine(mCommentTextCtrl->GetValue().c_str());
      mCommentTextCtrl->SetModified(false);
      EnableUpdate(false);
      canClose = true;
      #if DBGLVL_SEPANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("ScriptEventPanel::SaveData() only comment saved, just returning\n"));
      #endif
      return;
   }
   
   
   //-----------------------------------------------------------------
   // Add lines to stringstream
   //-----------------------------------------------------------------
   scriptText1 << wxT("BeginScript;") << wxT("\n");
   //=======================================================
   #ifdef __USE_STC_EDITOR__
   //=======================================================
   
   #if DBGLVL_SEPANEL_SAVE
   int numLines = mEditor->GetLineCount();
   MessageInterface::ShowMessage(wxT("   number of lines=%d\n"), numLines);
   #endif
   
   wxString text = mEditor->GetText();
   
   // Add EOL if not there.
   size_t length = text.Len();
   wxChar lastChar = text[length-1];
   
   if (lastChar != '\n' && lastChar != '\r' && lastChar != '\0')
      text = text + wxT("\n");
   
   scriptText1 << text;
   
   //=======================================================
   #else
   //=======================================================
   
   int numLines = mFileContentsTextCtrl->GetNumberOfLines();
   wxString line;
   for (int i=0; i<numLines; i++)
   {      
      line = mFileContentsTextCtrl->GetLineText(i);
      scriptText1 << line << wxT("\n");
      // Since GmatMdiChildFrame uses this TextCtrl for checking
      // modified flag, set the flag to false
      mFileContentsTextCtrl->SetModified(false);
   }
   
   //=======================================================
   #endif
   //=======================================================
   
   scriptText1 << wxT("EndScript;") << wxT("\n");
   
   
   //-----------------------------------------------------------------
   // Get previous command
   //-----------------------------------------------------------------
   wxString cmdString = GmatCommandUtil::GetCommandSeqString(theCommand);
   
   #if DBGLVL_SEPANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("   ==> Current BeginScript sequence=%s\n"), cmdString.c_str());
   #endif
   
   // Get previous command
   mPrevCommand = theCommand->GetPrevious();
   
   #if DBGLVL_SEPANEL_SAVE
   ShowCommand(wxT("   mPrevCommand = "), mPrevCommand);
   ShowCommand(wxT("   theCommand->GetPrevious() = "), theCommand->GetPrevious());   
   #endif
   
   //-----------------------------------------------------------------
   // If previous command is NULL, just return
   //-----------------------------------------------------------------
   if (mPrevCommand == NULL)
   {      
      #if DBGLVL_SEPANEL_SAVE
      GmatCommand *firstCmd = theGuiInterpreter->GetFirstCommand();
      ShowCommand(wxT("   firstCmd = "), firstCmd);
      #endif
      
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("ScriptEventPanal::SaveData() *** Internal Error ")
          wxT("Occurred ***\nthe previous command is empty. Cannot continue.\n"));
      
      return;
   }
   
   #if DBGLVL_SEPANEL_SAVE
   ShowCommand(wxT("   mPrevCommand = "), mPrevCommand);
   #endif
   
   //-----------------------------------------------------------------
   // Create temporary NoOp command, so that commands can be appended
   //-----------------------------------------------------------------
   GmatCommand *tempNoOp = new NoOp;
   GmatCommand *inCommand = tempNoOp;
   
   // Save old function list, so that new functions can be delete if there
   // is script errors
   StringArray oldFunctions = theGuiInterpreter->GetListOfObjects(Gmat::FUNCTION);
   for (UnsignedInt i=0; i<oldFunctions.size(); i++)
      MessageInterface::ShowMessage
         (wxT("   ==> oldFunctions[%d] = '%s'\n"), i, oldFunctions[i].c_str());
   
   try
   {
      //--------------------------------------------------------------
      // Set text lines to istringstream and interpret
      //--------------------------------------------------------------
      #if DBGLVL_SEPANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("   scriptText1 =\n%s\n"), scriptText1.c_str());
      #endif
      // memory leak -- new has no corresponding delete
      //std::istringstream *inStringStream = new std::istringstream;
      //inStringStream->str(scriptText1);
      wxStringInputStream inStringStream(scriptText1); 
      
      #if DBGLVL_SEPANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("====================> ScriptEvent calling ")
          wxT("theGuiInterpreter->Interpret(%p)\n"), inCommand);
      #endif
      
      canClose = theGuiInterpreter->Interpret(inCommand, &inStringStream);
      
      #if DBGLVL_SEPANEL_SAVE
      ShowCommand(wxT("   inCommand = "), inCommand);
      cmdString = GmatCommandUtil::GetCommandSeqString(inCommand);
      MessageInterface::ShowMessage
         (wxT("   ==> temp noOp seuqence=%s\n"), cmdString.c_str());
      MessageInterface::ShowMessage(wxT("   canClose = %d\n"), canClose);
      #endif
      
      //--------------------------------------------------------------
      // If error occurred during interpretation, handle and return
      //--------------------------------------------------------------
      bool ignoreErrors = true;
      if (!canClose)
      {         
         wxMessageDialog *msgDlg = new wxMessageDialog
            (this, wxT("Errors were found in the ScriptEvent; Do you want to save it anyway?"), wxT(""),
             wxYES_NO | wxICON_QUESTION, wxDefaultPosition);
         int result = msgDlg->ShowModal();
         
         ignoreErrors = false;
         if (result == wxID_YES)
         {
            ignoreErrors = true;
            canClose = true;
         }
      }
      
      if (!ignoreErrors)
      {
         #if DBGLVL_SEPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("   Deleting tempNoOp->GetNext()=%p\n"), tempNoOp->GetNext());
         #endif
         
         //--------------------------------------------
         // Delete newly created GmatFunctions first
         // since any non-command text will create as
         // GmatFunctions, such as aaaa; or bbbb;
         //--------------------------------------------
         StringArray allFunctions = theGuiInterpreter->GetListOfObjects(Gmat::FUNCTION);
         #if DBGLVL_SEPANEL_SAVE
         for (UnsignedInt i=0; i<allFunctions.size(); i++)
            MessageInterface::ShowMessage
               (wxT("   ==> allFunctions[%d] = '%s'\n"), i, allFunctions[i].c_str());
         #endif
         
         sort(oldFunctions.begin(), oldFunctions.end());
         sort(allFunctions.begin(), allFunctions.end());
         
         StringArray newFunctions;
         // Make list of new functions
         set_difference(allFunctions.begin(), allFunctions.end(), oldFunctions.begin(),
                        oldFunctions.end(), back_inserter(newFunctions));
         
         for (UnsignedInt i=0; i<newFunctions.size(); i++)
         {
            #if DBGLVL_SEPANEL_SAVE
            MessageInterface::ShowMessage
               (wxT("   ==> removing newFunctions[%d] = '%s'\n"), i, newFunctions[i].c_str());
            #endif
            theGuiInterpreter->RemoveObjectIfNotUsed(Gmat::FUNCTION, newFunctions[i]);
         }
         
         //--------------------------------------------
         // Delete commands appended to BeginScript
         //--------------------------------------------
         GmatCommand *temp =
            theGuiInterpreter->DeleteCommand(tempNoOp->GetNext());
         
         if (temp != NULL)
         {
            temp->ForceSetNext(NULL);
            delete temp;
            temp = NULL;
         }
         
         #if DBGLVL_SEPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("   New Script not saved. Deleting tempNoOp=%p\n"), tempNoOp);
         #endif
         
         if (tempNoOp != NULL)
         {
            tempNoOp->ForceSetNext(NULL);
            delete tempNoOp;
            tempNoOp = NULL;
         }
         
         return;
      }
      
      //--------------------------------------------------------------
      // Everthing is good to go, delete temporary NoOp
      //--------------------------------------------------------------
      mNewCommand = tempNoOp->GetNext();
      
      #if DBGLVL_SEPANEL_SAVE
      ShowCommand(wxT("   saving  mNewCommand = "), mNewCommand);
      ShowCommand(wxT("   deleting tempNoOp   = "), tempNoOp);
      #endif
      
      tempNoOp->ForceSetNext(NULL);
      delete tempNoOp;
      tempNoOp = NULL;
      
      //--------------------------------------------------------------
      // Replace current ScrptEvent with new one
      //--------------------------------------------------------------
      ReplaceScriptEvent();
      mObject = mNewCommand;
      theCommand = mNewCommand;
      
      // Save comment if modified
      if (mCommentTextCtrl->IsModified())
      {
         wxString comments = mCommentTextCtrl->GetValue().c_str();
         comments = comments + wxT("\n");
         theCommand->SetCommentLine(comments);
         #if DBGLVL_SEPANEL_SAVE
         MessageInterface::ShowMessage(wxT("   saving new comments\n"));
         #endif
      }
      
      EnableUpdate(false);
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage() + 
         wxT("\nError parsing the ScriptEvent; please correct the text"));
      canClose = false;
   }
   
   #if DBGLVL_SEPANEL_SAVE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::SaveData() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnCommentChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptEventPanel::OnCommentChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnScriptChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ScriptEventPanel::OnScriptChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnSashDrag(wxSashEvent &event)
//------------------------------------------------------------------------------
void ScriptEventPanel::OnSashDrag(wxSashEvent &event)
{
   int w, h;
   GetClientSize(&w, &h);

   #ifdef DEBUG_SASH_DRAG
   MessageInterface::ShowMessage
      (wxT("ScriptEventPanel::OnSashDrag() entered, clientW=%d, clientH=%d\n"), w, h);
   #endif
   
   if (event.GetDragStatus() == wxSASH_STATUS_OUT_OF_RANGE)
      return;
   
   int newH = event.GetDragRect().height;
   
   #ifdef DEBUG_SASH_DRAG
   MessageInterface::ShowMessage(wxT("Setting comments new height to %d\n"), newH);
   #endif
   
   // Sets size of comment window
   theCommentsWin->SetDefaultSize(wxSize(w, newH));
   
   wxLayoutAlgorithm layout;
   // Sets the rest of area to scripts window
   layout.LayoutWindow(this, theScriptsWin);
   
   theMiddleSizer->Layout();
   
   #ifdef DEBUG_SASH_DRAG
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::OnSashDrag() leaving\n"));
   #endif
}


// ------------------------------------------------------------------------------
// void OnSize(wxSizeEvent& event)
//------------------------------------------------------------------------------
/**
 * Handles resizing of the window
 *
 * @param <event> input event.
 */
//------------------------------------------------------------------------------
void ScriptEventPanel::OnSize(wxSizeEvent& event)
{
   int w, h;
   GetClientSize(&w, &h);

   #ifdef DEBUG_SIZE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::OnSize() entered\n"));
   MessageInterface::ShowMessage(wxT("   client size w=%d, h=%d\n"), w, h);
   #endif
   
   // adjust new maximum SashWindow size
   //@todo - Fix this to work (Why comments window disappears?)
   if (theCommentsWin != NULL && theScriptsWin != NULL)
   {
      int minH = 50;
      int maxH = 350;
      
      #ifdef DEBUG_SIZE
      int minComH = theCommentsWin->GetMinimumSizeY();
      int maxcomH = theCommentsWin->GetMaximumSizeY();
      int minScrH = theScriptsWin->GetMinimumSizeY();
      int maxScrH = theScriptsWin->GetMaximumSizeY();
      MessageInterface::ShowMessage(wxT("minComH=%d, maxcomH=%d\n"), minComH, maxcomH);
      MessageInterface::ShowMessage(wxT("minScrH=%d, maxScrH=%d\n"), minScrH, maxScrH);
      MessageInterface::ShowMessage(wxT("Setting comments min height to %d\n"), minH);
      MessageInterface::ShowMessage(wxT("Setting comments max height to %d\n"), maxH);
      #endif
      
      theCommentsWin->SetMinimumSizeY(minH);
      theCommentsWin->SetMaximumSizeY(maxH);
      
      wxLayoutAlgorithm layout;
      layout.LayoutWindow(this, theScriptsWin);
      
      theMiddleSizer->Layout();
   }
   
   event.Skip(true);
   
   #ifdef DEBUG_SIZE
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::OnSize() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void ReplaceScriptEvent()
//------------------------------------------------------------------------------
/**
 * If new command sequence was created, it replaces the old sequence of commands
 * with new sequence via command->ForceSetNext() and command->ForceSetPrevious().
 */
//------------------------------------------------------------------------------
void ScriptEventPanel::ReplaceScriptEvent()
{
   #if DBGLVL_SEPANEL_REPLACE_CMDS
   MessageInterface::ShowMessage(wxT("ScriptEventPanel::ReplaceScriptEvent() Entered\n"));
   ShowCommand(wxT("   theCommand   = "), theCommand);
   ShowCommand(wxT("   mNewCommand  = "), mNewCommand);
   ShowCommand(wxT("   mPrevCommand = "), mPrevCommand);
   #endif
   
   if (theCommand != NULL && mPrevCommand != NULL && mNewCommand != NULL)
   {
      // Get first command from the mission sequence
      GmatCommand *first = theGuiInterpreter->GetFirstCommand();
      // Get parent of current ScriptEvent
      GmatCommand *parent = GmatCommandUtil::GetParentCommand(first, theCommand);
      // Find matching EndScript for new ScriptEvent
      GmatCommand *endScript = GmatCommandUtil::GetMatchingEnd(mNewCommand);
      
      if (parent == NULL)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("ScriptEventPanel::ReplaceScriptEvent() ")
             wxT("*** INTERNAL ERROR ***  parent is NULL\n"));
         return;
      }
      
      #if DBGLVL_SEPANEL_REPLACE_CMDS
      ShowCommand(wxT("   first     = "), first);
      ShowCommand(wxT("   parent    = "), parent);
      ShowCommand(wxT("   endScript = "), endScript);
      #endif
      
      // If endScript found continue, otherwise internal error found      
      if (endScript)
      {
         //-----------------------------------------------------------
         // Get command after EndScript from current ScriptEvent
         //-----------------------------------------------------------
         mNextCommand = GmatCommandUtil::GetNextCommand(theCommand);
         
         #if DBGLVL_SEPANEL_REPLACE_CMDS
         ShowCommand(wxT("   mNextCommand = "), mNextCommand);
         #endif
         
         //-----------------------------------------------------------
         // Delete old ScriptEvent
         //-----------------------------------------------------------
         #if DBGLVL_SEPANEL_REPLACE_CMDS
         ShowCommand(wxT("   ==> calling gui->DeleteCommand() "), theCommand);
         #endif
         
         //Note: DeleteCommand() does not delete theCommand
         GmatCommand *temp = theGuiInterpreter->DeleteCommand(theCommand);
         
         if (temp != NULL)
         {
            temp->ForceSetNext(NULL);
            delete temp;
            temp = NULL;
         }
         
         #if DBGLVL_SEPANEL_REPLACE_CMDS
         MessageInterface::ShowMessage
            (wxT("   ==> after Calling gui->DeleteCommand()\n"));
         #endif
         
         //-----------------------------------------------------------
         // If this ScriptEvent is within BranchCommand, Insert Command
         //-----------------------------------------------------------
         if (parent->IsOfType(wxT("BranchCommand")))
         {
            #if DBGLVL_SEPANEL_REPLACE_CMDS
            ShowCommand(wxT("   insert commands to BranchCommand = "), parent);
            #endif
            
            GmatCommand *prevCmd = mPrevCommand;
            GmatCommand *current = mNewCommand;
            GmatCommand *next = NULL;
            
            // if previous command is BranchCommand or BeginScript,
            // find matching end command, and set it to previous command
            GmatCommand *realPrevCmd = prevCmd;
            if (prevCmd->GetTypeName() == wxT("BeginScript"))
            {
               realPrevCmd = GmatCommandUtil::GetMatchingEnd(prevCmd);
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               ShowCommand(wxT("   realPrevCmd = "), realPrevCmd);
               #endif
               
               prevCmd = realPrevCmd;
            }
            
            // insert commands in the new script event
            while (current != mNextCommand && current != NULL && prevCmd != NULL)
            {
               next = current->GetNext();
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               ShowCommand(wxT("   Inserting current = "), current);
               ShowCommand(wxT("   after     prevCmd = "), prevCmd);
               ShowCommand(wxT("             next    = "), next);
               #endif
               
               current->ForceSetNext(NULL);
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               MessageInterface::ShowMessage(wxT("   ==> calling gui->InsertCommand()\n"));
               #endif
               
               realPrevCmd = prevCmd;
               
               //----------------------------------------------------------
               // Handle nested BranchCommand inside Begin/EndScript.
               // Since Insert BranchCommand inserts whole branch to command
               // sequence, if previous command is BranchCommand, and current
               // command is not BeginScript, find matching End BranchCommand
               // and insert after it.
               //----------------------------------------------------------
               if ((prevCmd->IsOfType(wxT("BranchCommand")) &&
                    current->GetTypeName() != wxT("BeginScript")))
               {
                  realPrevCmd = GmatCommandUtil::GetMatchingEnd(prevCmd);
                  
                  #if DBGLVL_SEPANEL_REPLACE_CMDS
                  ShowCommand(wxT("   realPrevCmd = "), realPrevCmd);
                  #endif

                  prevCmd = realPrevCmd;
               }
               
               theGuiInterpreter->InsertCommand(current, prevCmd);               
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               MessageInterface::ShowMessage
                  (wxT("   ==> after calling gui->InsertCommand()\n"));
               ShowCommand(wxT("   prevCmd->next = "), prevCmd->GetNext());
               
               wxString cmdString = GmatCommandUtil::GetCommandSeqString(first);
               MessageInterface::ShowMessage
                  (wxT("===> new sequence=%s\n"), cmdString.c_str());
               #endif
               
               prevCmd = current;
               current = next;
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               ShowCommand(wxT("   Inserting current = "), current);
               ShowCommand(wxT("   after     prevCmd = "), prevCmd);
               #endif
            } // end while
            
            mNewCommand->ForceSetPrevious(mPrevCommand);
         }
         //-----------------------------------------------------------
         // Else, just set previous/next pointer
         //-----------------------------------------------------------
         else
         {
            #if DBGLVL_SEPANEL_REPLACE_CMDS
            MessageInterface::ShowMessage
               (wxT("   Set previous/next command of BeginScript\n"));
            #endif
            // Set previous/next command of BeginScript
            if (mPrevCommand->IsOfType(wxT("BranchEnd")))
            {
               // If previous command is BranchEnd, find matching parent of
               // BranchEnd and set BranchEnd's next command to matching parent, then
               // set next command of matching parent to new command
               GmatCommand *subParent =
                  GmatCommandUtil::GetParentCommand(first, mPrevCommand);
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               ShowCommand(wxT("   Setting "), subParent, wxT(" as next of "), mPrevCommand);
               #endif
               mPrevCommand->ForceSetNext(subParent);
               
               #if DBGLVL_SEPANEL_REPLACE_CMDS
               ShowCommand(wxT("   Setting "), mNewCommand, wxT(" as next of "), subParent);
               #endif
               subParent->ForceSetNext(mNewCommand);
            }
            else
            {
               mPrevCommand->ForceSetNext(mNewCommand);
               mNewCommand->ForceSetPrevious(mPrevCommand);
            }
            
            // Set previous/next command of EndScript
            if (mNextCommand != NULL)
               mNextCommand->ForceSetPrevious(endScript);
            
            endScript->ForceSetNext(mNextCommand);
         }
         
         //-----------------------------------------------------------
         // Set new ScriptEvent to item command
         //-----------------------------------------------------------
         theItem->SetCommand(mNewCommand);
         
         //-----------------------------------------------------------
         // Some sanity check
         //-----------------------------------------------------------
         if (mNewCommand->GetPrevious() != mPrevCommand)
         {
            if (!mPrevCommand->IsOfType(wxT("BranchEnd")))
            {
               MessageInterface::PopupMessage
                  (Gmat::ERROR_, wxT("ScriptEventPanel::ReplaceScriptEvent() ")
                   wxT("*** INTERNAL ERROR ***  \nmNewCommand->GetPrevious() != ")
                   wxT("mPrevCommand\n"));
               //ShowCommand("mPrevCommand = ", mPrevCommand);
            }
         }
         
         if (mNewCommand->GetNext() == NULL)
         {
            MessageInterface::PopupMessage
               (Gmat::ERROR_, wxT("ScriptEventPanel::ReplaceScriptEvent() ")
                wxT("*** INTERNAL ERROR ***  mNewCommand->GetNext() == NULL\n"));
         }
         else
         {
            #if DBGLVL_SEPANEL_REPLACE_CMDS
            wxString cmdString = GmatCommandUtil::GetCommandSeqString(first);    
            MessageInterface::ShowMessage
               (wxT("===> final new sequence=%s\n"), cmdString.c_str());
            #endif
         }
      }
      else
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("ScriptEventPanel::ReplaceScriptEvent() ")
             wxT("*** INTERNAL ERROR ***  last command should not be NULL\n"));
      }
   }
   
   #if DBGLVL_SEPANEL_REPLACE_CMDS
   MessageInterface::ShowMessage
      (wxT("ScriptEventPanel::ReplaceScriptEvent() Leaving\n\n"));
   #endif
}


//------------------------------------------------------------------------------
// void ShowCommand(const wxString &title1, GmatCommand *cmd1,
//                   const wxString &title2, GmatCommand *cmd2)
//------------------------------------------------------------------------------
void ScriptEventPanel::ShowCommand(const wxString &title1, GmatCommand *cmd1,
                                   const wxString &title2, GmatCommand *cmd2)
{
   if (title2 == wxT(""))
   {
      if (cmd1 == NULL)
         MessageInterface::ShowMessage(wxT("%s(%p)NULL\n"), title1.c_str(), cmd1);
      else
         MessageInterface::ShowMessage
            (wxT("%s(%p)%s\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str());
   }
   else
   {
      if (cmd2 == NULL)
         MessageInterface::ShowMessage
            (wxT("%s(%p)NULL%s(%p)NULL\n"), title1.c_str(), cmd1, title2.c_str(), cmd2);
      else
         MessageInterface::ShowMessage
            (wxT("%s(%p)%s%s(%p)%s\n"), title1.c_str(), cmd1, cmd1->GetTypeName().c_str(),
             title2.c_str(), cmd2, cmd2->GetTypeName().c_str());
   }
}

