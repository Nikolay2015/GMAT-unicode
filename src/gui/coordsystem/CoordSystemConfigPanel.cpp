//$Id: CoordSystemConfigPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                              CoordSystemConfigPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2004/10/26
// Updated: 2005/03/11 by A. Greene to implement Load/Save Data
/**
 * This class contains the Coordinate System Setup Panel.
 */
//------------------------------------------------------------------------------
#include "CoordSystemConfigPanel.hpp"

// gui includes
#include "gmatwxdefs.hpp"

// base includes
#include "gmatdefs.hpp"
#include "AxisSystem.hpp"
#include "MessageInterface.hpp"

//#define DEBUG_COORD_PANEL_LOAD 1
//#define DEBUG_COORD_PANEL_SAVE 1
//#define DEBUG_COORD_PANEL_TEXT

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(CoordSystemConfigPanel, GmatPanel)
    EVT_TEXT(ID_TEXTCTRL, CoordSystemConfigPanel::OnTextUpdate)
    EVT_COMBOBOX(ID_COMBO, CoordSystemConfigPanel::OnComboUpdate)
END_EVENT_TABLE()

//------------------------------------------------------------------------------
// CoordSystemConfigPanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
CoordSystemConfigPanel::CoordSystemConfigPanel(wxWindow *parent,
                                               const wxString &coordName)
   : GmatPanel(parent)
{
   theCoordSys =
      (CoordinateSystem*) theGuiInterpreter->GetConfiguredObject(coordName.c_str());
   mEpochFormat = wxT("A1ModJulian");
   mOriginChanged = false;
   mObjRefChanged = false;
   mEpochChanged  = false;
   
   Create();
   Show();
}


//------------------------------------------------------------------------------
// ~CoordSystemConfigPanel()
//------------------------------------------------------------------------------
CoordSystemConfigPanel::~CoordSystemConfigPanel()
{
}

//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void CoordSystemConfigPanel::Create()
{
   wxString coordSysName = theCoordSys->GetName();

   // if CoordinateSystem is non-default, allow user to edit
   if (theGuiInterpreter->IsDefaultCoordinateSystem(coordSysName))
      mCoordPanel = new CoordPanel(this, false);
   else
      mCoordPanel = new CoordPanel(this, true);
   
   theMiddleSizer->Add( mCoordPanel, 0, wxALIGN_CENTRE|wxALL, 3);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void CoordSystemConfigPanel::LoadData()
{
   #if DEBUG_COORD_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("CoordSystemConfigPanel::LoadData() entered\n"));
   #endif
   
   // Set the pointer for the "Show Script" button
   mObject = theCoordSys;
   
   epochTextCtrl = mCoordPanel->GetEpochTextCtrl();
//   intervalTextCtrl = mCoordPanel->GetIntervalTextCtrl();
   
   originComboBox = mCoordPanel->GetOriginComboBox();
   typeComboBox = mCoordPanel->GetTypeComboBox();
   primaryComboBox = mCoordPanel->GetPrimaryComboBox();
//   formatComboBox = mCoordPanel->GetFormatComboBox();
   secondaryComboBox = mCoordPanel->GetSecondaryComboBox();
   
   xComboBox = mCoordPanel->GetXComboBox();
   yComboBox = mCoordPanel->GetYComboBox();
   zComboBox = mCoordPanel->GetZComboBox();

   // get the data from the base
   try
   {
      wxString str = theCoordSys->GetStringParameter(wxT("Origin"));
      
      #if DEBUG_COORD_PANEL_LOAD
      MessageInterface::ShowMessage(wxT("   setting Origin to %s\n"), str.c_str());
      #endif
      
      originComboBox->SetValue(str.c_str());
      
      AxisSystem *axis =
         (AxisSystem *)theCoordSys->GetRefObject(Gmat::AXIS_SYSTEM, wxT(""));
      
      if (axis != NULL)
      {
         mCoordPanel->ShowAxisData(axis);
         mEpochFormat = wxString(axis->GetEpochFormat().c_str());
      
         if (mEpochFormat == wxT(""))
            mEpochFormat = wxT("A1ModJulian");
      }
      else
      {
         MessageInterface::ShowMessage
            (wxT("CoordSystemConfigPanel::LoadData() the AxisSystem of %s is NULL\n"),
             theCoordSys->GetName().c_str());
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void CoordSystemConfigPanel::SaveData()
{
   canClose = true;
   wxString originName = originComboBox->GetValue().Trim().c_str();
   
   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      //-------------------------------------------------------
      // check/set new epoch
      //-------------------------------------------------------
      if (mEpochChanged)
      {
         wxString savedEpoch = (mCoordPanel->epochValue).c_str();
         Real epoch;
         wxString str = mCoordPanel->GetEpochTextCtrl()->GetValue().c_str();
         #if DEBUG_COORD_PANEL_SAVE
         MessageInterface::ShowMessage(wxT("Epoch data has been changed!!!!\n"));
         MessageInterface::ShowMessage
            (wxT("CoordSystemConfigPanel::SaveData() epoch value = %s\n"),
             str.c_str());
         #endif
         bool isValid = CheckReal(epoch, str, wxT("Epoch"), wxT("Real Number >= 0"));
         #if DEBUG_COORD_PANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("CoordSystemConfigPanel::SaveData() isValid = %s, and epoch real value = %12.10f\n"),
             (isValid? wxT("true") : wxT("false")), epoch);
         #endif

         // check range here too

//         if (isValid && epoch < 0.0)
//            isValid = CheckReal(epoch, str, "Epoch", "Real Number >= 0", true);
         if (isValid)
            isValid = CheckRealRange(str, epoch, wxT("Epoch"), 6116.0, 0.0, true, false, true, false);
         if (!isValid)
         {
            canClose = false;
         }
      }

      if (!canClose)
         return;


      //-------------------------------------------------------
      // set new origin
      //-------------------------------------------------------
      if (mOriginChanged)
      {
         #if DEBUG_COORD_PANEL_SAVE
         MessageInterface::ShowMessage
            (wxT("CoordSystemConfigPanel::SaveData() originName = %s\n"),
             originName.c_str());
         #endif
         
         mOriginChanged = false;
         theCoordSys->SetStringParameter(wxT("Origin"), originName);
         
         // set coordinate system origin
         SpacePoint *origin =
            (SpacePoint*)theGuiInterpreter->GetConfiguredObject(originName);
         theCoordSys->SetOrigin(origin);
         
         // set Earth as J000Body if NULL
         if (origin->GetJ2000Body() == NULL)
         {
            SpacePoint *j2000body =
               (SpacePoint*)theGuiInterpreter->GetConfiguredObject(wxT("Earth"));
            origin->SetJ2000Body(j2000body);
         }
      }
      
      //-------------------------------------------------------
      // set new axis system
      //-------------------------------------------------------
      if (mObjRefChanged)
      {
         AxisSystem *axis = mCoordPanel->CreateAxis();
         
         if (axis != NULL)
         {
            //loj: 2007.07.10
            // old axis is deleted in the base CoordianteSyatem::SetRefObject();
            //AxisSystem *oldAxis =
            //   (AxisSystem *)theCoordSys->GetRefObject(Gmat::AXIS_SYSTEM, "");
            // delete old axis and set new axis
            //delete oldAxis;
            
            canClose = mCoordPanel->SaveData(theCoordSys->GetName(), axis, mEpochFormat);
            
            theCoordSys->SetRefObject(axis, Gmat::AXIS_SYSTEM, wxT(""));
            theCoordSys->Initialize();
         }
         else
         {
            MessageInterface::ShowMessage
               (wxT("CoordSystemConfigPanel::SaveData() Cannot create AxisSystem.\n"));
            canClose = false;
         }
      }
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
   }
}


//------------------------------------------------------------------------------
// void OnTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CoordSystemConfigPanel::OnTextUpdate(wxCommandEvent& event)
{
   mObjRefChanged = true;
   EnableUpdate(true);
   if (mCoordPanel->GetEpochTextCtrl()->IsModified() )
   {
      mEpochChanged = true;
      #ifdef DEBUG_COORD_PANEL_TEXT
         MessageInterface::ShowMessage(wxT("Text has been updated and epoch has been changed!!!\n"));
      #endif
   }
}


//------------------------------------------------------------------------------
// void OnComboUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void CoordSystemConfigPanel::OnComboUpdate(wxCommandEvent& event)
{
   if (event.GetEventObject() == originComboBox)
   {
      mOriginChanged = true;
   }
   else if (event.GetEventObject() == typeComboBox)
   {
      mCoordPanel->EnableOptions();
      mObjRefChanged = true;
   }
//   else if (event.GetEventObject() == formatComboBox)
//   {
//      mCoordPanel->ChangeEpoch(mEpochFormat);
//   }
   else if (event.GetEventObject() == primaryComboBox ||
            event.GetEventObject() == secondaryComboBox)
   {
      mObjRefChanged = true;
   }
   else if (event.GetEventObject() == xComboBox ||
            event.GetEventObject() == yComboBox ||
            event.GetEventObject() == zComboBox)
   {
      mObjRefChanged = true;
   }
   
   EnableUpdate(true);
}
