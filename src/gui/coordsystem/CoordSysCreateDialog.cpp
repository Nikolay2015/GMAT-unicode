//$Id: CoordSysCreateDialog.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              CoordSysCreateDialog
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Allison Greene
// Created: 2005/03/03
//
/**
 * Implements CoordSysCreateDialog class. This class shows dialog window where a
 * coordinate system can be created.
 * 
 */
//------------------------------------------------------------------------------

#include "CoordSysCreateDialog.hpp"
#include "MessageInterface.hpp"
#include "CoordinateSystem.hpp"
#include "AxisSystem.hpp"
#include <wx/config.h>

//#define DEBUG_COORD_DIALOG 1
//#define DEBUG_COORD_DIALOG_SAVE 1

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(CoordSysCreateDialog, GmatDialog)
   EVT_TEXT(ID_TEXTCTRL, CoordSysCreateDialog::OnTextUpdate)
   EVT_COMBOBOX(ID_COMBO, CoordSysCreateDialog::OnComboBoxChange)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// CoordSysCreateDialog(wxWindow *parent)
//------------------------------------------------------------------------------
CoordSysCreateDialog::CoordSysCreateDialog(wxWindow *parent)
   : GmatDialog(parent, -1, wxString(wxT("CoordSysCreateDialog")))
{
   mIsCoordCreated = false;
   mIsTextModified = false;
   
   Create();
   ShowData();
}

//------------------------------------------------------------------------------
// virtual void Create()
//------------------------------------------------------------------------------
void CoordSysCreateDialog::Create()
{
    // get the config object
    wxConfigBase *pConfig = wxConfigBase::Get();
    // SetPath() understands ".."
    pConfig->SetPath(wxT("/Coordinate System"));

    // wxStaticText
    nameStaticText = new wxStaticText( this, ID_TEXT,
         wxT("Coordinate System ") wxT(GUI_ACCEL_KEY) wxT("Name"), wxDefaultPosition, wxDefaultSize, 0 );

    // wxTextCtrl
    nameTextCtrl = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""),
         wxDefaultPosition, wxSize(160,-1), 0 );
    nameTextCtrl->SetToolTip(pConfig->Read(wxT("NameHint")));

    mCoordPanel = new CoordPanel(this, true);

    // wx*Sizers
    wxBoxSizer *boxsizer1 = new wxBoxSizer( wxVERTICAL );

    boxsizer1->Add( nameStaticText, 0, wxALIGN_CENTER|wxALL, 5 );
    boxsizer1->Add( nameTextCtrl, 0, wxALIGN_CENTER|wxALL, 5 );

    theMiddleSizer->Add(boxsizer1, 0, wxALIGN_CENTRE|wxALL, 5);
    theMiddleSizer->Add( mCoordPanel, 0, wxALIGN_CENTRE|wxALL, 5);
}


//------------------------------------------------------------------------------
// virtual void LoadData()
//------------------------------------------------------------------------------
void CoordSysCreateDialog::LoadData()
{
   try
   {
      epochTextCtrl = mCoordPanel->GetEpochTextCtrl();

      originComboBox = mCoordPanel->GetOriginComboBox();
      typeComboBox = mCoordPanel->GetTypeComboBox();
      primaryComboBox = mCoordPanel->GetPrimaryComboBox();
//      formatComboBox = mCoordPanel->GetFormatComboBox();
      secondaryComboBox = mCoordPanel->GetSecondaryComboBox();

      xComboBox = mCoordPanel->GetXComboBox();
      yComboBox = mCoordPanel->GetYComboBox();
      zComboBox = mCoordPanel->GetZComboBox();

//      intervalTextCtrl = mCoordPanel->GetIntervalTextCtrl();

      mCoordPanel->SetDefaultAxis();
            
//      wxFormatName = formatComboBox->GetValue().Trim();
      mCoordPanel->EnableOptions();
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("CoordSysCreateDialog:LoadData() error occurred!\n%s\n"),
            e.GetFullMessage().c_str());
   }
   
}


//------------------------------------------------------------------------------
// virtual void SaveData()
//------------------------------------------------------------------------------
void CoordSysCreateDialog::SaveData()
{
   #if DEBUG_COORD_DIALOG_SAVE
   MessageInterface::ShowMessage
      (wxT("CoordSysCreateDialog::SaveData() mIsTextModified=%d\n"),
       mIsTextModified);
   #endif
   
   canClose = true;
   
   wxString coordName = wxString(nameTextCtrl->GetValue().Trim().c_str());
   if (coordName == wxT(""))
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Please enter Coordinate System name"));
      canClose = false;
      return;
   }
   
   if (!IsValidName(coordName.c_str()))
      return;
   
   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   if (mIsTextModified)
   {
      Real epoch;
//      Real interval;
      wxString str = mCoordPanel->GetEpochTextCtrl()->GetValue().c_str();
      #if DEBUG_COORD_DIALOG_SAVE
      MessageInterface::ShowMessage
         (wxT("CoordSysCreateDialog::SaveData() epoch value = %s\n"),
          str.c_str());
      #endif
      bool isValid = CheckReal(epoch, str, wxT("Epoch"), wxT("Real Number >= 0"));
      #if DEBUG_COORD_DIALOG_SAVE
      MessageInterface::ShowMessage
         (wxT("CoordSysCreateDialog::SaveData() isValid = %s, and epoch real value = %12.10f\n"),
          (isValid? wxT("true") : wxT("false")), epoch);
      #endif
      
      // check range here
      
//      if (isValid && epoch < 0.0)
//         CheckReal(epoch, str, "Epoch", "Real Number >= 0", true);
      if (isValid)
         CheckRealRange(str, epoch, wxT("Epoch"), 6116.0, 0.0, true, false, true, false);
      
//      str = mCoordPanel->GetIntervalTextCtrl()->GetValue();
//      isValid = CheckReal(interval, str, "UpdateInterval", "Real Number >= 0");
//
////      if (isValid && epoch < 0.0)
//      if (isValid && interval < 0.0)
//         CheckReal(interval, str, "UpdateInterval", "Real Number >= 0", true);
      
   }
   
   if (!canClose)
      return;
   
   mIsTextModified = false;
   
   CoordinateSystem *coord =
      (CoordinateSystem*)theGuiInterpreter->GetConfiguredObject(coordName);
   
   if (mIsCoordCreated)
   {
      AxisSystem *axis =
         (AxisSystem *)coord->GetRefObject(Gmat::AXIS_SYSTEM, wxT(""));
      
      canClose = mCoordPanel->SaveData(coordName, axis, wxFormatName);
   }
   else
   {
      if (coord == NULL)
      {
         // create AxisSystem
         AxisSystem *axis = mCoordPanel->CreateAxis();
         
         if (axis != NULL)
         {
            canClose = mCoordPanel->SaveData(coordName, axis, wxFormatName);
            mIsCoordCreated = true;
            mCoordName = wxString(coordName.c_str());
            mIsCoordCreated = true;
         }
      }
      else
      {
         wxLogError(wxT("The CoordinateSystem \"") + wxString(coordName.c_str()) +
                    wxT("\" already exist. Please enter different name\n"));
         canClose = false;
      }
   }
}


//------------------------------------------------------------------------------
// virtual void ResetData()
//------------------------------------------------------------------------------
void CoordSysCreateDialog::ResetData()
{
   mIsCoordCreated = false;
   mCoordName = wxT("");
}


//---------------------------------
// event handling
//---------------------------------

//------------------------------------------------------------------------------
// void OnTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CoordSysCreateDialog::OnTextUpdate(wxCommandEvent& event)
{
   if (nameTextCtrl->GetValue().Trim() != wxT(""))
   {
      EnableUpdate(true);
   }
   
   if (mCoordPanel->GetEpochTextCtrl()->IsModified() )
//      if (mCoordPanel->GetEpochTextCtrl()->IsModified() ||
//          mCoordPanel->GetIntervalTextCtrl()->IsModified())
   {
      EnableUpdate(true);
      mIsTextModified = true;
   }
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CoordSysCreateDialog::OnComboBoxChange(wxCommandEvent& event)
{
   if (event.GetEventObject() == originComboBox)
   {
   }
   else if (event.GetEventObject() == typeComboBox)
   {
      mCoordPanel->EnableOptions();
   }
//   else if (event.GetEventObject() == formatComboBox)
//   {
//      mCoordPanel->ChangeEpoch(wxFormatName);
//   }
   
   if (nameTextCtrl->GetValue().Trim() != wxT(""))
      EnableUpdate(true);
}

