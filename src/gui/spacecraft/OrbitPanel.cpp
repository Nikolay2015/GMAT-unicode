//$Id: OrbitPanel.cpp 9807 2011-08-29 14:13:52Z wendys-dev $
//------------------------------------------------------------------------------
//                           OrbitPanel
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
// Author: Allison Greene
// Created: 2004/04/08
/**
 * This class contains information needed to setup users spacecraft orbit
 * through GUI
 * 
 * The basic design for state handling on this panel is that the spacecraft is
 * the repository for the state information.  When the panel is displayed, a 
 * clone of the spacecraft is made and the state information for that spacecraft
 * clone is accessed to populate the data on the panel.  This population is done
 * by accessing the spacecraft state from the spacecraft's internal PropState,
 * which is a Cartesian MJ2000 equatorial state expressed in terms of the 
 * current GMAT internal CoordinateSystem.  When the panel is populated, that 
 * state is converted to the coordinate system and representation desired for
 * display, and then used to fill in the elements on the panel.  The OrbitPanel
 * maintains the state in the internal MJ2000 equatorial Cartesian coordinates,
 * in the mCartState member.  All conversions and transformations return to this
 * representation internally.
 *
 * Steps taken on ComboBoxChange()
 *
 * When EpochFormat change
 *   If epoch is modified by user
 *      Get epoch string value from the epoch text field.
 *      If from format is not TAIModJulian,
 *         Convert to TAIModJulian and save as mTaiMjdStr
 *      Else
 *         Save epoch string as mTaiMjdStr
 *      EndIf
 *      Call TimeConverterUtil::Convert() to convert from old to new format
 *      Set epoch string value
 *   Else
 *      Call TimeConverterUtil::Convert() to convert from mTaiMjdStr to new format
 *      Set epoch string value
 *   EndIf
 *
 * When CoordinateSystem or StateType changed
 *    If state is modified
 *       Validate user input by calling CheckState()
 *       If not valid
 *          Popup error message and return;
 *       Endif
 *    EndIf
 *
 *    Get output CoordinateSystem pointer
 *    Call BuildValidStateTypes() to reset valid state type list
 *    Call DisplayState()
 *
 * When AnomalyType changed
 *    If state is modified
 *       Validate user input by calling CheckState()
 *       If not valid
 *          Popup error message and return;
 *       Endif
 *    EndIf
 *
 *    Call DisplayState()
 */
//------------------------------------------------------------------------------
#include "OrbitPanel.hpp"
#include "MessageInterface.hpp"
#include "Anomaly.hpp"
#include <sstream>
#include <wx/config.h>

//#define DEBUG_ORBIT_PANEL
//#define DEBUG_ORBIT_PANEL_LOAD
//#define DEBUG_ORBIT_PANEL_CONVERT
//#define DEBUG_ORBIT_PANEL_CHECKSTATE
//#define DEBUG_ORBIT_PANEL_COMBOBOX
//#define DEBUG_ORBIT_PANEL_STATE_TYPE
//#define DEBUG_ORBIT_PANEL_STATE_CHANGE
//#define DEBUG_ORBIT_PANEL_TEXT_CHANGE
//#define DEBUG_ORBIT_PANEL_SAVE
//#define DEBUG_ORBIT_PANEL_CHECK_RANGE

//------------------------------
// event tables for wxWindows
//------------------------------
BEGIN_EVENT_TABLE(OrbitPanel, wxPanel)
   EVT_COMBOBOX(ID_COMBOBOX, OrbitPanel::OnComboBoxChange)
   EVT_TEXT(ID_TEXTCTRL, OrbitPanel::OnTextChange)
   EVT_BUTTON(ID_BUTTON, OrbitPanel::OnButton)
END_EVENT_TABLE()

//------------------------------
// public methods
//------------------------------
//------------------------------------------------------------------------------
// OrbitPanel(GmatPanel *scPanel, wxWindow *parent, Spacecraft *spacecraft,
//            SolarSystem *solarsystem
//------------------------------------------------------------------------------
/**
 * Constructs OrbitPanel object.
 *
 * @param <parent> input parent.
 *
 * @note Creates the Universe GUI
 */
//------------------------------------------------------------------------------
OrbitPanel::OrbitPanel(GmatPanel *scPanel, wxWindow *parent,
                       Spacecraft *spacecraft, SolarSystem *solarsystem)
   : wxPanel(parent)
{
   // initialize data members
   theScPanel = scPanel;
   theGuiInterpreter = GmatAppData::Instance()->GetGuiInterpreter();
   theGuiManager = GuiItemManager::GetInstance();
   
   theSpacecraft = spacecraft;
   theSolarSystem = solarsystem;
   
   mIsCoordSysChanged = false;
   mIsStateTypeChanged = false;
   mIsAnomalyTypeChanged = false;
   mIsStateChanged = false;
   mShowFullStateTypeList = true;
   ResetStateFlags();   
   mIsEpochChanged = false;
   mIsEpochModified = false;
   canClose = true;
   coordSysCBOnly = false;
   dataChanged = false;
   
   Create();
}


//------------------------------------------------------------------------------
// ~OrbitPanel()
//------------------------------------------------------------------------------
OrbitPanel::~OrbitPanel()
{
   theGuiManager->UnregisterComboBox(wxT("CoordinateSystem"), mCoordSysComboBox);
}


//------------------------------------------------------------------------------
// void LoadData()
//------------------------------------------------------------------------------
/**
 * @note Gets the values from theSpacecraft and puts them in the text fields
 */
//------------------------------------------------------------------------------
void OrbitPanel::LoadData()
{
   #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage(wxT("OrbitPanel::LoadData() entered\n"));
   #endif
      
   mAnomaly = theSpacecraft->GetAnomaly();
   mTrueAnomaly = mAnomaly;
   
   mInternalCoord = theGuiInterpreter->GetInternalCoordinateSystem();
   
   if (mInternalCoord == NULL)
   {
      MessageInterface::ShowMessage(wxT("   mInternalCoord is NULL.\n"));
   }
   else
   {
      #ifdef DEBUG_ORBIT_PANEL_LOAD
         MessageInterface::ShowMessage(
            wxT("   mInternalCoord=%s, addr=%d\n"),
            mInternalCoord->GetName().c_str(), mInternalCoord);
      #endif
   }
   
   // load data from the core engine
   try
   {
      Rvector6 outState, displayState;
      
      // Load the epoch formats
      StringArray reps = TimeConverterUtil::GetValidTimeRepresentations();
      for (unsigned int i = 0; i < reps.size(); i++)
         epochFormatComboBox->Append(reps[i].c_str());
      
      // load the epoch
      wxString epochFormat = theSpacecraft->GetStringParameter(wxT("DateFormat"));      
      mEpoch = theSpacecraft->GetRealParameter(wxT("A1Epoch"));
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   epochFormat=%s, mEpoch=%f\n"), epochFormat.c_str(), mEpoch);
      #endif
      
      epochFormatComboBox->SetValue(epochFormat.c_str());
      mFromEpochFormat = epochFormat;
      
      mEpochStr = theSpacecraft->GetStringParameter(wxT("Epoch")); 
      epochValue->SetValue(mEpochStr.c_str());
      
      // Save to TAIModJulian string to avoid keep reading the field
      // and convert to proper format when ComboBox is changed.
      if (epochFormat == wxT("TAIModJulian"))
      {
         mTaiMjdStr = mEpochStr;
      }
      else
      {
         Real fromMjd = -999.999;
         Real outMjd;
         wxString outStr;
         TimeConverterUtil::Convert(mFromEpochFormat, fromMjd, mEpochStr,
                                    wxT("TAIModJulian"), outMjd, outStr);
         mTaiMjdStr = outStr;
         
         #ifdef DEBUG_ORBIT_PANEL_LOAD
         MessageInterface::ShowMessage(wxT("   mTaiMjdStr=%s\n"), mTaiMjdStr.c_str());
         #endif
      }
      
      // load the coordinate system
      wxString coordSystemStr =
         theSpacecraft->GetRefObjectName(Gmat::COORDINATE_SYSTEM);
      
      mCoordSysComboBox->SetValue(coordSystemStr.c_str());
      mFromCoordSysStr = coordSystemStr;
      
      mOutCoord = (CoordinateSystem*)theSpacecraft->GetRefObject(Gmat::COORDINATE_SYSTEM, wxT(""));
      #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   coordSystemStr=%s, mOutCoord=%p\n"), coordSystemStr.c_str(), mOutCoord);
      #endif
      
      if (mOutCoord == NULL)
      {
         mOutCoord = (CoordinateSystem*)theGuiInterpreter->
            GetConfiguredObject(coordSystemStr);
         
         #ifdef DEBUG_ORBIT_PANEL_LOAD
         MessageInterface::ShowMessage
            (wxT("   Get configured %s %p\n"), coordSystemStr.c_str(), mOutCoord);
         #endif
         
      }
      
      mFromCoord = mOutCoord;
      
      if (mOutCoord == NULL)
      {
         MessageInterface::ShowMessage
            (wxT("   The Spacecraft CoordinateSystem is NULL.\n"));
      }
      else
      {
         #ifdef DEBUG_ORBIT_PANEL_LOAD
            MessageInterface::ShowMessage(wxT("   mOutCoord=%s, addr=%d\n"),
               mOutCoord->GetName().c_str(), mOutCoord);
         #endif
         
         // Set the CS's on the spacecraft
         theSpacecraft->SetInternalCoordSystem(mInternalCoord);
         theSpacecraft->SetRefObjectName(Gmat::COORDINATE_SYSTEM, mOutCoord->GetName());
         theSpacecraft->SetRefObject(mOutCoord, Gmat::COORDINATE_SYSTEM);
      }
      
      // get the origin for the output coordinate system
      wxString originName = mOutCoord->GetStringParameter(wxT("Origin"));
      SpacePoint *origin = (SpacePoint*)theGuiInterpreter->GetConfiguredObject(originName);
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
         MessageInterface::ShowMessage
            (wxT("   origin=%s, addr=%d\n"), originName.c_str(), origin);
      #endif
         
      mFromStateTypeStr = wxT("Cartesian");
      if (origin->IsOfType(Gmat::CELESTIAL_BODY))
         mFromStateTypeStr = theSpacecraft->GetStringParameter(wxT("DisplayStateType"));
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
         MessageInterface::ShowMessage
            (wxT("   mFromStateTypeStr=%s\n"), mFromStateTypeStr.c_str());
      #endif
         
      BuildValidStateTypes();
      
      Integer typeCount = 0;
      
      // load the anomaly type - if state type is Keplerian or Modified Keplerian
      mAnomalyType = theSpacecraft->GetStringParameter(wxT("AnomalyType"));
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage(wxT("   mAnomalyType (from SC) = %s \n"), mAnomalyType.c_str());
      #endif
      
      // Get anomaly type list from the base code (Anomaly)
      const wxString *anomalyTypeList = Anomaly::GetLongTypeNameList();
      typeCount = Anomaly::AnomalyTypeCount;
      for (int i = 0; i<typeCount; i++)
      {
         mAnomalyTypeNames.push_back(anomalyTypeList[i]);
         anomalyComboBox->Append(wxString(anomalyTypeList[i].c_str()));
      }
      
      if ( (mFromStateTypeStr == mStateTypeNames[StateConverter::KEPLERIAN]) || 
           (mFromStateTypeStr == mStateTypeNames[StateConverter::MOD_KEPLERIAN]) )
         //if (mFromStateTypeStr == "Keplerian" || mFromStateTypeStr == "ModKeplerian")
      {
         anomalyComboBox->SetValue(mAnomalyType.c_str());
         mFromAnomalyTypeStr = mAnomalyType;
      }
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   mAnomaly=\n   [%s]\n"), mAnomaly.ToString(16).c_str());
      #endif
      
      // Get Spacecraft initial state
      mCartState.Set(theSpacecraft->GetState().GetState());
      mTempCartState = mCartState;
      mOutState = mCartState;
      
      #ifdef DEBUG_ORBIT_PANEL_LOAD
      MessageInterface::ShowMessage
         (wxT("   mCartState=\n   [%s]\n"), mCartState.ToString(16).c_str());
      #endif
      
      // if state type is Cartesian, compute true anomaly - ****
//      if (mFromStateTypeStr == "Cartesian")
//      {
//         Rvector6 st = stateConverter.FromCartesian(mCartState, "Keplerian", "TA");
//         mTrueAnomaly.Set(st[0], st[1], st[5], Anomaly::TA);
//         mAnomaly = mTrueAnomaly;
//         mAnomalyType = mAnomalyTypeNames[Anomaly::TA];
//         mFromAnomalyTypeStr = mAnomalyType;
//
//         #ifdef DEBUG_ORBIT_PANEL_LOAD
//         MessageInterface::ShowMessage
//            ("   Computed TrueAnomaly =\n   [%s]\n", mTrueAnomaly.ToString(16).c_str());
//         #endif
//      }
      
      DisplayState();
      
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage
         (wxT("OrbitPanel:LoadData() error occurred!\n%s\n"), e.GetFullMessage().c_str());
   }
   
   #ifdef DEBUG_ORBIT_PANEL_LOAD
   MessageInterface::ShowMessage(wxT("OrbitPanel::LoadData() leaving\n"));
   #endif
   
}


//------------------------------------------------------------------------------
// void SaveData()
//------------------------------------------------------------------------------
/**
 * @note Gets the values from the text fields and puts them in theSpacecraft
 */
//------------------------------------------------------------------------------
void OrbitPanel::SaveData()
{
   #ifdef DEBUG_ORBIT_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::SaveData() entered\n   mCartState=%s\n   ")
       wxT("mTempCartState=%s\n   mOutState=%s\n"), mCartState.ToString(16).c_str(), 
       mTempCartState.ToString(16).c_str(), mOutState.ToString(16).c_str());
   MessageInterface::ShowMessage(wxT("   mIsCoordSysChanged=%d\n"), mIsCoordSysChanged);
   MessageInterface::ShowMessage(wxT("   mIsStateTypeChanged=%d\n"), mIsStateTypeChanged);
   MessageInterface::ShowMessage(wxT("   mIsAnomalyTypeChanged=%d\n"), mIsAnomalyTypeChanged);
   MessageInterface::ShowMessage(wxT("   mIsStateChanged=%d\n"), mIsStateChanged);
   MessageInterface::ShowMessage(wxT("   mIsEpochChanged=%d\n"), mIsEpochChanged);
   #endif
   
   canClose = true;
   
   //-----------------------------------------------------------
   // check and save epoch
   //-----------------------------------------------------------
   if (mIsEpochChanged)
   {
      wxString newEpoch = epochValue->GetValue().c_str();
      wxString epochFormat = epochFormatComboBox->GetValue().c_str();
      Real fromMjd = -999.999;
//      Real taimjd = -999.999;
      Real a1mjd = -999.999;
      wxString outStr;
      
      #ifdef DEBUG_ORBIT_PANEL_SAVE
      MessageInterface::ShowMessage
         (wxT("   newEpoch=%s, epochFormat=%s\n"), newEpoch.c_str(), epochFormat.c_str());
      #endif
      
      if (mIsEpochChanged)
      {
         try
         {
            bool timeOK = theScPanel->CheckTimeFormatAndValue(epochFormat, newEpoch,
                  wxT("Epoch"), true);
//            TimeConverterUtil::Convert(epochFormat, fromMjd, newEpoch,
//                                       "TAIModJulian", taimjd, outStr);
            // time sent to the spacecraft should be in A1   WCS 2010.05.22
            if (timeOK)
            {
               TimeConverterUtil::Convert(epochFormat, fromMjd, newEpoch,
                                          wxT("A1ModJulian"), a1mjd, outStr);

   //            theSpacecraft->SetEpoch(epochFormat, newEpoch, taimjd);
               theSpacecraft->SetEpoch(epochFormat, newEpoch, a1mjd);
               mEpochStr = outStr;
   //            mEpoch = taimjd;
               mEpoch = a1mjd;
               mIsEpochChanged = false;
            }
         }
         catch (BaseException &e)
         {
            MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
            canClose = false;
         }
      }
   }
   
   
   wxString stateTypeStr = stateTypeComboBox->GetValue().c_str();
   
   try
   {
      //-----------------------------------------------------------
      // check and save orbital elements
      //-----------------------------------------------------------
      if (mIsStateChanged)
      {
         Rvector6 state;
         bool retval = CheckState(state);
         canClose = retval;
         
         if (retval)
         {
            // Build state uses new state type
            BuildState(state);
            theSpacecraft->SetState(stateTypeStr, mCartState);
            ComputeTrueAnomaly(mCartState, stateTypeStr);
            theSpacecraft->SetAnomaly(mAnomalyType, mTrueAnomaly);
            
            // Since BuildState() recomputes internal state and compute back to
            // current state type, numbers will be different, so re display
            // mOutState.
            for (int i=0; i<6; i++)
               textCtrl[i]->SetValue(ToString(mOutState[i]));
            
            ResetStateFlags(true);
            
            mIsStateChanged = false;
            mIsStateTypeChanged = false;
            mIsAnomalyTypeChanged = false;
         }
      }
      
      //--------------------------------------------------------
      // save state type if changed
      //--------------------------------------------------------
      if (mIsStateTypeChanged || mIsAnomalyTypeChanged)
      {
         theSpacecraft->SetState(stateTypeStr, mCartState);
         theSpacecraft->SetAnomaly(mAnomalyType, mTrueAnomaly);
         
         mIsStateTypeChanged = false;
         mIsAnomalyTypeChanged = false;        
      }
      
      //--------------------------------------------------------
      // save coordinate system name if changed
      //--------------------------------------------------------
      if (mIsCoordSysChanged)
      {
         wxString coordSysStr  = mCoordSysComboBox->GetValue();
         mIsCoordSysChanged = false;
         theSpacecraft->
            SetRefObjectName(Gmat::COORDINATE_SYSTEM, coordSysStr.c_str());
      }
      
      if (canClose)
         dataChanged = false;
   }
   catch (BaseException &e)
   {
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      canClose = false;
      return;
   }
      
   #ifdef DEBUG_ORBIT_PANEL_SAVE
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::SaveData() exiting\n   mCartState=%s\n")
       wxT("   mTempCartState=%s\n   mOutState=%s\n"), mCartState.ToString(16).c_str(),
       mTempCartState.ToString(16).c_str(), mOutState.ToString(16).c_str());
   #endif
}


//-------------------------------
// private methods
//-------------------------------

//------------------------------------------------------------------------------
// void Create()
//------------------------------------------------------------------------------
/**
 *
 * @note Creates the page for orbit information
 */
//------------------------------------------------------------------------------
void OrbitPanel::Create()
{
   #ifdef DEBUG_ORBIT_PANEL
      MessageInterface::ShowMessage(wxT("In OrbitPanel::Create() \n"));
   #endif

   Integer bsize = 2; // border size

   //causing VC++ error => wxString emptyList[] = {};
   wxArrayString emptyList;
   
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Spacecraft Orbit"));

   //-----------------------------------------------------------------
   //  create sizers 
   //-----------------------------------------------------------------
   // sizer for orbit tab
   wxBoxSizer *orbitSizer = new wxBoxSizer(wxHORIZONTAL);
   
   //flex grid sizer for the epoch format, coordinate system and state type
   //wxFlexGridSizer *epochSizer = new wxFlexGridSizer(5, 2, bsize, bsize);
   wxFlexGridSizer *epochSizer = new wxFlexGridSizer(2);
   
   // static box for the orbit def and elements
   wxStaticBoxSizer *orbitDefSizer =
      new wxStaticBoxSizer(wxVERTICAL, this, wxT(""));
   wxStaticBoxSizer *elementSizer =
      new wxStaticBoxSizer(wxVERTICAL, this, wxT("Elements"));
   
   //-----------------------------------------------------------------
   // epoch 
   //-----------------------------------------------------------------
   // label for epoch format
   wxStaticText *epochFormatStaticText = new wxStaticText( this, ID_TEXT,
      wxT("Epoch ") wxT(GUI_ACCEL_KEY) wxT("Format"), wxDefaultPosition, wxDefaultSize, 0 );
   
   // combo box for the epoch format
   epochFormatComboBox = new wxComboBox
      ( this, ID_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(150,-1), //0,
        emptyList, wxCB_DROPDOWN | wxCB_READONLY );
   epochFormatComboBox->SetToolTip(pConfig->Read(wxT("EpochFormatHint")));
   
   // label for epoch
   wxStaticText *epochStaticText = new wxStaticText( this, ID_TEXT,
      wxT(GUI_ACCEL_KEY) wxT("Epoch"), wxDefaultPosition, wxDefaultSize, 0 );
   
   // textfield for the epoch value
   epochValue = new wxTextCtrl( this, ID_TEXTCTRL, wxT(""),
      wxDefaultPosition, wxSize(150,-1), 0 );
   epochValue->SetToolTip(pConfig->Read(wxT("EpochHint")));

   //-----------------------------------------------------------------
   //  coordinate system 
   //-----------------------------------------------------------------
   // label for coordinate system
   wxStaticText *coordSysStaticText = new wxStaticText( this, ID_TEXT,
      wxT(GUI_ACCEL_KEY) wxT("Coordinate System"), wxDefaultPosition, wxDefaultSize, 0 );

   //Get CordinateSystem ComboBox from the GuiItemManager.
   mCoordSysComboBox =
      theGuiManager->GetCoordSysComboBox(this, ID_COMBOBOX, wxSize(150,-1));
   mCoordSysComboBox->SetToolTip(pConfig->Read(wxT("CoordinateSystemHint")));
   // get the names of the coordinate systems
   wxArrayString csNames = mCoordSysComboBox->GetStrings();
   for (Integer ii = 0; ii < (Integer) csNames.GetCount(); ii++)
      coordSystemNames.push_back((csNames.Item(ii)).c_str());

   //-----------------------------------------------------------------
   //  state type 
   //-----------------------------------------------------------------
   // label for state type
   wxStaticText *stateTypeStaticText = new wxStaticText( this, ID_TEXT,
      wxT(GUI_ACCEL_KEY) wxT("State Type"), wxDefaultPosition, wxDefaultSize, 0 );

   // combo box for the state
   stateTypeComboBox = new wxComboBox
      ( this, ID_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(150,-1),
        emptyList, wxCB_DROPDOWN | wxCB_READONLY);
   stateTypeComboBox->SetToolTip(pConfig->Read(wxT("StateTypeHint")));
   
   //-----------------------------------------------------------------
   //  anomaly 
   //-----------------------------------------------------------------
   // label for anomaly type
   anomalyStaticText = new wxStaticText( this, ID_TEXT,
      wxT(GUI_ACCEL_KEY) wxT("Anomaly Type "), wxDefaultPosition, wxDefaultSize, 0 );

   // combo box for the anomaly type
   anomalyComboBox = new wxComboBox
      ( this, ID_COMBOBOX, wxT(""), wxDefaultPosition, wxSize(150,-1),
        emptyList, wxCB_DROPDOWN | wxCB_READONLY );
   anomalyComboBox->SetToolTip(pConfig->Read(wxT("AnomalyHint")));
   
   // add to epoch sizer
   epochSizer->AddGrowableCol( 1 );
   epochSizer->Add( epochFormatStaticText, 0, wxALIGN_LEFT | wxALL, bsize );
   epochSizer->Add( epochFormatComboBox, 0, wxGROW|wxALIGN_LEFT | wxALL, bsize );
   
   epochSizer->Add( epochStaticText, 0, wxALIGN_LEFT | wxALL, bsize );
   epochSizer->Add( epochValue, 0, wxGROW|wxALIGN_LEFT | wxALL, bsize );
   
   epochSizer->Add( coordSysStaticText, 0, wxALIGN_LEFT | wxALL, bsize );
   epochSizer->Add( mCoordSysComboBox, 0, wxGROW|wxALIGN_LEFT | wxALL, bsize );
   
   epochSizer->Add( stateTypeStaticText, 0, wxALIGN_LEFT | wxALL, bsize );
   epochSizer->Add( stateTypeComboBox, 0, wxGROW|wxALIGN_LEFT | wxALL, bsize );
   
   epochSizer->Add( anomalyStaticText, 0, wxALIGN_LEFT | wxALL, bsize );
   epochSizer->Add( anomalyComboBox, 0, wxGROW|wxALIGN_LEFT | wxALL, bsize );
   
   // wcs 2010.01.27 remove the Anomaly combo box for now, per S. Hughes
   anomalyStaticText->Show(false);
   anomalyComboBox->Show(false);

   // panel that has the labels and text fields for the elements
   // adds default descriptors/labels
   AddElements(this);
   orbitDefSizer->Add( epochSizer, 1, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   elementSizer->Add( elementsPanel, 1, wxGROW|wxALIGN_CENTER|wxALL, bsize);

   //add orbit designer button
   orbitDesignerButton = new wxButton
      (this, ID_BUTTON, wxT("Orbit Designer"), wxDefaultPosition, wxDefaultSize, 0);
   elementSizer->Add(orbitDesignerButton, 0, wxALIGN_RIGHT|wxALIGN_BOTTOM, bsize );
   
   orbitSizer->Add( orbitDefSizer, 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   orbitSizer->Add( elementSizer, 1, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   
   this->SetSizer( orbitSizer );
}

//------------------------------------------------------------------------------
// void AddElements(wxWindow *parent)
//------------------------------------------------------------------------------
/**
 * @param <parent> input parent.
 *
 * @note Creates the default objects to put in the element static box
 */
//------------------------------------------------------------------------------
void OrbitPanel::AddElements(wxWindow *parent)
{
   Integer bsize = 2; // border size

   elementsPanel = new wxPanel(parent);
   wxGridSizer *item0 = new wxGridSizer( 1, 0, 0 );
   wxFlexGridSizer *item3 = new wxFlexGridSizer( 6, 3, 0, 0 );
   // Let's make growable column, so we can see more numbers when expand
   // Commented out since it doesn't look good on Linux(LOJ: 2010.02.19)
   //item3->AddGrowableCol( 0 );
   //item3->AddGrowableCol( 1 );
   //item3->AddGrowableCol( 2 );
   
   // Element 1
   description1 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[0] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, 
                     wxT(""), wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit1 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit1"), 
               wxDefaultPosition, wxDefaultSize, 0 );
   
   // Element 2
   description2 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[1] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, wxT(""), 
                     wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit2 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit2"), 
               wxDefaultPosition, wxDefaultSize, 0 );
   
   // Element 3
   description3 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[2] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, wxT(""), 
                   wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit3 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit3"), 
                wxDefaultPosition, wxDefaultSize, 0 );
   
   // Element 4
   description4 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[3] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, wxT(""), 
                   wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit4 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit4"), 
                wxDefaultPosition, wxDefaultSize, 0 );
   
   // Element 5    
   description5 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[4] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, wxT(""), 
                                 wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit5 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit5"), 
                wxDefaultPosition, wxDefaultSize, 0 );
   
   // Element 6
   description6 = new wxStaticText( elementsPanel, ID_TEXT, 
                      wxT(""), wxDefaultPosition, wxSize(75,-1), 0 );
   textCtrl[5] = new wxTextCtrl( elementsPanel, ID_TEXTCTRL, wxT(""),
                                wxDefaultPosition, wxSize(150,-1), 0, wxTextValidator(wxGMAT_FILTER_NUMERIC) );
   unit6 = new wxStaticText( elementsPanel, ID_TEXT, wxT("Unit6"), 
                wxDefaultPosition, wxDefaultSize, 0 );
   
   // Add to wx*Sizers 
   item3->Add( description1, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[0], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit1, 0, wxALIGN_LEFT|wxALL, bsize );
   
   item3->Add( description2, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[1], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit2, 0, wxALIGN_LEFT|wxALL, bsize );
   
   item3->Add( description3, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[2], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit3, 0, wxALIGN_LEFT|wxALL, bsize );
   
   item3->Add( description4, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[3], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit4, 0, wxALIGN_LEFT|wxALL, bsize );
   
   item3->Add( description5, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[4], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit5, 0, wxALIGN_LEFT|wxALL, bsize );
   
   item3->Add( description6, 0, wxALIGN_LEFT|wxALL, bsize );
   item3->Add( textCtrl[5], 0, wxGROW|wxALIGN_CENTER|wxALL, bsize );
   item3->Add( unit6, 0, wxALIGN_LEFT|wxALL, bsize );

   item0->Add( item3, 0, wxGROW|wxALL|wxALIGN_CENTER, bsize );

   elementsPanel->SetAutoLayout( TRUE );
   elementsPanel->SetSizer( item0 );

   item0->Fit( elementsPanel );
   item0->SetSizeHints( elementsPanel );
}


//------------------------------------------------------------------------------
// void OnComboBoxChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/*
 * Converts state to ComboBox selection using utility. It does not call
 * Spacecraft object for conversion.
 */
//------------------------------------------------------------------------------ 
void OrbitPanel::OnComboBoxChange(wxCommandEvent& event)
{
   wxString coordSysStr  = mCoordSysComboBox->GetValue().c_str();
   wxString stateTypeStr = stateTypeComboBox->GetValue().c_str();
   
   #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::OnComboBoxChange() coordSysStr=%s, stateTypeStr=%s\n"),
       coordSysStr.c_str(), stateTypeStr.c_str());
   #endif
   
   //-----------------------------------------------------------------
   // epoch format change
   //-----------------------------------------------------------------
   if (event.GetEventObject() == epochFormatComboBox)
   {
      mIsEpochChanged = true;
      wxString toEpochFormat = epochFormatComboBox->GetValue().c_str();    
      
      #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
      MessageInterface::ShowMessage
         (wxT("   OnComboBoxChange() mFromEpochFormat=%s, ")
          wxT("toEpochFormat=%s\n   mEpochStr=%s, mIsEpochModified=%d\n"),
          mFromEpochFormat.c_str(), toEpochFormat.c_str(), mEpochStr.c_str(),
          mIsEpochModified);
      #endif
      
      try
      {
         Real fromMjd = -999.999;
         Real outMjd;
         wxString outStr;
         
         // if modified by user, check if epoch is valid first
         if (mIsEpochModified)
         {
            mEpochStr = epochValue->GetValue().c_str();
            
            // Save to TAIModJulian string to avoid keep reading the field
            // and convert to proper format when ComboBox is changed.
            if (mFromEpochFormat == wxT("TAIModJulian"))
            {
               mTaiMjdStr = mEpochStr;
            }
            else
            {
               TimeConverterUtil::Convert(mFromEpochFormat, fromMjd, mEpochStr,
                                          wxT("TAIModJulian"), outMjd, outStr);
               mTaiMjdStr = outStr;
            }
            
            // Convert to desired format with new date
            TimeConverterUtil::Convert(mFromEpochFormat, fromMjd, mEpochStr,
                                       toEpochFormat, outMjd, outStr);
            
            epochValue->SetValue(outStr.c_str());
            mIsEpochModified = false;
            mFromEpochFormat = toEpochFormat;
         }
         else
         {
            //theSpacecraft->SetDateFormat(toEpochFormat);
            //epochValue->SetValue(theSpacecraft->GetStringParameter("Epoch").c_str());
            
            // Convert to desired format using TAIModJulian date
            TimeConverterUtil::Convert(wxT("TAIModJulian"), fromMjd, mTaiMjdStr,
                                       toEpochFormat, outMjd, outStr);
            
            epochValue->SetValue(outStr.c_str());
            mFromEpochFormat = toEpochFormat;
         }
      }
      catch (BaseException &e)
      {
         epochFormatComboBox->SetValue(mFromEpochFormat.c_str());
         theSpacecraft->SetDateFormat(mFromEpochFormat);
         MessageInterface::PopupMessage
            (Gmat::ERROR_, e.GetFullMessage() +
             wxT("\nPlease enter valid Epoch before changing the Epoch Format\n"));
      }
   }
   //-----------------------------------------------------------------
   // coordinate system or state type change
   //-----------------------------------------------------------------
   else if (event.GetEventObject() == mCoordSysComboBox ||
            event.GetEventObject() == stateTypeComboBox)
   {      
      #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
      MessageInterface::ShowMessage
         (wxT("   OnComboBoxChange() mFromCoordSysStr=%s, mFromStateTypeStr=%s\n"),
          mFromCoordSysStr.c_str(), mFromStateTypeStr.c_str());
      MessageInterface::ShowMessage(wxT("------ epochChanged = %s, epochModified = %s, stateModified = %s\n"),
           (mIsEpochChanged? wxT("true") : wxT("false")), (mIsEpochModified? wxT("true") : wxT("false")),
           (IsStateModified()? wxT("true") : wxT("false")));
      #endif
      
      if (event.GetEventObject() == mCoordSysComboBox)
         mIsCoordSysChanged = true;
      
      if (event.GetEventObject() == stateTypeComboBox)
      {
         Rvector6 state;
         mIsStateTypeChanged = true;
         bool retval = false;
         
         // If state modified by user, validate elements first
         if (IsStateModified())
         {
            retval = CheckState(state);
            
            if (!retval)
            {
               stateTypeComboBox->SetValue(mFromStateTypeStr.c_str());
               MessageInterface::PopupMessage
                  (Gmat::ERROR_, +
                   wxT("Please enter valid state before changing the State Type\n"));  // *****
               return;
            }
         }
         // make sure to compute the true anomaly if the state is converted form Cartesian
         if (mFromStateTypeStr == wxT("Cartesian"))
         {
            try
            {
               Rvector6 st = stateConverter.FromCartesian(mCartState, wxT("Keplerian"), wxT("TA"));
               mTrueAnomaly.Set(st[0], st[1], st[5], Anomaly::TA);
               mAnomaly = mTrueAnomaly;
               mAnomalyType = mAnomalyTypeNames[Anomaly::TA];
               mFromAnomalyTypeStr = mAnomalyType;

               #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
               MessageInterface::ShowMessage
                  (wxT("   Computed TrueAnomaly =\n   [%s]\n"), mTrueAnomaly.ToString(16).c_str());
               #endif
            }
            catch (BaseException&)
            {
               #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
               MessageInterface::ShowMessage
                  (wxT("ERROR computing True Anomaly - message: %s\n"), (be.GetFullMessage()).c_str());
               #endif
               //throw; // GMAT crashes if I put this in here ... ???
            }
         }


         BuildValidCoordinateSystemList(stateTypeStr);
      }
      
      CoordinateSystem *prevCoord = mOutCoord;
      
      mOutCoord = (CoordinateSystem*)theGuiInterpreter->
         GetConfiguredObject(mCoordSysComboBox->GetValue().c_str());
      
      BuildValidStateTypes();
      
      try
      {
         DisplayState();
         
         if (event.GetEventObject() == mCoordSysComboBox)
            mFromCoordSysStr = mCoordSysComboBox->GetValue().c_str();
         
         if (event.GetEventObject() == stateTypeComboBox)
            mFromStateTypeStr = stateTypeComboBox->GetValue().c_str();
         
         mFromCoord = mOutCoord;
         theSpacecraft->SetRefObjectName(Gmat::COORDINATE_SYSTEM, mOutCoord->GetName());
         theSpacecraft->SetRefObject(mOutCoord, Gmat::COORDINATE_SYSTEM);
      }
      catch (BaseException &)
      {
         mCoordSysComboBox->SetValue(mFromCoordSysStr.c_str());
         stateTypeComboBox->SetValue(mFromStateTypeStr.c_str());
         mOutCoord = prevCoord;
         BuildValidStateTypes();
         return;
      }
   }
   //-----------------------------------------------------------------
   // anomaly type change 
   //-----------------------------------------------------------------
   else if (event.GetEventObject() == anomalyComboBox)
   {
      mIsAnomalyTypeChanged = true;
      wxString description, stateValue;      
      mAnomalyType = anomalyComboBox->GetValue();
      
      #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
      MessageInterface::ShowMessage
         (wxT("   OnComboBoxChange() mFromAnomalyTypeStr=%s, ")
          wxT("mAnomalyType=%s\n"), mFromAnomalyTypeStr.c_str(),
          mAnomalyType.c_str());
      #endif
      
      Rvector6 state;
      bool retval = false;
      
      // If state modified by user, validate elements first
      if (IsStateModified())
      {
         retval = CheckState(state);
         
         if (!retval)
         {
            anomalyComboBox->SetValue(mFromAnomalyTypeStr.c_str());
            MessageInterface::PopupMessage
               (Gmat::ERROR_, +
                wxT("Please enter valid value before changing the Anomaly Type\n"));
            return;
         }
      }
      
      try
      {
         DisplayState();
         mFromAnomalyTypeStr = mAnomalyType;
      }
      catch (BaseException &)
      {
         anomalyComboBox->SetValue(mFromAnomalyTypeStr.c_str());
         return;
      }
   }
   
   dataChanged = true;
   theScPanel->EnableUpdate(true);
   
   #ifdef DEBUG_ORBIT_PANEL_COMBOBOX
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::OnComboBoxChange() leaving\n"));
   #endif
}


//------------------------------------------------------------------------------
// void OnTextChange(wxCommandEvent& event)
//------------------------------------------------------------------------------
/**
 * @note Activates the Apply button when text is changed
 */
//------------------------------------------------------------------------------
void OrbitPanel::OnTextChange(wxCommandEvent& event)
{
   wxObject *obj = event.GetEventObject();
   bool isStateText = false;
   
   for (int i=0; i<6; i++)
      if (obj == textCtrl[i])
         isStateText = true;
   
   if (isStateText)
   {
      for (int i=0; i<6; i++)
         if (textCtrl[i]->IsModified())
            mIsStateModified[i] = true;

      if (IsStateModified())
      {
         #ifdef DEBUG_ORBIT_PANEL_TEXT_CHANGE
            MessageInterface::ShowMessage(wxT("--- The state has been modified\n"));
         #endif
         mIsStateChanged = true;
         dataChanged = true;
         theScPanel->EnableUpdate(true);
      }
   }
   else if (obj == epochValue && epochValue->IsModified())
   {
      #ifdef DEBUG_ORBIT_PANEL_TEXT_CHANGE
         MessageInterface::ShowMessage(wxT("--- The epoch has been changed to: %s\n"),
               (epochValue->GetValue()).c_str());
      #endif
      mIsEpochChanged = true;
      mIsEpochModified = true;
      dataChanged = true;
      theScPanel->EnableUpdate(true);
   }
}


//------------------------------------------------------------------------------
// void OrbitPanel::SetLabelsUnits(const wxString &stateType)
//------------------------------------------------------------------------------
/**
 * @param <stateType> input state type for display.
 *
 * @note Sets the labels and units for the state.
 */
//------------------------------------------------------------------------------
void OrbitPanel::SetLabelsUnits(const wxString &stateType)
{
   // get the config object
   wxConfigBase *pConfig = wxConfigBase::Get();
   // SetPath() understands ".."
   pConfig->SetPath(wxT("/Spacecraft Orbit"));

   Integer baseLabel = theSpacecraft->GetParameterID(wxT("Element1"));
   Integer baseUnit  = theSpacecraft->GetParameterID(wxT("Element1Units"));
   
   wxString st = theSpacecraft->GetStringParameter(wxT("DisplayStateType"));
   theSpacecraft->SetStringParameter(wxT("DisplayStateType"), stateType);
   
   description1->SetLabel(theSpacecraft->GetParameterText(baseLabel).c_str());
   textCtrl[0]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel)+wxT("Hint")).c_str()));
   unit1->SetLabel(theSpacecraft->GetStringParameter(baseUnit).c_str());
  
   description2->SetLabel(theSpacecraft->GetParameterText(baseLabel+1).c_str());
   textCtrl[1]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel+1)+wxT("Hint")).c_str()));
   unit2->SetLabel(theSpacecraft->GetStringParameter(baseUnit+1).c_str());
   
   description3->SetLabel(theSpacecraft->GetParameterText(baseLabel+2).c_str());
   textCtrl[2]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel+2)+wxT("Hint")).c_str()));
   unit3->SetLabel(theSpacecraft->GetStringParameter(baseUnit+2).c_str());
   
   description4->SetLabel(theSpacecraft->GetParameterText(baseLabel+3).c_str());
   textCtrl[3]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel+3)+wxT("Hint")).c_str()));
   unit4->SetLabel(theSpacecraft->GetStringParameter(baseUnit+3).c_str());
   
   description5->SetLabel(theSpacecraft->GetParameterText(baseLabel+4).c_str());
   textCtrl[4]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel+4)+wxT("Hint")).c_str()));
   unit5->SetLabel(theSpacecraft->GetStringParameter(baseUnit+4).c_str());
   
   description6->SetLabel(theSpacecraft->GetParameterText(baseLabel+5).c_str());
   textCtrl[5]->SetToolTip(pConfig->Read((wxT("Elements")+theSpacecraft->GetParameterText(baseLabel+5)+wxT("Hint")).c_str()));
   unit6->SetLabel(theSpacecraft->GetStringParameter(baseUnit+5).c_str());
   
   if ( (stateType == mStateTypeNames[StateConverter::KEPLERIAN]) || 
        (stateType == mStateTypeNames[StateConverter::MOD_KEPLERIAN]) )
   {
      wxString label = Anomaly::GetTypeString(mAnomalyType).c_str();
      description6->SetLabel(label);
//      anomalyStaticText->Show(true); // commented out for now - wcs - 2010.01.27 - per S. Hughes
//      anomalyComboBox->Show(true);
      anomalyComboBox->SetSelection(Anomaly::GetAnomalyType(mAnomalyType));
   }
   else
   {
      anomalyStaticText->Show(false);
      anomalyComboBox->Show(false);
   }
   
   theSpacecraft->SetStringParameter(wxT("DisplayStateType"), st);
}

//------------------------------------------------------------------------------
// void InitializeCoordinateSystem()
//------------------------------------------------------------------------------
/**
 * @note Activates the Apply button when epoch combobox is changed
 */
//------------------------------------------------------------------------------
void OrbitPanel::InitializeCoordinateSystem(CoordinateSystem *cs)
{
   cs->SetSolarSystem(theSolarSystem); //Assume you have the SolarSystem pointer
   
   SpacePoint *sp;
   wxString spName;

   // Set the Origin for the coordinate system
   spName = cs->GetStringParameter(wxT("Origin"));
   sp = (SpacePoint*) theGuiInterpreter->GetConfiguredObject(spName);
   
   if (sp == NULL)
      throw GmatBaseException(wxT("Cannot find SpacePoint named \"") +
                              spName + wxT("\" used for the coordinate system ") +
                              cs->GetName() + wxT(" origin"));
   
   cs->SetRefObject(sp, Gmat::SPACE_POINT, spName);

   // Set the J2000Body for the coordinate system
   spName = cs->GetStringParameter(wxT("J2000Body"));
   sp = (SpacePoint*) theGuiInterpreter->GetConfiguredObject(spName);
   
   if (sp == NULL)
      throw GmatBaseException(wxT("Cannot find SpacePoint named \"") +
         spName + wxT("\" used for the coordinate system ") +
         cs->GetName() + wxT(" J2000 body"));
   
   cs->SetRefObject(sp, Gmat::SPACE_POINT, spName);
   
   cs->Initialize();
}


//------------------------------------------------------------------------------
// void DisplayState()
//------------------------------------------------------------------------------
void OrbitPanel::DisplayState()
{
   wxString coordSysStr  = mCoordSysComboBox->GetValue().c_str();
   wxString stateTypeStr = stateTypeComboBox->GetValue().c_str();
   
   #ifdef DEBUG_ORBIT_PANEL
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::DisplayState() coordSysStr = '%s', stateTypeStr='%s', ")
       wxT("mEpoch=%.11lf\n"), coordSysStr.c_str(), stateTypeStr.c_str(), mEpoch);
   #endif
   
   Rvector6 midState;
   bool isInternal = false;
   
   if ((IsStateModified()) || mIsEpochChanged)
//   if (IsStateModified())
   {      
      // User has typed in new state data
      for (int i=0; i<6; i++)
      {
         if (mIsStateModified[i])
            (textCtrl[i]->GetValue()).ToDouble(&(midState[i]));
         else
            midState[i] = mOutState[i];
      }
      
      #ifdef DEBUG_ORBIT_PANEL
      MessageInterface::ShowMessage
         (wxT("   state value modified\n   [%s]\n"), midState.ToString(16).c_str());
      #endif
      
      // compute current anomaly if Keplerian or ModKeplerian here
      // BuildState() does not compute mAnomaly
      if ( ((stateTypeStr == mStateTypeNames[StateConverter::KEPLERIAN]) || 
            (stateTypeStr == mStateTypeNames[StateConverter::MOD_KEPLERIAN])) &&
           (mIsStateModified[0] || mIsStateModified[1] || mIsStateModified[5]) )
      {
         #ifdef DEBUG_ORBIT_PANEL_STATE_CHANGE
            MessageInterface::ShowMessage(
                  wxT("In DisplayState, stateType = %s, elements [0,1,5] modified = %s  %s  %s\n"),
                  stateTypeStr.c_str(), (mIsStateModified[0]? wxT("true") : wxT("false")),
                  (mIsStateModified[1]? wxT("true") : wxT("false")), (mIsStateModified[5]? wxT("true") : wxT("false")));
            MessageInterface::ShowMessage(wxT("About to create new Anomaly with anomaly type = %s\n"),
                  mFromAnomalyTypeStr.c_str());
         #endif
//         Anomaly temp(midState[0], midState[1], midState[5], mFromAnomalyTypeStr);
//         mAnomaly = temp;
         mAnomaly.Set(midState[0], midState[1], midState[5], mFromAnomalyTypeStr);
         #ifdef DEBUG_ORBIT_PANEL_STATE_CHANGE
         MessageInterface::ShowMessage(wxT("In DisplayState, about to print out anomaly type ...\n"));
            MessageInterface::ShowMessage(
                  wxT("In DisplayState, after operator = , mAnomalyType = %s\n"),
                  (mAnomaly.GetTypeString()).c_str());
         #endif
      }
   }
   else
   {
      // Load midState with Cartesian spacecraft state in internal coordinates
      midState = mCartState;
      isInternal = true;
      
      #ifdef DEBUG_ORBIT_PANEL
      MessageInterface::ShowMessage
         (wxT("   using interval state\n   [%s]\n"), midState.ToString(16).c_str());
      #endif
   }
   
   //-----------------------------------------------------------
   // check and save epoch
   //-----------------------------------------------------------
   if (mIsEpochChanged)
   {
      wxString newEpoch = epochValue->GetValue().c_str();
      wxString epochFormat = epochFormatComboBox->GetValue().c_str();
      Real fromMjd = -999.999;
      Real a1mjd = -999.999;
      wxString outStr;

      try
      {
         TimeConverterUtil::Convert(epochFormat, fromMjd, newEpoch,
                                    wxT("A1ModJulian"), a1mjd, outStr);

         theSpacecraft->SetEpoch(epochFormat, newEpoch, a1mjd);
         mEpochStr = outStr;
         mEpoch = a1mjd;
         mIsEpochChanged = false;
      }
      catch (BaseException &e)
      {
         MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
         canClose = false;
      }
   }

   BuildState(midState, isInternal);
   
   #ifdef DEBUG_ORBIT_PANEL_CONVERT
   MessageInterface::ShowMessage
      (wxT("   DisplayState()--- after conversion, mOutState=\n   [%s]\n"),
       mOutState.ToString(16).c_str());
   #endif
   
   for (int i=0; i<6; i++)
      textCtrl[i]->SetValue(ToString(mOutState[i]));
   
   // labels for elements, anomaly and units
   SetLabelsUnits(stateTypeStr);
   ResetStateFlags(true);

   #ifdef DEBUG_ORBIT_PANEL_CONVERT
   MessageInterface::ShowMessage
      (wxT("OrbitPanel::DisplayState() leaving\n"));
   #endif
   
}


//------------------------------------------------------------------------------
// void BuildValidStateTypes()
//------------------------------------------------------------------------------
void OrbitPanel::BuildValidStateTypes()
{
   bool rebuild = false;
   if (mStateTypeNames.empty())
       rebuild = true;
   
   // get the origin for the output coordinate system
   wxString originName = mOutCoord->GetStringParameter(wxT("Origin"));
   SpacePoint *origin = (SpacePoint*)theGuiInterpreter->GetConfiguredObject(originName);
   
   #ifdef DEBUG_ORBIT_PANEL_STATE_TYPE
   MessageInterface::ShowMessage
      (wxT("   BuildValidStateTypes() origin=%s, addr=%d, mShowFullStateTypeList=%d\n"),
       originName.c_str(), origin, mShowFullStateTypeList);
   #endif
   
   if (origin->IsOfType(Gmat::CELESTIAL_BODY) && !mShowFullStateTypeList)
      rebuild = true;
   else if (!origin->IsOfType(Gmat::CELESTIAL_BODY) && mShowFullStateTypeList)
      rebuild = true;
   
   if (!rebuild)
      return;
   
   #ifdef DEBUG_ORBIT_PANEL_STATE_TYPE
   MessageInterface::ShowMessage(wxT("   Building new state type list...\n"));
   #endif
   
   Integer typeCount = 0;
   mStateTypeNames.clear();
   stateTypeComboBox->Clear();
   
   // get state type list from the base code (StateConverter)
   const wxString *stateTypeList = stateConverter.GetStateTypeList();
   typeCount = stateConverter.GetTypeCount();
   for (int i = 0; i<typeCount; i++)
      mStateTypeNames.push_back(stateTypeList[i]);
   
   // fill state type combobox
   if (origin->IsOfType(Gmat::CELESTIAL_BODY))
   {         
      for (int i = 0; i<typeCount; i++)
         stateTypeComboBox->Append(wxString(stateTypeList[i].c_str()));
      
      mShowFullStateTypeList = true;
      stateTypeComboBox->SetValue(mFromStateTypeStr.c_str());
   }
   else
   {
      // if origin is other calculated points (LibrationPoint, Barycenter)
      // there is no mu associated with it, so we don't want to
      // show Keplerian or ModKeplerian types
      for (Integer ii = 0; ii < typeCount; ii++)
      {
         if (!(stateConverter.RequiresCelestialBodyOrigin(stateTypeList[ii])))
            stateTypeComboBox->Append(wxString(stateTypeList[ii].c_str()));
      }
//      wxString stateTypeList[] =
//         {
//            wxT("Cartesian"),
//            wxT("SphericalAZFPA"),
//            wxT("SphericalRADEC")
//         };
//
//      for (int i = 0; i<3; i++)
//         stateTypeComboBox->Append(wxString(stateTypeList[i].c_str()));
      
      mShowFullStateTypeList = false;
      
      if ( (mFromStateTypeStr == mStateTypeNames[StateConverter::KEPLERIAN]) || 
           (mFromStateTypeStr == mStateTypeNames[StateConverter::MOD_KEPLERIAN]))
      {
         // default to Cartesian
         stateTypeComboBox->
            SetValue(mStateTypeNames[StateConverter::CARTESIAN].c_str());
      }
      else
      {
         stateTypeComboBox->SetValue(mFromStateTypeStr.c_str());
      }
   }
   
   #ifdef DEBUG_ORBIT_PANEL_STATE_TYPE
   MessageInterface::ShowMessage
      (wxT("   BuildValidStateTypes() Setting state type to %s\n"),
       mFromStateTypeStr.c_str());
   #endif
}

//------------------------------------------------------------------------------
// void BuildValidCoordinateSystemList(const wxString &forStateType)
//------------------------------------------------------------------------------
void OrbitPanel::BuildValidCoordinateSystemList(const wxString &forStateType)
{
   bool reqCBOnly = stateConverter.RequiresCelestialBodyOrigin(forStateType);
   // don't rebuild, if ti's not necessary
   if (((coordSysCBOnly) && reqCBOnly) || ((!coordSysCBOnly) && (!reqCBOnly))) return;

   CoordinateSystem *tmpCS = NULL;
   SpacePoint       *origin = NULL;
   wxString      originName;
   wxString      currentCS = mCoordSysComboBox->GetValue().c_str();
   wxString      newCS     = currentCS;
   mCoordSysComboBox->Clear();
   if (reqCBOnly)
   {
      for (Integer ii = 0; ii < (Integer) coordSystemNames.size(); ii++)
      {
         if (ii == 0)                                   newCS = coordSystemNames.at(ii);
         else if (currentCS == coordSystemNames.at(ii)) newCS = currentCS;
         tmpCS      = (CoordinateSystem*) theGuiInterpreter->GetConfiguredObject(coordSystemNames.at(ii));
         originName = tmpCS->GetStringParameter(wxT("Origin"));
         origin     = (SpacePoint*) theGuiInterpreter->GetConfiguredObject(originName);
         if (origin->IsOfType(wxT("CelestialBody")))  // add it to the list
            mCoordSysComboBox->Append(wxString(coordSystemNames[ii].c_str()));
      }
      mCoordSysComboBox->SetValue(newCS.c_str());
      coordSysCBOnly = true;
   }
   else
   {
      for (Integer ii = 0; ii < (Integer) coordSystemNames.size(); ii++)
            mCoordSysComboBox->Append(wxString(coordSystemNames[ii].c_str()));
      coordSysCBOnly = false;
      mCoordSysComboBox->SetValue(currentCS.c_str());
   }

}


//------------------------------------------------------------------------------
// void BuildState(const Rvector6 &inputState, bool isInternal = false)
//------------------------------------------------------------------------------
/**
 * This method takes the input state and converts it to match the state settings
 * on the GUI panel.
 * 
 * @param <inputState>  The state that gets converted.
 * @param <isInternal>  true if the input state is a Cartesian state in internal 
 *                      coordinates. 
 */
//------------------------------------------------------------------------------ 
void OrbitPanel::BuildState(const Rvector6 &inputState, bool isInternal)
{
   #ifdef DEBUG_ORBIT_PANEL_CONVERT
      Rvector6 tempState = inputState;
      MessageInterface::ShowMessage(
         wxT("   BuildState() --- The input state (%s %s coordinates)")
         wxT(" is\n   [%s]\n"),
         (isInternal ? wxT("Internal") : mFromCoord->GetName().c_str()),
         (isInternal ? wxT("Cartesian") : mFromStateTypeStr.c_str()),
         tempState.ToString(16).c_str());
      MessageInterface::ShowMessage(wxT(" ... and mAnomaly type is %s\n"),
            (mAnomaly.GetTypeString()).c_str());
   #endif
      
   Rvector6 midState;
   
   wxString stateTypeStr = stateTypeComboBox->GetValue().c_str();
   
   #ifdef DEBUG_ORBIT_PANEL_CONVERT
   MessageInterface::ShowMessage
      (wxT("   stateTypeStr=%s, mAnomalyType=%s\n"), stateTypeStr.c_str(),
       mAnomalyType.c_str());
   #endif
   
   try
   {
      if (isInternal)
      {
         #ifdef DEBUG_ORBIT_PANEL_CONVERT
         MessageInterface::ShowMessage
            (wxT("   isInternal is true, setting mCartState to the input state.\n"));
         #endif
         // Input state is Cartesian expressed in internal coordinates
         mCartState = inputState;
      }
      else
      {
         #ifdef DEBUG_ORBIT_PANEL_CONVERT
         MessageInterface::ShowMessage
            (wxT("   mAnomaly = [%s]\n"), mAnomaly.ToString(16).c_str());
         MessageInterface::ShowMessage(wxT("before Coordinate Conversion, mEpoch = %12.10f\n"), mEpoch);
         MessageInterface::ShowMessage(wxT("  mFromCoord = %s, mInternalCoord = %s\n"),
               mFromCoord->GetName().c_str(), mInternalCoord->GetName().c_str());
         #endif
         
         // Convert input state to the Cartesian representation
         stateConverter.SetMu(mFromCoord);
         midState = stateConverter.Convert(inputState, mFromStateTypeStr, 
                                           wxT("Cartesian"), mAnomaly);
         
         // Transform to internal coordinates
         mCoordConverter.Convert(A1Mjd(mEpoch), midState, mFromCoord, mCartState, 
                                 mInternalCoord);
      }
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage(
          wxT("   BuildState() --- The internal CS representation is\n   [%s]\n"),
          mCartState.ToString(16).c_str());
      #endif
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage(wxT("before Coordinate Conversion, mEpoch = %12.10f\n"), mEpoch);
      MessageInterface::ShowMessage(wxT("  mInternalCoord = %s, mOutCoord = %s\n"),
            mInternalCoord->GetName().c_str(), mOutCoord->GetName().c_str());
      #endif
      // Transform to the desired coordinate system
      mCoordConverter.Convert(A1Mjd(mEpoch), mCartState, mInternalCoord, midState, 
                              mOutCoord);
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage
         (wxT("   after mCoordConverter.Convert() midState=\n   [%s]\n"),
          midState.ToString().c_str());
      #endif
      
      // and convert to the desired representation
      stateConverter.SetMu(mOutCoord);
      mOutState = stateConverter.FromCartesian(midState, stateTypeStr, mAnomalyType);
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage
         (wxT("   after FromCartesian() mOutState=\n   [%s]\n"), mOutState.ToString().c_str());
      MessageInterface::ShowMessage(wxT("   OrbitPanel::BuildState() leaving\n"));
      #endif      
   }
   catch (BaseException &e)
   {
      MessageInterface::ShowMessage(wxT("**** ERROR in BuildState()\n"));
      MessageInterface::PopupMessage(Gmat::ERROR_, e.GetFullMessage());
      throw;
   }
   
}


//------------------------------------------------------------------------------
// wxString ToString(Real rval)
//------------------------------------------------------------------------------
wxString OrbitPanel::ToString(Real rval)
{
   return theGuiManager->ToWxString(rval);
}


//------------------------------------------------------------------------------
// bool IsStateModified()
//------------------------------------------------------------------------------
bool OrbitPanel::IsStateModified()
{
   for (int i=0; i<6; i++)
      if (mIsStateModified[i])
         return true;

   return false;
}


//------------------------------------------------------------------------------
// void ResetStateFlags(bool discardEdits = false)
//------------------------------------------------------------------------------
void OrbitPanel::ResetStateFlags(bool discardEdits)
{
   for (int i=0; i<6; i++)
      mIsStateModified[i] = false;

   if (discardEdits)
      for (int i=0; i<6; i++)
         textCtrl[i]->DiscardEdits();
}


//------------------------------------------------------------------------------
// bool CheckState(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckState(Rvector6 &state)
{
   #ifdef DEBUG_ORBIT_PANEL_CHECKSTATE
      MessageInterface::ShowMessage(
            wxT("OrbitPanel::CheckState, state = %12.10f  %12.10f  %12.10f  %12.10f  %12.10f  %12.10f\n"),
            state[0], state[1], state[2], state[3], state[4], state[5]);
      MessageInterface::ShowMessage(wxT("   current state type = %s\n"), mFromStateTypeStr.c_str());
   #endif
   for (int i=0; i<6; i++)
      mElements[i] = textCtrl[i]->GetValue();
   
   wxString stateTypeStr  = stateTypeComboBox->GetValue().c_str();
   bool retval = false;
   
   if (mFromStateTypeStr == wxT("Cartesian"))
      retval = CheckCartesian(state);
   else if (mFromStateTypeStr == wxT("Keplerian"))
      retval = CheckKeplerian(state);
   else if (mFromStateTypeStr == wxT("ModifiedKeplerian"))
      retval = CheckModKeplerian(state);
   else if (mFromStateTypeStr == wxT("SphericalAZFPA") ||
            mFromStateTypeStr == wxT("SphericalRADEC"))
      retval = CheckSpherical(state, stateTypeStr);
   else if (mFromStateTypeStr == wxT("Equinoctial"))
      retval = CheckEquinoctial(state);
   else
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, + wxT("*** Internal Error ***\nUnknown State Type: ") +
          stateTypeStr);
   }
   
   if (!retval)
      return retval;
   
   state = mOutState;
   
   // Copy only modified fields
   for (int i=0; i<6; i++)
   {
      if (mIsStateModified[i])
         mElements[i].ToDouble(&(state[i]));
   }
   #ifdef DEBUG_ORBIT_PANEL_CHECKSTATE
      MessageInterface::ShowMessage(
            wxT("OrbitPanel::CheckState, LEAVING, state = %12.10f  %12.10f  %12.10f  %12.10f  %12.10f  %12.10f\n"),
            state[0], state[1], state[2], state[3], state[4], state[5]);
      MessageInterface::ShowMessage(wxT("   and current state type = %s\n"), mFromStateTypeStr.c_str());
   #endif
   
   return retval;
   
}


//------------------------------------------------------------------------------
// bool CheckCartesian(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckCartesian(Rvector6 &state)
{
   bool retval = true;
   
   if (!theScPanel->CheckReal(state[0], mElements[0], wxT("X"), wxT("Real Number")))
      retval = false;
   if (!theScPanel->CheckReal(state[1], mElements[1], wxT("Y"), wxT("Real Number")))
      retval = false;
   if (!theScPanel->CheckReal(state[2], mElements[2], wxT("Z"), wxT("Real Number")))
      retval = false;
   if (!theScPanel->CheckReal(state[3], mElements[3], wxT("VX"), wxT("Real Number")))
      retval = false;
   if (!theScPanel->CheckReal(state[4], mElements[4], wxT("VY"), wxT("Real Number")))
      retval = false;
   if (!theScPanel->CheckReal(state[5], mElements[5], wxT("VZ"), wxT("Real Number")))
      retval = false;
   
   #ifdef DEBUG_ORBIT_PANEL_CHECK_RANGE
   MessageInterface::ShowMessage(wxT("CheckCartesian() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool CheckKeplerian(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckKeplerian(Rvector6 &state)
{
   mAnomalyType = anomalyComboBox->GetValue().c_str();
   bool retval = true;
   
   if (theScPanel->CheckReal(state[0], mElements[0], wxT("SMA"), wxT("Real Number")))
   {
      if (state[0] == 0.0)
      {
         theScPanel->CheckReal(state[0], mElements[0], wxT("SMA"), wxT("Real Number != 0.0"), true);
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }
   
   if (theScPanel->CheckReal(state[1], mElements[1], wxT("ECC"), wxT("Real Number")))
   {
      if ((state[1] < 0.0) || (state[1] == 1.0))
      {
         theScPanel->CheckReal(state[1], mElements[1], wxT("ECC"),
                               wxT("0.0 <= Real Number, Real Number != 1.0"), true);
         if (state[1] == 1.0)
            MessageInterface::PopupMessage
               (Gmat::WARNING_, wxT("GMAT does not support parabolic orbits"));
         
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }
   
   // check coupling restrictions on SMA and ECC for circular and elliptical orbits
   if((state[0] < 0.0) && (state[1] <= 1.0))
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_,
          wxT("For circular or elliptical orbit (0.0 <= ECC <= 1.0)\n")
          wxT(" SMA should only be a positive Real Number"));
      retval = false;
      canClose = false;
   }
   
   // for hyperbolic orbit
   else if((state[0] > 0.0) && (state[1] > 1.0))
   {
      MessageInterface::PopupMessage
         (Gmat::ERROR_, wxT("For hyperbolic orbit (ECC > 1) \n")
          wxT("SMA should only be a negative Real Number"));
      retval = false;
      canClose = false;
   }
   
   if (!theScPanel->CheckReal(state[2], mElements[2], wxT("INC"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[3], mElements[3], wxT("RAAN"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[4], mElements[4], wxT("AOP"), wxT("Real Number")))
      retval = false;
   
   // check Anomaly
   if (theScPanel->CheckReal(state[5], mElements[5], mAnomalyType, wxT("Real Number")))
   {
      mAnomaly.Set(state[0], state[1], state[5], mAnomalyType);
   }
   else
   {
      retval = false;
   }
   
   #ifdef DEBUG_ORBIT_PANEL_CHECK_RANGE
   MessageInterface::ShowMessage(wxT("CheckKeplerian() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool CheckModKeplerian(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckModKeplerian(Rvector6 &state)
{
   mAnomalyType = anomalyComboBox->GetValue().c_str();
   bool retval = true;
   
   #ifdef DEBUG_ORBIT_PANEL_CHECKSTATE
      MessageInterface::ShowMessage(
            wxT("OrbitPanel::CheckModKeplerian, state = %12.10f  %12.10f  %12.10f  %12.10f  %12.10f  %12.10f\n"),
            state[0], state[1], state[2], state[3], state[4], state[5]);
      MessageInterface::ShowMessage(wxT("   current anomaly type = %s\n"), mAnomalyType.c_str());
   #endif

   if (theScPanel->CheckReal(state[0], mElements[0], wxT("RadPer"), wxT("Real Number")))
   {
      if (state[0] == 0.0)
      {
         theScPanel->CheckReal(state[0], mElements[0], wxT("RadPer"), wxT("Real Number != 0.0"), true);
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }
   
   if (theScPanel->CheckReal(state[1], mElements[1], wxT("RadApo"), wxT("Real Number")))
   {
      if (state[1] < 0.0)
      {
         theScPanel->CheckReal(state[1], mElements[1], wxT("RadApo"), wxT("Real Number >= 0.0"), true);
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }
   
   if (!theScPanel->CheckReal(state[2], mElements[2], wxT("INC"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[3], mElements[3], wxT("RAAN"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[4], mElements[4], wxT("AOP"), wxT("Real Number")))
      retval = false;
   
   // check Anomaly
   if (theScPanel->CheckReal(state[5], mElements[5], mAnomalyType, wxT("Real Number")))
   {
      #ifdef DEBUG_ORBIT_PANEL_CHECKSTATE
         MessageInterface::ShowMessage(
               wxT("OrbitPanel::CheckModKeplerian, about to set anomaly with %12.10f  %12.10f  %12.10f  %s\n"),
               state[0], state[1], state[5], mAnomalyType.c_str());
      #endif
      mAnomaly.Set(state[0], state[1], state[5], mAnomalyType);
   }
   else
   {
      retval = false;
   }
   
   #ifdef DEBUG_ORBIT_PANEL_CHECK_RANGE
   MessageInterface::ShowMessage(wxT("CheckModKeplerian() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool CheckSpherical(Rvector6 &state, const wxString &stateType)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckSpherical(Rvector6 &state, const wxString &stateType)
{
   bool retval = true;
   
   if (theScPanel->CheckReal(state[0], mElements[0], wxT("RMAG"), wxT("Real Number")))
   {
      if (state[0] <= 0.0)
      {
         theScPanel->CheckReal(state[0], mElements[0], wxT("RMAG"), wxT("Real Number > 0.0"), true);
      }
   }
   else
   {
      retval = false;
   }
   
   if (!theScPanel->CheckReal(state[1], mElements[1], wxT("RA"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[2], mElements[2], wxT("DEC"), wxT("Real Number")))
      retval = false;
   
   if (theScPanel->CheckReal(state[3], mElements[3], wxT("VMAG"), wxT("Real Number")))
   {
      if (state[3] < 0.0)
      {
         theScPanel->CheckReal(state[4], mElements[3], wxT("VMAG"), wxT("Real Number >= 0.0"), true);
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }

   wxString label5, label6;
   
   if (stateType ==  wxT("SphericalAZFPA"))
   {
      label5 = wxT("AZI");
      label6 = wxT("FPA");
   }
   else if (stateType ==  wxT("SphericalRADEC"))
   {
      label5 = wxT("RAV");
      label6 = wxT("DECV");
   }
   
   if (!theScPanel->CheckReal(state[4], mElements[4], label5, wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[5], mElements[5], label6, wxT("Real Number")))
      retval = false;
   
   #ifdef DEBUG_ORBIT_PANEL_CHECK_RANGE
   MessageInterface::ShowMessage(wxT("CheckSpherical() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool CheckEquinoctial(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckEquinoctial(Rvector6 &state)
{
   bool retval = true;
   
   if (theScPanel->CheckReal(state[0], mElements[0], wxT("SMA"), wxT("Real Number")))
   {
      if (state[0] == 0.0)
      {
         theScPanel->CheckReal(state[0], mElements[0], wxT("SMA"), wxT("Real Number != 0.0"), true);
         retval = false;
         canClose = false;
      }
   }
   else
   {
      retval = false;
   }
   
   if ((!theScPanel->CheckReal(state[1], mElements[1], wxT("EquinoctialH"), wxT("Real Number"))) ||
       (!theScPanel->CheckRealRange(mElements[1], state[1], wxT("EquinoctialH"), -1.0, 1.0, true, true, true, true)))
         retval =  false;
   
   if ((!theScPanel->CheckReal(state[2], mElements[2], wxT("EquinoctialK"), wxT("Real Number"))) ||
         (!theScPanel->CheckRealRange(mElements[2], state[2], wxT("EquinoctialK"), -1.0, 1.0, true, true, true, true)))
      retval = false;
   
   if (!theScPanel->CheckReal(state[3], mElements[3], wxT("EquinoctialP"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[4], mElements[4], wxT("EquinoctialQ"), wxT("Real Number")))
      retval = false;
   
   if (!theScPanel->CheckReal(state[5], mElements[5], wxT("Mean Longitude"), wxT("Real Number")))
      retval = false;
   
   #ifdef DEBUG_ORBIT_PANEL_CHECK_RANGE
   MessageInterface::ShowMessage(wxT("CheckEquinoctial() returning %d\n"), retval);
   #endif
   
   return retval;
}


//------------------------------------------------------------------------------
// bool CheckAnomaly(Rvector6 &state)
//------------------------------------------------------------------------------
bool OrbitPanel::CheckAnomaly(Rvector6 &state)
{
   bool validSma = true;
   bool validEcc = true;
   bool validVal = true;
   Real sma, ecc, value;
   wxString str;
   
   sma = mOutState[0];
   if (mIsStateModified[0])
   {
      str = textCtrl[0]->GetValue().c_str();
      validSma = theScPanel->CheckReal(sma, str, wxT("SMA"), wxT("Real Number"));
   }
   
   ecc = mOutState[1];
   if (mIsStateModified[1])
   {
      str = textCtrl[1]->GetValue().c_str();
      validEcc = theScPanel->CheckReal(ecc, str, wxT("ECC"), wxT("Real Number"));
   }
   
   value = mOutState[5];
   if (mIsStateModified[5])
   {
      str = textCtrl[5]->GetValue().c_str();
      validVal = theScPanel->CheckReal(value, str, mFromAnomalyTypeStr,
                                       wxT("Real Number"));
   }
   
   return validSma && validEcc && validVal;
}


//------------------------------------------------------------------------------
// bool ComputeTrueAnomaly(const wxString &stateTypeStr)
//------------------------------------------------------------------------------
/*
 * Computes true anomaly (mTrueAnomaly) from the current anomaly (mAnomaly).
 */
//------------------------------------------------------------------------------
bool OrbitPanel::ComputeTrueAnomaly(Rvector6 &state, const wxString &stateTypeStr)
{
   #ifdef DEBUG_ORBIT_PANEL_CONVERT
   MessageInterface::ShowMessage
      (wxT("   OrbitPanel::ComputeTrueAnomaly() stateTypeStr=%s\n"), stateTypeStr.c_str());
   #endif

   if (mAnomalyType == mAnomalyTypeNames[Anomaly::TA])
      return true;

   Rvector6 midState, outState;
   
   if (stateTypeStr == mStateTypeNames[StateConverter::CARTESIAN])
   {
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage(wxT("before Coordinate Conversion in ComputeTrueAnomaly, mEpoch = %12.10f\n"), mEpoch);
      #endif
      // Transform to the desired coordinate system
      mCoordConverter.Convert(A1Mjd(mEpoch), mCartState, mInternalCoord, midState, 
                              mOutCoord);
      
      // and convert to the desired representation
      stateConverter.SetMu(mOutCoord);
      outState = stateConverter.FromCartesian(midState, stateTypeStr, wxT("True Anomaly"));
   }
   else if ((stateTypeStr == mStateTypeNames[StateConverter::KEPLERIAN]) || 
            (stateTypeStr == mStateTypeNames[StateConverter::MOD_KEPLERIAN]))
   {
      Real sma = mOutState[0];
      Real ecc = mOutState[1];
      Real anomaly = mOutState[5];
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage
         (wxT("   before computing TA, SMA=%f, ECC=%f, %s:%f\n"), sma, ecc,
          mAnomalyType.c_str(), anomaly);
      #endif
      
      // if state type is MOD_KEPLERIAN, compute sma and ecc
      if (stateTypeStr == mStateTypeNames[StateConverter::MOD_KEPLERIAN])
      {
         Rvector kepl = stateConverter.FromCartesian
            (state, mStateTypeNames[StateConverter::KEPLERIAN], mAnomalyType);
         sma = kepl[0];
         ecc = kepl[1];
         anomaly = kepl[5];         
         
         #ifdef DEBUG_ORBIT_PANEL_CONVERT
         MessageInterface::ShowMessage
            (wxT("   after convert to Keplerian, SMA=%f, ECC=%f, %s:%f\n"), sma, ecc,
             mAnomalyType.c_str(), anomaly);
         #endif
      }
      
      mAnomaly.Set(sma, ecc, anomaly, mAnomalyType);
      mTrueAnomaly = mAnomaly.ConvertToAnomaly(Anomaly::TA);
      
      #ifdef DEBUG_ORBIT_PANEL_CONVERT
      MessageInterface::ShowMessage
         (wxT("   mAnomaly     = [%s]\n   mTrueAnomaly = [%s]\n"),
          mAnomaly.ToString(16).c_str(), mTrueAnomaly.ToString(16).c_str());
      #endif
   }
   
   return true;
}


//------------------------------------------------------------------------------
// void OnButton(wxCommandEvent& event)
//------------------------------------------------------------------------------
void OrbitPanel::OnButton(wxCommandEvent& event)
{    

   OrbitDesignerDialog orbitDlg(this, theSpacecraft);
   orbitDlg.ShowModal();

   if (!orbitDlg.updateOrbit)
	   return;
   stateTypeComboBox->SetValue(wxT("Keplerian"));
   wxString stateTypeStr = stateTypeComboBox->GetValue().c_str();


   Rvector6 state;
   mIsStateTypeChanged = true;
   bool retval = false;

   retval = CheckState(state);

   CoordinateSystem *prevCoord = mOutCoord;

   if (retval)
   {
      BuildValidCoordinateSystemList(stateTypeStr);
      
      mOutCoord = (CoordinateSystem*)theGuiInterpreter->
      GetConfiguredObject(mCoordSysComboBox->GetValue().c_str());
   }

   BuildValidStateTypes();

   try
   {
      DisplayState();
         
      if (event.GetEventObject() == mCoordSysComboBox)
         mFromCoordSysStr = mCoordSysComboBox->GetValue().c_str();
         
      if (event.GetEventObject() == stateTypeComboBox)
         mFromStateTypeStr = stateTypeComboBox->GetValue().c_str();
         
      mFromCoord = mOutCoord;
      theSpacecraft->SetRefObjectName(Gmat::COORDINATE_SYSTEM, mOutCoord->GetName());
      theSpacecraft->SetRefObject(mOutCoord, Gmat::COORDINATE_SYSTEM);
   }
   catch (BaseException &)
   {
      mCoordSysComboBox->SetValue(mFromCoordSysStr.c_str());
      stateTypeComboBox->SetValue(mFromStateTypeStr.c_str());
      mOutCoord = prevCoord;
      BuildValidStateTypes();
      return;
   }

   wxArrayString keplerianElements;
   keplerianElements = orbitDlg.GetElementsString();

   for(int i=0; i<6; i++)
      textCtrl[i]->SetValue(keplerianElements[i]);

   Rvector6 kepState = orbitDlg.GetElementsDouble();
   mCartState = stateConverter.FromKeplerian(kepState, wxT("Cartesian"), wxT("TA"));
   mOutState = mCartState;

   dataChanged = true;
   theScPanel->EnableUpdate(true);

   if (orbitDlg.isEpochChanged)
   {
      mIsEpochChanged = true;
	  mIsEpochFormatChanged = true;
	  
      wxString toEpochFormat = orbitDlg.GetEpochFormat();   
	  epochFormatComboBox->SetValue(toEpochFormat.c_str());
      
      try
      {
         Real fromMjd = -999.999;
         Real outMjd;
         wxString outStr;
         
         // if modified by user, check if epoch is valid first
         if (mIsEpochModified)
         {
			 mEpochStr = orbitDlg.GetEpoch();
            
            // Save to TAIModJulian string to avoid keep reading the field
            // and convert to proper format when ComboBox is changed.
            if (mFromEpochFormat == wxT("TAIModJulian"))
            {
               mTaiMjdStr = mEpochStr;
            }
            else
            {
               TimeConverterUtil::Convert(mFromEpochFormat, fromMjd, mEpochStr,
                                          wxT("TAIModJulian"), outMjd, outStr);
               mTaiMjdStr = outStr;
            }
            
            // Convert to desired format with new date
            TimeConverterUtil::Convert(mFromEpochFormat, fromMjd, mEpochStr,
                                       toEpochFormat, outMjd, outStr);
            
            epochValue->SetValue(outStr.c_str());
            mIsEpochModified = false;
            mFromEpochFormat = toEpochFormat;
         }
         else
         {            
            // Convert to desired format using TAIModJulian date
            TimeConverterUtil::Convert(wxT("TAIModJulian"), fromMjd, mTaiMjdStr,
                                       toEpochFormat, outMjd, outStr);
            
            epochValue->SetValue(outStr.c_str());
            mFromEpochFormat = toEpochFormat;
         }
      }
      catch (BaseException &e)
      {
         epochFormatComboBox->SetValue(mFromEpochFormat.c_str());
         theSpacecraft->SetDateFormat(mFromEpochFormat);
         MessageInterface::PopupMessage
            (Gmat::ERROR_, e.GetFullMessage() +
             wxT("\nPlease enter valid Epoch before changing the Epoch Format\n"));
      }

	  epochValue->SetValue(orbitDlg.GetEpoch().c_str());
   }

}
