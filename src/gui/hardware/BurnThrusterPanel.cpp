//$Id: BurnThrusterPanel.cpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                            BurnThrusterPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number NNG04CC06P.
//
//
// Author: Linda Jun (NASA/GSFC)
// Created: 2009.02.11
/**
 * This class sets up Thruster or ImpulsiveBurn parameters.
 */
//------------------------------------------------------------------------------
#include "BurnThrusterPanel.hpp"
#include "MessageInterface.hpp"
#include "StringUtil.hpp"
#include "ThrusterCoefficientDialog.hpp"
#include "GmatStaticBoxSizer.hpp"
#include <wx/variant.h>
#include <wx/config.h>

//#define DEBUG_BURNPANEL_CREATE
//#define DEBUG_BURNPANEL_LOAD
//#define DEBUG_BURNPANEL_SAVE

//------------------------------
// event tables for wxWindows
//------------------------------
BEGIN_EVENT_TABLE(BurnThrusterPanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_TEXT(ID_TEXTCTRL, BurnThrusterPanel::OnTextChange)
   EVT_TEXT(ID_COMBOBOX, BurnThrusterPanel::OnTextChange)
   EVT_CHECKBOX(ID_CHECKBOX, BurnThrusterPanel::OnCheckBoxChange)
   EVT_COMBOBOX(ID_COMBOBOX, BurnThrusterPanel::OnComboBoxChange)
   EVT_BUTTON(ID_BUTTON, BurnThrusterPanel::OnButtonClick)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------

//------------------------------------------------------------------------------
// BurnThrusterPanel(wxWindow *parent, const wxString &name)
//------------------------------------------------------------------------------
/**
 * Constructs BurnThrusterPanel object.
 */
//------------------------------------------------------------------------------
BurnThrusterPanel::BurnThrusterPanel(wxWindow *parent, const wxString &name)
   : GmatPanel(parent, true, true)
{
   mObjectName = name.c_str();
   theObject = theGuiInterpreter->GetConfiguredObject(name.c_str());
   
   #ifdef DEBUG_BURNPANEL_CREATE
   MessageInterface::ShowMessage
      (wxT("BurnThrusterPanel() constructor entered, theObject=<%p>'%s'\n"),
       theObject, theObject->GetTypeName().c_str());
   #endif
   
   isCoordSysChanged = false;
   isTankChanged = false;
   isTankEmpty = false;
   coordSysName = wxT("");
   tankName = wxT("");
}


//------------------------------------------------------------------------------
// ~Burnthrusterpanel()
//------------------------------------------------------------------------------
BurnThrusterPanel::~BurnThrusterPanel()
{
   theGuiManager->UnregisterComboBox(wxT("CoordinateSystem"), coordSysComboBox);
   theGuiManager->UnregisterComboBox(wxT("CelestialBody"), originComboBox);
   theGuiManager->UnregisterComboBox(wxT("FuelTank"), tankComboBox);
}

//-------------------------------
// private methods
//-------------------------------

//----------------------------------
// methods inherited from GmatPanel
//----------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void BurnThrusterPanel::Create()
{
   #ifdef DEBUG_BURNPANEL_CREATE
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::Create() entered\n"));
   #endif
   
   Integer bsize = 2; // border size
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Burn Thruster"));
   
   //-----------------------------------------------------------------
   // coordinate system items
   //-----------------------------------------------------------------
   // Coordinate Systems 
   wxStaticText *coordSysLabel =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Coordinate System"));
   coordSysComboBox  =
      theGuiManager->GetCoordSysComboBox(this, ID_COMBOBOX, wxSize(150,-1));
   coordSysComboBox->SetToolTip(pConfig->Read(wxT("CoordinateSystemHint")));
   
   // Addd Local to CoordinateSystem list
   coordSysComboBox->Insert(wxT("Local"), 0);
   
   // Origin
   originLabel = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Origin"));
   originComboBox =
      theGuiManager->GetCelestialBodyComboBox(this, ID_COMBOBOX,
                                              wxSize(150,-1));
   originComboBox->SetToolTip(pConfig->Read(wxT("OriginHint")));
   
   // Axes 
   StringArray axesLabels = theObject->GetPropertyEnumStrings(wxT("Axes"));
   wxArrayString wxAxesLabels = ToWxArrayString(axesLabels);
   
   axisLabel = new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Axes"));
   
   axesComboBox = 
      new wxComboBox(this, ID_COMBOBOX, wxT(""), wxDefaultPosition, 
                     wxSize(150,-1), wxAxesLabels, wxCB_DROPDOWN|wxCB_READONLY);
   axesComboBox->SetSelection(0);
   axesComboBox->SetToolTip(pConfig->Read(wxT("AxesHint")));
   
   //----- Add to sizer
   wxFlexGridSizer *coordSysSizer = new wxFlexGridSizer(3, 0, 0);
   coordSysSizer->Add(coordSysLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   coordSysSizer->Add(coordSysComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   coordSysSizer->Add(20,20);
   
   coordSysSizer->Add(originLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   coordSysSizer->Add(originComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   coordSysSizer->Add(20,20);
   
   coordSysSizer->Add(axisLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   coordSysSizer->Add(axesComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   coordSysSizer->Add(20,20);
   
   GmatStaticBoxSizer *coordSysBoxSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Coordinate System"));
   coordSysBoxSizer->Add(coordSysSizer, 0, wxALIGN_CENTER|wxALL, 0);
   
   //-----------------------------------------------------------------
   // thrust vector items
   //-----------------------------------------------------------------
   
   // ThrustDirection1
   wxStaticText *elem1Unit = new wxStaticText(this, ID_TEXT, wxT(" km/s"));
   wxStaticText *XLabel = new wxStaticText(this, ID_TEXT, wxT("ThrustDirection") wxT(GUI_ACCEL_KEY) wxT("1"));
   elem1TextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                      wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
   elem1TextCtrl->SetToolTip(pConfig->Read(wxT("ThrustDirection1Hint")));
   
   // ThrustDirection2
   wxStaticText *elem2Unit =
      new wxStaticText(this, ID_TEXT, wxT(" km/s"));
   wxStaticText *YLabel =
      new wxStaticText(this, ID_TEXT, wxT("ThrustDirection") wxT(GUI_ACCEL_KEY) wxT("2"),
                        wxDefaultPosition,wxDefaultSize, 0);
   elem2TextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                      wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
   elem2TextCtrl->SetToolTip(pConfig->Read(wxT("ThrustDirection2Hint")));
   
   // ThrustDirection3
   wxStaticText *elem3Unit = new wxStaticText(this, ID_TEXT, wxT(" km/s"));
   wxStaticText *ZLabel = new wxStaticText(this, ID_TEXT, wxT("ThrustDirection") wxT(GUI_ACCEL_KEY) wxT("3"));
   elem3TextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                     wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
   elem3TextCtrl->SetToolTip(pConfig->Read(wxT("ThrustDirection3Hint")));
   
   wxStaticText *dutyCycleLabel = NULL;
   wxStaticText *scaleFactorLabel = NULL;
   
   if (theObject->IsOfType(Gmat::THRUSTER))
   {
      // Thruster Duty Cycle
      dutyCycleLabel =
         new wxStaticText(this, ID_TEXT, wxT("Duty ") wxT(GUI_ACCEL_KEY) wxT("Cycle"));
      dutyCycleTextCtrl =
         new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                        wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      dutyCycleTextCtrl->SetToolTip(pConfig->Read(wxT("DutyCycleHint")));
      
      // Thruster Scale Factor
      scaleFactorLabel =
         new wxStaticText(this, ID_TEXT, wxT("Thrust ") wxT(GUI_ACCEL_KEY) wxT("Scale Factor"));
      scaleFactorTextCtrl =
         new wxTextCtrl(this, ID_TEXTCTRL, wxT(""),
                        wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      scaleFactorTextCtrl->SetToolTip(pConfig->Read(wxT("ThrustScaleFactorHint")));
   }
   
   //----- Add to sizer   
   wxFlexGridSizer *vectorSizer = new wxFlexGridSizer(3, 0, 0);
   vectorSizer->Add(XLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   vectorSizer->Add(elem1TextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   vectorSizer->Add(elem1Unit, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   
   vectorSizer->Add(YLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   vectorSizer->Add(elem2TextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   vectorSizer->Add(elem2Unit, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   
   vectorSizer->Add(ZLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   vectorSizer->Add(elem3TextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   vectorSizer->Add(elem3Unit, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   
   if (theObject->IsOfType(Gmat::THRUSTER))
   {
      vectorSizer->Add(dutyCycleLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
      vectorSizer->Add(dutyCycleTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
      vectorSizer->Add(20,20);
      
      vectorSizer->Add(scaleFactorLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
      vectorSizer->Add(scaleFactorTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
      vectorSizer->Add(20,20);
   }
   
   GmatStaticBoxSizer *vectorBoxSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Thrust Vector"));
   vectorBoxSizer->Add(vectorSizer, 0, wxALIGN_CENTER|wxALL, 0);
   
   //-----------------------------------------------------------------
   // mass change items
   //-----------------------------------------------------------------
   // Decrement mass
   decMassCheckBox =
      new wxCheckBox(this, ID_CHECKBOX, wxT(GUI_ACCEL_KEY) wxT("Decrement Mass"),
                     wxDefaultPosition, wxSize(-1, -1), bsize);
   decMassCheckBox->SetToolTip(pConfig->Read(wxT("DecrementMassHint")));
   
   //Tank
   tankLabel =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Tank"));
   tankComboBox =
      theGuiManager->GetFuelTankComboBox(this, ID_COMBOBOX, wxSize(150,-1));
   tankComboBox->SetToolTip(pConfig->Read(wxT("TankHint")));
   
   ispLabel = NULL;
   ispTextCtrl = NULL;
   ispUnit = NULL;
   // Isp for ImpulsiveBurn only
   if (theObject->IsOfType(Gmat::IMPULSIVE_BURN))
   {
      ispLabel =
         new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Isp"));
      ispTextCtrl =
         new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                        wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
      ispTextCtrl->SetToolTip(pConfig->Read(wxT("IspHint")));
      ispUnit =
         new wxStaticText(this, ID_TEXT, wxT(" s"));
   }
   
   // Gravitational Acceleration
   wxStaticText *gravityAccelLabel =
      new wxStaticText(this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("GravitationalAccel"));
   gravityAccelTextCtrl =
      new wxTextCtrl(this, ID_TEXTCTRL, wxT(""), 
                     wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC));
   gravityAccelTextCtrl->SetToolTip(pConfig->Read(wxT("GravitationalAccelHint")));
   wxStaticText *gravityAccelUnit =
      new wxStaticText(this, ID_TEXT, wxT(" m/s"));
   
   // Coefficients for Thruster only
   if (theObject->IsOfType(Gmat::THRUSTER))
   {
      cCoefButton = new wxButton(this, ID_BUTTON, wxT("Edit ") wxT(GUI_ACCEL_KEY) wxT("Thruster Coef."));
      cCoefButton->SetToolTip(pConfig->Read(wxT("EditThrusterCoefficientHint")));
      kCoefButton = new wxButton(this, ID_BUTTON, wxT("Edit ") wxT(GUI_ACCEL_KEY) wxT("Impulse Coef."));
      kCoefButton->SetToolTip(pConfig->Read(wxT("EditImpulseCoefficientHint")));
   }
   
   wxBoxSizer *coefSizer = new wxBoxSizer(wxHORIZONTAL);
   if (theObject->IsOfType(Gmat::THRUSTER))
   {
      coefSizer->Add(cCoefButton, 0, wxALIGN_CENTER|wxALL, 5);
      coefSizer->Add(kCoefButton, 0, wxALIGN_CENTER|wxALL, 5);
   }
   
   //----- Add to sizer   
   wxFlexGridSizer *massSizer = new wxFlexGridSizer(3, 0, 0);
   massSizer->Add(decMassCheckBox, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   massSizer->Add(20,20);
   massSizer->Add(20,20);
   
   massSizer->Add(tankLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   massSizer->Add(tankComboBox, 0, wxALIGN_LEFT|wxALL, bsize);
   massSizer->Add(20,20);
   
   if (theObject->IsOfType(Gmat::IMPULSIVE_BURN))
   {
      massSizer->Add(ispLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
      massSizer->Add(ispTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
      massSizer->Add(ispUnit, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   }
   
   massSizer->Add(gravityAccelLabel, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   massSizer->Add(gravityAccelTextCtrl, 0, wxALIGN_LEFT|wxALL, bsize);
   massSizer->Add(gravityAccelUnit, 0, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   
   if (theObject->IsOfType(Gmat::THRUSTER))
   {      
      massSizer->Add(20, 20, 0, wxALIGN_LEFT|wxALL, bsize);
      massSizer->Add(20, 20, 0, wxALIGN_LEFT|wxALL, bsize);
      massSizer->Add(20, 20, 0, wxALIGN_LEFT|wxALL, bsize);
   }
   
   GmatStaticBoxSizer *massBoxSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Mass Change"));
   massBoxSizer->Add(massSizer, 0, wxALIGN_CENTER|wxALL, 0);
   
   //-----------------------------------------------------------------
   // add to page sizer
   //-----------------------------------------------------------------
   wxBoxSizer *pageSizer = new wxBoxSizer(wxVERTICAL);
   pageSizer->Add(coordSysBoxSizer, 0, wxALIGN_CENTER|wxGROW|wxALL, bsize);
   pageSizer->Add(vectorBoxSizer, 0, wxALIGN_CENTER|wxGROW|wxALL, bsize);
   pageSizer->Add(massBoxSizer, 0, wxALIGN_CENTER|wxGROW|wxALL, bsize);
   
   if (theObject->IsOfType(Gmat::THRUSTER))
      pageSizer->Add(coefSizer, 0, wxALIGN_CENTER|wxALL, bsize);
   
   theMiddleSizer->Add(pageSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   
   #ifdef DEBUG_BURNPANEL_CREATE
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::Create() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void BurnThrusterPanel::LoadData()
{
   #ifdef DEBUG_BURNPANEL_LOAD
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::LoadData() entered\n"));
   #endif
   
   // Set object pointer for "Show Script"
   mObject = theObject;
   bool isImpBurn = false;
   if (theObject->GetType() == Gmat::IMPULSIVE_BURN)
      isImpBurn = true;
   
   if (isImpBurn)
   {
      thrustDir1 = wxT("Element1");
      thrustDir2 = wxT("Element2");
      thrustDir3 = wxT("Element3");
   }
   else
   {
      thrustDir1 = wxT("ThrustDirection1");
      thrustDir2 = wxT("ThrustDirection2");
      thrustDir3 = wxT("ThrustDirection3");
   }
   
   Integer paramID;
   
   try
   {
      paramID = theObject->GetParameterID(wxT("CoordinateSystem"));
      coordSysName = theObject->GetStringParameter(paramID);
      coordSysComboBox->SetValue(coordSysName.c_str());
      
      paramID = theObject->GetParameterID(wxT("Origin"));
      wxString objName = theObject->GetStringParameter(paramID);
      originComboBox->SetValue(objName.c_str());
      
      paramID = theObject->GetParameterID(wxT("Axes"));
      objName = theObject->GetStringParameter(paramID);
      axesComboBox->SetValue(objName.c_str());
      
      paramID = theObject->GetParameterID(thrustDir1);
      elem1TextCtrl->SetValue(wxVariant(theObject->GetRealParameter(paramID)));
      
      paramID = theObject->GetParameterID(thrustDir2);
      elem2TextCtrl->SetValue(wxVariant(theObject->GetRealParameter(paramID)));
      
      paramID = theObject->GetParameterID(thrustDir3);
      elem3TextCtrl->SetValue(wxVariant(theObject->GetRealParameter(paramID)));
      
      paramID = theObject->GetParameterID(wxT("DecrementMass"));
      decMassCheckBox->SetValue((wxVariant(theObject->GetBooleanParameter(paramID))));
      
      paramID = theObject->GetParameterID(wxT("GravitationalAccel"));
      gravityAccelTextCtrl->SetValue((wxVariant(theObject->GetRealParameter(paramID))));
      
      paramID = theObject->GetParameterID(wxT("Tank"));
      StringArray tanks = theObject->GetStringArrayParameter(paramID);   
      
      if (tanks.empty())
      {
         if (theGuiManager->GetNumFuelTank() > 0)
         {
            tankComboBox->Insert(wxT("No Fuel Tank Selected"), 0);
            tankComboBox->SetSelection(0);
         }
      }
      else
      {
         tankName = tanks[0];
         tankComboBox->SetValue(tankName.c_str());
         isTankEmpty = false;
      }
      
      // Disable tank combo box if decrement mass is not checked
      if (!decMassCheckBox->IsChecked())
      {
         tankLabel->Disable();
         tankComboBox->Disable();

         if (theObject->GetType() == Gmat::IMPULSIVE_BURN)
         {
            ispLabel->Disable();
            ispTextCtrl->Disable();
            ispUnit->Disable();
         }
      }
      
      // Update Origin and Axes
      UpdateOriginAxes();
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
   }
   
   #ifdef DEBUG_BURNPANEL_LOAD
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::LoadData() exiting\n"));
   #endif
}

//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
void BurnThrusterPanel::SaveData()
{
   #ifdef DEBUG_BURNPANEL_SAVE
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::SaveData() entered\n"));
   #endif
   
   canClose = true;
   wxString str;
   Real elem1, elem2, elem3, gravityAccel;
   bool realDataChanged = false;
   
   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   if (elem1TextCtrl->IsModified() || elem2TextCtrl->IsModified() ||
       elem3TextCtrl->IsModified() || gravityAccelTextCtrl->IsModified())
   {
      str = elem1TextCtrl->GetValue();
      CheckReal(elem1, str, thrustDir1, wxT("Real Number"));
      
      str = elem2TextCtrl->GetValue();
      CheckReal(elem2, str, thrustDir2, wxT("Real Number"));
      
      str = elem3TextCtrl->GetValue();
      CheckReal(elem3, str, thrustDir3, wxT("Real Number"));
      
      str = gravityAccelTextCtrl->GetValue();
      CheckReal(gravityAccel, str, wxT("GravitationalAccel"), wxT("Real Number > 0"), false,
                true, true);
      
      realDataChanged = true;      
   }
   
   if (!canClose)
      return;
   
   try 
   {
      Integer paramID;
      
      // Coordinate System      
      if (isCoordSysChanged)
      {
         paramID = theObject->GetParameterID(wxT("CoordinateSystem"));
         theObject->SetStringParameter(paramID, coordSysName);
         isCoordSysChanged = false;
      }
      
      if (coordSysName == wxT("Local"))
      {
         // Origin
         paramID = theObject->GetParameterID(wxT("Origin"));
         theObject->SetStringParameter(paramID, originComboBox->GetValue().c_str());
         
         // Axes
         paramID = theObject->GetParameterID(wxT("Axes"));
         theObject->SetStringParameter(paramID, axesComboBox->GetValue().c_str());
      }
      
      // Save ThrustDirections
      if (realDataChanged)
      {
         paramID = theObject->GetParameterID(thrustDir1);
         theObject->SetRealParameter(paramID, elem1);
         
         paramID = theObject->GetParameterID(thrustDir2);
         theObject->SetRealParameter(paramID, elem2);
         
         paramID = theObject->GetParameterID(thrustDir3);
         theObject->SetRealParameter(paramID, elem3);
         
         paramID = theObject->GetParameterID(wxT("GravitationalAccel"));
         theObject->SetRealParameter(paramID, gravityAccel);
      }
      
      // Always save DecrementMass
      // @todo If some base code computation involved, have separate flag (LOJ)
      paramID = theObject->GetParameterID(wxT("DecrementMass"));
      if (decMassCheckBox->IsChecked())
         theObject->SetBooleanParameter(paramID, true);
      else
         theObject->SetBooleanParameter(paramID, false);
      
      // Save Tank
      if (isTankChanged)
      {
         isTankChanged = false;
         paramID = theObject->GetParameterID(wxT("Tank"));
         
         if (theObject->TakeAction(wxT("ClearTanks"), wxT("")))
            if (tankName != wxT(""))
               theObject->SetStringParameter(paramID, tankName.c_str());
      }
   }
   catch(BaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }
   
   #ifdef DEBUG_BURNPANEL_SAVE
   MessageInterface::ShowMessage(wxT("BurnThrusterPanel::SaveData() exiting\n"));
   #endif
}

//------------------------------------------------------------------------------
// void OnTextChange()
//------------------------------------------------------------------------------
void BurnThrusterPanel::OnTextChange(wxCommandEvent &event)
{
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnCheckBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
void BurnThrusterPanel::OnCheckBoxChange(wxCommandEvent& event)
{
   if (decMassCheckBox->IsChecked())
   {
      tankLabel->Enable();
      tankComboBox->Enable();
      if (theObject->GetType() == Gmat::IMPULSIVE_BURN)
      {
         ispLabel->Enable();
         ispTextCtrl->Enable();
         ispUnit->Enable();
      }
   }
   else
   {
      tankLabel->Disable();
      tankComboBox->Disable();
      if (theObject->GetType() == Gmat::IMPULSIVE_BURN)
      {
         ispLabel->Disable();
         ispTextCtrl->Disable();
         ispUnit->Disable();
      }
   }
   
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnComboBoxChange()
//------------------------------------------------------------------------------
void BurnThrusterPanel::OnComboBoxChange(wxCommandEvent &event)
{
   if (event.GetEventObject() == coordSysComboBox)
   {
      UpdateOriginAxes();      
      isCoordSysChanged =  true;
      coordSysName = coordSysComboBox->GetStringSelection().c_str();
      EnableUpdate(true);
   }
   else if (event.GetEventObject() == tankComboBox)
   {
      isTankChanged = true;
      tankName = tankComboBox->GetStringSelection().c_str();
      if (tankName == wxT("No Fuel Tank Selected"))
         tankName = wxT("");
      
      // remove "No Tank Selected" once tank is selected
      int pos = tankComboBox->FindString(wxT("No Fuel Tank Selected"));
      if (pos != wxNOT_FOUND)
         tankComboBox->Delete(pos);
      
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnButtonClick()
//------------------------------------------------------------------------------
void BurnThrusterPanel::OnButtonClick(wxCommandEvent &event)
{  
    if (event.GetEventObject() == cCoefButton)
    {
       ThrusterCoefficientDialog tcDlg(this, -1, wxT("ThrusterCoefficientDialog"), theObject, wxT("C"));
       tcDlg.ShowModal();
    }
    else if (event.GetEventObject() == kCoefButton)
    {
       ThrusterCoefficientDialog tcDlg(this, -1, wxT("ImpulseCoefficientDialog"), theObject, wxT("K"));
       tcDlg.ShowModal();
    }
}


//------------------------------------------------------------------------------
// void UpdateOriginAxes()
//------------------------------------------------------------------------------
void BurnThrusterPanel::UpdateOriginAxes()
{
   if (coordSysComboBox->GetValue() == wxT("Local"))
   {
      originLabel->Enable();
      originComboBox->Enable();
      axisLabel->Enable();
      axesComboBox->Enable();
   }
   else
   {
      originLabel->Disable();
      originComboBox->Disable();
      axisLabel->Disable();
      axesComboBox->Disable();
   }
}
