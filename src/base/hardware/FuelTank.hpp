//$Id: FuelTank.hpp 9513 2011-04-30 21:23:06Z djcinsb $
//------------------------------------------------------------------------------
//                               FuelTank
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Author: Darrel J. Conway
// Created: 2004/11/08
// Modified: 
//    2010.03.18 Thomas Grubb 
//      - Changed visiblity of PARAMETER_TEXT, PARAMETER_TYPE, and enum from
//        protected to public
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under MOMS Task
// Order 124.
//
/**
 * Class definition for the Fuel Tanks.
 */
//------------------------------------------------------------------------------


#ifndef FUELTANK_HPP
#define FUELTANK_HPP

#include "Hardware.hpp"


/**
 * Basic fuel tank model used in finite maneuvers.
 */ 
class GMAT_API FuelTank : public Hardware
{
public:

   FuelTank(wxString nomme);
   virtual ~FuelTank();
   FuelTank(const FuelTank& ft);
   FuelTank&            operator=(const FuelTank& ft);
   
   // Parameter access methods - overridden from GmatBase
   virtual wxString  GetParameterText(const Integer id) const;
   virtual wxString  GetParameterUnit(const Integer id) const;
   virtual Integer      GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
                        GetParameterType(const Integer id) const;
   virtual wxString  GetParameterTypeString(const Integer id) const;
   
   virtual bool         IsParameterReadOnly(const Integer id) const;
   
   virtual Real         GetRealParameter(const Integer id) const;
   virtual Real         SetRealParameter(const Integer id,
                                         const Real value);
   virtual Real         GetRealParameter(const wxString &label) const;
   virtual Real         SetRealParameter(const wxString &label,
                                         const Real value);
   virtual bool         GetBooleanParameter(const Integer id) const;
   virtual bool         SetBooleanParameter(const Integer id,
                                            const bool value);
   virtual wxString  GetStringParameter(const Integer id) const;
   virtual bool         SetStringParameter(const Integer id,
                                           const wxString &value);
   virtual wxString  GetStringParameter(const wxString &label) const;
   virtual bool         SetStringParameter(const wxString &label,
                                           const wxString &value);
   
   // for enumerated strings
   virtual const StringArray&
                        GetPropertyEnumStrings(const Integer id) const;
   virtual const StringArray&
                        GetPropertyEnumStrings(const wxString &label) const;
   
   // required method for all subclasses
   virtual GmatBase*    Clone() const;
   virtual void         Copy(const GmatBase* orig);
   
   virtual bool         Initialize();
   virtual bool         Validate();
   
protected:
   /// Mass of the fuel in the tank
   Real                 fuelMass;
   /// Tank pressure
   Real                 pressure;
   /// Fuel temperature
   Real                 temperature;
   /// Reference temperature
   Real                 refTemperature;
   /// Tank volume
   Real                 volume;
   /// Fuel density
   Real                 density;
   /// Flag indicating if the tank is pressure regulated or blow-down
   bool                 pressureRegulated; // deprecated
   /// Flag indicating if negative fuel mass is allowed
   bool                 allowNegativeFuelMass;
   /// Pressure model used
   Integer              pressureModel;
   
   // Parameters used for internal calculations
   
   /// Pressurant volume, a calculated parameter
   Real                 gasVolume;
   /// Baseline product of the pressure and temperature
   Real                 pvBase;
   /// Flag used to force an update to the pressure and temperature calculations
   bool                 initialized;
   
   virtual void         UpdateTank();
   virtual void         DepleteFuel(Real dm);
   
   /// Pressure model list
   enum
   {
      TPM_PRESSURE_REGULATED,
      TPM_BLOW_DOWN
   };
   
   /// Availabel pressure model list
   static StringArray   pressureModelList;
   //static const wxString pressureModelList[2];
public:
   /// Published parameters for generic fuel tanks
   enum
   {
      ALLOW_NEGATIVE_FUEL_MASS = HardwareParamCount,
      FUEL_MASS,
      PRESSURE, 
      TEMPERATURE,
      REFERENCE_TEMPERATURE,
      VOLUME,
      FUEL_DENSITY,
      PRESSURE_MODEL,
      PRESSURE_REGULATED,  // deprecated
      FuelTankParamCount
   };
   
   /// Parameter labels
   static const wxString 
      PARAMETER_TEXT[FuelTankParamCount - HardwareParamCount];
   /// Parameter types
   static const Gmat::ParameterType 
      PARAMETER_TYPE[FuelTankParamCount - HardwareParamCount];
};

#endif // FUELTANK_HPP
