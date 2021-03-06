//$Id: TogglePanel.cpp 9850 2011-09-09 18:48:32Z lindajun $
//------------------------------------------------------------------------------
//                              TogglePanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Linda Jun
// Created: 2004/10/20
//
/**
 * This class contains the Toggle setup window.
 */
//------------------------------------------------------------------------------

#include "TogglePanel.hpp"
#include "MessageInterface.hpp"


//#define DEBUG_TOGGLE_PANEL 1


//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(TogglePanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_RADIOBUTTON(ID_RADIOBUTTON, TogglePanel::OnRadioButtonChange)
   EVT_CHECKLISTBOX(ID_CHECKLISTBOX, TogglePanel::OnCheckListBoxChange)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// TogglePanel(wxWindow *parent)
//------------------------------------------------------------------------------
/**
 * Constructs TogglePanel object.
 *
 * @param <parent> input parent.
 */
//------------------------------------------------------------------------------
TogglePanel::TogglePanel(wxWindow *parent, GmatCommand *cmd, bool forXyPlotOnly,
                         bool showToggleState)
   : GmatPanel(parent)
{
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage
      (wxT("TogglePanel::TogglePanel() entered, parent=<%p>, cmd=<%p>, forXyPlotOnly=%d\n"),
       parent, cmd, forXyPlotOnly);
   #endif
   
   theCommand = cmd;
   isForXyPlotOnly = forXyPlotOnly;
   mShowToggleState = showToggleState;
   
   if (theCommand != NULL)
   {
      Create();
      Show();
   }
   
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage(wxT("TogglePanel::TogglePanel() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// ~TogglePanel()
//------------------------------------------------------------------------------
TogglePanel::~TogglePanel()
{
   if (isForXyPlotOnly)
      theGuiManager->UnregisterCheckListBox(wxT("XYPlot"), mSubsCheckListBox);
   else
      theGuiManager->UnregisterCheckListBox(wxT("Subscriber"), mSubsCheckListBox);
}


//------------------------------------------------------------------------------
// virtual bool TakeAction(const wxString &action)
//------------------------------------------------------------------------------
bool TogglePanel::TakeAction(const wxString &action)
{
   #ifdef DEBUG_TAKE_ACTION
   MessageInterface::ShowMessage
      (wxT("TogglePanel::TakeAction() for '%s' entered, action = '%s'\n"),
       mCmdTypeName.c_str(), action.c_str());
   #endif
   
   if (action == wxT("EnableUpdate"))
      EnableUpdate(true);
   
   return true;
}


//---------------------------------
// protected methods
//---------------------------------

//----------------------------------
// methods inherited from GmatPanel
//----------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 * Creates the panel for the Maneuver Command
 */
//------------------------------------------------------------------------------
void TogglePanel::Create()
{
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage(wxT("TogglePanel::Create() entered\n"));
   #endif
   
   int bsize = 2;
   mCmdTypeName = theCommand->GetTypeName().c_str();
   
   // create object label
   wxStaticText *objectLabel =
      new wxStaticText(this, ID_TEXT, wxT("Select Subscribers to " + mCmdTypeName),
                       wxDefaultPosition, wxDefaultSize, 0);
   
   // create subscriber check list box
   if (isForXyPlotOnly)
      mSubsCheckListBox =
         theGuiManager->GetXyPlotCheckListBox(this, ID_CHECKLISTBOX, wxSize(150,-1));
   else
      mSubsCheckListBox =
         theGuiManager->GetSubscriberCheckListBox(this, ID_CHECKLISTBOX, wxSize(150,-1));
   
   // On or Off button
   if (mShowToggleState)
   {
      mOnRadioButton =
         new wxRadioButton(this, ID_RADIOBUTTON, wxT("On"),
                           wxDefaultPosition, wxDefaultSize, 0);
      
      mOffRadioButton =
         new wxRadioButton(this, ID_RADIOBUTTON, wxT("Off"),
                           wxDefaultPosition, wxDefaultSize, 0);
   }
   
   // create sizers
   wxBoxSizer *buttonSizer = NULL;
   wxBoxSizer *pageSizer = new wxBoxSizer(wxHORIZONTAL);
   if (mShowToggleState)
   {
      buttonSizer = new wxBoxSizer(wxHORIZONTAL);
      buttonSizer->Add(mOnRadioButton, 0, wxALIGN_CENTER | wxALL, bsize);
      buttonSizer->Add(mOffRadioButton, 0, wxALIGN_CENTER | wxALL, bsize);
   }
   
   pageSizer->Add(objectLabel, 0, wxALIGN_CENTER | wxALL, bsize);
   pageSizer->Add(mSubsCheckListBox, 0, wxALIGN_CENTER | wxALL, bsize);
   if (mShowToggleState)
      pageSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, bsize);
   
   // add to middle sizer
   theMiddleSizer->Add(pageSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage(wxT("TogglePanel::Create() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void TogglePanel::LoadData()
{
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage
      (wxT("TogglePanel::LoadData() '%s' entered, mShowToggleState=%d\n"),
       theCommand->GetTypeName().c_str(), mShowToggleState);
   #endif
   
   // Set the pointer for the "Show Script" button
   mObject = theCommand;
   
   try
   {
      // get subscriber names 
      StringArray subNames = theCommand->GetRefObjectNameArray(Gmat::SUBSCRIBER);
      int subsize = subNames.size();
      #if DEBUG_TOGGLE_PANEL
      MessageInterface::ShowMessage("   There are %d subscribers\n", subsize);
      for (unsigned int i=0; i<subNames.size(); i++)
         MessageInterface::ShowMessage
            ("      subNames[%d]='%s'\n", i, subNames[i].c_str());
      #endif
      
      wxString toggleState;
      if (mShowToggleState)
      {
         toggleState = theCommand->GetStringParameter
            (theCommand->GetParameterID(wxT("ToggleState")));
         
         #if DEBUG_TOGGLE_PANEL
         for (unsigned int i=0; i<subNames.size(); i++)
            MessageInterface::ShowMessage
               (wxT("   subName[%d]=%s, toggleState=%s\n"), i,
                subNames[i].c_str(), toggleState.c_str());
         #endif
      }
      
      // Initialize check list box
      wxString name;
      for (UnsignedInt i=0; i<mSubsCheckListBox->GetCount(); i++)
      {
         name = mSubsCheckListBox->GetString(i).c_str();
         #if DEBUG_TOGGLE_PANEL
         MessageInterface::ShowMessage(wxT("   available name = '%s'\n"), name.c_str());
         #endif
         for (int j=0; j<subsize; j++)
         {
            if (name == subNames[j])
            {
               mSubsCheckListBox->Check(i);
               break;
            }
         }
      }
      
      if (mShowToggleState)
      {
         if (toggleState == wxT("On"))
            mOnRadioButton->SetValue(true);
         else
            mOffRadioButton->SetValue(true);
      }
   }
   catch (GmatBaseException &ex)
   {
      MessageInterface::ShowMessage(ex.GetFullMessage() + wxT("\n"));
   }
   
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage(wxT("TogglePanel::LoadData() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void TogglePanel::SaveData()
{
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage
      (wxT("TogglePanel::SaveData() '%s' entered, mShowToggleState=%d\n"),
       theCommand->GetTypeName().c_str(), mShowToggleState);
   #endif
   
   int count = mSubsCheckListBox->GetCount();
   int checkedCount = 0;
   wxString onOff = wxT("Off");
   
   if (mShowToggleState)
      onOff = mOnRadioButton->GetValue() ? wxT("On") : wxT("Off");
   
   canClose = true;
   
   //-----------------------------------------------------------------
   // check for number of subscribers checked
   //-----------------------------------------------------------------
   for (int i=0; i<count; i++)
      if (mSubsCheckListBox->IsChecked(i))
         checkedCount++;
   
   if (checkedCount == 0)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("Please select one or more subscribers to %s."), theCommand->GetTypeName().c_str());
      canClose = false;
      return;
   }
   
   
   //-----------------------------------------------------------------
   // save values to base, base code should do any range checking
   //-----------------------------------------------------------------
   
   try
   {
      theCommand->TakeAction(wxT("Clear"));
      
      wxString subName;
      //wxString cmdStr = theCommand->GetTypeName();
      for (int i=0; i<count; i++)
      {
         subName = mSubsCheckListBox->GetString(i);
         #if DEBUG_TOGGLE_PANEL
         MessageInterface::ShowMessage(wxT("   name = '%s' and "), subName.c_str());
         #endif
         if (mSubsCheckListBox->IsChecked(i))
         {
            #if DEBUG_TOGGLE_PANEL
            MessageInterface::ShowMessage(wxT("it is checked\n"));
            #endif
            
            //cmdStr = cmdStr + wxT(" ") + subName;
            theCommand->SetStringParameter(theCommand->GetParameterID(wxT("Subscriber")), subName);
         }
         else
         {
            #if DEBUG_TOGGLE_PANEL
            MessageInterface::ShowMessage(wxT("it is not checked\n"));
            #endif
         }
      }

      #if 0
      if (mShowToggleState)
         cmdStr = cmdStr + wxT(" ") + onOff;
      cmdStr = cmdStr + wxT(";");
      
      #if DEBUG_TOGGLE_PANEL
      MessageInterface::ShowMessage(wxT("   cmdStr = '%s'\n"), cmdStr.c_str());
      #endif
      #endif
      
      if (mShowToggleState)
         theCommand->SetStringParameter(theCommand->GetParameterID(wxT("ToggleState")), onOff);
      
   }
   catch (BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
   }
   
   #if DEBUG_TOGGLE_PANEL
   MessageInterface::ShowMessage(wxT("TogglePanel::SaveData() leaving\n"));
   #endif
}

//---------------------------------
// event handling
//---------------------------------

//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void TogglePanel::OnComboBoxChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnRadioButtonChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void TogglePanel::OnRadioButtonChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnCheckListBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void TogglePanel::OnCheckListBoxChange(wxCommandEvent& event)
{
   EnableUpdate(true);
}


