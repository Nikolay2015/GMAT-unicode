//$Id: Planet.hpp 9718 2011-07-20 19:03:40Z wendys-dev $
//------------------------------------------------------------------------------
//                                  Planet
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
 * This is the class for planets.
 *
 * @note Currently, this code assumes that it can set the parameter default
 *       values, based on the input name of the planet (e.g., if the planet's
 *       name is Mercury, the default values for Mercury will be used)
 *
 * @note When the Planet is the Earth, the initial epoch and Keplerian elements
 *       used for the Low Fidelity modeling are set, they must be 'forwarded'
 *       on to the Sun.  This is necessary because:
 *       1. the KeplersProbelm code computes the state of a body with respect to
 *          its central body, then returms it with respect to the Earth
 *       2. the Sun has no cental body, so
 *       3. the Sun will have to use the Earth's information to compute and
 *          return its state
 *
 */
//------------------------------------------------------------------------------


#ifndef Planet_hpp
#define Planet_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Rmatrix.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"

/**
 * Planet class, for all planets in the solar system : ).
 *
 * The Planet class will contain all data and methods for any planet that exists 
 * in the solar system.
 */
class GMAT_API Planet : public CelestialBody
{
public:
   // default constructor, with optional name
   Planet(wxString name = SolarSystem::EARTH_NAME);
   // additional constructor
   Planet(wxString name, const wxString &cBody);
   // copy constructor
   Planet(const Planet &pl);
   // operator=
   Planet& operator=(const Planet &pl);
   // destructor
   virtual ~Planet();

   virtual Rvector       GetBodyCartographicCoordinates(
                            const A1Mjd &forTime) const;
   virtual Real          GetHourAngle(A1Mjd atTime); 
   
   // For the Earth, these methods need to update the Sun's information, as the 
   // Sun has no cental body and its initial epoch and keplerian elements need 
   // to be set and stored differently
   virtual bool          SetTwoBodyEpoch(const A1Mjd &toTime);
   virtual bool          SetTwoBodyElements(const Rvector6 &kepl);
   
   virtual Real          GetNutationUpdateInterval() const;
   virtual bool          SetNutationUpdateInterval(Real val);

   // inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   // required method for all subclasses that can be copied in a script
   virtual void      Copy(const GmatBase* orig);
   virtual bool      NeedsOnlyMainSPK();

   // Parameter access methods - overridden from GmatBase - may need these later??
   virtual wxString     GetParameterText(const Integer id) const;     
   virtual Integer         GetParameterID(const wxString &str) const; 
   virtual Gmat::ParameterType
                           GetParameterType(const Integer id) const;
   virtual wxString     GetParameterTypeString(const Integer id) const;
   virtual bool            IsParameterReadOnly(const Integer id) const;
   virtual Real            GetRealParameter(const Integer id) const;
   virtual Real            SetRealParameter(const Integer id,
                                            const Real value);
   virtual Real            GetRealParameter(const wxString &label) const;
   virtual Real            SetRealParameter(const wxString &label,
                                            const Real value);
                                            
   virtual bool         IsParameterCloaked(const Integer id) const;
   virtual bool         IsParameterEqualToDefault(const Integer id) const;
   virtual bool         SaveAllAsDefault();
   virtual bool         SaveParameterAsDefault(const Integer id);

protected:

   enum
   {
      NUTATION_UPDATE_INTERVAL = CelestialBodyParamCount,
      PlanetParamCount
   };
   
   static const wxString PARAMETER_TEXT[PlanetParamCount - CelestialBodyParamCount];
   
   static const Gmat::ParameterType PARAMETER_TYPE[PlanetParamCount - CelestialBodyParamCount];
   
   Real     nutationUpdateInterval;
   /// @todo add other ones as needed
   /// default values for the parameter(s)
   Real     default_nutationUpdateInterval;

                                          

private:

};
#endif // Planet_hpp

