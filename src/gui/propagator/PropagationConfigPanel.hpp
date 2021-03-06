//$Id: PropagationConfigPanel.hpp 9514 2011-04-30 21:44:00Z djcinsb $
//------------------------------------------------------------------------------
//                           PropagationConfigPanel
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Waka Waktola
// Created: 2003/08/29
// Copyright: (c) 2003 NASA/GSFC. All rights reserved.
//
/**
 * This class contains the Propagation configuration window.
 */
//------------------------------------------------------------------------------
#ifndef PropagationConfigPanel_hpp
#define PropagationConfigPanel_hpp

// gui includes
#include "GmatStaticBoxSizer.hpp"

// base includes
#include "gmatdefs.hpp"
#include "Propagator.hpp"
#include "PropSetup.hpp"
#include "ODEModel.hpp"
#include "DragForce.hpp"
#include "GravityField.hpp"
#include "HarmonicField.hpp"
#include "PointMassForce.hpp"
#include "SolarRadiationPressure.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"
#include "AtmosphereModel.hpp"
#include "MessageInterface.hpp"
#include "GmatPanel.hpp"
#include "gmatwxdefs.hpp"

class PropagationConfigPanel : public GmatPanel
{
public:

   PropagationConfigPanel(wxWindow *parent, const wxString &propName);
   ~PropagationConfigPanel();  

private:
   
   // Integrator types -- No longer hard coded
//   enum IntegratorType
//   {
//      RKV89 = 0,
//      RKN68,
//      RKF56,
//      PD45,
//      PD78,
//      BS,
//      ABM,
////      CW,
//      IntegratorCount,
//   };
   Integer IntegratorCount;

   // Earth gravity field model
   enum EarthGravModelType
   {
      E_NONE_GM = 0,
      JGM2,
      JGM3,
      EGM96,
      E_OTHER,
      EarthGravModelCount,
   };   
   
   // Luna gravity field model
   enum LunaGravModelType
   {
      L_NONE_GM = 0,
      LP165,
      L_OTHER,
      LunaGravModelCount,
   };   
   
   // Venus gravity field model
   enum VenusGravModelType
   {
      V_NONE_GM = 0,
      MGNP180U,
      V_OTHER,
      VenusGravModelCount,
   };   
   
   // Mars gravity field model
   enum MarsGravModelType
   {
      M_NONE_GM = 0,
      MARS50C,
      M_OTHER,
      MarsGravModelCount,
   };   
   
   // Other gravity field model
   enum OthersGravModelType
   {
      O_NONE_GM = 0,
      O_OTHER,
      OthersGravModelCount,
   };
   
   // Earth drag model
   enum EarthDragModelType
   {
      NONE_DM = 0,
//      EXPONENTIAL,
      MSISE90,
      JR,
      EarthDragModelCount,
   };
   
   // Magnetic force drag model
   enum MagfModelType
   {
      NONE_MM = 0,
      MagfModelCount,
   };

   // Error Control
   enum ErrorControlType
   {
      NONE_EC = 0,
      RSSSTEP,
      RSSSTATE,
      LARGESTSTEP,
      LARGESTSTATE,
      ErrorControlCount,
   };
   
   struct ForceType
   {
      wxString bodyName;
      wxString gravType;
      wxString dragType;
      wxString magfType;
      wxString gravDegree;
      wxString gravOrder;
      wxString magfDegree;
      wxString magfOrder;
      wxString potFilename;
      PointMassForce *pmf;
      GravityField *gravf;
      DragForce *dragf;
      SolarRadiationPressure *srpf;
      bool useSrp; // for future use (SRP on individual body is future implementation)
      
      ForceType(const wxString &body, const wxString &grav = wxT("None"),
                const wxString &drag = wxT("None"), const wxString &mag = wxT("None"),
                PointMassForce *pf = NULL, GravityField *gf = NULL,
                DragForce *df = NULL)
         {
            bodyName = body; gravType = grav; dragType = drag; magfType = mag;
            gravDegree = wxT("4"); gravOrder = wxT("4"); magfDegree = wxT("0"); 
            magfOrder = wxT("0"); potFilename = wxT(""); pmf = pf; gravf = gf; 
            dragf = df; srpf = NULL; useSrp = false;
         }
      
      ForceType& operator= (const ForceType& right)
         {
            if (this == &right)
               return *this;
            
            bodyName = right.bodyName; gravType = right.gravType;
            dragType = right.dragType; magfType = right.magfType; 
            gravDegree = right.gravDegree; gravOrder = right.gravOrder;
            magfDegree = right.magfDegree; magfOrder = right.magfOrder;
            potFilename = right.potFilename; pmf = right.pmf;
            gravf = right.gravf; dragf = right.dragf; srpf = right.srpf;
            useSrp = right.useSrp;
            return *this;
         }
   };
   
   // SPK parameter buffers
   wxString spkStep;
   wxString spkBody;
   wxString spkEpFormat;
   wxString spkEpoch;

   bool isSpkStepChanged;
   bool isSpkBodyChanged;
   bool isSpkEpFormatChanged;
   bool isSpkEpochChanged;

   wxFlexGridSizer *intFlexGridSizer;
   GmatStaticBoxSizer *intStaticSizer;
   GmatStaticBoxSizer *fmStaticSizer;
   GmatStaticBoxSizer *magfStaticSizer;      // So it can be hidden

   wxStaticText *minIntErrorStaticText;
   wxStaticText *nomIntErrorStaticText;
   wxStaticText *potFileStaticText;
   
   wxStaticText *initialStepSizeStaticText;
   wxStaticText *unitsInitStepSizeStaticText;
   wxStaticText *accuracyStaticText;
   wxStaticText *minStepStaticText;
   wxStaticText *unitsMinStepStaticText;
   wxStaticText *maxStepStaticText;
   wxStaticText *unitsMaxStepStaticText;
   wxStaticText *maxStepAttemptStaticText;


   wxTextCtrl *initialStepSizeTextCtrl;
   wxTextCtrl *accuracyTextCtrl;
   wxTextCtrl *minStepTextCtrl;
   wxTextCtrl *maxStepTextCtrl;
   wxTextCtrl *maxStepAttemptTextCtrl;
   wxTextCtrl *minIntErrorTextCtrl;
   wxTextCtrl *nomIntErrorTextCtrl;
//   wxTextCtrl *bodyTextCtrl;
   wxTextCtrl *gravityDegreeTextCtrl;
   wxTextCtrl *gravityOrderTextCtrl;
   wxTextCtrl *potFileTextCtrl;
   wxTextCtrl *magneticDegreeTextCtrl;
   wxTextCtrl *magneticOrderTextCtrl;
   wxTextCtrl *pmEditTextCtrl;
   
   wxComboBox *theIntegratorComboBox;
   wxComboBox *theOriginComboBox;
   wxComboBox *thePrimaryBodyComboBox;
   wxComboBox *theGravModelComboBox;
   wxComboBox *theAtmosModelComboBox;
   wxComboBox *theMagfModelComboBox;
   wxComboBox *theErrorComboBox;
   
   wxBitmapButton *theGravModelSearchButton;
   wxButton *theDragSetupButton;
   wxButton *theMagModelSearchButton;
   
   wxCheckBox *theSrpCheckBox;
   wxCheckBox *theStopCheckBox;
   
   // Controls used by Propagators that aren't Integrators (SPK for now)
   wxStaticText *propagatorStepSizeStaticText;
   wxTextCtrl *propagatorStepSizeTextCtrl;
   wxStaticText *unitsPropagatorStepSizeStaticText;
   wxStaticText *propCentralBodyStaticText;
   wxComboBox *propCentralBodyComboBox;
   wxStaticText *propagatorEpochFormatStaticText;
   wxComboBox *propagatorEpochFormatComboBox;
   wxStaticText *startEpochStaticText;
   wxTextCtrl *startEpochTextCtrl;

   wxString integratorString;
   wxString primaryBodyString;
   wxString gravityFieldString;
   wxString atmosModelString;
   
   wxBoxSizer *leftBoxSizer;
   
   wxString propSetupName;
   wxString thePropagatorName;
   wxString theForceModelName;
   wxString mFmPrefaceComment;
   
   wxString currentBodyName;
   wxString gravTypeName;
   wxString dragTypeName;
   wxString propOriginName;
   wxString errorControlTypeName;
   
   wxArrayString integratorTypeArray;
   wxArrayString earthGravModelArray;
   wxArrayString lunaGravModelArray;
   wxArrayString venusGravModelArray;
   wxArrayString marsGravModelArray;
   wxArrayString othersGravModelArray;
   wxArrayString dragModelArray;
   wxArrayString magfModelArray;
   wxArrayString errorControlArray;
   
   std::map<wxString, wxString> theFileMap;
   
   // Restrict to one primary body:
   wxString primaryBody;
//   wxArrayString primaryBodiesArray;
   wxArrayString secondaryBodiesArray;
   wxArrayString integratorArray;
   
//   Integer numOfBodies;
   Integer numOfForces;
   Integer currentBodyId;
   
   /// normalized harmonic coefficients
   Real               Cbar[361][361];
   /// normalized harmonic coefficients
   Real               Sbar[361][361];
   /// coefficient drifts per year
   Real               dCbar[17][17];
   /// coefficient drifts per year
   Real               dSbar[17][17];
   
   bool useDragForce;
   bool usePropOriginForSrp;
   bool stopOnAccViolation;
   bool isForceModelChanged;
   bool isAtmosChanged;
   bool isDegOrderChanged;
   bool isPotFileChanged;
   bool isMagfTextChanged;
   bool isIntegratorChanged;
   bool isIntegratorDataChanged;
   bool isOriginChanged;
   bool isErrControlChanged;
   
   Propagator                     *thePropagator;
   PropSetup                      *thePropSetup;
   ODEModel                       *theForceModel;
   PointMassForce                 *thePMF;
   DragForce                      *theDragForce;
   GravityField                   *theGravForce;
   SolarRadiationPressure         *theSRP;
   SolarSystem                    *theSolarSystem;
   CelestialBody                  *theCelestialBody;
   AtmosphereModel                *theAtmosphereModel;
   std::vector<PointMassForce *>  thePMForces;
//   std::vector<ForceType*> primaryBodyList;
   std::vector<ForceType*> pointMassBodyList;
   
   // Restricted to one primary, so using a single ForceType rather than array
   ForceType                      *primaryBodyData;

   // methods inherited from GmatPanel
   virtual void Create();
   virtual void LoadData();
   virtual void SaveData();
   
   // Layout & data handling methods
   Integer FindPrimaryBody(const wxString& bodyName, bool create = true,
                           const wxString& gravType = wxT("None"),
                           const wxString& dragType = wxT("None"),
                           const wxString& magfType = wxT("None"));
   
   Integer FindPointMassBody(const wxString& bodyName);
   
   void Initialize();
   void DisplayIntegratorData(bool integratorChanged);
   void DisplayPrimaryBodyData();
   void DisplayForceData();
   void DisplayGravityFieldData(const wxString& bodyName);
   void DisplayAtmosphereModelData();
   void DisplayPointMassData();
   void DisplayMagneticFieldData();
   void DisplaySRPData();
   void DisplayErrorControlData();
   void EnablePrimaryBodyItems(bool enable = true, bool clear = false);
   void UpdatePrimaryBodyItems();
   void UpdatePrimaryBodyComboBoxList();
   
   // Saving data
   bool SaveIntegratorData();
   bool SavePropagatorData();
   bool SaveDegOrder();
   bool SavePotFile();
   bool SaveAtmosModel();
   
   // Converting Data
   wxString ToString(Real rval);
   
   // Text control event method
   void OnIntegratorTextUpdate(wxCommandEvent &event);
   void OnGravityTextUpdate(wxCommandEvent& event);
   void OnMagneticTextUpdate(wxCommandEvent& event);

   // Checkbox event method
   void OnSRPCheckBoxChange(wxCommandEvent &event);
   void OnStopCheckBoxChange(wxCommandEvent &event);
   
   // Combobox event method
   void OnIntegratorComboBox(wxCommandEvent &event);
   void OnPrimaryBodyComboBox(wxCommandEvent &event);
   void OnOriginComboBox(wxCommandEvent &event);
   void OnGravityModelComboBox(wxCommandEvent &event);
   void OnAtmosphereModelComboBox(wxCommandEvent &event);
   void OnErrorControlComboBox(wxCommandEvent &event);
   void OnPropOriginComboBox(wxCommandEvent &);
   void OnPropEpochComboBox(wxCommandEvent &);

   
   // Button event methods
   void OnAddBodyButton(wxCommandEvent &event);
   void OnGravSearchButton(wxCommandEvent &event);
   void OnSetupButton(wxCommandEvent &event);
   void OnMagSearchButton(wxCommandEvent &event);
   void OnPMEditButton(wxCommandEvent &event);
   void OnSRPEditButton(wxCommandEvent &event);
   
   // for Debug
   void ShowPropData(const wxString &header);
   void ShowForceList(const wxString &header);
   void ShowForceModel(const wxString &header);
   
   // for reading gravity files
   void ParseDATGravityFile(const wxString& fname);
   void ParseGRVGravityFile(const wxString& fname);
   void ParseCOFGravityFile(const wxString& fname);
   void PrepareGravityArrays();
   
   // Strictly for reading gravity files
   static const Integer GRAV_MAX_DRIFT_DEGREE = 2;
   
   void ShowIntegratorLayout(bool isIntegrator = true, bool isEphem = true);

   void PopulateForces();

   // any class wishing to process wxWindows events must use this macro
   DECLARE_EVENT_TABLE();
   
   // IDs for the controls and the menu commands
   enum
   {     
      ID_TEXT = 42000,
      ID_TEXTCTRL,
      ID_TEXTCTRL_PROP,
      ID_TEXTCTRL_GRAV,
      ID_TEXTCTRL_MAGF,
      ID_SRP_CHECKBOX,
      ID_STOP_CHECKBOX,
      ID_CB_INTGR,
      ID_CB_BODY,
      ID_CB_ORIGIN,
      ID_CB_GRAV,
      ID_CB_ATMOS,
      ID_CB_MAG,
      ID_CB_ERROR,
      ID_BUTTON_ADD_BODY,
      ID_BUTTON_GRAV_SEARCH,
      ID_BUTTON_SETUP,
      ID_BUTTON_MAG_SEARCH,
      ID_BUTTON_PM_EDIT,
      ID_BUTTON_SRP_EDIT,
      ID_CB_PROP_ORIGIN,
      ID_CB_PROP_EPOCHFORMAT
   };
};

#endif // PropagationConfigPanel_hpp

