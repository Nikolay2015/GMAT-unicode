//$Id: ReportPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              ReportPanel
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
// Created: 2006/08/10
/**
 * Implements ReportPanel class. This class allows user to setup Report command.
 */
//------------------------------------------------------------------------------
#include "ReportPanel.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "GuiInterpreter.hpp"
#include "GmatAppData.hpp"
#include "ParameterSelectDialog.hpp"
#include "ParameterInfo.hpp"            // for GetDepObjectType()
#include "ReportFile.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_REPORTPANEL
//#define DEBUG_REPORTPANEL_LOAD
//#define DEBUG_REPORTPANEL_SAVE

//------------------------------
// event tables for wxWindows
//------------------------------
BEGIN_EVENT_TABLE(ReportPanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_BUTTON(ID_BUTTON_HELP, GmatPanel::OnHelp)
   
   EVT_BUTTON(ID_BUTTON, ReportPanel::OnButtonClick)
   EVT_COMBOBOX(ID_COMBOBOX, ReportPanel::OnComboBoxChange)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------
    
//------------------------------------------------------------------------------
// ReportPanel(wxWindow *parent, const GmatCommand *cmd)
//------------------------------------------------------------------------------
/**
 * Constructs ReportPanel object.
 *
 * @param <parent> input parent.
 * @param <cmd> input command.
 */
//------------------------------------------------------------------------------
ReportPanel::ReportPanel(wxWindow *parent, GmatCommand *cmd)
   : GmatPanel(parent)
{
   #ifdef DEBUG_REPORTPANEL
   MessageInterface::ShowMessage(wxT("ReportPanel() entering...\n"));
   MessageInterface::ShowMessage(wxT("ReportPanel() cmd = ") +
                                 cmd->GetTypeName() + wxT("\n"));
   #endif
   
   theCommand = cmd;
   mHasReportFileChanged = false;
   mHasParameterChanged = false;
   
   if (theCommand != NULL)
   {
      mObjectTypeList.Add(wxT("Spacecraft"));
      mObjectTypeList.Add(wxT("ImpulsiveBurn"));
      Create();
      Show();
      EnableUpdate(false);
   }
}


//------------------------------------------------------------------------------
// ~ReportPanel()
//------------------------------------------------------------------------------
ReportPanel::~ReportPanel()
{
   mObjectTypeList.Clear();
   theGuiManager->UnregisterComboBox(wxT("ReportFile"), mReportFileComboBox);   
}


//----------------------------------
// methods inherited from GmatPanel
//----------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 * @param <scName> input spacecraft name.
 *
 * @note Creates the notebook for spacecraft information
 */
//------------------------------------------------------------------------------
void ReportPanel::Create()
{
   #ifdef DEBUG_REPORTPANEL
   MessageInterface::ShowMessage(wxT("ReportPanel::Create() entering...\n"));
   #endif
   
   Integer bsize = 2; // border size
   wxArrayString emptyList;   
   
   //------------------------------------------------------
   // report file
   //------------------------------------------------------
   wxStaticText *reportFileLabel =
      new wxStaticText(this, -1, wxT("ReportFile: "), wxDefaultPosition,
                       wxDefaultSize, 0);
   
   mReportFileComboBox = theGuiManager->
      GetReportFileComboBox(this, ID_COMBOBOX, wxSize(200, 20));
   
   wxBoxSizer *reportFileSizer = new wxBoxSizer(wxHORIZONTAL);
   reportFileSizer->Add(reportFileLabel, 0, wxALIGN_CENTRE|wxALL, bsize);
   reportFileSizer->Add(mReportFileComboBox, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   //-------------------------------------------------------
   // selected parameter list
   //-------------------------------------------------------   
   mSelectedListBox =
      new wxListBox(this, ID_LISTBOX, wxDefaultPosition, wxSize(200, 150),
                    emptyList, wxLB_SINGLE);
   
   mViewButton = new wxButton( this, ID_BUTTON, wxT("View"), 
                                wxDefaultPosition, wxDefaultSize, 0 );
   
   GmatStaticBoxSizer *selectedSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Parameter List"));
   
   selectedSizer->Add(mSelectedListBox, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);
   selectedSizer->Add(mViewButton, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   //-------------------------------------------------------
   // put in the order
   //-------------------------------------------------------
   wxFlexGridSizer *panelSizer = new wxFlexGridSizer(2, 1);
   panelSizer->Add(reportFileSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   panelSizer->Add(20, 10, 0, wxALIGN_CENTRE|wxALL, bsize);
   panelSizer->Add(selectedSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);
   
   //-------------------------------------------------------
   // add to parent sizer
   //-------------------------------------------------------
   theMiddleSizer->Add(panelSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   #ifdef DEBUG_REPORTPANEL
   MessageInterface::ShowMessage(wxT("ReportPanel::Create() exiting...\n"));
   #endif
}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void ReportPanel::LoadData()
{
   #ifdef DEBUG_REPORTPANEL_LOAD
   MessageInterface::ShowMessage(wxT("ReportPanel::LoadData() entering...\n"));
   #endif

   try
   {
      // Set the pointer for the "Show Script" button
      mObject = theCommand;
      
      // Get ReportFile name
      wxString rfName = theCommand->GetRefObjectName(Gmat::SUBSCRIBER);
      mReportFileComboBox->SetValue(rfName.c_str());
      
      #ifdef DEBUG_REPORTPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   ReportFile name='%s'\n"), rfName.c_str());
      #endif
      
      // Get parameters to report
      StringArray parameterList = theCommand->GetRefObjectNameArray(Gmat::PARAMETER);
      mNumParameters = parameterList.size();
      
      #ifdef DEBUG_REPORTPANEL_LOAD
      MessageInterface::ShowMessage(wxT("   mNumParameters=%d\n"), mNumParameters);
      #endif
      
      if (mNumParameters > 0)
      {      
         Parameter *param;
         wxString paramName;
         
         for (int i=0; i<mNumParameters; i++)
         {
            param = theGuiInterpreter->GetParameter(parameterList[i]);
            paramName = parameterList[i].c_str();
            mSelectedListBox->Append(paramName);
            mReportWxStrings.Add(paramName);
         }
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_REPORTPANEL_LOAD
   MessageInterface::ShowMessage(wxT("ReportPanel::LoadData() exiting...\n"));
   #endif
   
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void ReportPanel::SaveData()
{
   #ifdef DEBUG_REPORTPANEL_SAVE
   MessageInterface::ShowMessage(wxT("ReportPanel::SaveData() entering...\n"));
   #endif
   
   canClose = true;
   
   wxString rfName = mReportFileComboBox->GetValue().c_str();
   
   ReportFile *reportFile =
      (ReportFile*)theGuiInterpreter->GetConfiguredObject(rfName);
   
   #ifdef DEBUG_REPORTPANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("   ReportFile name='%s', addr=%p\n"), rfName.c_str(), reportFile);
   #endif
   
   if (reportFile == NULL)
      return;
   
   try
   {
      // save ReportFile
      if (mHasReportFileChanged)
      {
         #ifdef DEBUG_REPORTPANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("    rfName=%s, reportFile=%p\n"), rfName.c_str(), reportFile);
         #endif
         
         theCommand->SetRefObject(reportFile, Gmat::SUBSCRIBER, rfName, 0);
         mHasReportFileChanged = false;
      }
      
      // save parameters
      if (mHasParameterChanged)
      {
         mHasParameterChanged = false;
         theCommand->TakeAction(wxT("Clear"));
         Parameter *param = NULL;
         mNumParameters = mSelectedListBox->GetCount();
         
         #ifdef DEBUG_REPORTPANEL_SAVE
         MessageInterface::ShowMessage(wxT("   mNumParameters=%d\n"), mNumParameters);
         #endif
         
         for (int i=0; i<mNumParameters; i++)
         {
            wxString selName =
               wxString(mSelectedListBox->GetString(i).c_str());
            
            #ifdef DEBUG_REPORTPANEL_SAVE
            MessageInterface::ShowMessage(wxT("   selName='%s'\n"), selName.c_str());
            #endif
            
            param = theGuiInterpreter->GetParameter(selName);
            theCommand->SetStringParameter(wxT("Add"), selName);
            theCommand->SetRefObject(param, Gmat::PARAMETER, selName, i);
         }
         
         // Now validate command and create element wrappers
         if (!theGuiInterpreter->ValidateCommand(theCommand))
            canClose = false;
         
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
   }
   
   #ifdef DEBUG_REPORTPANEL_SAVE
   MessageInterface::ShowMessage(wxT("ReportPanel::SaveData() exiting...\n"));
   #endif
   
}


//------------------------------------------------------------------------------
// void OnButtonClick()
//------------------------------------------------------------------------------
void ReportPanel::OnButtonClick(wxCommandEvent& event)
{
   if (event.GetEventObject() == mViewButton)
   {
      // Note:: 2008.01.23
      // The Report command cannot take Arrays yet, so set allowArray to false
      ParameterSelectDialog paramDlg(this, mObjectTypeList,
                                     GuiItemManager::SHOW_REPORTABLE, true, true,
                                     false, true, true, false);
      
      paramDlg.SetParamNameArray(mReportWxStrings);
      paramDlg.ShowModal();
      
      if (paramDlg.HasSelectionChanged())
      {
         mHasParameterChanged = true;
         EnableUpdate(true);
         mReportWxStrings = paramDlg.GetParamNameArray();
         
         if (mReportWxStrings.Count() > 0)
         {
            mSelectedListBox->Clear();
            for (unsigned int i=0; i<mReportWxStrings.Count(); i++)
               mSelectedListBox->Append(mReportWxStrings[i]);
         }
         else // no selections
         {
            mSelectedListBox->Clear();
         }
      }
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void ReportPanel::OnComboBoxChange(wxCommandEvent& event)
{
   wxObject *obj = event.GetEventObject();
   
   if (obj == mReportFileComboBox)
   {
      mHasReportFileChanged = true;
      EnableUpdate(true);
   }
}


