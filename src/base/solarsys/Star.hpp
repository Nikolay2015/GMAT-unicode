//$Id: Star.hpp 9718 2011-07-20 19:03:40Z wendys-dev $
//------------------------------------------------------------------------------
//                                  Star
//------------------------------------------------------------------------------
// GMAT: General Mission Analysis Tool.
//
// Copyright (c) 2002-2011 United States Government as represented by the
// Administrator of The National Aeronautics and Space Administration.
// All Other Rights Reserved.
//
// Developed jointly by NASA/GSFC and Thinking Systems, Inc. under contract
// number S-67573-G
//
// Author: Wendy C. Shoan
// Created: 2004/01/29
//
/**
 * This is the class for stars.
 *
 */
//------------------------------------------------------------------------------


#ifndef Star_hpp
#define Star_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Rmatrix.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"

/**
 * Star class, for all stars in the solar system : ).
 *
 * The Star class will contain all data and methods for any star that exists in
 * the solar system.
 */
class GMAT_API Star : public CelestialBody
{
public:
   // default constructor, with optional name
   Star(wxString name = SolarSystem::SUN_NAME);
   // copy constructor
   Star(const Star &st);
   // operator=
   Star& operator=(const Star &st);
   // destructor
   virtual ~Star();

   Real GetRadiantPower() const;       // W / m^2
   Real GetReferenceDistance() const;  // km

   bool SetRadiantPower(Real radPower, Real refDistance);
   bool SetPhotosphereRadius(Real rad);

   // overridden access methods from CelestialBody
   virtual wxString    GetParameterText(const Integer id) const;
   virtual Integer        GetParameterID(const wxString &str) const;
   virtual Gmat::ParameterType
      GetParameterType(const Integer id) const;
   virtual wxString    GetParameterTypeString(const Integer id) const;

   virtual Real           GetRealParameter(const Integer id) const;
   virtual Real           SetRealParameter(const Integer id,
                                           const Real value);

   
   virtual bool           IsParameterReadOnly(const Integer id) const;
   // add these, if needed later (currently all parameters are read-only anyway
//   virtual bool           IsParameterCloaked(const Integer id) const;
//   virtual bool           IsParameterEqualToDefault(const Integer id) const;
//   virtual bool           SaveAllAsDefault();
//   virtual bool           SaveParameterAsDefault(const Integer id);

   // inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   // required method for all subclasses that can be copied in a script
   virtual void      Copy(const GmatBase* orig);
   virtual bool      NeedsOnlyMainSPK();


protected:

   enum
   {
      RADIANT_POWER = CelestialBodyParamCount,
      REFERENCE_DISTANCE,
      PHOTOSPHERE_RADIUS,
      StarParamCount
   };


   static const wxString PARAMETER_TEXT[
      StarParamCount - CelestialBodyParamCount];

   static const Gmat::ParameterType PARAMETER_TYPE[
      StarParamCount - CelestialBodyParamCount];

      // radiant power and reference distance
   Real      radiantPower;
   Real      referenceDistance;
   Real      photosphereRadius;  // meters

   virtual Rvector6 ComputeTwoBody(const A1Mjd &forTime);

private:

};
#endif // Star_hpp

