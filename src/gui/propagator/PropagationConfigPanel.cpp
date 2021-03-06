//$Id: PropagationConfigPanel.cpp 9628 2011-06-21 17:57:39Z wendys-dev $
//------------------------------------------------------------------------------
//                              PropagationConfigPanel
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
// Author: Waka Waktola
// Created: 2003/08/29
//
/**
 * This class contains the Propagation configuration window.
 */
//------------------------------------------------------------------------------
#include "gmatwxdefs.hpp"
#include "GmatAppData.hpp"
#include "CelesBodySelectDialog.hpp"
#include "DragInputsDialog.hpp"
#include "PropagationConfigPanel.hpp"
#include "GmatStaticBoxSizer.hpp"
#include "StringUtil.hpp"               // for GmatStringUtil::ToUpper()
#include "MessageInterface.hpp"
#include "bitmaps/OpenFolder.xpm"
#include "TimeSystemConverter.hpp"
#include "GmatDefaults.hpp"

#include "wx/platform.h"
#include <wx/config.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/txtstrm.h>

//#define DEBUG_PROP_PANEL_SETUP
//#define DEBUG_PROP_PANEL_LOAD
//#define DEBUG_PROP_PANEL_SAVE
//#define DEBUG_PROP_PANEL_GRAV
//#define DEBUG_PROP_PANEL_ATMOS
//#define DEBUG_PROP_PANEL_ERROR
//#define DEBUG_PROP_INTEGRATOR

//------------------------------------------------------------------------------
// event tables and other macros for wxWindows
//------------------------------------------------------------------------------
BEGIN_EVENT_TABLE(PropagationConfigPanel, GmatPanel)
   EVT_BUTTON(ID_BUTTON_OK, GmatPanel::OnOK)
   EVT_BUTTON(ID_BUTTON_APPLY, GmatPanel::OnApply)
   EVT_BUTTON(ID_BUTTON_CANCEL, GmatPanel::OnCancel)
   EVT_BUTTON(ID_BUTTON_SCRIPT, GmatPanel::OnScript)
   EVT_BUTTON(ID_BUTTON_HELP, GmatPanel::OnHelp)
   EVT_BUTTON(ID_BUTTON_ADD_BODY, PropagationConfigPanel::OnAddBodyButton)
   EVT_BUTTON(ID_BUTTON_GRAV_SEARCH, PropagationConfigPanel::OnGravSearchButton)
   EVT_BUTTON(ID_BUTTON_SETUP, PropagationConfigPanel::OnSetupButton)
   EVT_BUTTON(ID_BUTTON_MAG_SEARCH, PropagationConfigPanel::OnMagSearchButton)
   EVT_BUTTON(ID_BUTTON_PM_EDIT, PropagationConfigPanel::OnPMEditButton)
   EVT_BUTTON(ID_BUTTON_SRP_EDIT, PropagationConfigPanel::OnSRPEditButton)
   EVT_TEXT(ID_TEXTCTRL_PROP, PropagationConfigPanel::OnIntegratorTextUpdate)
   EVT_TEXT(ID_TEXTCTRL_GRAV, PropagationConfigPanel::OnGravityTextUpdate)
   EVT_TEXT(ID_TEXTCTRL_MAGF, PropagationConfigPanel::OnMagneticTextUpdate)
   EVT_COMBOBOX(ID_CB_INTGR, PropagationConfigPanel::OnIntegratorComboBox)
   EVT_COMBOBOX(ID_CB_BODY, PropagationConfigPanel::OnPrimaryBodyComboBox)
   EVT_COMBOBOX(ID_CB_ORIGIN, PropagationConfigPanel::OnOriginComboBox)
   EVT_COMBOBOX(ID_CB_GRAV, PropagationConfigPanel::OnGravityModelComboBox)
   EVT_COMBOBOX(ID_CB_ATMOS, PropagationConfigPanel::OnAtmosphereModelComboBox)
   EVT_CHECKBOX(ID_SRP_CHECKBOX, PropagationConfigPanel::OnSRPCheckBoxChange)
   EVT_CHECKBOX(ID_STOP_CHECKBOX, PropagationConfigPanel::OnStopCheckBoxChange)
   EVT_COMBOBOX(ID_CB_ERROR, PropagationConfigPanel::OnErrorControlComboBox)
   EVT_COMBOBOX(ID_CB_PROP_ORIGIN, PropagationConfigPanel::OnPropOriginComboBox)
   EVT_COMBOBOX(ID_CB_PROP_EPOCHFORMAT, PropagationConfigPanel::OnPropEpochComboBox)
END_EVENT_TABLE()


//------------------------------------------------------------------------------
// PropagationConfigPanel()
//------------------------------------------------------------------------------
/**
 * A constructor.
 */
//------------------------------------------------------------------------------
PropagationConfigPanel::PropagationConfigPanel(wxWindow *parent,
                                               const wxString &propName)
   : GmatPanel(parent)
{
   mObjectName = propName.c_str();
   propSetupName = propName.c_str();
   theForceModelName = propSetupName + wxT("_ForceModel");
   primaryBodyData = NULL;
   primaryBody = wxT("None");

   #ifdef DEBUG_PROP_PANEL_SETUP
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel() entered propSetupName='%s'\n"),
       propSetupName.c_str());
   #endif

   Initialize();

   // Get configured SolarSystem and PropSetup
   theSolarSystem = theGuiInterpreter->GetSolarSystemInUse();

   if (theSolarSystem == NULL)
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("Cannot populate the panel, the SolarSystem is NULL"));
      return;
   }

   GmatBase *obj = theGuiInterpreter->GetConfiguredObject(propSetupName);
   if (SetObject(obj))
   {
      thePropSetup = (PropSetup*)obj;
      #ifdef DEBUG_PROP_PANEL_SETUP
      MessageInterface::ShowMessage(wxT("   thePropSetup=<%p>\n"), thePropSetup);
      #endif
      Create();
      Show();
   }

   canClose = true;
   EnableUpdate(false);
}


//------------------------------------------------------------------------------
// ~PropagationConfigPanel()
//------------------------------------------------------------------------------
PropagationConfigPanel::~PropagationConfigPanel()
{
//   for (Integer i=0; i<(Integer)primaryBodyList.size(); i++)
//      delete primaryBodyList[i];
   if (primaryBodyData != NULL)
      delete primaryBodyData;
   for (Integer i=0; i<(Integer)pointMassBodyList.size(); i++)
      delete pointMassBodyList[i];

   // Unregister GUI components
   theGuiManager->UnregisterComboBox(wxT("CoordinateSystem"), theOriginComboBox);
   theGuiManager->UnregisterComboBox(wxT("CelestialBody"), theOriginComboBox);
   theGuiManager->UnregisterComboBox(wxT("CelestialBody"), thePrimaryBodyComboBox);

   theGuiManager->UnregisterComboBox(wxT("CoordinateSystem"), propCentralBodyComboBox);
   theGuiManager->UnregisterComboBox(wxT("CelestialBody"), propCentralBodyComboBox);
}

//------------------------------------------
// Protected methods inherited from GmatPanel
//------------------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
void PropagationConfigPanel::Create()
{
   #ifdef DEBUG_PROP_PANEL_SETUP
   MessageInterface::ShowMessage(wxT("PropagationConfigPanel::Setup() entered\n"));
   #endif

   #if __WXMAC__
   int buttonWidth = 40;
   #else
   int buttonWidth = 25;
   #endif

   Integer bsize = 2; // border size
   wxBitmap openBitmap = wxBitmap(OpenFolder_xpm);

   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Propagator"));

   //-----------------------------------------------------------------
   // Integrator
   //-----------------------------------------------------------------
   // Type
   wxStaticText *integratorStaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Type"),
                        wxDefaultPosition, wxDefaultSize);

   wxString *intgArray = new wxString[IntegratorCount];
   for (Integer i=0; i<IntegratorCount; i++)
      intgArray[i] = integratorArray[i];

   theIntegratorComboBox =
      new wxComboBox( this, ID_CB_INTGR, integratorString,
                      wxDefaultPosition, wxDefaultSize, IntegratorCount,
                      intgArray, wxCB_DROPDOWN|wxCB_READONLY );
   theIntegratorComboBox->SetToolTip(pConfig->Read(wxT("IntegratorTypeHint")));

   // Initial Step Size
   initialStepSizeStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Initial ") wxT(GUI_ACCEL_KEY) wxT("Step Size"),
                        wxDefaultPosition, wxDefaultSize);

   initialStepSizeTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   initialStepSizeTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorInitialStepSizeHint")));

   unitsInitStepSizeStaticText =
      new wxStaticText( this, ID_TEXT, wxT("sec"),
                        wxDefaultPosition, wxDefaultSize );
   // Accuracy
   accuracyStaticText =
      new wxStaticText( this, ID_TEXT, wxT("A") wxT(GUI_ACCEL_KEY) wxT("ccuracy"),
                        wxDefaultPosition, wxDefaultSize );
   accuracyTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   accuracyTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorAccuracyHint")));

   // Minimum Step Size
   minStepStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Mi") wxT(GUI_ACCEL_KEY) wxT("n Step Size"),
                        wxDefaultPosition, wxDefaultSize );
   minStepTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   minStepTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorMinStepSizeHint")));
   unitsMinStepStaticText =
      new wxStaticText( this, ID_TEXT, wxT("sec"),
                        wxDefaultPosition, wxDefaultSize );

   // Maximum Step Size
   maxStepStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Ma") wxT(GUI_ACCEL_KEY) wxT("x Step Size"),
                        wxDefaultPosition, wxDefaultSize );
   maxStepTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   maxStepTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorMaxStepSizeHint")));
   unitsMaxStepStaticText =
      new wxStaticText( this, ID_TEXT, wxT("sec"),
                        wxDefaultPosition, wxDefaultSize );

   // Maximum Step Attempt
   maxStepAttemptStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Max Step ") wxT(GUI_ACCEL_KEY) wxT("Attempts"),
                        wxDefaultPosition, wxDefaultSize );
   maxStepAttemptTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   maxStepAttemptTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorMaxStepAttemptsHint")));

   // Minimum Integration Error
   minIntErrorStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Min ") wxT(GUI_ACCEL_KEY) wxT("Integration Error"),
                        wxDefaultPosition, wxDefaultSize );
   minIntErrorTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC)  );
   minIntErrorTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorMinIntegrationErrorHint")));

   // Nominal Integration Error
   nomIntErrorStaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Nominal Integration Error"),
                        wxDefaultPosition, wxDefaultSize );
   nomIntErrorTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                      wxDefaultPosition, wxSize(160,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   nomIntErrorTextCtrl->SetToolTip(pConfig->Read(wxT("IntegratorNominalIntegrationErrorHint")));

   //-----------------------------------------------------------------
   // StopOnAccuracyViolation
   //-----------------------------------------------------------------
   theStopCheckBox =
      new wxCheckBox( this, ID_STOP_CHECKBOX, wxT(GUI_ACCEL_KEY) wxT("Stop If Accuracy Is Violated"),
                      wxDefaultPosition, wxDefaultSize, 0 );
   theStopCheckBox->SetToolTip(pConfig->Read(wxT("IntegratorStopOnAccuracyViolationHint")));

   //-----------------------------------------------------------------
   // SPK Settings
   // (todo: generalize to other propagators that are not integrators)
   //-----------------------------------------------------------------
   //StepSize = 60;
   propagatorStepSizeStaticText =
         new wxStaticText( this, ID_TEXT, wxT("Step Size"),
                           wxDefaultPosition, wxDefaultSize );
   propagatorStepSizeTextCtrl =
         new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                         wxDefaultPosition, wxSize(160,-1), 0,
                         wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unitsPropagatorStepSizeStaticText =
         new wxStaticText( this, ID_TEXT, wxT("sec"), wxDefaultPosition,
               wxDefaultSize );
   propCentralBodyStaticText =
         new wxStaticText( this, ID_TEXT, wxT("Central Body"),
                           wxDefaultPosition, wxDefaultSize );
   propCentralBodyComboBox=
         theGuiManager->GetCelestialBodyComboBox(this, ID_CB_PROP_ORIGIN, wxSize(100,-1));
   propagatorEpochFormatStaticText =
         new wxStaticText( this, ID_TEXT, wxT("Epoch Format"),
                           wxDefaultPosition, wxDefaultSize );
   wxArrayString emptyList;
   propagatorEpochFormatComboBox = new wxComboBox
      ( this, ID_CB_PROP_EPOCHFORMAT, wxT(""), wxDefaultPosition, wxSize(150,-1), //0,
        emptyList, wxCB_DROPDOWN | wxCB_READONLY );
   propagatorEpochFormatComboBox->SetToolTip(pConfig->Read(wxT("EpochFormatHint")));
   StringArray reps = TimeConverterUtil::GetValidTimeRepresentations();
   for (unsigned int i = 0; i < reps.size(); i++)
      propagatorEpochFormatComboBox->Append(reps[i].c_str());
   startEpochStaticText =
         new wxStaticText( this, ID_TEXT, wxT("Start Epoch"),
                           wxDefaultPosition, wxDefaultSize );
   startEpochTextCtrl =
         new wxTextCtrl( this, ID_TEXTCTRL_PROP, wxT(""),
                         wxDefaultPosition, wxSize(160,-1), 0);

   intFlexGridSizer = new wxFlexGridSizer( 3, 0, 0 );
   intFlexGridSizer->AddGrowableCol(1);
   intFlexGridSizer->Add( integratorStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( theIntegratorComboBox, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 20, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( initialStepSizeStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( initialStepSizeTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( unitsInitStepSizeStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( accuracyStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( accuracyTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( minStepStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( minStepTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( unitsMinStepStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( maxStepStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( maxStepTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( unitsMaxStepStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( maxStepAttemptStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( maxStepAttemptTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( minIntErrorStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( minIntErrorTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( nomIntErrorStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( nomIntErrorTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);

   intFlexGridSizer->Add( propagatorStepSizeStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( propagatorStepSizeTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( unitsPropagatorStepSizeStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( propCentralBodyStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( propCentralBodyComboBox, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( propagatorEpochFormatStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( propagatorEpochFormatComboBox, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( startEpochStaticText, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( startEpochTextCtrl, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 0, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);

   intFlexGridSizer->Add( theStopCheckBox, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);
   intFlexGridSizer->Add( 20, 20, 0, wxGROW|wxALIGN_LEFT|wxALL, bsize);

   // define minimum size for the integrator labels
   int minLabelSize;
   minLabelSize = integratorStaticText->GetBestSize().x;
   minLabelSize = (minLabelSize < initialStepSizeStaticText->GetBestSize().x) ? initialStepSizeStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < accuracyStaticText->GetBestSize().x) ? accuracyStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < minStepStaticText->GetBestSize().x) ? minStepStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < maxStepStaticText->GetBestSize().x) ? maxStepStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < maxStepAttemptStaticText->GetBestSize().x) ? maxStepAttemptStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < minIntErrorStaticText->GetBestSize().x) ? minIntErrorStaticText->GetBestSize().x : minLabelSize;
   minLabelSize = (minLabelSize < nomIntErrorStaticText->GetBestSize().x) ? nomIntErrorStaticText->GetBestSize().x : minLabelSize;

   integratorStaticText->SetMinSize(wxSize(minLabelSize, integratorStaticText->GetMinHeight()));

   intStaticSizer =
      new GmatStaticBoxSizer( wxVERTICAL, this, wxT("Integrator"));
   intStaticSizer->Add( intFlexGridSizer, 0, wxALIGN_CENTRE|wxALL, bsize);

   //-----------------------------------------------------------------
   // Force Model
   //-----------------------------------------------------------------
   // Error Control
   Integer w, h, ecWidth, cbWidth;
   wxString text = wxT("Error Control");
   GetTextExtent(text, &w, &h);
   ecWidth = (Integer)(w * 1.2);
   text = wxT("Central Body");
   GetTextExtent(text, &w, &h);
   cbWidth = (Integer)(w * 1.2);

   w = (ecWidth > cbWidth ? ecWidth : cbWidth);

   wxStaticText *errorCtrlStaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Error Control"),
                        wxDefaultPosition, wxSize(w,20), wxST_NO_AUTORESIZE );

   theErrorComboBox =
      new wxComboBox( this, ID_CB_ERROR, errorControlArray[0],
                      wxDefaultPosition, wxSize(100,-1),
                      errorControlArray, wxCB_DROPDOWN|wxCB_READONLY );
   theErrorComboBox->SetToolTip(pConfig->Read(wxT("ForceModelErrorControlHint")));

   wxFlexGridSizer *errorFlexGridSizer = new wxFlexGridSizer( 2, 0, 0 );
   errorFlexGridSizer->Add( errorCtrlStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   errorFlexGridSizer->Add( theErrorComboBox, 0, wxALIGN_LEFT|wxALL, bsize);

   // Central Body
   wxStaticText *centralBodyStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Central ") wxT(GUI_ACCEL_KEY) wxT("Body"),
                        wxDefaultPosition, wxSize(w,20), wxST_NO_AUTORESIZE);
   theOriginComboBox  =
      theGuiManager->GetCelestialBodyComboBox(this, ID_CB_ORIGIN, wxSize(100,-1));
   theOriginComboBox->SetToolTip(pConfig->Read(wxT("ForceModelCentralBodyHint")));

   wxFlexGridSizer *centralBodySizer = new wxFlexGridSizer( 2, 0, 2 );
   centralBodySizer->Add( centralBodyStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   centralBodySizer->Add( theOriginComboBox, 0, wxALIGN_LEFT|wxALL, bsize);

   // Primary Bodies -- for 1.0, changed to allow only one body; commented code
   // is a start on allowing multiple bodies
   GmatStaticBoxSizer *primaryStaticSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Primary ") wxT(GUI_ACCEL_KEY) wxT("Body"));
//   GmatStaticBoxSizer *primaryStaticSizer =
//      new GmatStaticBoxSizer(wxVERTICAL, this, "Primary "GUI_ACCEL_KEY"Bodies");
   wxArrayString bodyArray;

   thePrimaryBodyComboBox = theGuiManager->GetCelestialBodyComboBox(this,
         ID_CB_BODY, wxSize(100, -1));

//   thePrimaryBodyComboBox =
//      new wxComboBox( this, ID_CB_BODY, wxT(primaryBodyString),
//                      wxDefaultPosition,  wxSize(100,-1),// 0,
//                      bodyArray, wxCB_DROPDOWN|wxCB_READONLY );
   thePrimaryBodyComboBox->SetToolTip(pConfig->Read(wxT("ForceModelPrimaryBodiesComboHint")));

   // Not used; left in place for later use
//   bodyTextCtrl =
//      new wxTextCtrl( this, ID_TEXTCTRL, wxT(""),
//                      wxDefaultPosition, wxSize(160,-1), wxTE_READONLY );
//   bodyTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelPrimaryBodiesEditHint")));
   // Not used; left in place for later use
//   wxButton *primaryBodySelectButton =
//      new wxButton( this, ID_BUTTON_ADD_BODY, wxT("Select"),
//                    wxDefaultPosition, wxDefaultSize, 0 );
//   primaryBodySelectButton->SetToolTip(pConfig->Read(wxT("ForceModelPrimaryBodiesSelectHint")));

   wxBoxSizer *bodySizer = new wxBoxSizer( wxHORIZONTAL );
   bodySizer->Add( thePrimaryBodyComboBox, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);
//   bodySizer->Add( bodyTextCtrl, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);
//   bodySizer->Add( primaryBodySelectButton, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);

   // Gravity
   wxStaticText *type1StaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Model"),
                        wxDefaultPosition, wxDefaultSize, 0 );

   wxArrayString gravArray;
   theGravModelComboBox =
      new wxComboBox( this, ID_CB_GRAV, wxT(""),
                      wxDefaultPosition, wxSize(150,-1), // 0,
                      gravArray, wxCB_DROPDOWN|wxCB_READONLY );
   theGravModelComboBox->SetToolTip(pConfig->Read(wxT("ForceModelGravityModelHint")));
   wxStaticText *degree1StaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Degree"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   gravityDegreeTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_GRAV, wxT(""), wxDefaultPosition,
                      wxSize(30,-1), 0 );
   gravityDegreeTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelGravityDegreeHint")));
   wxStaticText *order1StaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Order"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   gravityOrderTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_GRAV, wxT(""), wxDefaultPosition,
                      wxSize(30,-1), 0 );
   gravityOrderTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelGravityOrderHint")));
   theGravModelSearchButton =
      new wxBitmapButton(this, ID_BUTTON_GRAV_SEARCH, openBitmap, wxDefaultPosition,
                         wxSize(buttonWidth, 20));
   theGravModelSearchButton->SetToolTip(pConfig->Read(wxT("ForceModelGravitySearchHint")));

   wxBoxSizer *degOrdSizer = new wxBoxSizer( wxHORIZONTAL );
   degOrdSizer->Add( type1StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( theGravModelComboBox, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( degree1StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( gravityDegreeTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( order1StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( gravityOrderTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);
   degOrdSizer->Add( theGravModelSearchButton, 0, wxALIGN_CENTRE|wxALL, bsize);

   potFileStaticText =
      new wxStaticText( this, ID_TEXT, wxT("Potential ") wxT(GUI_ACCEL_KEY) wxT("File"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   potFileTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_GRAV, wxT(""), wxDefaultPosition,
                      wxSize(290,-1), 0 );
   potFileTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelGravityPotentialFileHint")));

   wxBoxSizer *potFileSizer = new wxBoxSizer( wxHORIZONTAL );
   potFileSizer->Add( potFileStaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   potFileSizer->Add( potFileTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);

   GmatStaticBoxSizer *gravStaticSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Gravity"));
   gravStaticSizer->Add( degOrdSizer, 0, wxALIGN_LEFT|wxALL, bsize);
   gravStaticSizer->Add( potFileSizer, 0, wxALIGN_LEFT|wxALL, bsize);

   // Drag
   wxStaticText *type2StaticText =
      new wxStaticText( this, ID_TEXT, wxT("Atmosphere ") wxT(GUI_ACCEL_KEY) wxT("Model"),
                        wxDefaultPosition, wxDefaultSize, 0 );

   theAtmosModelComboBox =
      new wxComboBox( this, ID_CB_ATMOS, dragModelArray[0],
                      wxDefaultPosition, wxDefaultSize,
                      dragModelArray, wxCB_DROPDOWN|wxCB_READONLY );
   theAtmosModelComboBox->SetToolTip(pConfig->Read(wxT("ForceModelDragAtmosphereModelHint")));
   theDragSetupButton =
      new wxButton( this, ID_BUTTON_SETUP, wxT("Setup"),
                    wxDefaultPosition, wxDefaultSize, 0 );
   theDragSetupButton->SetToolTip(pConfig->Read(wxT("ForceModelDragSetupHint")));

   wxBoxSizer *atmosSizer = new wxBoxSizer( wxHORIZONTAL );
   atmosSizer->Add( type2StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   atmosSizer->Add( theAtmosModelComboBox, 0, wxALIGN_CENTRE|wxALL, bsize);
   atmosSizer->Add( theDragSetupButton, 0, wxALIGN_CENTRE|wxALL, bsize);

   GmatStaticBoxSizer *atmosStaticSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Drag"));
   atmosStaticSizer->Add( atmosSizer, 0, wxALIGN_LEFT|wxALL, bsize);

   // Magnetic Field
   wxStaticText *type3StaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Model"),
                        wxDefaultPosition, wxDefaultSize, 0 );

   theMagfModelComboBox =
      new wxComboBox( this, ID_CB_MAG, magfModelArray[0],
                      wxDefaultPosition, wxDefaultSize,
                      magfModelArray, wxCB_DROPDOWN|wxCB_READONLY );
   theMagfModelComboBox->SetToolTip(pConfig->Read(wxT("ForceModelMagneticFieldModelHint")));

   wxStaticText *degree2StaticText =
      new wxStaticText( this, ID_TEXT, wxT("Degree"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   magneticDegreeTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_MAGF, wxT(""), wxDefaultPosition,
                      wxSize(30,-1), 0 );
   magneticDegreeTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelMagneticDegreeHint")));
   wxStaticText *order2StaticText =
      new wxStaticText( this, ID_TEXT, wxT("Order"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   magneticOrderTextCtrl =
      new wxTextCtrl( this, ID_TEXTCTRL_MAGF, wxT(""), wxDefaultPosition,
                      wxSize(30,-1), 0 );
   magneticOrderTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelMagneticOrderHint")));
   theMagModelSearchButton =
      new wxButton( this, ID_BUTTON_MAG_SEARCH, wxT("Search"),
                    wxDefaultPosition, wxDefaultSize, 0 );
   theMagModelSearchButton->SetToolTip(pConfig->Read(wxT("ForceModelMagneticSearchHint")));

   wxBoxSizer *magfSizer = new wxBoxSizer( wxHORIZONTAL );
   magfSizer->Add( type3StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( theMagfModelComboBox, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( degree2StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( magneticDegreeTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( order2StaticText, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( magneticOrderTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);
   magfSizer->Add( theMagModelSearchButton, 0, wxALIGN_CENTRE|wxALL, bsize);

   magfStaticSizer =
      new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Magnetic Field"));
   magfStaticSizer->Add( magfSizer, 0, wxALIGN_LEFT|wxALL, bsize);

   //-----------------------------------------------------------------
   // Point Masses
   //-----------------------------------------------------------------
   wxStaticText *pointMassStaticText =
      new wxStaticText( this, ID_TEXT, wxT(GUI_ACCEL_KEY) wxT("Point Masses"),
                        wxDefaultPosition, wxDefaultSize, 0 );
   pmEditTextCtrl =
      new wxTextCtrl( this, -1, wxT(""), wxDefaultPosition,
                      wxSize(235,-1), wxTE_READONLY );
   pmEditTextCtrl->SetToolTip(pConfig->Read(wxT("ForceModelPointMassesHint")));
   wxButton *editPmfButton =
      new wxButton( this, ID_BUTTON_PM_EDIT, wxT("Select"),
                    wxDefaultPosition, wxDefaultSize, 0 );
   editPmfButton->SetToolTip(pConfig->Read(wxT("ForceModelSelectPointMassesHint")));

   wxFlexGridSizer *pointMassSizer = new wxFlexGridSizer( 3, 0, 2 );
   pointMassSizer->Add( pointMassStaticText, 0, wxALIGN_LEFT|wxALL, bsize);
   pointMassSizer->Add( pmEditTextCtrl, 0, wxALIGN_CENTRE|wxALL, bsize);
   pointMassSizer->Add( editPmfButton, 0, wxALIGN_CENTRE|wxALL, bsize);

   //-----------------------------------------------------------------
   // SRP
   //-----------------------------------------------------------------
   theSrpCheckBox =
      new wxCheckBox( this, ID_SRP_CHECKBOX, wxT(GUI_ACCEL_KEY) wxT("Use Solar Radiation Pressure"),
                      wxDefaultPosition, wxDefaultSize, 0 );
   theSrpCheckBox->SetToolTip(pConfig->Read(wxT("ForceModelUseSolarRadiationPressureHint")));

   //-----------------------------------------------------------------
   // Primary Bodies
   //-----------------------------------------------------------------
   primaryStaticSizer->Add( bodySizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   primaryStaticSizer->Add( gravStaticSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   primaryStaticSizer->Add( atmosStaticSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   primaryStaticSizer->Add( magfStaticSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   primaryStaticSizer->Hide( magfStaticSizer, true ); // hide, not using magnetic right now, TGG, 04/09/10

   //-----------------------------------------------------------------
   // Force Model
   //-----------------------------------------------------------------
   fmStaticSizer =
         new GmatStaticBoxSizer(wxVERTICAL, this, wxT("Force Model"));
   fmStaticSizer->Add( errorFlexGridSizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   fmStaticSizer->Add( centralBodySizer, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);
   fmStaticSizer->Add( primaryStaticSizer, 0, wxALIGN_CENTRE|wxALL, bsize);
   fmStaticSizer->Add( pointMassSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, bsize);
   fmStaticSizer->Add( theSrpCheckBox, 0, wxGROW|wxALIGN_CENTER_VERTICAL|wxALL, bsize);

   //-----------------------------------------------------------------
   // Add panelSizer
   //-----------------------------------------------------------------
   wxBoxSizer *intBoxSizer = new wxBoxSizer( wxVERTICAL );
   intBoxSizer->Add( intStaticSizer, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, bsize);
   intBoxSizer->Add( 0, 0, 1);

   leftBoxSizer = new wxBoxSizer( wxVERTICAL );
   leftBoxSizer->Add( intBoxSizer, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, bsize);

   wxBoxSizer *pageSizer = new wxBoxSizer( wxHORIZONTAL );
   pageSizer->Add( leftBoxSizer, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, bsize);
   pageSizer->Add( fmStaticSizer, 0, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, bsize);

   wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );
   panelSizer->Add( pageSizer, 0, wxALIGN_CENTRE|wxALL, bsize);

   theMiddleSizer->Add(panelSizer, 0, wxGROW, bsize);

   if ((thePropagator == NULL) || (thePropagator->IsOfType(wxT("Integrator"))))
   {
      ShowIntegratorLayout(true);
   }
   else
   {
      ShowIntegratorLayout(false);
   }

   //-----------------------------------------------------------------
   // disable components for future implementation
   //-----------------------------------------------------------------
   theMagfModelComboBox->Enable(false);
   magneticDegreeTextCtrl->Enable(false);
   magneticOrderTextCtrl->Enable(false);
   theMagModelSearchButton->Enable(false);
   type3StaticText->Enable(false);
   degree2StaticText->Enable(false);
   order2StaticText->Enable(false);

   //-----------------------------------------------------------------
   // initially disable components
   //-----------------------------------------------------------------
   theDragSetupButton->Enable(false);

   #ifdef DEBUG_PROP_PANEL_SETUP
   MessageInterface::ShowMessage(wxT("PropagationConfigPanel::Setup() exiting\n"));
   #endif
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::LoadData()
{
   // Enable the "Show Script" button
   mObject = thePropSetup;

   #ifdef DEBUG_PROP_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("PropConfigPanel::LoadData() entered\n"));
   #endif

   thePropagator = thePropSetup->GetPropagator();
   if (thePropagator->UsesODEModel())
   {
      theForceModel = thePropSetup->GetODEModel();
      theForceModelName = theForceModel->GetName();
      numOfForces   = thePropSetup->GetNumForces();
   }
   else
   {
      theForceModel = NULL;
      // Load the name in case the user changes to an integrator
      theForceModelName = thePropSetup->GetName() + wxT("_ForceModel");
      numOfForces   = 0;

      // Set the data buffers used in the dialog
      Real ss     = thePropagator->GetRealParameter(wxT("StepSize"));
      spkStep     = ToString(ss);
      spkBody     = thePropagator->GetStringParameter(wxT("CentralBody")).c_str();
      spkEpFormat = thePropagator->GetStringParameter(wxT("EpochFormat")).c_str();
      spkEpoch    = thePropagator->GetStringParameter(wxT("StartEpoch")).c_str();

      // Set the flags so that dialog populates correctly
      isSpkStepChanged     = true;
      isSpkBodyChanged     = true;
      isSpkEpFormatChanged = true;
      isSpkEpochChanged    = true;
   }

   PopulateForces();

   #ifdef DEBUG_PROP_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("   Getting Primary bodies array.\n"));
   #endif

//   if (!primaryBodiesArray.IsEmpty())
   if (primaryBody != wxT("None"))
   {
//      primaryBodyString = primaryBodiesArray.Item(0).c_str();
      primaryBodyString = primaryBody.c_str();
      currentBodyName = primaryBodyString;
      currentBodyName = primaryBodyString;
      currentBodyId = FindPrimaryBody(currentBodyName);
   }

   #ifdef DEBUG_PROP_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("   primaryBodyString=%s\n"), primaryBodyString.c_str());
   #endif

//   numOfBodies = (Integer)primaryBodiesArray.GetCount();

   wxString propType = thePropagator->GetTypeName();

   #ifdef DEBUG_PROP_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("   propType=%s\n"), propType.c_str());
   #endif

   Integer typeId = 0;
   for (size_t i = 0; i < integratorArray.GetCount(); ++i)
      if (integratorArray[i] == propType.c_str())
         typeId = i;

   // Display primary bodies
   UpdatePrimaryBodyComboBoxList();
//   if ( !primaryBodiesArray.IsEmpty() )
//      thePrimaryBodyComboBox->SetValue(primaryBodyList[0]->bodyName.c_str());
   thePrimaryBodyComboBox->SetValue(primaryBody.c_str());
//      for (Integer i = 0; i < (Integer)primaryBodiesArray.GetCount(); i++)
//         thePrimaryBodyComboBox->Append(primaryBodiesArray[i]);

   theIntegratorComboBox->SetSelection(typeId);
   integratorString = integratorArray[typeId];

   // Display the force model origin (central body)
   theOriginComboBox->SetValue(propOriginName);

   DisplayIntegratorData(false);
   DisplayForceData();

//   Integer count = (Integer)primaryBodyList.size();
   Integer count = (primaryBodyData == NULL ? 0 : 1);

   if (count == 0)
      EnablePrimaryBodyItems(false, false);
   else
      EnablePrimaryBodyItems(true);

   if ((thePropagator == NULL) || (thePropagator->IsOfType(wxT("Integrator"))))
   {
      ShowIntegratorLayout(true);
   }
   else
   {
      ShowIntegratorLayout(false);
   }


   #ifdef DEBUG_PROP_PANEL_LOAD
   ShowPropData(wxT("LoadData() PropData on exit"));
   if (thePropagator->IsOfType(wxT("Integrator")))
      ShowForceList(wxT("LoadData() ForceList on exit"));
   #endif

}

//------------------------------------------------------------------------------
// void PropagationConfigPanel::PopulateForces()
//------------------------------------------------------------------------------
void PropagationConfigPanel::PopulateForces()
{
   try
   {
      if (theForceModel != NULL)
      {
         mFmPrefaceComment = theForceModel->GetCommentLine();
         propOriginName = theForceModel->GetStringParameter(wxT("CentralBody")).c_str();
         errorControlTypeName = theForceModel->GetStringParameter(wxT("ErrorControl")).c_str();

         PhysicalModel *force;
         Integer paramId;
         wxString wxBodyName;
         wxString wxForceType;
         wxString tempStr;
         wxString useSRP;

         paramId = theForceModel->GetParameterID(wxT("SRP"));
         useSRP = theForceModel->GetOnOffParameter(paramId).c_str();
         usePropOriginForSrp = (useSRP == wxT("On")) ? true : false;

         for (Integer i = 0; i < numOfForces; i++)
         {
            force = theForceModel->GetForce(i);
            if (force == NULL)
            {
               MessageInterface::ShowMessage
                  (wxT("**** INTERNAL ERROR *** PropagationConfigPanel::LoadData() ")
                   wxT("Cannot continue, force pointer is NULL for index %d\n"), i);
               return;
            }

            wxForceType = force->GetTypeName().c_str();
            wxBodyName = force->GetStringParameter(wxT("BodyName")).c_str();

            #ifdef DEBUG_PROP_PANEL_LOAD
            MessageInterface::ShowMessage
               (wxT("   Getting %s for body %s\n"), wxForceType.c_str(), wxBodyName.c_str());
            #endif

            if (wxForceType == wxT("PointMassForce"))
            {
               thePMF = (PointMassForce *)force;
               secondaryBodiesArray.Add(wxBodyName);
               pointMassBodyList.push_back(new ForceType(wxBodyName, wxT("None"),
                                                         dragModelArray[NONE_DM],
                                                         magfModelArray[NONE_MM],
                                                         thePMF));

               //Warn user about bodies already added as Primary body
//               Integer fmSize = (Integer)primaryBodyList.size();
               Integer fmSize = (primaryBodyData == NULL ? 0 : 1);
               Integer last = (Integer)pointMassBodyList.size() - 1;

               for (Integer i = 0; i < fmSize; i++)
               {
                  if (pointMassBodyList[last]->bodyName.c_str()
                       == primaryBodyData->bodyName.c_str())
                  {
                     MessageInterface::PopupMessage
                        (Gmat::WARNING_,
                         wxT("Cannot set %s both as Primary body and Point Mass"),
                         pointMassBodyList[last]->bodyName.c_str());
                  }
               }
            }
            else if (wxForceType == wxT("GravityField"))
            {
               theGravForce = (GravityField*)force;
               wxString potFilename =
                     theGravForce->GetStringParameter(wxT("PotentialFile")).c_str();

//               currentBodyId = FindPrimaryBody(wxBodyName);
//               primaryBodyList[currentBodyId]->bodyName = wxBodyName;
//               primaryBodyList[currentBodyId]->potFilename = potFilename;
               if (primaryBodyData == NULL)
               {
                  #ifdef DEBUG_PROP_PANEL_GRAV
                     MessageInterface::ShowMessage(wxT("Creating primaryBodyData for ")
                           wxT("%s\n"), wxBodyName.c_str());
                  #endif
                  primaryBodyData = new ForceType(wxBodyName);
               }

               primaryBody = wxBodyName;

               primaryBodyData->bodyName = wxBodyName;
               primaryBodyData->potFilename = potFilename;

               #ifdef DEBUG_PROP_PANEL_GRAV
               MessageInterface::ShowMessage
                  (wxT("   Getting gravity model type for %s, potFilename=%s\n"),
                   wxBodyName.c_str(), potFilename.c_str());
               #endif

               // Make potential upper case for comparison
               potFilename.MakeUpper();

               if (wxBodyName == wxT("Earth"))
               {
                  EarthGravModelType eGravModelType;

                  if (potFilename.find(wxT("JGM2")) != wxString::npos)
                     eGravModelType = JGM2;
                  else if (potFilename.find(wxT("JGM3")) != wxString::npos)
                     eGravModelType = JGM3;
                  else if (potFilename.find(wxT("EGM")) != wxString::npos)
                     eGravModelType = EGM96;
                  else
                     eGravModelType = E_OTHER;

//                  primaryBodyList[currentBodyId]->gravType =
//                        earthGravModelArray[eGravModelType];
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theFileMap[earthGravModelArray[eGravModelType]];
                  primaryBodyData->gravType =
                        earthGravModelArray[eGravModelType];
                  primaryBodyData->potFilename =
                     theFileMap[earthGravModelArray[eGravModelType]];
               }
               else if (wxBodyName == wxT("Luna"))
               {
                  LunaGravModelType lGravModelType;

                  if (potFilename.find(wxT("LP165P")) != wxString::npos)
                     lGravModelType = LP165;
                  else
                     lGravModelType = L_OTHER;

//                  primaryBodyList[currentBodyId]->gravType =
//                        lunaGravModelArray[lGravModelType];
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theFileMap[lunaGravModelArray[lGravModelType]];
                  primaryBodyData->gravType = lunaGravModelArray[lGravModelType];
                  primaryBodyData->potFilename =
                     theFileMap[lunaGravModelArray[lGravModelType]];
               }
               else if (wxBodyName == wxT("Venus"))
               {
                  VenusGravModelType vGravModelType;

                  if (potFilename.find(wxT("MGN")) != wxString::npos)
                     vGravModelType = MGNP180U;
                  else
                     vGravModelType = V_OTHER;

//                  primaryBodyList[currentBodyId]->gravType =
//                        venusGravModelArray[vGravModelType];
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theFileMap[venusGravModelArray[vGravModelType]];
                  primaryBodyData->gravType = venusGravModelArray[vGravModelType];
                  primaryBodyData->potFilename =
                     theFileMap[venusGravModelArray[vGravModelType]];
               }
               else if (wxBodyName == wxT("Mars"))
               {
                  MarsGravModelType mGravModelType;

                  if (potFilename.find(wxT("MARS50C")) != wxString::npos)
                     mGravModelType = MARS50C;
                  else
                     mGravModelType = M_OTHER;

//                  primaryBodyList[currentBodyId]->gravType =
//                        marsGravModelArray[mGravModelType];
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theFileMap[marsGravModelArray[mGravModelType]];
                  primaryBodyData->gravType =
                        marsGravModelArray[mGravModelType];
                  primaryBodyData->potFilename =
                        theFileMap[marsGravModelArray[mGravModelType]];
               }
               else //other bodies
               {
                  OthersGravModelType oGravModelType;

                  oGravModelType = O_OTHER;

//                  primaryBodyList[currentBodyId]->gravType =
//                        othersGravModelArray[oGravModelType];
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theFileMap[othersGravModelArray[oGravModelType]];
                  primaryBodyData->gravType =
                        othersGravModelArray[oGravModelType];
                  primaryBodyData->potFilename =
                     theFileMap[othersGravModelArray[oGravModelType]];
               }

               #ifdef DEBUG_PROP_PANEL_GRAV
               MessageInterface::ShowMessage(wxT("   Getting the gravity force\n"));
               #endif

//               primaryBodyList[currentBodyId]->gravf = theGravForce;
               primaryBodyData->gravf = theGravForce;

               // Set actual full potential file path (loj: 2007.10.26)
//               wxString gravTypeName = primaryBodyList[currentBodyId]->gravType;
               wxString gravTypeName = primaryBodyData->gravType;
               wxString fileType = theFileMap[gravTypeName].c_str();
               if (gravTypeName != wxT("None") && gravTypeName != wxT("Other"))
               {
//                  primaryBodyList[currentBodyId]->potFilename =
//                     theGuiInterpreter->GetFileName(fileType).c_str();
                  primaryBodyData->potFilename =
                     theGuiInterpreter->GetFileName(fileType).c_str();
               }

               // Warn user about bodies already added as Primary body
               Integer pmSize = (Integer)pointMassBodyList.size();
//               Integer last = (Integer)primaryBodyList.size() - 1;

               for (Integer i = 0; i < pmSize; i++)
               {
//                  if (strcmp(primaryBodyList[last]->bodyName.c_str(),
//                             pointMassBodyList[i]->bodyName.c_str()) == 0)
                  if (primaryBodyData->bodyName.c_str()
                         ==  pointMassBodyList[i]->bodyName.c_str())
                  {
                     MessageInterface::PopupMessage
                        (Gmat::WARNING_, wxT("Cannot set %s both as Primary body ")
                         wxT("and Point Mass"),
//                         pointMassBodyList[last]->bodyName.c_str());
                         primaryBodyData->bodyName.c_str());
                  }
               }

//               if (primaryBodyList[currentBodyId]->potFilename == "")
               if (primaryBodyData->potFilename == wxT(""))
               {
                  MessageInterface::PopupMessage
                     (Gmat::WARNING_, wxT("Cannot find Potential File for %s.\n"),
                      wxBodyName.c_str());
               }

               tempStr = wxT("");
               tempStr << theGravForce->GetIntegerParameter(wxT("Degree"));
//               primaryBodyList[currentBodyId]->gravDegree = tempStr;
               primaryBodyData->gravDegree = tempStr;

               tempStr = wxT("");
               tempStr << theGravForce->GetIntegerParameter(wxT("Order"));
//               primaryBodyList[currentBodyId]->gravOrder = tempStr;
               primaryBodyData->gravOrder = tempStr;

//               bool found = false;
//               for (Integer i = 0; i < (Integer)primaryBodiesArray.GetCount(); i++)
//               {
//                  if ( primaryBodiesArray[i].CmpNoCase(wxBodyName) == 0 )
//                     found = true;
//               }
//
//               if (!found)
//                  primaryBodiesArray.Add(wxBodyName);
            }
            else if (wxForceType == wxT("DragForce"))
            {
               theDragForce = (DragForce*)force;
               paramId = theDragForce->GetParameterID(wxT("AtmosphereModel"));
               atmosModelString =
                     theDragForce->GetStringParameter(paramId).c_str();

               currentBodyId = FindPrimaryBody(wxBodyName);
//               primaryBodyList[currentBodyId]->bodyName = wxBodyName;
//               primaryBodyList[currentBodyId]->dragType = atmosModelString;
//               primaryBodyList[currentBodyId]->dragf = theDragForce;
               primaryBodyData->bodyName = wxBodyName;
               primaryBodyData->dragType = atmosModelString;
               primaryBodyData->dragf = theDragForce;

               //Warn user about bodies already added as Primary body
               Integer pmSize = (Integer)pointMassBodyList.size();
//               Integer last = (Integer)primaryBodyList.size() - 1;

               for (Integer i = 0; i < pmSize; i++)
               {
//                  if (strcmp(primaryBodyList[last]->bodyName.c_str(),
//                             pointMassBodyList[i]->bodyName.c_str()) == 0)
                  if (primaryBodyData->bodyName.c_str()
                          == pointMassBodyList[i]->bodyName.c_str())
                  {
                     MessageInterface::PopupMessage
                        (Gmat::WARNING_, wxT("Cannot set %s both as Primary body ")
                              wxT("and Point Mass"),
//                              pointMassBodyList[last]->bodyName.c_str());
                              primaryBodyData->bodyName.c_str());
                  }
               }

//               bool found = false;
//               for (Integer i = 0; i < (Integer)primaryBodiesArray.GetCount(); i++)
//               {
//                  if ( primaryBodiesArray[i].CmpNoCase(wxBodyName) == 0 )
//                     found = true;
//               }
//
//               if (!found)
//                  primaryBodiesArray.Add(wxBodyName.c_str());
            }
            else if (wxForceType == wxT("SolarRadiationPressure"))
            {
               // Currently SRP can only be applied to force model central body,
               // so we don't need to set to primary body list (loj:2007.10.19)
               //currentBodyId = FindPrimaryBody(wxBodyName);
               //primaryBodyList[currentBodyId]->useSrp = true;
               //primaryBodyList[currentBodyId]->bodyName = wxBodyName;
               //primaryBodyList[currentBodyId]->srpf = theSRP;
            }
         }
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
void PropagationConfigPanel::SaveData()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("SaveData() thePropagatorName=%s\n"), thePropagator->GetTypeName().c_str());
   MessageInterface::ShowMessage(wxT("   isIntegratorChanged=%d\n"), isIntegratorChanged);
   MessageInterface::ShowMessage(wxT("   isIntegratorDataChanged=%d\n"),isIntegratorDataChanged);
   MessageInterface::ShowMessage(wxT("   isForceModelChanged=%d\n"), isForceModelChanged);
   MessageInterface::ShowMessage(wxT("   isDegOrderChanged=%d\n"), isDegOrderChanged);
   MessageInterface::ShowMessage(wxT("   isPotFileChanged=%d\n"), isPotFileChanged);
   MessageInterface::ShowMessage(wxT("   isAtmosChanged=%d\n"), isAtmosChanged);
   MessageInterface::ShowMessage(wxT("   isOriginChanged=%d\n"), isOriginChanged);
   MessageInterface::ShowMessage(wxT("   isErrControlChanged=%d\n"), isErrControlChanged);
   #endif

   canClose = true;

   if (thePropagator->IsOfType(wxT("Integrator")))
   {
      //-----------------------------------------------------------------
      // check for valid gravity model before saving
      //-----------------------------------------------------------------
//      for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)

      if (primaryBodyData != NULL)
      {
         if (primaryBodyData->bodyName != wxT("None"))
         {
   //         if (primaryBodyList[i]->gravType == "None")
            if ((primaryBodyData->gravType == wxT("None")) &&
                (primaryBodyData->bodyName != wxT("None")))
            {
               MessageInterface::PopupMessage
                  (Gmat::WARNING_, wxT("Please select Gravity Field Model for %s\n"),
   //                     primaryBodyList[i]->bodyName.c_str());
                        primaryBodyData->bodyName.c_str());
               canClose = false;
               return;
            }
            //loj: 2007.10.26
            // Do we need to check empty potential file?
            // Fow now allow to use default coefficients from the body.
            //else if (primaryBodyList[i]->gravType == "Other" &&
            //         primaryBodyList[i]->potFilename == "")
            //{
            //   MessageInterface::PopupMessage
            //      (Gmat::WARNING_, "Please select potential file for %s\n",
            //       primaryBodyList[i]->bodyName.c_str());
            //   canClose = false;
            //   return;
            //}
         }
         else
         {
            delete primaryBodyData;
            primaryBodyData = NULL;
         }
      }

      //-----------------------------------------------------------------
      // check for empty primary bodies or point mass
      //-----------------------------------------------------------------
//      if (primaryBodyList.size() == 0 && pointMassBodyList.size() == 0)
      if (primaryBodyData == NULL && pointMassBodyList.size() == 0)
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("Please select primary bodies or point mass ")
                  wxT("bodies\n"));
         canClose = false;
         return;
      }

      //-----------------------------------------------------------------
      // save values to base, base code should do the range checking
      //-----------------------------------------------------------------

      //-------------------------------------------------------
      // Saving the Integrator
      //-------------------------------------------------------
      if (isIntegratorChanged)
      {
         #ifdef DEBUG_PROP_PANEL_SAVE
         ShowPropData(wxT("SaveData() BEFORE saving Integrator"));
         #endif

         isIntegratorChanged = false;

         if (isIntegratorDataChanged)
            if (SaveIntegratorData())
               isIntegratorDataChanged = false;

         thePropSetup->SetPropagator(thePropagator, true);
         // Since the propagator is cloned in the base code, get new pointer
         thePropagator = thePropSetup->GetPropagator();
         if (!thePropagator->IsOfType(wxT("Integrator")))
         {
            thePropSetup->SetODEModel(NULL);
            theForceModel = thePropSetup->GetODEModel();
         }
      }
      else if (isIntegratorDataChanged)
      {
         if (SaveIntegratorData())
            isIntegratorDataChanged = false;
      }
      //-------------------------------------------------------
      // Saving the force model
      //-------------------------------------------------------
      if ((theForceModel != NULL) && isForceModelChanged)
      {
         #ifdef DEBUG_PROP_PANEL_SAVE
         ShowForceList(wxT("SaveData() BEFORE saving ForceModel"));
         #endif

         // save force model name for later use
         wxString fmName = theForceModel->GetName();

         isForceModelChanged = false;
         ODEModel *newFm = new ODEModel(fmName, wxT("ForceModel"));
         newFm->SetCommentLine(mFmPrefaceComment);
         wxString bodyName;

         //----------------------------------------------------
         // save point mass force model
         //----------------------------------------------------
         for (Integer i=0; i < (Integer)pointMassBodyList.size(); i++)
         {
            thePMF = new PointMassForce();
            bodyName = pointMassBodyList[i]->bodyName.c_str();
            thePMF->SetBodyName(bodyName);
            pointMassBodyList[i]->pmf = thePMF;
            newFm->AddForce(thePMF);
         }

         //----------------------------------------------------
         // save gravity force model
         //----------------------------------------------------
         try
         {
//            for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
            if (primaryBodyData != NULL)
            {
//               GravityField *gf = primaryBodyList[i]->gravf;
               GravityField *gf = primaryBodyData->gravf;

               Integer deg = -999;
               Integer ord = -999;

               // save deg and order for later use
               if (gf != NULL)
               {
                  deg = gf->GetIntegerParameter(wxT("Degree"));
                  ord = gf->GetIntegerParameter(wxT("Order"));
               }

               // Create new GravityField since ForceModel destructor will
               // delete all PhysicalModel
//               bodyName = primaryBodyList[i]->bodyName.c_str();
               bodyName = primaryBodyData->bodyName.c_str();
               #ifdef DEBUG_PROP_PANEL_SAVE
                  MessageInterface::ShowMessage(wxT("SaveData() Creating ")
                        wxT("GravityField for %s\n"), bodyName.c_str());
               #endif
               theGravForce = (GravityField*)theGuiInterpreter->CreateObject(wxT("GravityField"),wxT(""));

//               theGravForce = new GravityField("", bodyName);
               theGravForce->SetSolarSystem(theSolarSystem);
               theGravForce->SetStringParameter(wxT("BodyName"), bodyName);
               theGravForce->SetStringParameter(wxT("PotentialFile"),
//                     primaryBodyList[i]->potFilename.c_str());
                     primaryBodyData->potFilename.c_str());

               if (deg != -999)
               {
                  theGravForce->SetIntegerParameter(wxT("Degree"), deg);
                  theGravForce->SetIntegerParameter(wxT("Order"), ord);
               }

//               primaryBodyList[i]->gravf = theGravForce;
               primaryBodyData->gravf = theGravForce;
               newFm->AddForce(theGravForce);
            }
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         }

         if (isDegOrderChanged)
            SaveDegOrder();

         if (isPotFileChanged)
            SavePotFile();

         //----------------------------------------------------
         // save drag force model
         //----------------------------------------------------
         Integer paramId;

         try
         {
//            for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
            if (primaryBodyData != NULL)
            {
//               if (primaryBodyList[i]->dragType == dragModelArray[NONE_DM])
//               {
//                  primaryBodyList[i]->dragf = NULL;
               if (primaryBodyData->dragType == dragModelArray[NONE_DM])
               {
                  primaryBodyData->dragf = NULL;
               }
               else
               {
                  #ifdef DEBUG_PROP_PANEL_SAVE
                  ShowForceList(wxT("SaveData() BEFORE  saving DragForce"));
                  #endif

                  Real fluxF107 = -999.999;
                  Real fluxF107A = -999.999;
                  Real kp = -999.999;

//                  DragForce *df = primaryBodyList[i]->dragf;
                  DragForce *df = primaryBodyData->dragf;
                  // save drag flux info for later use
                  if (df != NULL)
                  {
                     fluxF107 = df->GetRealParameter(wxT("F107"));
                     fluxF107A = df->GetRealParameter(wxT("F107A"));
                     kp = df->GetRealParameter(wxT("MagneticIndex"));
                  }

                  // create new DragForce
//                  bodyName = primaryBodyList[i]->bodyName.c_str();
                  bodyName = primaryBodyData->bodyName.c_str();
//                  theDragForce = new DragForce(primaryBodyList[i]->dragType.c_str());
                  theDragForce = new DragForce(
                        primaryBodyData->dragType.c_str());
                  theCelestialBody = theSolarSystem->GetBody(bodyName);
                  theAtmosphereModel = theCelestialBody->GetAtmosphereModel();

                  #ifdef DEBUG_PROP_PANEL_SAVE
                  ShowForceList(wxT("Entering if (theAtmosphereModel == NULL)"));
                  #endif

                  if (theAtmosphereModel == NULL)
                  {
                     theAtmosphereModel = (AtmosphereModel*)theGuiInterpreter->CreateObject
//                           (primaryBodyList[i]->dragType.c_str(), primaryBodyList[i]->dragType.c_str());
                           (primaryBodyData->dragType.c_str(),
                            primaryBodyData->dragType.c_str());

                     #ifdef DEBUG_PROP_PANEL_SAVE
                     ShowForceList(wxT("Exiting if (theAtmosphereModel == NULL)"));
                     #endif
                  }

                  theDragForce->SetInternalAtmosphereModel(theAtmosphereModel);

                  #ifdef DEBUG_PROP_PANEL_SAVE
                  ShowForceList(wxT("theDragForce->SetInternalAtmosphereModel(theAtmosphereModel);"));
                  #endif

                  paramId = theDragForce->GetParameterID(wxT("AtmosphereModel"));
//                  bodyName = primaryBodyList[i]->bodyName.c_str();
                  bodyName = primaryBodyData->bodyName.c_str();
//                  theDragForce->SetStringParameter(paramId, primaryBodyList[i]->dragType.c_str());
                  theDragForce->SetStringParameter(paramId,
                        primaryBodyData->dragType.c_str());
                  theDragForce->SetStringParameter(wxT("BodyName"), bodyName);

                  // if drag force was previous defined, set previous flux value
                  if (fluxF107 != -999.999)
                  {
                     theDragForce->SetRealParameter(wxT("F107"), fluxF107);
                     theDragForce->SetRealParameter(wxT("F107A"), fluxF107A);
                     theDragForce->SetRealParameter(wxT("MagneticIndex"), kp);
                  }

//                  primaryBodyList[i]->dragf = theDragForce;
                  primaryBodyData->dragf = theDragForce;
                  newFm->AddForce(theDragForce);

                  #ifdef DEBUG_PROP_PANEL_SAVE
                  ShowForceList(wxT("SaveData() AFTER  saving DragForce"));
                  #endif
               }
            }
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         }

         //----------------------------------------------------
         // save SRP data
         //----------------------------------------------------
         try
         {
            paramId= newFm->GetParameterID(wxT("SRP"));

            if (usePropOriginForSrp)
            {
               theSRP = new SolarRadiationPressure();
               bodyName = propOriginName;
               theSRP->SetStringParameter(wxT("BodyName"), bodyName);
               newFm->AddForce(theSRP);
               newFm->SetOnOffParameter(paramId, wxT("On"));
            }
            else
            {
               newFm->SetOnOffParameter(paramId, wxT("Off"));
            }

            #ifdef DEBUG_PROP_PANEL_SAVE
            ShowForceList(wxT("SaveData() AFTER  saving SRP"));
            #endif

            // Since SRP is only applied to force model central body,
            // we don't need to go through primary body list (loj: 2007.10.19)
            //for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
            //{
            //   if (primaryBodyList[i]->useSrp)
            //   {
            //      theSRP = new SolarRadiationPressure();
            //      primaryBodyList[i]->srpf = theSRP;
            //      bodyName = primaryBodyList[i]->bodyName.c_str();
            //      theSRP->SetStringParameter("BodyName", bodyName);
            //      newFm->AddForce(theSRP);
            //
            //      paramId= newFm->GetParameterID("SRP");
            //      newFm->SetOnOffParameter(paramId, "On");
            //
            //      #ifdef DEBUG_PROP_PANEL_SAVE
            //      ShowForceList("SaveData() AFTER  saving SRP");
            //      #endif
            //   }
            //}
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         }

         //----------------------------------------------------
         // Saving the error control and Origin
         // Always set this to new forcemodel
         //----------------------------------------------------
         try
         {
            newFm->SetStringParameter(wxT("ErrorControl"), errorControlTypeName.c_str());
            newFm->SetStringParameter(wxT("CentralBody"), propOriginName.c_str());
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         }

         //----------------------------------------------------
         // Saving forces to the prop setup
         //----------------------------------------------------
         thePropSetup->SetODEModel(newFm);
         numOfForces = thePropSetup->GetNumForces();

         // ForceModel is deleted in PropSetup::SetForceModel()
         //theForceModel = newFm;

         // Since the force model and it's physical models are cloned in the
         // base code, get new pointers
         theForceModel = thePropSetup->GetODEModel();
         if (theForceModel != NULL)
         {
            theForceModel->SetName(fmName);

            PhysicalModel *pm;
//            int size = primaryBodyList.size();
            int size = (primaryBodyData == NULL ? 0 : 1);
            for (int i=0; i<numOfForces; i++)
            {
               pm = theForceModel->GetForce(i);

               #ifdef DEBUG_PROP_PANEL_SAVE
               MessageInterface::ShowMessage
                  (wxT("   pm=<%p><%s>'%s', Body='%s'\n"), pm, pm->GetTypeName().c_str(),
                   pm->GetName().c_str(), pm->GetBodyName().c_str());
               #endif

               for (int j=0; j<size; j++)
               {
//                  if ((primaryBodyList[j]->bodyName).IsSameAs(pm->GetBodyName().c_str()))
                  if ((primaryBodyData->bodyName).IsSameAs(
                        pm->GetBodyName().c_str()))
                  {
                     if (pm->GetTypeName() == wxT("PointMassForce"))
//                        primaryBodyList[j]->pmf = (PointMassForce*)pm;
                        primaryBodyData->pmf = (PointMassForce*)pm;
                     else if (pm->GetTypeName() == wxT("GravityField"))
//                        primaryBodyList[j]->gravf = (GravityField*)pm;
                        primaryBodyData->gravf = (GravityField*)pm;
                     else if (pm->GetTypeName() == wxT("DragForce"))
//                        primaryBodyList[j]->dragf = (DragForce*)pm;
                        primaryBodyData->dragf = (DragForce*)pm;
                     else if (pm->GetTypeName() == wxT("SolarRadiationPressure"))
//                        primaryBodyList[j]->srpf = (SolarRadiationPressure*)pm;
                        primaryBodyData->srpf = (SolarRadiationPressure*)pm;
                  }
               }
            }
         }
         #ifdef DEBUG_PROP_PANEL_SAVE
         ShowForceList(wxT("SaveData() AFTER  saving ForceModel"));
         #endif
      } // end if(isForceModelChange)
      else
      {
         //----------------------------------------------------
         // Saving Error Control and the Origin (Central Body)
         //----------------------------------------------------
         if (theForceModel != NULL)
         {
            try
            {
               if (isErrControlChanged)
               {
                  theForceModel->SetStringParameter
                     (wxT("ErrorControl"), theErrorComboBox->GetStringSelection().c_str());
                  isErrControlChanged = false;
               }

               if (isOriginChanged)
               {
                  theForceModel->SetStringParameter(wxT("CentralBody"), propOriginName.c_str());
                  isOriginChanged = false;
               }
            }
            catch (BaseException &e)
            {
               MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
            }

            if (isDegOrderChanged)
               SaveDegOrder();

            // Save only GravComboBox or PotFileText is changed
            if (isPotFileChanged)
               SavePotFile();

            if (isAtmosChanged)
               SaveAtmosModel();
         }
      }
   }
   else // It's an analytic propagator -- Just SPK for now
   {
      #ifdef DEBUG_PROP_PANEL_SAVE
         MessageInterface::ShowMessage(wxT("Saving a force-free propagator\n"));
      #endif


      if (isIntegratorChanged)
      {
         #ifdef DEBUG_PROP_PANEL_SAVE
         ShowPropData(wxT("SaveData() BEFORE saving Propagator"));
         #endif

         isIntegratorChanged = false;

         if (isIntegratorDataChanged)
            if (SavePropagatorData())
               isIntegratorDataChanged = false;

         thePropSetup->SetPropagator(thePropagator);
         thePropSetup->SetODEModel(NULL);  // No force model for these puppies

         // Since the propagator is cloned in the base code, get new pointer
         thePropagator = thePropSetup->GetPropagator();
         theForceModel = thePropSetup->GetODEModel();
      }
      else if (isIntegratorDataChanged)
      {
         if (SavePropagatorData())
            isIntegratorDataChanged = false;
      }
   }

   #ifdef DEBUG_PROP_PANEL_SAVE
      MessageInterface::ShowMessage(wxT("SaveData complete; prop = %p, ODE = %p\n"),
            thePropagator, theForceModel);
   #endif
}


//------------------------------------------------------------------------------
// Integer FindPrimaryBody(const wxString &bodyName, bool create = true,
//                         const wxString &gravType, const wxString &dragType,
//                         const wxString &magfType)
//------------------------------------------------------------------------------
/*
 * Finds the primary body from the primaryBodyList and returns index if body
 * was found return index. If body was not found and creaet is false,
 * it returns -1, otherwise create a new ForceType and return new index
 *
 * @param  bodyName  Name of the body to look for
 * @param  create    If true, it creates new ForceType
 * @param  gravType  The type name of GravityForce
 * @param  dragType  The type name of DragForce
 * @param  magfType  The type name of MagneticForce
 *
 * @return body index or -1 if body not found and create is false
 *
 */
//------------------------------------------------------------------------------
Integer PropagationConfigPanel::FindPrimaryBody(const wxString &bodyName,
                                                bool create,
                                                const wxString &gravType,
                                                const wxString &dragType,
                                                const wxString &magfType)
{
//   for (Integer i=0; i<(Integer)primaryBodyList.size(); i++)
   if (primaryBodyData != NULL)
   {
//      if (primaryBodyList[i]->bodyName == bodyName)
      if (primaryBodyData->bodyName == bodyName)
         return 0;
   }

   if (!create)
      return -1;

//   primaryBodyList.push_back(new ForceType(bodyName, gravType, dragType, magfType));
   primaryBodyData = new ForceType(bodyName, gravType, dragType, magfType);

   // Set gravity model file
   if (theFileMap.find(gravType) != theFileMap.end())
   {
      wxString potFileType = theFileMap[gravType].c_str();
      wxString wxPotFileName = theGuiInterpreter->GetFileName(potFileType).c_str();
      //MessageInterface::ShowMessage("===> potFile=%s\n", potFileType.c_str());
//      primaryBodyList.back()->potFilename = wxPotFileName;
      primaryBodyData->potFilename = wxPotFileName;
   }

   #ifdef DEBUG_PROP_PANEL_FIND_BODY
   ShowForceList(wxT("FindPrimaryBody() after add body to primaryBodyList"));
   #endif

//   return (Integer)(primaryBodyList.size() - 1);
   return (primaryBodyData == NULL ? 0 : 1);
}


//------------------------------------------------------------------------------
// Integer FindPointMassBody(const wxString &bodyName)
//------------------------------------------------------------------------------
/*
 * Finds the point mass body from the pointMassBodyList and returns index if body
 * was found return index. If body was not found, it returns -1,
 *
 * @param  bodyName  Name of the body to look for
 *
 * @return body index or -1 if body was not found
 *
 */
//------------------------------------------------------------------------------
Integer PropagationConfigPanel::FindPointMassBody(const wxString &bodyName)
{
   for (Integer i=0; i<(Integer)pointMassBodyList.size(); i++)
   {
      if (pointMassBodyList[i]->bodyName == bodyName)
         return i;
   }

   return -1;
}


//------------------------------------------------------------------------------
// void Initialize()
//------------------------------------------------------------------------------
void PropagationConfigPanel::Initialize()
{
   #ifdef DEBUG_PROP_PANEL_INIT
   MessageInterface::ShowMessage(wxT("PropagationConfigPanel::Initialize() entered\n"));
   #endif

   thePropagator      = NULL;
   thePropSetup       = NULL;
   theForceModel      = NULL;
   thePMF             = NULL;
   theDragForce       = NULL;
   theGravForce       = NULL;
   theSRP             = NULL;
   theSolarSystem     = NULL;
   theCelestialBody   = NULL;
   theAtmosphereModel = NULL;

   // Default integrator values
   thePropagatorName = wxT("");
   integratorString  = wxT("RKV 8(9)");

   // Default force model values
   useDragForce        = false;
   usePropOriginForSrp = false;
   numOfForces         = 0;

   // Changed flags
   isForceModelChanged = false;
   isAtmosChanged = false;
   isDegOrderChanged = false;
   isPotFileChanged = false;
   isMagfTextChanged = false;
   isIntegratorChanged = false;
   isIntegratorDataChanged = false;
   isOriginChanged = false;
   isErrControlChanged = false;

   isSpkStepChanged     = false;
   isSpkBodyChanged     = false;
   isSpkEpFormatChanged = false;
   isSpkEpochChanged    = false;

   //Note: All the settings should match enum types in the header.

   StringArray propTypes = theGuiInterpreter->GetListOfFactoryItems(Gmat::PROPAGATOR);
   IntegratorCount = (Integer)propTypes.size();
   for (UnsignedInt i = 0; i < propTypes.size(); ++i)
   {
      integratorArray.Add(propTypes[i].c_str());
      integratorTypeArray.Add(propTypes[i].c_str());
   }

   // initialize gravity model type arrays
   earthGravModelArray.Add(wxT("None"));
   earthGravModelArray.Add(wxT("JGM-2"));
   earthGravModelArray.Add(wxT("JGM-3"));
   earthGravModelArray.Add(wxT("EGM-96"));
   earthGravModelArray.Add(wxT("Other"));

   lunaGravModelArray.Add(wxT("None"));
   lunaGravModelArray.Add(wxT("LP-165"));
   lunaGravModelArray.Add(wxT("Other"));

   venusGravModelArray.Add(wxT("None"));
   venusGravModelArray.Add(wxT("MGNP-180U"));
   venusGravModelArray.Add(wxT("Other"));

   marsGravModelArray.Add(wxT("None"));
   marsGravModelArray.Add(wxT("Mars-50C"));
   marsGravModelArray.Add(wxT("Other"));

   othersGravModelArray.Add(wxT("None"));
   othersGravModelArray.Add(wxT("Other"));

   // initialize drag model type array
   dragModelArray.Add(wxT("None"));
//   dragModelArray.Add(wxT("Exponential"));
   dragModelArray.Add(wxT("MSISE90"));
   dragModelArray.Add(wxT("JacchiaRoberts"));

   // initialize error control type array
   errorControlArray.Add(wxT("None"));
   errorControlArray.Add(wxT("RSSStep"));
   errorControlArray.Add(wxT("RSSState"));
   errorControlArray.Add(wxT("LargestStep"));
   errorControlArray.Add(wxT("LargestState"));

   // for actual file keyword used in FileManager
   theFileMap[wxT("JGM-2")] = wxT("JGM2_FILE");
   theFileMap[wxT("JGM-3")] = wxT("JGM3_FILE");
   theFileMap[wxT("EGM-96")] = wxT("EGM96_FILE");
   theFileMap[wxT("LP-165")] = wxT("LP165P_FILE");
   theFileMap[wxT("MGNP-180U")] = wxT("MGNP180U_FILE");
   theFileMap[wxT("Mars-50C")] = wxT("MARS50C_FILE");

   #ifdef DEBUG_PROP_PANEL_INIT
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::Initialize() Initialized local arrays.\n"));
   #endif

   // initialize mag. filed model type array
   magfModelArray.Add(wxT("None"));

}


//------------------------------------------------------------------------------
// void DisplayIntegratorData(bool integratorChanged)
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayIntegratorData(bool integratorChanged)
{
    int propIndex = theIntegratorComboBox->GetSelection();

   #ifdef DEBUG_PROP_INTEGRATOR
   MessageInterface::ShowMessage
      (wxT("DisplayIntegratorData() entered, integratorChanged=%d, integratorString=<%s>\n"),
       integratorChanged, integratorString.c_str());
   #endif

   if (integratorChanged)
   {
      wxString integratorType = integratorTypeArray[propIndex].c_str();
      thePropagatorName = integratorType;
      thePropagator = (Propagator*)theGuiInterpreter->GetConfiguredObject(thePropagatorName);
      if (thePropagator == NULL)
      {
         thePropagator = (Propagator*)
            theGuiInterpreter->CreateObject(integratorType, thePropagatorName);
      }

      if (!thePropagator->IsOfType(wxT("Integrator")))
      {
         Real ss     = thePropagator->GetRealParameter(wxT("StepSize"));
         spkStep     = ToString(ss);
         spkBody     = thePropagator->GetStringParameter(wxT("CentralBody")).c_str();
         spkEpFormat = thePropagator->GetStringParameter(wxT("EpochFormat")).c_str();
         spkEpoch    = thePropagator->GetStringParameter(wxT("StartEpoch")).c_str();

         isSpkStepChanged     = true;
         isSpkBodyChanged     = true;
         isSpkEpFormatChanged = true;
         isSpkEpochChanged    = true;
      }

      if (theForceModel == NULL)    // Switched from an ODE-model free propagator
      {
         theForceModelName = thePropSetup->GetStringParameter(wxT("FM"));
         if ((theForceModelName == wxT("UndefinedODEModel")) ||
             (theForceModelName == wxT("")))
            theForceModelName = thePropSetup->GetName() + wxT("_ForceModel");

         theForceModel = (ODEModel*)theGuiInterpreter->GetConfiguredObject(theForceModelName);

         if (theForceModel == NULL)
         {
            theForceModel = (ODEModel*)theGuiInterpreter->CreateNewODEModel(theForceModelName);
         }

         // Load the panel with the force model data
         numOfForces   = theForceModel->GetNumForces();

         PopulateForces();
         // Display the force model origin (central body)
         theOriginComboBox->SetValue(propOriginName);

//         if (!primaryBodiesArray.IsEmpty())
         if (primaryBody != wxT("None"))
         {
//            primaryBodyString = primaryBodiesArray.Item(0).c_str();
            primaryBodyString = primaryBody.c_str();
            currentBodyName = primaryBodyString;
            currentBodyId = FindPrimaryBody(currentBodyName);
         }

//         numOfBodies = (Integer)primaryBodiesArray.GetCount();
//         thePrimaryBodyComboBox->Clear();
//         if (numOfBodies > 0)
//         {
//            for (Integer index = 0; index < numOfBodies; ++index)
//               thePrimaryBodyComboBox->Append(primaryBodiesArray[index].c_str());
//            thePrimaryBodyComboBox->SetSelection(0);
//         }
         UpdatePrimaryBodyComboBoxList();
//         thePrimaryBodyComboBox->SetValue(primaryBodyList[0]->bodyName.c_str());
         if (primaryBodyData != NULL)
            thePrimaryBodyComboBox->SetValue(primaryBodyData->bodyName.c_str());
         else
            thePrimaryBodyComboBox->SetValue(wxT("None"));
//         if (numOfBodies == 0)
         if (primaryBody == wxT("None"))
            EnablePrimaryBodyItems(false, false);
         else
            EnablePrimaryBodyItems(true);

         DisplayForceData();
         isForceModelChanged = true;
      }

      #ifdef DEBUG_PROP_INTEGRATOR
      MessageInterface::ShowMessage
         (wxT("   integratorType='%s', thePropagatorName='%s'\n   thePropagator=<%p>'%s'\n"),
          integratorType.c_str(), thePropagatorName.c_str(), thePropagator,
          thePropagator ? thePropagator->GetName().c_str() : wxT("NULL"));
      #endif
   }

   if (thePropagator->IsOfType(wxT("Integrator")))
   {
      #ifdef __WXMAC__    // TBD: Is this distinction still needed?
      //   if (integratorString.IsSameAs(integratorArray[ABM]))
         if (thePropagator->IsOfType(wxT("PredictorCorrector")))
         {
            minIntErrorStaticText->Show(true);
            nomIntErrorStaticText->Show(true);
            minIntErrorTextCtrl->Show(true);
            nomIntErrorTextCtrl->Show(true);
            minIntErrorStaticText->Enable(true);
            nomIntErrorStaticText->Enable(true);
            minIntErrorTextCtrl->Enable(true);
            nomIntErrorTextCtrl->Enable(true);
         }
         else
         {
            minIntErrorStaticText->Enable(false);
            nomIntErrorStaticText->Enable(false);
            minIntErrorTextCtrl->Enable(false);
            nomIntErrorTextCtrl->Enable(false);
            minIntErrorStaticText->Show(false);
            nomIntErrorStaticText->Show(false);
            minIntErrorTextCtrl->Show(false);
            nomIntErrorTextCtrl->Show(false);
         }
      #else
      //   if (integratorString.IsSameAs(integratorArray[ABM]))
         if (thePropagator->IsOfType(wxT("PredictorCorrector")))
         {
            minIntErrorStaticText->Show(true);
            nomIntErrorStaticText->Show(true);
            minIntErrorTextCtrl->Show(true);
            nomIntErrorTextCtrl->Show(true);
         }
         else
         {
            minIntErrorStaticText->Show(false);
            nomIntErrorStaticText->Show(false);
            minIntErrorTextCtrl->Show(false);
            nomIntErrorTextCtrl->Show(false);
         }
      #endif

      Integer stopId     = thePropagator->GetParameterID(wxT("StopIfAccuracyIsViolated"));
      stopOnAccViolation = thePropagator->GetBooleanParameter(stopId);
      theStopCheckBox->SetValue(stopOnAccViolation);
      theStopCheckBox->Show(true);
      theStopCheckBox->Enable(true);

      leftBoxSizer->Layout();

      Real i1 = thePropagator->GetRealParameter(wxT("InitialStepSize"));
      Real i2 = thePropagator->GetRealParameter(wxT("Accuracy"));
      Real i3 = thePropagator->GetRealParameter(wxT("MinStep"));
      Real i4 = thePropagator->GetRealParameter(wxT("MaxStep"));
      Integer i5 = (long)thePropagator->GetIntegerParameter(wxT("MaxStepAttempts"));

      initialStepSizeTextCtrl->SetValue(ToString(i1));
      accuracyTextCtrl->SetValue(ToString(i2));
      minStepTextCtrl->SetValue(ToString(i3));
      maxStepTextCtrl->SetValue(ToString(i4));
      maxStepAttemptTextCtrl->SetValue(ToString(i5));

//      if (integratorString.IsSameAs(integratorArray[ABM]))
      if (thePropagator->IsOfType(wxT("PredictorCorrector")))
      {
         Real i6 = thePropagator->GetRealParameter(wxT("LowerError"));
         Real i7 = thePropagator->GetRealParameter(wxT("TargetError"));

         minIntErrorTextCtrl->SetValue(ToString(i6));
         nomIntErrorTextCtrl->SetValue(ToString(i7));
      }
   }
   else
   {
      // Fill in Propagator data
      if (isSpkStepChanged)
      {
         propagatorStepSizeTextCtrl->SetValue(spkStep);
      }

      if (isSpkBodyChanged)
      {
         Integer cbIndex = propCentralBodyComboBox->FindString(spkBody);
         propCentralBodyComboBox->SetSelection(cbIndex);
      }
      if (isSpkEpFormatChanged)
      {
         Integer epIndex = propagatorEpochFormatComboBox->FindString(spkEpFormat);
         propagatorEpochFormatComboBox->SetSelection(epIndex);
      }
      if (isSpkEpochChanged)
      {
            startEpochTextCtrl->SetValue(spkEpoch);
      }

//      spkStep = propagatorStepSizeTextCtrl->GetValue();
//      spkBody = propCentralBodyComboBox->GetValue();
//      spkEpFormat = propagatorEpochFormatComboBox->GetValue();
//      spkEpoch = startEpochTextCtrl->GetValue();

      isSpkStepChanged     = false;
      isSpkBodyChanged     = false;
      isSpkEpFormatChanged = false;
      isSpkEpochChanged    = false;

      theStopCheckBox->Show(false);
      theStopCheckBox->Enable(false);
   }

   if ((thePropagator == NULL) || (thePropagator->IsOfType(wxT("Integrator"))))
   {
      ShowIntegratorLayout(true);
   }
   else
   {
      ShowIntegratorLayout(false);
   }

   #ifdef DEBUG_PROP_INTEGRATOR
   ShowPropData(wxT("DisplayIntegratorData() exiting..."));
   #endif
}

//------------------------------------------------------------------------------
// void DisplayForceData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayForceData()
{
   DisplayErrorControlData();
   DisplaySRPData();

   if (!pointMassBodyList.empty())
      DisplayPointMassData();

//   if (primaryBodyList.empty())
   if (primaryBodyData == NULL)
      return;

   DisplayPrimaryBodyData();
   DisplayGravityFieldData(currentBodyName);
   DisplayAtmosphereModelData();
   DisplayMagneticFieldData();
}


//------------------------------------------------------------------------------
// void DisplayPrimaryBodyData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayPrimaryBodyData()
{
//   Integer bodyIndex = 0;
//
//   bodyTextCtrl->Clear();
//
//   for (Integer i = 0; i < (Integer)primaryBodiesArray.GetCount(); i++)
//   {
//      bodyTextCtrl->AppendText(primaryBodiesArray.Item(i) + " ");
//      if ( primaryBodiesArray[i].CmpNoCase(primaryBodyString) == 0 )
//         bodyIndex = i;
//   }
//
//   thePrimaryBodyComboBox->SetSelection(bodyIndex);

   // todo If none, should we check (and delete) primaryBodyData pointer here?
   thePrimaryBodyComboBox->SetValue(primaryBody);
}

//------------------------------------------------------------------------------
// void DisplayGravityFieldData(const wxString& bodyName)
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayGravityFieldData(const wxString& bodyName)
{
   #ifdef DEBUG_PROP_PANEL_GRAV
   MessageInterface::ShowMessage
      (wxT("DisplayGravityFieldData() currentBodyName=%s gravType=%s\n"),
//       currentBodyName.c_str(), primaryBodyList[currentBodyId]->gravType.c_str());
       currentBodyName.c_str(), primaryBodyData->gravType.c_str());
   ShowForceList(wxT("DisplayGravityFieldData() entered"));
   #endif

   theGravModelComboBox->Clear();

//   wxString gravType = primaryBodyList[currentBodyId]->gravType;
   wxString gravType = primaryBodyData->gravType;

   // for gravity model ComboBox
   if (bodyName == wxT("Earth"))
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("DisplayGravityFieldData() Displaying Earth gravity model\n"));
      #endif

      for (Integer i = 0; i < (Integer)EarthGravModelCount; i++)
         theGravModelComboBox->Append(earthGravModelArray[i]);

   }
   else if (bodyName == wxT("Luna"))
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("DisplayGravityFieldData() Displaying Luna gravity model\n"));
      #endif

      for (Integer i = 0; i < (Integer)LunaGravModelCount; i++)
         theGravModelComboBox->Append(lunaGravModelArray[i]);
   }
   else if (bodyName == wxT("Venus"))
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("DisplayGravityFieldData() Displaying Venus gravity model\n"));
      #endif

      for (Integer i = 0; i < (Integer)VenusGravModelCount; i++)
         theGravModelComboBox->Append(venusGravModelArray[i]);

   }
   else if (bodyName == wxT("Mars"))
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("DisplayGravityFieldData() Displaying Mars gravity model\n"));
      #endif

      for (Integer i = 0; i < (Integer)MarsGravModelCount; i++)
         theGravModelComboBox->Append(marsGravModelArray[i]);

   }
   else // other bodies
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("DisplayGravityFieldData() Displaying other gravity model\n"));
      #endif

      for (Integer i = 0; i < (Integer)OthersGravModelCount; i++)
         theGravModelComboBox->Append(othersGravModelArray[i]);

   }

   theGravModelSearchButton->Enable(false);
   ////potFileStaticText->Enable(false);
   potFileTextCtrl->Enable(false);
   gravityDegreeTextCtrl->Enable(true);
   gravityOrderTextCtrl->Enable(true);
//   potFileTextCtrl->SetValue(primaryBodyList[currentBodyId]->potFilename);
   potFileTextCtrl->SetValue(primaryBodyData->potFilename);

   if (gravType == wxT("None"))
   {
      gravityDegreeTextCtrl->Enable(false);
      gravityOrderTextCtrl->Enable(false);
   }
   else
   {
//      if (primaryBodyList[currentBodyId]->gravType == "Other")
      if (primaryBodyData->gravType == wxT("Other"))
      {
         theGravModelSearchButton->Enable(true);
         potFileStaticText->Enable(true);
         potFileTextCtrl->Enable(true);
      }
   }

   theGravModelComboBox->SetValue(gravType);
//   gravityDegreeTextCtrl->SetValue(primaryBodyList[currentBodyId]->gravDegree);
//   gravityOrderTextCtrl->SetValue(primaryBodyList[currentBodyId]->gravOrder);
   gravityDegreeTextCtrl->SetValue(primaryBodyData->gravDegree);
   gravityOrderTextCtrl->SetValue(primaryBodyData->gravOrder);

   #ifdef DEBUG_PROP_PANEL_GRAV
   ShowForceList(wxT("DisplayGravityFieldData() exiting"));
   #endif
}

//------------------------------------------------------------------------------
// void DisplayAtmosphereModelData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayAtmosphereModelData()
{
   #ifdef DEBUG_PROP_PANEL_DISPLAY
   MessageInterface::ShowMessage
      (wxT("DisplayAtmosphereModelData() currentBodyName=%s dragType=%s\n"),
       currentBodyName.c_str(), primaryBodyList[currentBodyId]->dragType.c_str());
   #endif

   // Enable atmosphere model only for Earth
   if (currentBodyName == wxT("Earth"))
   {
      theAtmosModelComboBox->Enable(true);
   }
   else
   {
      theAtmosModelComboBox->Enable(false);
      theDragSetupButton->Enable(false);
   }

   // Set current drag force pointer
//   theDragForce = primaryBodyList[currentBodyId]->dragf;
   theDragForce = primaryBodyData->dragf;

//   if (primaryBodyList[currentBodyId]->dragType == dragModelArray[NONE_DM])
   if (primaryBodyData->dragType == dragModelArray[NONE_DM])
   {
      theAtmosModelComboBox->SetSelection(NONE_DM);
      theDragSetupButton->Enable(false);
   }
////   else if (primaryBodyList[currentBodyId]->dragType == dragModelArray[EXPONENTIAL])
//   else if (primaryBodyData->dragType == dragModelArray[EXPONENTIAL])
//   {
//      theAtmosModelComboBox->SetSelection(EXPONENTIAL);
//      theDragSetupButton->Enable(false);
//   }
//   else if (primaryBodyList[currentBodyId]->dragType == dragModelArray[MSISE90])
   else if (primaryBodyData->dragType == dragModelArray[MSISE90])
   {
      theAtmosModelComboBox->SetSelection(MSISE90);
      theDragSetupButton->Enable(true);
   }
//   else if (primaryBodyList[currentBodyId]->dragType == dragModelArray[JR])
   else if (primaryBodyData->dragType == dragModelArray[JR])
   {
      theAtmosModelComboBox->SetSelection(JR);
      theDragSetupButton->Enable(true);
   }
}

//------------------------------------------------------------------------------
// void DisplayPointMassData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayPointMassData()
{
   pmEditTextCtrl->Clear();
   if (!secondaryBodiesArray.IsEmpty())
   {
      for (Integer i = 0; i < (Integer)secondaryBodiesArray.GetCount(); i++)
         pmEditTextCtrl->AppendText(secondaryBodiesArray.Item(i) + wxT(" "));
   }

   UpdatePrimaryBodyComboBoxList();
}

//------------------------------------------------------------------------------
// void DisplayMagneticFieldData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayMagneticFieldData()
{
//   if (primaryBodyList[currentBodyId]->magfType == magfModelArray[NONE_MM])
   if (primaryBodyData->magfType == magfModelArray[NONE_MM])
   {
      theMagfModelComboBox->SetSelection(NONE_MM);
   }
}


//------------------------------------------------------------------------------
// void DisplaySRPData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplaySRPData()
{
   theSrpCheckBox->SetValue(usePropOriginForSrp);
}


//------------------------------------------------------------------------------
// void DisplayErrorControlData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::DisplayErrorControlData()
{
   #ifdef DEBUG_PROP_PANEL_DISPLAY
   MessageInterface::ShowMessage(wxT("On DisplayErrorControlData()\n"));
   #endif

   wxString wxEcTypeName = errorControlTypeName.c_str();

   if (wxEcTypeName == errorControlArray[NONE_EC])
      theErrorComboBox->SetSelection(NONE_EC);
   else if (wxEcTypeName == errorControlArray[RSSSTEP])
      theErrorComboBox->SetSelection(RSSSTEP);
   else if (wxEcTypeName == errorControlArray[RSSSTATE])
      theErrorComboBox->SetSelection(RSSSTATE);
   else if (wxEcTypeName == errorControlArray[LARGESTSTEP])
      theErrorComboBox->SetSelection(LARGESTSTEP);
   else if (wxEcTypeName == errorControlArray[LARGESTSTATE])
      theErrorComboBox->SetSelection(LARGESTSTATE);
}


//------------------------------------------------------------------------------
// void EnablePrimaryBodyItems(bool enable = true, bool clear = false)
//------------------------------------------------------------------------------
void PropagationConfigPanel::EnablePrimaryBodyItems(bool enable, bool clear)
{
   if (enable)
   {
      theGravModelComboBox->Enable(true);
      gravityDegreeTextCtrl->Enable(true);
      gravityOrderTextCtrl->Enable(true);

      if (theGravModelComboBox->GetStringSelection() == wxT("Other"))
      {
         theGravModelSearchButton->Enable(true);
         potFileTextCtrl->Enable(true);
      }
      else
      {
         theGravModelSearchButton->Enable(false);
         potFileTextCtrl->Enable(false);
      }

      if (thePrimaryBodyComboBox->GetValue() == wxT("Earth"))
      {
         theAtmosModelComboBox->Enable(true);
         if (theAtmosModelComboBox->GetValue() == dragModelArray[NONE_DM] //||
             //theAtmosModelComboBox->GetValue() == dragModelArray[EXPONENTIAL]
             )
            theDragSetupButton->Enable(false);
         else
            theDragSetupButton->Enable(true);
      }
      else
      {
         theAtmosModelComboBox->Enable(false);
         theDragSetupButton->Enable(false);
      }

      //theMagfModelComboBox->Enable(true);
      //theSrpCheckBox->Enable(true);
   }
   else
   {
      if (clear)
      {
//         primaryBodyList.clear();
         if (primaryBodyData != NULL)
         {
            delete primaryBodyData;
            primaryBodyData = NULL;
         }
//         primaryBodiesArray.Clear();
         primaryBody = wxT("None");
//         thePrimaryBodyComboBox->Clear();
//         bodyTextCtrl->Clear();
      }

      theGravModelComboBox->Enable(false);
      gravityDegreeTextCtrl->Enable(false);
      gravityOrderTextCtrl->Enable(false);
      potFileTextCtrl->Enable(false);
      theGravModelSearchButton->Enable(false);
      theAtmosModelComboBox->Enable(false);
      theDragSetupButton->Enable(false);
      //theMagfModelComboBox->Enable(false);
      //theSrpCheckBox->Enable(false);
   }
}


//------------------------------------------------------------------------------
// void UpdatePrimaryBodyItems()
//------------------------------------------------------------------------------
void PropagationConfigPanel::UpdatePrimaryBodyItems()
{
   // Reworked for only one primary allowed; the multiple-body code is
   // commented out below
   mDataChanged = true;

   wxString selBody = thePrimaryBodyComboBox->GetStringSelection();

   if (currentBodyName != selBody)
   {
      primaryBodyString = thePrimaryBodyComboBox->GetStringSelection();
      currentBodyName = selBody;
      currentBodyId = FindPrimaryBody(currentBodyName);
      primaryBody = selBody;

      isForceModelChanged = true;

      if (currentBodyName == wxT("None"))
      {
         EnablePrimaryBodyItems(false, true);
         // Delete primary body data
         if (primaryBodyData != NULL)
         {
            delete primaryBodyData;
            primaryBodyData = NULL;
         }
      }
      else
      {
         EnablePrimaryBodyItems();
         DisplayGravityFieldData(currentBodyName);
         DisplayAtmosphereModelData();
         DisplayMagneticFieldData();
         DisplaySRPData();
         isForceModelChanged = true;
      }
   }

//   if (primaryBodiesArray.IsEmpty())
//      return;
//
//   wxString selBody = thePrimaryBodyComboBox->GetStringSelection();
//
//   if (currentBodyName != selBody)
//   {
//      primaryBodyString = thePrimaryBodyComboBox->GetStringSelection();
//      currentBodyName = selBody;
//      currentBodyId = FindPrimaryBody(currentBodyName);
//
//      DisplayGravityFieldData(currentBodyName);
//      DisplayAtmosphereModelData();
//      DisplayMagneticFieldData();
//      DisplaySRPData();
//   }
}


//------------------------------------------------------------------------------
// void PropagationConfigPanel::UpdatePrimaryBodyComboBoxList()
//------------------------------------------------------------------------------
/**
 * This method updates the list of bodies available for use as the primary body.
 *
 * The thePrimaryBodyComboBox widget for release 1.0 of GMAT is used to select
 * a primary body for the single allowed source of full field gravity (a 1.0
 * restriction that might be removed in a later release).  This method populates
 * the combobox with the available bodies, omitting any that are used as point
 * masses and adding an option to select "None", indicating that there is no
 * ful field gravity in teh force model.
 */
//------------------------------------------------------------------------------
void PropagationConfigPanel::UpdatePrimaryBodyComboBoxList()
{
   wxArrayString cBodies = theGuiManager->GetConfigBodyList();
   thePrimaryBodyComboBox->Clear();
   thePrimaryBodyComboBox->Append(wxT("None"));

   #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage(wxT("Celestial bodies:\n"));
   #endif

   for (UnsignedInt i = 0; i < cBodies.GetCount(); ++i)
   {
      if (secondaryBodiesArray.Index(cBodies[i]) == wxNOT_FOUND)
      {
         #ifdef DEBUG_PROP_PANEL_GRAV
            MessageInterface::ShowMessage(wxT("   %s\n"), cBodies[i].c_str());
         #endif
         thePrimaryBodyComboBox->Append(cBodies[i].c_str());
      }
   }
}


//------------------------------------------------------------------------------
// bool SaveIntegratorData()
//------------------------------------------------------------------------------
bool PropagationConfigPanel::SaveIntegratorData()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::SaveIntegratorData() entered\n"));
   #endif

   Integer maxAttempts;
   Real initStep, accuracy, minStep, maxStep, minError, nomError;
   wxString str;

   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   str = initialStepSizeTextCtrl->GetValue();
   CheckReal(initStep, str, wxT("InitialStepSize"), wxT("Real Number"));

   str = accuracyTextCtrl->GetValue();
   CheckReal(accuracy, str, wxT("Accuracy"), wxT("Real Number > 0"), false, true, true);

   str = minStepTextCtrl->GetValue();
   CheckReal(minStep, str, wxT("Min Step Size"), wxT("Real Number >= 0, MinStep <= MaxStep"),
             false, true, true, true);

   str = maxStepTextCtrl->GetValue();
   CheckReal(maxStep, str, wxT("Max Step Size"), wxT("Real Number > 0, MinStep <= MaxStep"),
             false, true, true);

   str = maxStepAttemptTextCtrl->GetValue();
   CheckInteger(maxAttempts, str, wxT("Max Step Attempts"), wxT("Integer Number > 0"),
                false, true, true);

   if (thePropagator->IsOfType(wxT("PredictorCorrector")))
   {
      str = minIntErrorTextCtrl->GetValue();
      CheckReal(minError, str, wxT("Min Integration Error"), wxT("Real Number > 0"),
                false, true, true);

      str = nomIntErrorTextCtrl->GetValue();
      CheckReal(nomError, str, wxT("Nominal Integration Error"), wxT("Real Number > 0"),
                false, true, true);
   }

   try
   {
      if (minStep >= maxStep)
         throw GmatBaseException(wxT("The step control values are invalid; ")
               wxT("Integrators require 0.0 <= Min Step Size < Max Step Size"));

      if (thePropagator->IsOfType(wxT("PredictorCorrector")))
      {
         // Check the error control settings
         if ((minError > nomError) ||
             (nomError > accuracy) ||
             (minError > accuracy))
            throw GmatBaseException(wxT("The step control values are invalid; ")
                  wxT("Predictor-Corrector Integrators require 0.0 < Min ")
                  wxT("Integration Error < Nominal Integration Error < Accuracy"));
      }

   }
   catch (GmatBaseException &ex)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, ex.GetFullMessage());
      canClose = false;
   }

   if (!canClose)
      return false;

   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      Integer id;

      id = thePropagator->GetParameterID(wxT("InitialStepSize"));
      thePropagator->SetRealParameter(id, initStep);

      id = thePropagator->GetParameterID(wxT("Accuracy"));
      thePropagator->SetRealParameter(id, accuracy);

      id = thePropagator->GetParameterID(wxT("MinStep"));
      thePropagator->SetRealParameter(id, minStep);

      id = thePropagator->GetParameterID(wxT("MaxStep"));
      thePropagator->SetRealParameter(id, maxStep);

      id = thePropagator->GetParameterID(wxT("MaxStepAttempts"));
      thePropagator->SetIntegerParameter(id, maxAttempts);

//      if (integratorString.IsSameAs(integratorArray[ABM]))
      if (thePropagator->IsOfType(wxT("PredictorCorrector")))
      {
         id = thePropagator->GetParameterID(wxT("LowerError"));
         thePropagator->SetRealParameter(id, minError);

         id = thePropagator->GetParameterID(wxT("TargetError"));
         thePropagator->SetRealParameter(id, nomError);
      }

      if (thePropagator->IsOfType(wxT("Integrator")))
      {
         id = thePropagator->GetParameterID(wxT("StopIfAccuracyIsViolated"));
         thePropagator->SetBooleanParameter(id, stopOnAccViolation);
      }

      #ifdef DEBUG_PROP_PANEL_SAVE
      ShowPropData(wxT("SaveData() AFTER  saving Integrator"));
      #endif

      return true;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return false;
   }
}

//------------------------------------------------------------------------------
// bool SavePropagatorData()
//------------------------------------------------------------------------------
bool PropagationConfigPanel::SavePropagatorData()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("PropagationConfigPanel::SavePropagatorData() entered\n"));
   #endif

   Real step;
   wxString str;

   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   str = propagatorStepSizeTextCtrl->GetValue().c_str();
   CheckReal(step, str, wxT("StepSize"), wxT("Real Number"));

   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      Integer id;

      id = thePropagator->GetParameterID(wxT("StepSize"));
      if (step <= 0.0)
         throw GmatBaseException(wxT("Step size must be a real positive number"));
      thePropagator->SetRealParameter(id, step);

      str = propCentralBodyComboBox->GetValue().c_str();
      id = thePropagator->GetParameterID(wxT("CentralBody"));
      thePropagator->SetStringParameter(id, str);

      // Range check the epoch value
      Real fromVal;
      Real toVal = -999.999;
      wxString newStr;
      if (spkEpFormat.Find(wxT("ModJulian")) == wxNOT_FOUND)
      {
         fromVal = -999.999;
         TimeConverterUtil::Convert(spkEpFormat.c_str(), fromVal,
               startEpochTextCtrl->GetValue().c_str(), wxT("A1ModJulian"), toVal,
               newStr);
         if (toVal < 6116.0)
            throw GmatBaseException(wxT("Start epochs must be later than ")
                  wxT("A.1 date 04 Oct 1957 12:00:00.000."));
      }
      else
      {
         startEpochTextCtrl->GetValue().ToDouble(&fromVal);
         if (fromVal < 6116.0)
            throw GmatBaseException(wxT("ModJulian epochs must be later than ")
                  wxT("(or equal to) 6116, the date Sputnik launched."));
      }

      str = propagatorEpochFormatComboBox->GetValue().c_str();
      id = thePropagator->GetParameterID(wxT("EpochFormat"));
      thePropagator->SetStringParameter(id, str);

      str = startEpochTextCtrl->GetValue().c_str();
      id = thePropagator->GetParameterID(wxT("StartEpoch"));
      thePropagator->SetStringParameter(id, str);
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return false;
   }

   #ifdef DEBUG_PROP_PANEL_SAVE
   ShowPropData(wxT("SaveData() AFTER  saving Propagator"));

   MessageInterface::ShowMessage(wxT("<%p> and <%p>\n"), thePropagator, theForceModel);

   #endif

   return true;
}


//------------------------------------------------------------------------------
// bool SaveDegOrder()
//------------------------------------------------------------------------------
bool PropagationConfigPanel::SaveDegOrder()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::SaveDegOrder() entered\n"));
   #endif

   Integer degree, order;
   wxString str;

   //-----------------------------------------------------------------
   // check values from text field
   //-----------------------------------------------------------------
   str = gravityDegreeTextCtrl->GetValue();
   CheckInteger(degree, str, wxT("Degree"), wxT("Integer Number >= 0")
            wxT("and < the maximum specified by the model, Order <= Degree]."));

   str = gravityOrderTextCtrl->GetValue();
   CheckInteger(order, str, wxT("Order"), wxT("Integer Number >= 0")
            wxT("and < the maximum specified by the model, Order <= Degree]."));

   if (!canClose)
      return false;

   //-----------------------------------------------------------------
   // save values to base, base code should do the range checking
   //-----------------------------------------------------------------
   try
   {
      #ifdef DEBUG_PROP_PANEL_SAVE
      MessageInterface::ShowMessage(wxT("   degree=%d, order=%d\n"), degree, order);
      #endif

      // check to see if degree is less than order
      if (degree < order)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, wxT("Degree can not be less than Order.\n")
             wxT("The allowed values are: [Integer >= 0 ")
             wxT("and < the maximum specified by the model, ")
             wxT("Order <= Degree]."));
         canClose = false;
         return false;
      }

      // save degree and order
      wxString bodyName = thePrimaryBodyComboBox->GetValue();

      #ifdef DEBUG_PROP_PANEL_SAVE
      MessageInterface::ShowMessage(wxT("   bodyName=%s\n"), bodyName.c_str());
      #endif

      // find gravity force pointer
//      for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
      if (primaryBodyData != NULL)
      {
//         if (primaryBodyList[i]->gravType != "None")
         if (primaryBodyData->gravType != wxT("None"))
         {
//            theGravForce = primaryBodyList[i]->gravf;
            theGravForce = primaryBodyData->gravf;
//            if (theGravForce != NULL && primaryBodyList[i]->bodyName == bodyName)
            if (theGravForce != NULL && primaryBodyData->bodyName == bodyName)
            {
               theGravForce->SetIntegerParameter(wxT("Degree"), degree);
               theGravForce->SetIntegerParameter(wxT("Order"), order);
            }
         }
      }

      isDegOrderChanged = false;
      return true;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return false;
   }
}


//------------------------------------------------------------------------------
// bool SavePotFile()
//------------------------------------------------------------------------------
bool PropagationConfigPanel::SavePotFile()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::SavePotFile() entered\n"));
   #endif

   // save data to core engine
   try
   {

      wxString inputString;
      wxString msg = wxT("The value of \"%s\" for field \"%s\" on object \"") +
                         thePropSetup->GetName() + wxT("\" is not an allowed value.  ")
                        wxT("\nThe allowed values are: [ %s ].");

      //      for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
      if (primaryBodyData != NULL)
      {
//         if (primaryBodyList[i]->gravType != "None")
         if (primaryBodyData->gravType != wxT("None"))
         {
//            theGravForce = primaryBodyList[i]->gravf;
            theGravForce = primaryBodyData->gravf;
            if (theGravForce != NULL)
            {
               #ifdef DEBUG_PROP_PANEL_SAVE
                  MessageInterface::ShowMessage
                    (wxT("SavePotFile() Saving Body:%s, potFile=%s\n"),
//                          primaryBodyList[i]->bodyName.c_str(), primaryBodyList[i]->potFilename.c_str());
                     primaryBodyData->bodyName.c_str(),
                     primaryBodyData->potFilename.c_str());
               #endif

//               inputString = primaryBodyList[i]->potFilename.c_str();
               inputString = primaryBodyData->potFilename.c_str();
               //std::ifstream filename(inputString.c_str());

//               // Check if the file doesn't exist then stop
//               if (!filename)
//               {
//                  MessageInterface::PopupMessage
//                     (Gmat::ERROR_, msg.c_str(), inputString.c_str(),
//                      "Model File", "File must exist");
//
//                  return false;
//               }

               //filename.close();
               theGravForce->SetStringParameter(wxT("PotentialFile"),
//                     primaryBodyList[i]->potFilename.c_str());
                     primaryBodyData->potFilename.c_str());
            }
         }
      }

      isPotFileChanged = false;
      return true;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return false;
   }
}


//------------------------------------------------------------------------------
// bool SaveAtmosModel()
//------------------------------------------------------------------------------
bool PropagationConfigPanel::SaveAtmosModel()
{
   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::SaveAtmosModel() entered\n"));
   #endif

   //-------------------------------------------------------
   // find drag force model
   //-------------------------------------------------------
   Integer paramId;
   bool dragForceFound = false;

   wxString bodyName = thePrimaryBodyComboBox->GetValue();
   wxString dragType = theAtmosModelComboBox->GetValue();

   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("   bodyName=%s, dragType=%s\n"), bodyName.c_str(), dragType.c_str());
   #endif

//   for (Integer i=0; i < (Integer)primaryBodyList.size(); i++)
   if (primaryBodyData != NULL)
   {
//      if (primaryBodyList[i]->dragType != dragModelArray[NONE_DM])
      if (primaryBodyData->dragType != dragModelArray[NONE_DM])
      {
//         theDragForce = primaryBodyList[i]->dragf;
         theDragForce = primaryBodyData->dragf;
//         if (theDragForce != NULL && primaryBodyList[i]->bodyName == bodyName)
         if (theDragForce != NULL && primaryBodyData->bodyName == bodyName)
         {
            dragForceFound = true;
//            break;
         }
      }
   }

   if (!dragForceFound)
   {
      MessageInterface::ShowMessage
         (wxT("PropagationConfigPanel::SaveAtmosModel() Drag Force not found ")
          wxT("for body:%s\n"), bodyName.c_str());

      return false;
   }

   theCelestialBody = theSolarSystem->GetBody(bodyName.c_str());
   theAtmosphereModel = theCelestialBody->GetAtmosphereModel();

   #ifdef DEBUG_PROP_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("PropagationConfigPanel::SaveAtmosModel() theCelestialBody=<%p>'%s', ")
       wxT("theAtmosphereModel=<%p>'%s'\n"), theCelestialBody,
       theCelestialBody->GetName().c_str(), theAtmosphereModel, theAtmosphereModel ?
       theAtmosphereModel->GetName().c_str() : wxT("NULL"));
   #endif

   if (theAtmosphereModel == NULL)
   {
      #ifdef DEBUG_PROP_PANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("PropagationConfigPanel::SaveAtmosModel() AtmosphereModel not found ")
          wxT("for body '%s'\n"), bodyName.c_str());
      #endif
   }

   //-------------------------------------------------------
   // save drag force model
   //-------------------------------------------------------
   try
   {
      if (theAtmosphereModel != NULL)
         theDragForce->SetInternalAtmosphereModel(theAtmosphereModel);
      paramId = theDragForce->GetParameterID(wxT("AtmosphereModel"));
      theDragForce->SetStringParameter(paramId, dragType.c_str());
      theDragForce->SetStringParameter(wxT("BodyName"), bodyName.c_str());

      isAtmosChanged = false;
      canClose = true;
      return true;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return false;
   }
}


//------------------------------------------------------------------------------
// wxString ToString(Real rval)
//------------------------------------------------------------------------------
wxString PropagationConfigPanel::ToString(Real rval)
{
   return theGuiManager->ToWxString(rval);
}


//------------------------------------------------------------------------------
// void OnIntegratorComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnIntegratorComboBox(wxCommandEvent &event)
{
   if (!integratorString.IsSameAs(theIntegratorComboBox->GetStringSelection()))
   {
      isIntegratorChanged = true;
      integratorString = theIntegratorComboBox->GetStringSelection();
      DisplayIntegratorData(true);
      isIntegratorDataChanged = false;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnPrimaryBodyComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnPrimaryBodyComboBox(wxCommandEvent &event)
{
   UpdatePrimaryBodyItems();
}


//------------------------------------------------------------------------------
// void OnOriginComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnOriginComboBox(wxCommandEvent &event)
{
   propOriginName = theOriginComboBox->GetValue();

   isOriginChanged = true;
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnGravityModelComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnGravityModelComboBox(wxCommandEvent &event)
{
//   if (primaryBodiesArray.IsEmpty())
   if (primaryBody == wxT("None"))
      return;

   gravTypeName = theGravModelComboBox->GetStringSelection();

//   if (primaryBodyList[currentBodyId]->gravType != gravTypeName)
   if (primaryBodyData->gravType != gravTypeName)
   {
      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("OnGravityModelComboBox() grav changed from=%s to=%s for body=%s\n"),
//          primaryBodyList[currentBodyId]->gravType.c_str(), gravTypeName.c_str(),
//          primaryBodyList[currentBodyId]->bodyName.c_str());
            primaryBodyData->gravType.c_str(), gravTypeName.c_str(),
            primaryBodyData->bodyName.c_str());
      #endif

//      primaryBodyList[currentBodyId]->gravType = gravTypeName;
      primaryBodyData->gravType = gravTypeName;

      if (gravTypeName != wxT("None") && gravTypeName != wxT("Other"))
      {
         wxString fileType = theFileMap[gravTypeName].c_str();
         //MessageInterface::ShowMessage("===> Found %s\n", fileType.c_str());

         try
         {
//            primaryBodyList[currentBodyId]->potFilename =
            primaryBodyData->potFilename =
               theGuiInterpreter->GetFileName(fileType).c_str();
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage
               (Gmat::WARNING_, e.GetFullMessage() +
                wxT("\nPlease select Other and specify file name\n"));
         }
      }
      else if (gravTypeName == wxT("Other"))
      {
//         primaryBodyList[currentBodyId]->potFilename = potFileTextCtrl->GetValue();
         primaryBodyData->potFilename = potFileTextCtrl->GetValue();
      }

      #ifdef DEBUG_PROP_PANEL_GRAV
      MessageInterface::ShowMessage
         (wxT("OnGravityModelComboBox() bodyName=%s, potFile=%s\n"),
//          primaryBodyList[currentBodyId]->bodyName.c_str(),
//          primaryBodyList[currentBodyId]->potFilename.c_str());
          primaryBodyData->bodyName.c_str(),
          primaryBodyData->potFilename.c_str());
      #endif

//      DisplayGravityFieldData(primaryBodyList[currentBodyId]->bodyName);
      DisplayGravityFieldData(primaryBodyData->bodyName);

      isPotFileChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnAtmosphereModelComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnAtmosphereModelComboBox(wxCommandEvent &event)
{
//   if (primaryBodiesArray.IsEmpty())
   if (primaryBody == wxT("None"))
      return;

   #ifdef DEBUG_PROP_PANEL_ATMOS
   MessageInterface::ShowMessage(wxT("OnAtmosphereModelComboBox() body=%s\n"),
                                 primaryBodyList[currentBodyId]->bodyName.c_str());
   #endif

   dragTypeName = theAtmosModelComboBox->GetStringSelection();

   // if we are creating new DragForce, set isForceModelChanged
//   if (primaryBodyList[currentBodyId]->dragf == NULL)
   if (primaryBodyData->dragf == NULL)
      isForceModelChanged = true;

//   if (primaryBodyList[currentBodyId]->dragType != dragTypeName)
   if (primaryBodyData->dragType != dragTypeName)
   {
      #ifdef DEBUG_PROP_PANEL_ATMOS
      MessageInterface::ShowMessage
         (wxT("OnAtmosphereModelComboBox() drag changed from=%s to=%s for body=%s\n"),
          primaryBodyList[currentBodyId]->dragType.c_str(), dragTypeName.c_str(),
          primaryBodyList[currentBodyId]->bodyName.c_str());
      #endif

//      primaryBodyList[currentBodyId]->dragType = dragTypeName;
      primaryBodyData->dragType = dragTypeName;
      DisplayAtmosphereModelData();

      isForceModelChanged = true;
      isAtmosChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnErrorControlComboBox(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnErrorControlComboBox(wxCommandEvent &event)
{
   #ifdef DEBUG_PROP_PANEL_ERROR
   MessageInterface::ShowMessage(wxT("OnErrorControlComboBox()\n"));
   #endif

   wxString eType = theErrorComboBox->GetStringSelection();

   if (errorControlTypeName != eType)
   {
      #ifdef DEBUG_PROP_PANEL_ERROR
      MessageInterface::ShowMessage
         (wxT("OnErrorControlComboBox() error control changed from=%s to=%s\n"),
          errorControlTypeName.c_str(), eType.c_str());
      #endif

      errorControlTypeName = eType;
      DisplayErrorControlData();

      // We don't want to create a new ForceModel if only Error Control is changed
      //isForceModelChanged = true;
      isErrControlChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnPropOriginComboBox(wxCommandEvent &)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnPropOriginComboBox(wxCommandEvent &)
{
   if (thePropagator->IsOfType(wxT("Integrator")))
      return;

   wxString propSelection = propCentralBodyComboBox->GetStringSelection();

   if (!spkBody.IsSameAs(propSelection))
   {
      isIntegratorDataChanged = true;
      spkBody = propSelection;
      isSpkBodyChanged = true;
      Integer cbIndex = propCentralBodyComboBox->FindString(propSelection);
      propCentralBodyComboBox->SetSelection(cbIndex);
      DisplayIntegratorData(false);
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnPropEpochComboBox(wxCommandEvent &)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnPropEpochComboBox(wxCommandEvent &)
{
   if (thePropagator->IsOfType(wxT("Integrator")))
      return;

   wxString epochSelection = propagatorEpochFormatComboBox->GetStringSelection();
   wxString newStr;

   if (!spkEpFormat.IsSameAs(epochSelection))
   {
      isIntegratorDataChanged = true;

      spkEpoch = startEpochTextCtrl->GetValue();

      // Update the epoch string
      try
      {
         Real fromVal;
         Real toVal = -999.999;

         if (spkEpFormat.Find(wxT("ModJulian")) == wxNOT_FOUND)
            fromVal = -999.999;
         else
         {
            spkEpoch.ToDouble(&fromVal);
            if (fromVal < 6116.0)
               throw GmatBaseException(wxT("ModJulian epochs must be later than ")
                     wxT("(or equal to) 6116, the date Sputnik launched."));
         }

         TimeConverterUtil::Convert(spkEpFormat.c_str(), fromVal, spkEpoch.c_str(),
               epochSelection.c_str(), toVal, newStr);
      }
      catch (BaseException &e)
      {
         MessageInterface::PopupMessage
            (Gmat::ERROR_, e.GetFullMessage() +
             wxT("\nPlease enter valid Epoch before changing the Epoch Format\n"));

         Integer epIndex = propagatorEpochFormatComboBox->FindString(spkEpFormat);
         propagatorEpochFormatComboBox->SetSelection(epIndex);
         return;
      }

      spkEpFormat = epochSelection;
      isSpkEpFormatChanged = true;
      Integer epIndex = propagatorEpochFormatComboBox->FindString(epochSelection);
      propagatorEpochFormatComboBox->SetSelection(epIndex);

      spkEpoch = newStr.c_str();
      isSpkEpochChanged = true;

      DisplayIntegratorData(false);
      EnableUpdate(true);
   }
}


// wxButton events
//------------------------------------------------------------------------------
// void OnAddBodyButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnAddBodyButton(wxCommandEvent &event)
{
//   CelesBodySelectDialog bodyDlg(this, primaryBodiesArray, secondaryBodiesArray);
//   bodyDlg.ShowModal();

//   if (bodyDlg.IsBodySelected())
//   {
//      wxArrayString &names = bodyDlg.GetBodyNames();
//
//      if (names.IsEmpty())
//      {
//         EnablePrimaryBodyItems(false, true);
//
//         EnableUpdate(true);
//         isForceModelChanged = true;
//         return;
//      }
//      else
//      {
//         EnablePrimaryBodyItems(true);
//      }
//
//      std::vector<ForceType*> oldList;
//      wxString bodyName;
//
//      oldList = primaryBodyList;
//      primaryBodyList.clear();
//      primaryBodiesArray.Clear();
//      primaryBody = "None";
//
//      for (Integer i = 0; i < (Integer)names.GetCount(); i++)
//      {
//         bodyName = names[i];
//         primaryBodiesArray.Add(bodyName.c_str());
//
//         // Set default gravity model file for display
//         if (bodyName == "Earth")
//            currentBodyId = FindPrimaryBody(bodyName, true, earthGravModelArray[JGM2]);
//         else if (bodyName == "Luna")
//            currentBodyId = FindPrimaryBody(bodyName, true, lunaGravModelArray[LP165]);
//         else if (bodyName == "Venus")
//            currentBodyId = FindPrimaryBody(bodyName, true, venusGravModelArray[MGNP180U]);
//         else if (bodyName == "Mars")
//            currentBodyId = FindPrimaryBody(bodyName, true, marsGravModelArray[MARS50C]);
//         else
//            currentBodyId = FindPrimaryBody(bodyName, true, othersGravModelArray[O_NONE_GM]);
//
//
//         // Copy old body force model
//         for (Integer j = 0; j < (Integer)oldList.size(); j++)
//            if (bodyName == oldList[j]->bodyName)
//               primaryBodyList[currentBodyId] = oldList[j];
//      }
//   }

   //----------------------------------------------
   // Append body names to combobox and text field
   //----------------------------------------------
//   thePrimaryBodyComboBox->Clear();
//   bodyTextCtrl->Clear();
//   wxString name;
//   for (Integer i = 0; i < (Integer)primaryBodyList.size(); i++)
//   {
//      name = primaryBodyList[i]->bodyName.c_str();
//      thePrimaryBodyComboBox->Append(primaryBodyList[i]->bodyName.c_str());
//      thePrimaryBodyComboBox->SetValue(primaryBodyList[i]->bodyName.c_str());
//   }
   UpdatePrimaryBodyComboBoxList();
//   thePrimaryBodyComboBox->SetValue(primaryBodyList[0]->bodyName.c_str());
   thePrimaryBodyComboBox->SetValue(primaryBodyData->bodyName.c_str());

   UpdatePrimaryBodyItems();
   DisplayForceData();

   EnableUpdate(true);
   isForceModelChanged = true;
}


//------------------------------------------------------------------------------
// void OnGravSearchButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnGravSearchButton(wxCommandEvent &event)
{
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.*"));

   if (dialog.ShowModal() == wxID_OK)
   {
      wxString filename;

      filename = dialog.GetPath();
      wxString upperCaseFilename = filename;

      // Make filename upper case for comparison
      upperCaseFilename.MakeUpper();

      // Determine the type of file
      if (upperCaseFilename.find(wxT(".DAT"),0) != wxString::npos)
      {
         ParseDATGravityFile(filename);
      }
      else if (upperCaseFilename.find(wxT(".GRV"),0) != wxString::npos)
      {
         ParseGRVGravityFile(filename);
      }
      else if (upperCaseFilename.find(wxT(".COF"),0) != wxString::npos)
      {
         ParseCOFGravityFile(filename);
      }
      else
      {
         MessageInterface::PopupMessage
            (Gmat::WARNING_, wxT("Gravity file \"") + filename + wxT("\" is of unknown format."));
         return;
      }

//      primaryBodyList[currentBodyId]->potFilename = filename;
      primaryBodyData->potFilename = filename;

//      if (primaryBodyList[currentBodyId]->bodyName == "Earth")
//         primaryBodyList[currentBodyId]->gravType = earthGravModelArray[E_OTHER];
//      else if (primaryBodyList[currentBodyId]->bodyName == "Luna")
//         primaryBodyList[currentBodyId]->gravType = lunaGravModelArray[L_OTHER];
//      else if (primaryBodyList[currentBodyId]->bodyName == "Mars")
//         primaryBodyList[currentBodyId]->gravType = marsGravModelArray[M_OTHER];
//      else //other bodies
//         primaryBodyList[currentBodyId]->gravType = othersGravModelArray[O_OTHER];
      if (primaryBodyData->bodyName == wxT("Earth"))
         primaryBodyData->gravType = earthGravModelArray[E_OTHER];
      else if (primaryBodyData->bodyName == wxT("Luna"))
         primaryBodyData->gravType = lunaGravModelArray[L_OTHER];
      else if (primaryBodyData->bodyName == wxT("Mars"))
         primaryBodyData->gravType = marsGravModelArray[M_OTHER];
      // todo Add Venus here?
      else //other bodies
         primaryBodyData->gravType = othersGravModelArray[O_OTHER];

      //loj: Do we need to show? body name didn't change
      //waw: Yes, we need to update the degree & order displays (10/17/06)
//      DisplayGravityFieldData(primaryBodyList[currentBodyId]->bodyName);
      DisplayGravityFieldData(primaryBodyData->bodyName);
      isDegOrderChanged = true;
      EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OnSetupButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnSetupButton(wxCommandEvent &event)
{
   DragForce *dragForce = NULL;

   // if DragForce has not been created, create it first by calling SaveData()
//   if (primaryBodyList[currentBodyId]->dragf == NULL)
   if (primaryBodyData->dragf == NULL)
   {
      isForceModelChanged = true;
      SaveData();
   }
   else if (isAtmosChanged)
   {
      SaveAtmosModel();
   }

//   dragForce = primaryBodyList[currentBodyId]->dragf;
   dragForce = primaryBodyData->dragf;
   if (dragForce != NULL)
   {
////      if (primaryBodyList[currentBodyId]->dragType == dragModelArray[EXPONENTIAL])
//      if (primaryBodyData->dragType == dragModelArray[EXPONENTIAL])
//      {
//         // TBD by Code 595
//         //DragInputsDialog dragDlg(this, dragForce, "ExponentialDragDialog");
//         //dragDlg.ShowModal();
//      }
//      else
//      if (primaryBodyList[currentBodyId]->dragType == dragModelArray[MSISE90])
      if (primaryBodyData->dragType == dragModelArray[MSISE90])
      {
         DragInputsDialog dragDlg(this, dragForce, wxT("MSISE90DragDialog"));
         dragDlg.ShowModal();
      }
//      else if (primaryBodyList[currentBodyId]->dragType == dragModelArray[JR])
      else if (primaryBodyData->dragType == dragModelArray[JR])
      {
         DragInputsDialog dragDlg(this, dragForce, wxT("JacchiaRobertsDialog"));
         dragDlg.ShowModal();
      }
   }
}


//------------------------------------------------------------------------------
// void OnMagSearchButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnMagSearchButton(wxCommandEvent &event)
{
   wxFileDialog dialog(this, wxT("Choose a file"), wxT(""), wxT(""), wxT("*.*"));

   if (dialog.ShowModal() == wxID_OK)
   {
      wxString filename;

      filename = dialog.GetPath().c_str();
      theMagfModelComboBox->Append(filename);
   }
   EnableUpdate(true);
}

//------------------------------------------------------------------------------
// void OnPMEditButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnPMEditButton(wxCommandEvent &event)
{
   wxArrayString primaryBodiesArray;
   if (primaryBody != wxT("None"))
      primaryBodiesArray.Add(primaryBody);

   CelesBodySelectDialog bodyDlg(this, secondaryBodiesArray, primaryBodiesArray);
   bodyDlg.ShowModal();

   if (bodyDlg.IsBodySelected())
   {
      wxArrayString &names = bodyDlg.GetBodyNames();

      for (Integer i=0; i<(Integer)pointMassBodyList.size(); i++)
         delete pointMassBodyList[i];

      if (names.IsEmpty())
      {
         pointMassBodyList.clear();
         secondaryBodiesArray.Clear();
         pmEditTextCtrl->Clear();
         EnableUpdate(true);
         isForceModelChanged = true;
         UpdatePrimaryBodyComboBoxList();
         // Redisplay primary body after combo box change
         DisplayPrimaryBodyData();
         return;
      }

      wxString bodyName;

      pointMassBodyList.clear();
      secondaryBodiesArray.Clear();
      pmEditTextCtrl->Clear();

      //--------------------------
      // Add bodies to pointMassBodyList
      //--------------------------
      for (Integer i=0; i < (Integer)names.GetCount(); i++)
      {
         bodyName = names[i];
         pointMassBodyList.push_back(new ForceType(bodyName));

         secondaryBodiesArray.Add(bodyName);
         pmEditTextCtrl->AppendText(names[i] + wxT(" "));
      }

      UpdatePrimaryBodyComboBoxList();

      EnableUpdate(true);
      isForceModelChanged = true;
   }
   
   // Redisplay primary body after combo box change
   DisplayPrimaryBodyData();
}

//------------------------------------------------------------------------------
// void OnSRPEditButton(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnSRPEditButton(wxCommandEvent &event)
{
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnIntegratorTextUpdate(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnIntegratorTextUpdate(wxCommandEvent &event)
{
   //isIntegratorChanged = true;
   isIntegratorDataChanged = true;
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnGravityTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnGravityTextUpdate(wxCommandEvent& event)
{
   EnableUpdate(true);

   if (event.GetEventObject() == gravityDegreeTextCtrl)
   {
//      primaryBodyList[currentBodyId]->gravDegree = gravityDegreeTextCtrl->GetValue();
      primaryBodyData->gravDegree = gravityDegreeTextCtrl->GetValue();
      isDegOrderChanged = true;
      // Do not set to true if only text changed
      //isForceModelChanged = true;
   }
   else if (event.GetEventObject() == gravityOrderTextCtrl)
   {
//      primaryBodyList[currentBodyId]->gravOrder = gravityOrderTextCtrl->GetValue();
      primaryBodyData->gravOrder = gravityOrderTextCtrl->GetValue();
      isDegOrderChanged = true;
      // Do not set to true if only text changed
      //isForceModelChanged = true;
   }
   else if (event.GetEventObject() == potFileTextCtrl)
   {
//      primaryBodyList[currentBodyId]->potFilename = potFileTextCtrl->GetValue();
      primaryBodyData->potFilename = potFileTextCtrl->GetValue();
      isPotFileChanged = true;
      // Do not set to true if only text changed
      //isForceModelChanged = true;
   }
}

//------------------------------------------------------------------------------
// void OnMagneticTextUpdate(wxCommandEvent& event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnMagneticTextUpdate(wxCommandEvent& event)
{
   EnableUpdate(true);
   isMagfTextChanged = true;
}


//------------------------------------------------------------------------------
// void OnSRPCheckBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnSRPCheckBoxChange(wxCommandEvent &event)
{
   usePropOriginForSrp = theSrpCheckBox->GetValue();
   isForceModelChanged = true;
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void OnSRPCheckBoxChange(wxCommandEvent &event)
//------------------------------------------------------------------------------
void PropagationConfigPanel::OnStopCheckBoxChange(wxCommandEvent &event)
{
   stopOnAccViolation      = theStopCheckBox->GetValue();
   isIntegratorDataChanged = true;
   EnableUpdate(true);
}


//------------------------------------------------------------------------------
// void ShowPropData()
//------------------------------------------------------------------------------
void PropagationConfigPanel::ShowPropData(const wxString& header)
{
   MessageInterface::ShowMessage(wxT(">>>>>=======================================\n"));
   MessageInterface::ShowMessage(wxT("%s\n"), header.c_str());
   MessageInterface::ShowMessage(wxT("   thePropSetup =%p, name=%s\n"),
                                 thePropSetup, thePropSetup->GetName().c_str());
   MessageInterface::ShowMessage(wxT("   thePropagator=%p, name=%s\n"), thePropagator,
                                 thePropagator->GetTypeName().c_str());
   if ((theForceModel != NULL) && (thePropagator->IsOfType(wxT("Integrator"))))
   {
      MessageInterface::ShowMessage(wxT("   theForceModel=%p, name=%s\n"),
            theForceModel, (theForceModel == NULL ? wxT("") :
            theForceModel->GetName().c_str()));
      MessageInterface::ShowMessage(wxT("   numOfForces=%d\n"), numOfForces);

      wxString forceType;
      wxString forceBody;
      PhysicalModel *force;

      for (int i=0; i<numOfForces; i++)
      {
         force = theForceModel->GetForce(i);
         forceType = force->GetTypeName();

         forceBody = force->GetStringParameter(wxT("BodyName"));

         MessageInterface::ShowMessage(wxT("   forceBody=%s, forceType=%s\n"), forceBody.c_str(),
                                       forceType.c_str());
      }
   }
   MessageInterface::ShowMessage(wxT("============================================\n"));
}


//------------------------------------------------------------------------------
// void ShowForceList()
//------------------------------------------------------------------------------
void PropagationConfigPanel::ShowForceList(const wxString &header)
{
   MessageInterface::ShowMessage(wxT(">>>>>=======================================\n"));
   MessageInterface::ShowMessage(wxT("%s\n"), header.c_str());
   MessageInterface::ShowMessage(wxT("   theForceModel=%p\n"), theForceModel);

   if (theForceModel != NULL)
   {
      // srp force
      MessageInterface::ShowMessage
         (wxT("   usePropOriginForSrp=%d\n"), usePropOriginForSrp);

      // primary body list
//      for (unsigned int i=0; i<primaryBodyList.size(); i++)
      if (primaryBodyData != NULL)
      {
//         MessageInterface::ShowMessage
//            ("   id=%d, body=%s, gravType=%s, dragType=%s, magfType=%s\n   potFile=%s\n"
//             "   gravf=%p, dragf=%p, srpf=%p\n", i, primaryBodyList[i]->bodyName.c_str(),
//             primaryBodyList[i]->gravType.c_str(), primaryBodyList[i]->dragType.c_str(),
//             primaryBodyList[i]->magfType.c_str(), primaryBodyList[i]->potFilename.c_str(),
//             primaryBodyList[i]->gravf, primaryBodyList[i]->dragf,
//             primaryBodyList[i]->srpf);
         MessageInterface::ShowMessage
            (wxT("   id=%d, body=%s, gravType=%s, dragType=%s, magfType=%s\n   potFile=%s\n")
             wxT("   gravf=%p, dragf=%p, srpf=%p\n"), 0, primaryBodyData->bodyName.c_str(),
             primaryBodyData->gravType.c_str(), primaryBodyData->dragType.c_str(),
             primaryBodyData->magfType.c_str(), primaryBodyData->potFilename.c_str(),
             primaryBodyData->gravf, primaryBodyData->dragf,
             primaryBodyData->srpf);
      }

      // pointmass body list
      MessageInterface::ShowMessage(wxT("   pointMassBodyListSize=%d\n"), pointMassBodyList.size());
      for (Integer i=0; i < (Integer)pointMassBodyList.size(); i++)
      {
         MessageInterface::ShowMessage
            (wxT("   id=%d, body=%s, pmf=%p\n"), i, pointMassBodyList[i]->bodyName.c_str(),
             pointMassBodyList[i]->pmf);
      }
   }
   MessageInterface::ShowMessage(wxT("============================================\n"));
}


//------------------------------------------------------------------------------
// void ShowForceModel()
//------------------------------------------------------------------------------
void PropagationConfigPanel::ShowForceModel(const wxString &header)
{
   if (theForceModel != NULL)
   {
      MessageInterface::ShowMessage(wxT(">>>>>=======================================\n"));
      MessageInterface::ShowMessage(wxT("%s%s\n"), header.c_str(), wxT(" --- ForceModel"));

      Integer numForces = theForceModel->GetNumForces();
      PhysicalModel *pm;

      for (int i = 0; i < numForces; i++)
      {
         pm = theForceModel->GetForce(i);
         MessageInterface::ShowMessage
            (wxT("id=%d, body=%s, type=%s, addr=%p\n"), i, pm->GetBodyName().c_str(),
             pm->GetTypeName().c_str(), pm);
      }
      MessageInterface::ShowMessage(wxT("============================================\n"));
   }
}


//------------------------------------------------------------------------------
// bool ParseDATGravityFile(const wxString& fname)
//------------------------------------------------------------------------------
void PropagationConfigPanel::ParseDATGravityFile(const wxString& fname)
{
   Integer      cc, dd, sz=0;
   Integer      iscomment, rtn;
   Integer      n=0, m=0;
   Integer      fileDegree, fileOrder;
   Real         Cnm=0.0, Snm=0.0, dCnm=0.0, dSnm=0.0;
   // @to do should mu & radius be constant?? - waw
   Real         mu=GmatSolarSystemDefaults::PLANET_MU[GmatSolarSystemDefaults::EARTH]; // gravity parameter of central body
   Real         a=GmatSolarSystemDefaults::PLANET_EQUATORIAL_RADIUS[GmatSolarSystemDefaults::EARTH];  // radius of central body ( mean equatorial )
   char         buf[CelestialBody::BUFSIZE];
   FILE        *fp;

   for (cc = 2;cc <= HarmonicField::HF_MAX_DEGREE; ++cc)
   {
      for (dd = 0; dd <= cc; ++dd)
      {
         sz++;
      }
   }

   /* read coefficients from file */
   fp = fopen( fname.char_str(), "r");
   if (!fp)
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Error reading gravity model file."));
         return;
   }

   PrepareGravityArrays();
   iscomment = 1;

   while ( iscomment )
   {
      rtn = fgetc( fp );

      if ( (char)rtn == '#' )
      {
         fgets( buf, CelestialBody::BUFSIZE, fp );
      }
      else
      {
         ungetc( rtn, fp );
         iscomment = 0;
      }
   }

   fscanf(fp, "%lg\n", &mu ); mu = (Real)mu / 1.0e09;      // -> Km^3/sec^2
   fscanf(fp, "%lg\n", &a ); a = (Real)a / 1000.0;         // -> Km
   fgets( buf, CelestialBody::BUFSIZE, fp );

   while ( ( (char)(rtn=fgetc(fp)) != '#' ) && (rtn != EOF) )
   {
      ungetc( rtn, fp );
      fscanf( fp, "%i %i %le %le\n", &n, &m, &dCnm, &dSnm );
      if ( n <= GRAV_MAX_DRIFT_DEGREE  && m <= n )
      {
         dCbar[n][m] = (Real)dCnm;
         dSbar[n][m] = (Real)dSnm;
      }
   }

   fgets( buf, CelestialBody::BUFSIZE, fp );

   fileDegree = 0;
   fileOrder  = 0;
   cc=0;n=0;m=0;

   do
   {
      if ( n <= HarmonicField::HF_MAX_DEGREE && m <= HarmonicField::HF_MAX_ORDER )
      {
         Cbar[n][m] = (Real)Cnm;
         Sbar[n][m] = (Real)Snm;
      }
      if (n > fileDegree) fileDegree = n;
      if (n > fileOrder)  fileOrder  = n;

      cc++;
   } while ( ( cc<=sz ) && ( fscanf( fp, "%i %i %le %le\n", &n, &m, &Cnm, &Snm ) > 0 ));

   // Save as string
//   primaryBodyList[currentBodyId]->gravDegree.Printf("%d", fileDegree);
//   primaryBodyList[currentBodyId]->gravOrder.Printf("%d", fileOrder);
   primaryBodyData->gravDegree.Printf(wxT("%d"), fileDegree);
   primaryBodyData->gravOrder.Printf(wxT("%d"), fileOrder);
}


//------------------------------------------------------------------------------
// bool ParseGRVGravityFile(const wxString& fname)
//------------------------------------------------------------------------------
void PropagationConfigPanel::ParseGRVGravityFile(const wxString& fname)
{
   Integer       fileOrder, fileDegree;

   wxFileInputStream inFileStream(fname);
   wxTextInputStream inFile(inFileStream);

   if (!inFileStream.IsOk())
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Error reading gravity model file."));
         return;
   }

   wxString s;
   wxString firstStr;

   while (!inFileStream.Eof())
   {
      s = inFile.ReadLine();
      wxStringInputStream lineStringStream(s);
      wxTextInputStream lineStr(lineStringStream);

      // ignore comment lines
      if (s[0] != '#')
      {
         lineStr >> firstStr;
         wxString upperString = GmatStringUtil::ToUpper(firstStr);
         //VC++ error C3861: 'strcasecmp': identifier not found
         // since using wxString, use GmatStringUtil and ==
         //if (strcmpi(firstStr.c_str(),"Degree") == 0)
         if (upperString == wxT("DEGREE"))
            lineStr >> fileDegree;
         //else if (strcmpi(firstStr.c_str(),"Order") == 0)
         else if (upperString == wxT("ORDER"))
            lineStr >> fileOrder;
      }
   }

   // Save as string
//   primaryBodyList[currentBodyId]->gravDegree.Printf("%d", fileDegree);
//   primaryBodyList[currentBodyId]->gravOrder.Printf("%d", fileOrder);
   primaryBodyData->gravDegree.Printf(wxT("%d"), fileDegree);
   primaryBodyData->gravOrder.Printf(wxT("%d"), fileOrder);
}

//------------------------------------------------------------------------------
// bool ParseCOFGravityFile(const wxString& fname)
//------------------------------------------------------------------------------
void PropagationConfigPanel::ParseCOFGravityFile(const wxString& fname)
{
   Integer       fileOrder = 70, fileDegree = 70;

   wxFileInputStream inFileStream(fname);
   wxTextInputStream inFile(inFileStream);

   bool done = false;

   if (!inFileStream.IsOk())
   {
      MessageInterface::PopupMessage
         (Gmat::WARNING_, wxT("Error reading gravity model file."));
      return;
   }

   wxString s;
   wxString firstStr;
   wxString degStr, ordStr;

   Integer counter = 0;
   while (!done && (counter < 129650))
   {
      s = inFile.ReadLine();


//      lineStr.str(s);

      // ignore comment lines
      if (s[0] != 'C')
      {

         firstStr = s.substr(0, 8);
         firstStr = GmatStringUtil::Trim(firstStr);

         if (counter > 10)
            break;

         if (firstStr == wxT("POTFIELD"))
         {
            // This line DOES NOT WORK for fields 100x100 or larger
//            lineStr >> fileDegree >> fileOrder >> int1 >> real1 >> real2 >> real3;
            degStr = s.substr(8, 3);
            ordStr = s.substr(11, 3);

            if ((GmatStringUtil::ToInteger(degStr, fileDegree) == false) ||
                (GmatStringUtil::ToInteger(ordStr, fileOrder) == false))
            {
               MessageInterface::PopupMessage
                  (Gmat::WARNING_, wxT("Error reading degree and/or order from ")
                        wxT("gravity model file."));
            }
            done = true;
         }

         // Count non-comment lines to be sure we're not in an infinite loop
         ++counter;
      }
   }

   // Save as string
//   primaryBodyList[currentBodyId]->gravDegree.Printf("%d", fileDegree);
//   primaryBodyList[currentBodyId]->gravOrder.Printf("%d", fileOrder);
   primaryBodyData->gravDegree.Printf(wxT("%d"), fileDegree);
   primaryBodyData->gravOrder.Printf(wxT("%d"), fileOrder);
}

//------------------------------------------------------------------------------
// void PrepareGravityArrays()
//------------------------------------------------------------------------------
void PropagationConfigPanel::PrepareGravityArrays()
{
   Integer m, n;

   for (n=0; n <= HarmonicField::HF_MAX_DEGREE; ++n)
      for ( m=0; m <= HarmonicField::HF_MAX_ORDER; ++m)
      {
         Cbar[n][m] = 0.0;
         Sbar[n][m] = 0.0;
      }

   for (n = 0; n <= GRAV_MAX_DRIFT_DEGREE; ++n)
   {
      for (m = 0; m <= GRAV_MAX_DRIFT_DEGREE; ++m)
      {
         dCbar[n][m] = 0.0;
         dSbar[n][m] = 0.0;
      }
   }
}


//-----------------------------------------------------------------------------
// void PropagationConfigPanel::ShowIntegratorLayout(bool isIntegrator,
//-----------------------------------------------------------------------------
void PropagationConfigPanel::ShowIntegratorLayout(bool isIntegrator,
      bool isEphem)
{
   bool isPredictorCorrector = false;

   if (thePropagator != NULL)
      if (thePropagator->IsOfType(wxT("PredictorCorrector")))
         isPredictorCorrector = true;

   if (isIntegrator)
   {
      fmStaticSizer->Show(true);
      fmStaticSizer->Hide( magfStaticSizer, true );

      initialStepSizeStaticText->Show(true);
      unitsInitStepSizeStaticText->Show(true);
      accuracyStaticText->Show(true);
      minStepStaticText->Show(true);
      unitsMinStepStaticText->Show(true);
      maxStepStaticText->Show(true);
      unitsMaxStepStaticText->Show(true);
      maxStepAttemptStaticText->Show(true);

      initialStepSizeTextCtrl->Show(true);
      accuracyTextCtrl->Show(true);
      minStepTextCtrl->Show(true);
      maxStepTextCtrl->Show(true);
      maxStepAttemptTextCtrl->Show(true);

      if (isPredictorCorrector)
      {
         minIntErrorStaticText->Show(true);
         nomIntErrorStaticText->Show(true);
         minIntErrorTextCtrl->Show(true);
         nomIntErrorTextCtrl->Show(true);
      }

      propagatorStepSizeStaticText->Show(false);
      propagatorStepSizeTextCtrl->Show(false);
      unitsPropagatorStepSizeStaticText->Show(false);
      propCentralBodyStaticText->Show(false);
      propCentralBodyComboBox->Show(false);
      propagatorEpochFormatStaticText->Show(false);
      propagatorEpochFormatComboBox->Show(false);
      startEpochStaticText->Show(false);
      startEpochTextCtrl->Show(false);

      theStopCheckBox->Show(true);
   }
   else // It's either analytic or ephem based; hide other stuff
   {
      if (isEphem)
      {
         fmStaticSizer->Show(false);

         minIntErrorStaticText->Show(false);
         nomIntErrorStaticText->Show(false);

         initialStepSizeStaticText->Show(false);
         unitsInitStepSizeStaticText->Show(false);
         accuracyStaticText->Show(false);
         minStepStaticText->Show(false);
         unitsMinStepStaticText->Show(false);
         maxStepStaticText->Show(false);
         unitsMaxStepStaticText->Show(false);
         maxStepAttemptStaticText->Show(false);

         initialStepSizeTextCtrl->Show(false);
         accuracyTextCtrl->Show(false);
         minStepTextCtrl->Show(false);
         maxStepTextCtrl->Show(false);
         maxStepAttemptTextCtrl->Show(false);
         minIntErrorTextCtrl->Show(false);
         nomIntErrorTextCtrl->Show(false);

         propagatorStepSizeStaticText->Show(true);
         propagatorStepSizeTextCtrl->Show(true);
         unitsPropagatorStepSizeStaticText->Show(true);
         propCentralBodyStaticText->Show(true);
         propCentralBodyComboBox->Show(true);
         propagatorEpochFormatStaticText->Show(true);
         propagatorEpochFormatComboBox->Show(true);
         startEpochStaticText->Show(true);
         startEpochTextCtrl->Show(true);
      }
      else
      {
         // Not yet supported
      }
   }

   theMiddleSizer->Layout();
   Refresh();
}
