//$Id: GmatSavePanel.cpp 9585 2011-06-10 19:54:28Z lindajun $
//------------------------------------------------------------------------------
//                              GmatSavePanel
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
// Author: Linda Jun
// Created: 2004/02/02
//
/**
 * Implements GmatSavePanel class.
 */
//------------------------------------------------------------------------------

#include "GmatSavePanel.hpp"
#include "GmatAppData.hpp"
#include "GmatMainFrame.hpp"
#include "ShowScriptDialog.hpp"
#include "MessageInterface.hpp"

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(GmatSavePanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_SAVE, GmatSavePanel::OnSave)
   EVT_BUTTON(ID_BUTTON_SAVE_AS, GmatSavePanel::OnSaveAs)
   EVT_BUTTON(ID_BUTTON_CLOSE, GmatSavePanel::OnClosePanel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatSavePanel::OnScript)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// GmatSavePanel(wxWindow *parent)
//------------------------------------------------------------------------------
/**
 * Constructs GmatSavePanel object.
 *
 * @param <parent> parent window.
 *
 */
//------------------------------------------------------------------------------
GmatSavePanel::GmatSavePanel(wxWindow *parent, bool showScriptButton,
                             wxString filename, bool showScriptActiveStatus,
                             bool isScriptActive)
   : GmatPanel(parent, false, showScriptButton)
{
   theGuiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   theGuiManager = GuiItemManager::GetInstance();
   canClose = true;
   mEditorModified = false;
   hasFileLoaded = false;
   mShowScriptButton = showScriptButton;
   mFilename = filename;
   mShowScriptActiveStatus = showScriptActiveStatus;
   mIsScriptActive = isScriptActive;
   
   theParent = parent;
   
   int borderSize = 2;
   wxStaticBox *topStaticBox = new wxStaticBox( this, -1, wxT("") );
   wxStaticBox *middleStaticBox = new wxStaticBox( this, -1, wxT("") );
   wxStaticBox *bottomStaticBox = new wxStaticBox( this, -1, wxT("") );
   
   // create sizers
   thePanelSizer = new wxBoxSizer(wxVERTICAL);
   theTopSizer = new wxStaticBoxSizer( topStaticBox, wxVERTICAL );
   theMiddleSizer = new wxStaticBoxSizer( middleStaticBox, wxVERTICAL );
   theBottomSizer = new wxStaticBoxSizer( bottomStaticBox, wxVERTICAL );
   theButtonSizer = new wxBoxSizer(wxHORIZONTAL);
   
   // create script button
   theScriptButton = new wxButton(this, ID_BUTTON_SCRIPT, wxT("Show Script"),
                                  wxDefaultPosition, wxDefaultSize, 0);
   
   // create bottom buttons
   theSaveButton =
      new wxButton(this, ID_BUTTON_SAVE, wxT("Save"), wxDefaultPosition, wxDefaultSize, 0);
   theSaveAsButton =
      new wxButton(this, ID_BUTTON_SAVE_AS, wxT("Save As"), wxDefaultPosition, wxDefaultSize, 0);
   theCloseButton =
      new wxButton(this, ID_BUTTON_CLOSE, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
   
   // add items to top sizer
   theTopSizer->Add(theScriptButton, 0, wxALIGN_RIGHT | wxALL, borderSize);
   
   //------------------------------------------------------
   // for active or inactive script indicator
   //------------------------------------------------------
   wxFlexGridSizer *bottomGridSizer = new wxFlexGridSizer(5);
   
   if (mShowScriptActiveStatus)
   {
      mScriptActiveLabel = new wxStaticText
         (this, -1, wxT(""), wxDefaultPosition, wxSize(110, 18), wxALIGN_CENTRE);
      mScriptActiveLabel->Centre();
      // Make font bold face
      wxFont font = mScriptActiveLabel->GetFont();
      font.SetWeight(wxFONTWEIGHT_BOLD);
      // Make font size little bigger
      int fontSize = font.GetPointSize();
      font.SetPointSize(fontSize + 1);
      mScriptActiveLabel->SetFont(font);
      
      RefreshScriptActiveStatus(mIsScriptActive);
      bottomGridSizer->Add(mScriptActiveLabel, 0, wxALIGN_LEFT | wxALL, borderSize*2);
      bottomGridSizer->Add(30, 20);
   }
   
   // Add script status label
   mScriptDirtyLabel = new wxStaticText
      (this, -1, wxT(""), wxDefaultPosition, wxSize(80, 20), wxALIGN_CENTRE);
   bottomGridSizer->Add(mScriptDirtyLabel, 0, wxALIGN_LEFT | wxALL, borderSize*2);
   bottomGridSizer->Add(50, 20);
   
   // adds the buttons to button sizer
   theButtonSizer->Add(theSaveButton, 0, wxALIGN_CENTER | wxALL, borderSize);
   theButtonSizer->Add(theSaveAsButton, 0, wxALIGN_CENTER | wxALL, borderSize);
   theButtonSizer->Add(theCloseButton, 0, wxALIGN_CENTER | wxALL, borderSize);
   if (mIsScriptActive)
      theButtonSizer->Hide(theSaveButton);
   
   bottomGridSizer->Add(theButtonSizer, 0, wxALIGN_RIGHT | wxALL, borderSize);
   theBottomSizer->Add(bottomGridSizer, 0, wxALIGN_LEFT | wxALL, borderSize);
   
   topStaticBox->Show(mShowScriptButton);
   
   mObject = NULL;
}

//-------------------------------
// protected methods
//-------------------------------

//------------------------------------------------------------------------------
// void Show()
//------------------------------------------------------------------------------
/**
 * Shows the panel.
 */
//------------------------------------------------------------------------------
void GmatSavePanel::Show()
{
   // add items to middle sizer
   thePanelSizer->Add(theTopSizer, 0, wxGROW | wxALL, 1);
   thePanelSizer->Add(theMiddleSizer, 1, wxGROW | wxALL, 1);
   thePanelSizer->Add(theBottomSizer, 0, wxGROW | wxALL, 1);
   
   // displays the script button
   thePanelSizer->Show(theTopSizer, mShowScriptButton);
   theScriptButton->Show(mShowScriptButton);
   thePanelSizer->Layout();
   
   // tells the enclosing window to adjust to the size of the sizer
   SetAutoLayout( TRUE );
   SetSizer(thePanelSizer); //use the sizer for layout
   thePanelSizer->Fit(this); //loj: if theParent is used it doesn't show the scroll bar
   thePanelSizer->SetSizeHints(this); //set size hints to honour minimum size
   
   LoadData();
   mScriptDirtyLabel->SetLabel(wxT(""));

}


//------------------------------------------------------------------------------
// void OnOk()
//------------------------------------------------------------------------------
/**
 * Saves the data and closes the page
 */
//------------------------------------------------------------------------------
void GmatSavePanel::OnSave(wxCommandEvent &event)
{
   // if it is temp script file, call OnSaveAs() to bring up file dialog to save
   if (mFilename == GmatAppData::Instance()->GetTempScriptName())
   {
      OnSaveAs(event);
      return;
   }
   
   bool saveScript = false;
   if (theGuiManager->GetGuiStatus() == 1)
   {
      saveScript = true;
   }
   // If GUI is dirty, prompt user to select an action
   else if (theGuiManager->GetGuiStatus() == 2)
   {
      wxMessageDialog *msgDlg = new wxMessageDialog
         (this, wxT("You will lose changes made in GUI, do you want to save and ")
          wxT("refresh the GUI?"), wxT("Save script..."),
          wxYES_NO | wxCANCEL |wxICON_QUESTION, wxDefaultPosition);
      
      int result = msgDlg->ShowModal();
      if (result == wxID_YES)
         saveScript = true;
      
      delete msgDlg;
   }
   
   
   if (saveScript)
   {
      SaveData();
      mScriptDirtyLabel->SetLabel(wxT(""));
      
      // If script is active, build the script 
      if (mIsScriptActive)
      {
         GmatAppData *gmatAppData = GmatAppData::Instance();
         if (gmatAppData->GetMainFrame()->SetScriptFileName(mFilename.c_str()))
            gmatAppData->GetMainFrame()->OnScriptBuildObject(event);
      }
   }
}


//------------------------------------------------------------------------------
// void OnSaveAs()
//------------------------------------------------------------------------------
/**
 * Saves the data and remain unclosed.
 */
//------------------------------------------------------------------------------
void GmatSavePanel::OnSaveAs(wxCommandEvent &event)
{
   #ifdef DEBUG_SAVE
   MessageInterface::ShowMessage
      (wxT("GmatSavePanel::OnSaveAs() '%s' entered\n"), mFilename.c_str());
   #endif
   
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""),
         wxT("Script files (*.script, *.m)|*.script;*.m|")\
            wxT("Text files (*.txt, *.text)|*.txt;*.text|")\
            wxT("All files (*.*)|*.*"), wxSAVE);

   bool saveScript = false;
   
   if (dialog.ShowModal() == wxID_OK)
   {
      wxString path = dialog.GetPath().c_str();
      
      if(FileExists(path.c_str()))
      {
         if (wxMessageBox(wxT("File already exists.\nDo you want to overwrite?"), 
                          wxT("Please confirm"), wxICON_QUESTION | wxYES_NO) == wxYES)
         {
            mFilename = path;
            saveScript = true;
         }
      }
      else
      {
         mFilename = path;
         saveScript = true;
      }
   }
   
   if (saveScript)
   {
      GmatAppData *gmatAppData = GmatAppData::Instance();
      
      // If script is active, build the script 
      if (mIsScriptActive)
      {
         SaveData();
         mScriptDirtyLabel->SetLabel(wxT(""));
         
         if (gmatAppData->GetMainFrame()->SetScriptFileName(mFilename.c_str()))
            gmatAppData->GetMainFrame()->OnScriptBuildObject(event);
      }
      else
      {
         wxString activeScriptName =
            gmatAppData->GetMainFrame()->GetActiveScriptFileName().c_str();
         
         #ifdef DEBUG_SAVE
         MessageInterface::ShowMessage
            (wxT("   activeScriptName='%s'\n           mFilename='%s'\n"),
             activeScriptName.c_str(), mFilename.c_str());
         #endif
         
         if (mFilename == activeScriptName)
         {
            // If active script is open, close it first
            if (gmatAppData->GetMainFrame()->GetChild(activeScriptName) != NULL)
            {
               #ifdef DEBUG_SAVE
               MessageInterface::ShowMessage
                  (wxT("   Now about to close opened active script\n"));
               #endif
               gmatAppData->GetMainFrame()->CloseChild(activeScriptName, GmatTree::SCRIPT_FILE);
            }
            
            SaveData();      
            mScriptDirtyLabel->SetLabel(wxT(""));
            
            // Now build the script 
            if (gmatAppData->GetMainFrame()->SetScriptFileName(mFilename.c_str()))
               gmatAppData->GetMainFrame()->OnScriptBuildObject(event);
         }
      }
   }
   #ifdef DEBUG_SAVE
   MessageInterface::ShowMessage
      (wxT("GmatSavePanel::OnSaveAs() '%s' leaving\n"), mFilename.c_str());
   #endif
}


//------------------------------------------------------------------------------
// void OnClosePanel()
//------------------------------------------------------------------------------
/**
 * Closes panel.
 */
//------------------------------------------------------------------------------
void GmatSavePanel::OnClosePanel(wxCommandEvent &event)
{
   #ifdef DEBUG_CLOSE_PANEL
   MessageInterface::ShowMessage
      (wxT("GmatSavePanel::OnClosePanel() '%s' entered, mEditorModified=%d\n"),
       mFilename.c_str(), mEditorModified);
   #endif
   
   // We don't want to show duplicate save message when GmatMdiChildFrame is closing,
   // so set override dirty flag to false
   ((GmatMdiChildFrame*)
    (GmatAppData::Instance()->GetMainFrame()->GetActiveChild()))->OverrideDirty(false);
   
   if (mEditorModified)
      ((GmatMdiChildFrame*)
       (GmatAppData::Instance()->GetMainFrame()->GetActiveChild()))->SetDirty(true);
   else
      ((GmatMdiChildFrame*)
       (GmatAppData::Instance()->GetMainFrame()->GetActiveChild()))->SetDirty(false);
   
   GmatAppData::Instance()->GetMainFrame()->CloseActiveChild();
}


//------------------------------------------------------------------------------
// void OnScript()
//------------------------------------------------------------------------------
/**
 * Shows Scripts
 */
//------------------------------------------------------------------------------
void GmatSavePanel::OnScript(wxCommandEvent &event)
{
   wxString title = wxT("Object Script");
   // open separate window to show scripts?
   if (mObject != NULL) {
      title = wxT("Scripting for ");
      title += mObject->GetName().c_str();
   }
   ShowScriptDialog ssd(this, -1, title, mObject);
   ssd.ShowModal();
}


//------------------------------------------------------------------------------
// void UpdateScriptActiveStatus(bool isActive)
//------------------------------------------------------------------------------
void GmatSavePanel::UpdateScriptActiveStatus(bool isActive)
{
   #ifdef DEBUG_UPDATE_ACTIVE_STATUS
   MessageInterface::ShowMessage
      (wxT("GmatSavePanel::UpdateScriptActiveStatus() entered, isActive=%d, ")
       wxT("mFilename='%s'\n"), isActive, mFilename.c_str());
   #endif
   
   mIsScriptActive = isActive;
   
   if (mShowScriptActiveStatus)
      RefreshScriptActiveStatus(mIsScriptActive);
}


//------------------------------------------------------------------------------
// void ReloadFile()
//------------------------------------------------------------------------------
/**
 * Reloads file contents.
 */
//------------------------------------------------------------------------------
void GmatSavePanel::ReloadFile()
{
   LoadData();
   mScriptDirtyLabel->SetLabel(wxT(""));
}


//------------------------------------------------------------------------------
// void SetEditorModified(bool flag)
//------------------------------------------------------------------------------
void GmatSavePanel::SetEditorModified(bool flag)
{
   #ifdef DEBUG_TEXT_MODIFIED
   MessageInterface::ShowMessage
      (wxT("GmatSavePanel::SetEditorModified() entered, flag=%d, mEditorModified=%d, ")
       wxT("hasFileLoaded=%d, mIsScriptActive=%d\n"), flag, mEditorModified, hasFileLoaded,
       mIsScriptActive);
   #endif
   
   // Update script modified status
   if (hasFileLoaded)
   {
      int scriptStatus = flag ? 2 : 1; // 1 = clean, 2 = dirty
      if (mEditorModified != flag)
      {
         if (scriptStatus == 1)
            mScriptDirtyLabel->SetLabel(wxT(""));
         else
            mScriptDirtyLabel->SetLabel(wxT("**modified**"));
      }
      
      // For active script, update sync status
      if (mIsScriptActive)
      {
         theGuiManager->SetActiveScriptStatus(scriptStatus);
         GmatAppData::Instance()->GetMainFrame()->
            UpdateGuiScriptSyncStatus(0, scriptStatus);
      }
      
      mEditorModified = flag;
   }
}


//------------------------------------------------------------------------------
// bool FileExists(wxString scriptFilename)
//------------------------------------------------------------------------------
bool GmatSavePanel::FileExists(wxString scriptFilename)
{
   FILE * pFile;
   pFile = fopen (scriptFilename.char_str(),"rt+");
   if (pFile!=NULL)
   {
      fclose (pFile);
      return true;
   }
   else
      return false;
   
}


//------------------------------------------------------------------------------
// void RefreshScriptActiveStatus(bool isActive)
//------------------------------------------------------------------------------
void GmatSavePanel::RefreshScriptActiveStatus(bool isActive)
{
   if (mIsScriptActive)
   {
      theButtonSizer->Hide(theSaveButton);
      theButtonSizer->Layout();
      
      mScriptActiveLabel->SetLabel(wxT(" Active Script "));
      wxColour bgcolor = wxTheColourDatabase->Find(wxT("DIM GREY"));
      mScriptActiveLabel->SetBackgroundColour(bgcolor);
      mScriptActiveLabel->SetForegroundColour(*wxGREEN);
   }
   else
   {
      theButtonSizer->Show(theSaveButton);
      theButtonSizer->Layout();
      
      mScriptActiveLabel->SetLabel(wxT(" Inactive Script "));
      wxColour bgcolor = wxTheColourDatabase->Find(wxT("LIGHT GREY"));
      mScriptActiveLabel->SetBackgroundColour(bgcolor);
      mScriptActiveLabel->SetForegroundColour(*wxRED);
   }
}


