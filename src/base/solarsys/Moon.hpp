//$Id: Moon.hpp 9718 2011-07-20 19:03:40Z wendys-dev $
//------------------------------------------------------------------------------
//                                  Moon
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
 * The Moon class will contain all data and methods for any moon that exists in
 * the solar system.
 *
 * @note Currently, only contains default data for Luna, Phobos, and Deimos.
 */
//------------------------------------------------------------------------------


#ifndef Moon_hpp
#define Moon_hpp

#include "gmatdefs.hpp"
#include "GmatBase.hpp"
#include "Rmatrix.hpp"
#include "SolarSystem.hpp"
#include "CelestialBody.hpp"

class GMAT_API Moon : public CelestialBody
{
public:
   // default constructor, with optional name
   Moon(wxString name = SolarSystem::MOON_NAME);
   // additional constructor
   Moon(wxString name, const wxString &cBody);
   // copy constructor
   Moon(const Moon &m);
   // operator=
   Moon& operator=(const Moon &m);
   // destructor
   virtual ~Moon();

   virtual Rvector GetBodyCartographicCoordinates(const A1Mjd &forTime) const;
   
   // inherited from GmatBase
   virtual GmatBase* Clone(void) const;
   // required method for all subclasses that can be copied in a script
   virtual void      Copy(const GmatBase* orig);

   virtual bool      NeedsOnlyMainSPK();

protected:
   enum
   {
      MoonParamCount = CelestialBodyParamCount,
      
   };
   
   //static const wxString PARAMETER_TEXT[MoonParamCount - CelestialBodyParamCount];
   
   //static const Gmat::ParameterType 
   //                         PARAMETER_TYPE[MoonParamCount - CelestialBodyParamCount];
   

private:

};
#endif // Moon_hpp

